An archetype represents a unique combination of component types. This document explains NCS's archetype-based 
storage model and how it enables efficient queries and component operations.

## What is an Archetype?

In NCS, an archetype is a unique set of component types. For example, `{Position}`, `{Position, Velocity}`, 
and `{Position, Health, Player}` are three different archetypes. 
All entities with the exact same set of components belong to the same archetype, 
sharing the same memory layout and storage structure.

## Archetype Storage

Each archetype stores component data in a structured way:

```cpp
struct Archetype
{
    /* graph structure for fast archetype transitions */
    std::unordered_map<Component, GraphEdge*> add_edge;
    std::unordered_map<Component, GraphEdge*> remove_edge;

    /* storage for entities and their components */
    std::unordered_map<Entity, size_t> entity_rows;
    std::unordered_map<Component, Column> columns;
    std::vector<Component> components;
    std::vector<Entity> entities;
    size_t entity_count = 0;
    uint64_t id = 0;
    DirtyFlags flags = {};
};
```

The archetype organizes data in a columnar structure, where each component type gets its own memory column.
Entities are stored in rows, with an entity's components available at the same row index across all columns.
The `entity_rows` map provides O(1) lookup to find an entity's row within the archetype.
This organization optimizes for cache coherence during system iteration, as components of the same type are stored
contiguously in memory.

## Archetype Graph

NCS maintains a graph of archetypes to efficiently handle component addition and removal:

```cpp
struct GraphEdge
{
    Archetype* from;
    Archetype* to;
    Component id; /* what component causes the transition */
};
```

The archetype graph represents the relationships between archetypes when adding or removing components.
When you modify an entity's components, NCS uses this graph to quickly find the destination archetype without
having to search through all existing archetypes. Each edge represents a single component addition or removal operation.
This graph structure makes archetype transitions much faster, especially in systems with many different component
combinations.

## Archetype Operations

### Adding an Entity

```cpp
size_t Archetype::append(Entity entity)
{
    const size_t row = entity_count++;
    if (row >= entities.size())
    {
        const size_t newsz = entities.empty() ? 16 : entities.size() * 2;
        entities.resize(newsz);
        for (auto &[comp_id, column]: columns)
            column.resize(newsz);
    }

    entities[row] = entity;
    entity_rows[entity] = row;
    flags |= DirtyFlags::ADDED;
    return row;
}
```

When an entity joins an archetype, it gets assigned to the next available row. If the archetype needs more capacity,
all columns resize together to maintain alignment across component storage. The entity ID is recorded and mapped to
its row position, and the archetype is marked as modified with the `ADDED` flag to assist with query optimization.

### Removing an Entity

```cpp
void Archetype::remove(Entity entity)
{
    const auto it = entity_rows.find(entity);
    if (it == entity_rows.end())
        return;

    const size_t row = it->second;
    if (const size_t last_row = entity_count - 1; row != last_row)
    {
        /* Move the last entity to this row for O(1) removal */
        const Entity last_entity = entities[last_row];
        /* ...copy memory from last row to this row... */
        entities[row] = last_entity;
        entity_rows[last_entity] = row;
    }

    entity_count--;
    entity_rows.erase(entity);
    flags |= DirtyFlags::REMOVED;
}
```

Entity removal uses the swap-with-last technique for O(1) complexity. Rather than shifting all entities to 
fill the gap, the last entity in the archetype is moved to the freed position. 
This approach makes data contiguous without expensive memory moves. 
The archetype's `REMOVED` flag is set to indicate that query results needs updating.

### Moving an Entity

When an entity changes archetypes (adding/removing components), its data moves:

```cpp
void Archetype::move(size_t row, Archetype *dest, Entity entity)
{
    const size_t dest_row = dest->append(entity);
    
    /* copy compatible components to the destination archetype */
    for (Component comp_id: components)
    {
        if (dest->has(comp_id))
        {
            /* ...copy memory... */
        }
    }

    remove(entity);
}
```

Moving entities between archetypes involves creating space in the destination archetype, 
copying over all compatible components, and then removing the entity from the source archetype.
Only components that exist in both archetypes are transferred; new components will be initialized separately, 
and removed components are destroyed if necessary.

## Dirty Flags

NCS uses dirty flags to track changes to archetypes:

```cpp
enum class DirtyFlags : uint64_t
{
    NONE = 0x0,
    DIRTY = 0x1,
    ADDED = 0x2,
    REMOVED = 0x4,
    UPDATED = 0x8,
};
```

These flags enable a powerful optimization for queries. Instead of rebuilding query results from scratch each time, 
NCS can intelligently update previous results based on what has changed. When entities are added to or removed 
from an archetype, or when component data is updated, the corresponding flag is set. 
The query system then uses these flags to determine the most efficient way to update its results.

The query system is the heart of NCS's entity processing capabilities. 
It allows you to efficiently find and iterate over entities that have specific combinations of components. 
This document explains how queries work and how to use them effectively.

## Basic Query Usage

Queries allow you to find all entities that have a specific set of components. Here's a simple example:

```cpp
/* find all entities with Position and Velocity */
auto results = world.query<Position, Velocity>();

/* iterate over the results */
for (auto& [entity, position, velocity] : results)
{
    /* update position based on velocity */
    position->x += velocity->x * dt;
    position->y += velocity->y * dt;
    position->z += velocity->z * dt;
}
```

The query method returns a vector of tuples, where each tuple contains the entity ID 
and pointers to its components. This makes it convenient to access and modify component data for all matching entities.

## Query Implementation

Behind the scenes, NCS uses a sophisticated template-based system for queries:

```cpp
template<typename... Components>
std::vector<std::tuple<Entity, Components *...> > World::query()
{
    const std::vector<Component> cids = { get_cid<Components>()... };
    const uint64_t qhash = archash(cids);

    /* check cache first */
    auto cache_it = qcaches.find(qhash);
    QueryCache<Components...> *cache = nullptr;

    /* cache exists and is valid; fast path */
    if (cache_it != qcaches.end())
    {
        cache = static_cast<QueryCache<Components...> *>(cache_it->second.first);
        if (cache->archetype && cache->entity_count == cache->archetype->entity_count &&
            !has_flag(cache->archetype->flags, DirtyFlags::ADDED | DirtyFlags::REMOVED | DirtyFlags::UPDATED)) 
        {
            return cache->result;
        }
        
        /* cache exists but needs updating */
        /* ...incremental update logic... */
    }
    else /* create new cache; slow path */
    {
        cache = new QueryCache<Components...>();
        qcaches[qhash] = {
            cache,
            [](void *ptr) 
            { 
                delete static_cast<QueryCache<Components...> *>(ptr); 
            }
        };
    }
    
    /* populate & match result */
}
```

The query method first converts the template parameter pack into a list of component IDs, 
then uses this to compute a hash that uniquely identifies the query. 
his hash is used to look up and manage cached query results.

## Query Caching

One of NCS's most powerful optimizations is query caching. Rather than scanning all archetypes and entities for 
each query, NCS caches results and updates them incrementally based on what has changed.

When a query is first executed, NCS scans all archetypes to find those that contain all 
the requested component types. It then builds a vector of tuples containing entities and their component pointers.

For subsequent calls to the same query, NCS checks if any relevant archetypes have been 
modified using the dirty flags. If nothing has changed, it returns the cached results immediately.
If entities have been added or removed, it updates the cache incrementally instead of rebuilding it from scratch.

```cpp
/* example of incremental cache update when entities are added */
if (has_flag(cache->archetype->flags, DirtyFlags::ADDED) &&
    !has_flag(cache->archetype->flags, DirtyFlags::REMOVED | DirtyFlags::UPDATED)) 
{
    /* append the new entities */
    for (size_t i = cache->entity_count; i < cache->archetype->entity_count; ++i)
    {
        Entity entity_id = cache->archetype->entities[i];
        const auto gen_it = generations.find(entity_id);
        if (gen_it == generations.end())
            continue;

        Entity encoded_entity = encode_entity(entity_id, gen_it->second);
        cache->result.emplace_back(std::make_tuple(
            encoded_entity,
            get_component_ptr<Components>(cache->archetype, i)...
        ));
    }
    cache->entity_count = cache->archetype->entity_count;
    cache->archetype->flags = static_cast<DirtyFlags>(
        static_cast<uint64_t>(cache->archetype->flags) &
        ~static_cast<uint64_t>(DirtyFlags::ADDED) /* reset the flag */
    );
    return cache->result;
}
```

This caching system dramatically improves performance, especially in scenarios where you run 
the same queries repeatedly which is pretty common in polling loop.

## Query Performance

The performance of queries in NCS is determined by several factors:

The archetype-based storage model ensures that component data for entities within the same archetype is stored 
contiguously. This layout provides excellent cache locality when iterating over query results, 
leading to efficient CPU usage.

Query caching reduces the overhead of finding entities with specific component combinations. After the first execution,
subsequent calls to the same query can be much faster, especially if few entities have changed.

Incremental cache updates further optimize performance by only updating the parts of the cache that have changed,
rather than rebuilding the entire result set. This is particularly beneficial in scenarios with stable entity 
compositions.

The query result structure (a vector of tuples) provides direct pointers to component data, allowing for immediate
access without additional lookups or indirection. This direct access pattern is both intuitive
for the programmer and efficient for the CPU.

## Advanced Query Techniques

### Component Order in Queries

The order of component types in a query doesn't affect which entities are returned, but it does affect the order of component pointers in the result tuples:

```cpp
auto q1 = world.query<Position, Velocity>();      /* tuple: (Entity, Position*, Velocity*) */
auto q2 = world.query<Velocity, Position>();      /* tuple: (Entity, Velocity*, Position*) */
```

### Modifying Components During Iteration

Since queries return pointers to component data, you can directly modify components while iterating:

```cpp
auto results = world.query<Health>();
for (auto& [entity, health] : results)
    health->value = std::min(health->value + 10, health->max);  /* heal entities */

```

### Adding/Removing Components During Iteration

While it's possible to add or remove components during iteration, you should be careful about modifying the entities
that are part of your current query. Changing an entity's archetype can invalidate pointers and indexes. 
A safer approach is to collect entities that need modification first, then process in another pass.

```cpp
std::vector<Entity> entities_to_mod;

auto results = world.query<Health>();
for (auto& [e, hp] : results) 
{
    if (hp->value <= 0)
        entities_to_mod.emplace_back(e);
}

/* second pass: modify entities */
for (auto entity : entities_to_mod)
    world.set<Dead>(entity, {});  /* add dead component */
```

## NOTE

To get the most out of NCS's query system, structure your components around how you'll query them. 
Related data that's always processed together should generally be in the same component, 
while data that's often processed separately should be in different components.

Reuse query patterns in your systems. The caching system works best when the same query is executed repeatedly, 
so design your systems around consistent component combinations.

Be mindful of the cost of archetype transitions. Adding or removing components causes entities to 
move between archetypes, which can be expensive. If you need to frequently toggle a behavior, consider using a 
flag within a component rather than adding/removing a component.

For performance-critical systems, you might want to cache query results between frames if you know they won't change, 
rather than calling query() every frame. However, be careful to invalidate your cache when relevant 
entities are modified.

Components are pure data classes that store the properties of entities. 
This document describes how NCS handles component storage and type management.

## Component IDs

NCS uses a simple type ID system to identify component types at runtime.

```cpp
using Component = uint16_t;
```

Each unique component type gets assigned a unique ID the first time it's used. 
This mapping happens automatically when you call methods like `set<T>`, `get<T>`, etc.

## Type Registration

When a component type is first encountered, NCS automatically registers information about it:

```cpp
template<typename T>
Component get_cid()
{
    if (const auto it = component_types.find(typeid(T)); it != component_types.end())
    {
        return it->second;
    }

    const Component id = next_cid++;
    component_types[typeid(T)] = id;
    component_sizes[id] = sizeof(T);
    
    /* special case for non-trivial types that need destructors */
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        cdtors[id] = [](void *ptr)
        {
            static_cast<T *>(ptr)->~T();
        };
    }
    
    return id;
}
```

This registration system maintains the mapping between C++ runtime types and component IDs while tracking the size 
of each component type. For components with non-trivial destructors, it also stores function pointers to properly 
clean up memory when components are removed or entities are destroyed. This automatic type handling means you 
don't need manual registration steps before using your custom component types.

## Memory Management

NCS stores component data in contiguous memory blocks organized by archetype. Each component type within an 
archetype gets its own Column object:

```cpp
struct Column
{
    void *data = nullptr;
    size_t size = 0;      /* size of each component */
    size_t capacity = 0;  /* total capacity in elements */
};
```

The Column structure manages a contiguous block of memory where component instances are stored. When you add a 
component to an entity, NCS determines the correct archetype for the entity, allocates memory for the component 
if needed, and then copies the component data into the appropriate memory location. This is to make memory access 
pattern uniform and efficient during iteration.

## Component Operations

### Adding/Updating Components

```cpp
template<typename T>
World *set(Entity entity, const T &data);
```

The set method handles both adding new components and updating existing ones. It first finds the entity's current 
archetype and determines if adding this component requires an archetype change. If necessary, 
it moves the entity to the new archetype before copying the component data into storage. 
For existing components, it simply updates the data in place. The implementation handles both 
trivially copyable types and complex objects with custom copy semantics.

### Accessing Components

```cpp
template<typename T>
T *get(Entity entity);
```

The get method provides direct access to component data, returning a pointer that can both read and modify 
the component. This avoids unnecessary copies and updates efficiently in-place. 
The pointer remains valid until the entity changes archetypes or is destroyed.

### Removing Components

```cpp
template<typename T>
World *remove(Entity entity);
```

When removing a component, the entity moves to a different archetype that doesn't include the removed component type.
If the component has a non-trivial destructor, it gets called during this process. The memory remains part of 
the original archetype, but it's no longer associated with this entity. This approach maintains the contiguous memory 
layout within each archetype.

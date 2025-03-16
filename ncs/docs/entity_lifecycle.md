The first entry explained what an ECS is and NCS design. Now that we understand what an ECS is, let's get into the internal on how NCS handles entity management. Entities are jus IDs, sure, but there's actually some clever stuff behind the scene to make them work well.

## Entity IDs and Generations

In NCS, an entity is a 64-bit value that bitwise-packs two pieces of information which is the actual entity identifier and a generation counter to track recycling. The identifier takes the upper 48-bit and 16-bit for the generation counter.

```cpp
using Entity = uint64_t;
using Generation = uint16_t;
```

Why do we need a generation counter? Well, it solves the ABA problems which stem from dangling reference. When an entity gets deleted and its ID recycled, any old references to that entity should become invalid. The generation counter lets us detect this situation.

## World

The World class is the manager for all entities. It keeps track of which entities exist, which ones have been deleted, and handles recycling entity IDs efficiently.

```cpp

class World 
{
private:
    std::unordered_map<Entity, Generation> generations; /* tracks the current generation for each entity ID */
    std::unordered_map<uint64_t, size_t> entity_indices; /* maps entity IDs to their positions in the pool */
    std::vector<Entity> entity_pool; /* stores all entity IDs (both active and recyclable) */
    uint64_t alive_count; /* divides the pool into active and inactive sections */
    uint64_t next_id; /* the next fresh ID to assign when we can't recycle */
    
public:
    /* entity management methods */
    
};

```

## Creating Entities

When we create a new entity, we have two paths; either recycling an ID from a previously deleted entity or Generating a brand new ID when there's nothing to recycle. alive_count handles this by making entities at indices 0 to alive_count - 1 are active, and entities at indices alive_count and beyond are available for recycling.

## Destroying Entities

When an entity is despawned, we mark it as inactive, move it to the recyclable section of the pool, then increment its generation counter for safety.

```cpp
/* find the entity in the pool */

size_t index = 0;
for (size_t i = 0; i < alive_count; ++i)
{
    if (entity_pool[i] == entity_id)
    {
        index = i;
        break;
    }
}

```

Now, while this looks pretty simple, there is a problem with it. The fact that we need an O(n) search to see if an entity has a component. This is a very common operation in an ECS, so an O(n) operation is going to be slow especially since we would also have to do this for almost any ECS operation i.e. add, get, remove, target and so on. So instead of iterating, we use an unordered_map to store an entity's position in the pool so that we can lookup in O(1).

## What now?

The entity management system is now both fast and memory-efficient! Both creation and destruction are constant time, reusing entity IDs keeps memory usage predictable, the generation counter prevents dangling reference bugs and the pool remains contiguous with no gaps (see implementation details).

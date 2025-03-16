This is the first page of the NCS wiki. I will go through the design NCS uses since they are foundations. But first, let's talk about what an ECS actually is and its benefits.

## What's an ECS anyway?

An ECS or Entity Component System separates identity (entities) from data (components) and behavior (systems). This architecture creates better data organization, more efficient memory access patterns and greater code reusability—write once for all.

Entities are unique "things" in our game which are often represented as a unique idenfier. They can have components which can be represented as a stride or a vector of values. A combination of components is called an "archetype."

For instance, a gacha game could have a "Player" entity that has components `Currency` and `Inventory`. The game could also have a "Monster" entity with components `Health`, `Attack`, and `Position`.

Systems then operate on entities that have specific components. For example, a "MovementSystem" would process all entities that have both `Position` and `Velocity` components, regardless of what kind of entity they are (player, monster, projectile, etc).

## Design & Implementation

NCS implements an archetypal storage rather than alternatives like sparse sets.

Sparse sets work great when you are iterating over just one or two component types which is common is many games. They also shine when you need to frequently add or remove components. However, they struggle a lot when you need to iterate over many component types at once.

Is that really true? Not entirely. Sparse sets can actually work well with a particular grouping functionality that efficiently organizes entities. This approach can be more efficient than archetypes in some ways, since archetype-based models require jumps across different archetypes while groups don't have this issue.  There are some limitations (you can't easily apply it to all possible combinations), though you can mitigate this with nested groups.

Here, NCS uses the archetype approach because it offers better predictability and performance for the most common ECS patterns in game development. When entities with the same components are grouped together in memory, you get excellent cache coherence during system updates.

The archetype-based model also makes much more sense to reason about data organization and provides a more intuitive mental model (try to imagine a graph in your head!) when working with entities and components. While there are tradeoffs with any ECS implementation strategy, NCS prioritizes the data organization and performance characteristics that typically benefits most scenarios.

As for design decisions, NCS uses 48 upper bits for the actual entity ID and 16 lower bits for a generation counter. A world can theoretically contains 281,474,976,710,655 unique entities and an entity can be used 65,536 times before expiration.

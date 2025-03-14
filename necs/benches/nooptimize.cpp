/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <random>
#include <string>
#include <benchmark/benchmark.h>
#include <necs/world.hpp>

struct Position
{
	float x, y, z;
};

struct Velocity
{
	float x, y, z;
};

struct Name
{
	std::string name;

	Name() = default;

	explicit Name(std::string s) : name(std::move(s)) {}
};

struct Health
{
	int current;
	int max;
};

struct AI
{
	int state;
	float target_x, target_y;
};

struct Tag1 {};
struct Tag2 {};
struct Tag3 {};

static void BM_EntityCreation(benchmark::State &state)
{
	for (auto _: state)
	{
		necs::World world;
		for (auto i = 0; i < state.range(0); ++i)
			benchmark::DoNotOptimize(world.entity());
	}
}

BENCHMARK(BM_EntityCreation)->Range(1 << 10, 1 << 18)->Unit(benchmark::kMillisecond);

static void BM_EntityWithComponents(benchmark::State &state)
{
	for (auto _: state)
	{
		necs::World world;
		for (auto i = 0; i < state.range(0); ++i)
		{
			const auto entity = world.entity();
			world.set<Position>(entity, { 1.0f, 2.0f, 3.0f })
					->set<Position>(entity, { 0.1f, 0.2f, 0.3f });
		}
	}
}

BENCHMARK(BM_EntityWithComponents)->Range(1 << 10, 1 << 16)->Unit(benchmark::kMillisecond);

static void BM_ComponentAccess(benchmark::State &state)
{
	necs::World world;
	std::vector<necs::Entity> entities;

	for (int i = 0; i < state.range(0); ++i)
	{
		auto entity = world.entity();
		world.set<Position>(entity, { static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3) })
				->set<Velocity>(entity, { 0.1f, 0.2f, 0.3f });
		entities.push_back(entity);
	}

	/* shuffle to avoid cache effects */
	std::random_device rd;
	std::mt19937 g(rd());
	std::ranges::shuffle(entities, g);
	for (auto _: state)
	{
		auto sum = 0.0f;
		for (const auto entity: entities)
		{
			const auto *pos = world.get<Position>(entity);
			if (const auto *vel = world.get<Velocity>(entity);
				pos && vel)
				sum += pos->x + vel->x;
		}
		benchmark::DoNotOptimize(sum);
	}
}

BENCHMARK(BM_ComponentAccess)->Range(1 << 10, 1 << 16)->Unit(benchmark::kMillisecond);

static void BM_ArchetypeTransitions(benchmark::State &state)
{
	necs::World world;
	std::vector<necs::Entity> entities;

	for (int i = 0; i < state.range(0); ++i)
	{
		auto entity = world.entity();
		world.set<Position>(entity, { static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3) });
		entities.push_back(entity);
	}

	for (auto _: state)
	{
		for (auto i = 0; i < entities.size() / 2; ++i)
			world.set<Velocity>(entities[i], { 0.1f, 0.2f, 0.3f });

		for (auto i = 0; i < entities.size() / 4; ++i)
			world.set(entities[i], Health { 100, 100 });

		for (auto i = 0; i < entities.size() / 8; ++i)
			world.remove<Velocity>(entities[i]);

		for (const auto entity: entities)
			world.despawn(entity);

		entities.clear();
		for (auto i = 0; i < state.range(0); ++i)
		{
			auto entity = world.entity();
			world.set<Position>(entity, { static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3) });
			entities.push_back(entity);
		}
	}
}

BENCHMARK(BM_ArchetypeTransitions)->Range(1 << 8, 1 << 14)->Unit(benchmark::kMillisecond);

static void BM_EntityIteration(benchmark::State &state)
{
	necs::World world;
	std::vector<necs::Entity> entities;

	for (auto i = 0; i < state.range(0); ++i)
	{
		auto entity = world.entity();
		world.set(entity, Position { static_cast<float>(i % 100), static_cast<float>((i + 1) % 100), 0.0f })
				->set<Velocity>(entity, { (i % 10 * 0.1f), (i % 5 * 0.1f), 0.0f });

		if (i % 3 == 0)
			world.set<Health>(entity, Health { 100, 100 });
		if (i % 5 == 0)
			world.set<Name>(entity, Name(std::string("Entity") + std::to_string(i)));
		entities.push_back(entity);
	}

	for (auto _: state)
	{
		for (const auto entity: entities)
		{
			auto *pos = world.get<Position>(entity);
			if (const auto *vel = world.get<Velocity>(entity);
				pos && vel)
			{
				constexpr auto dt = 0.016f;
				pos->x += vel->x * dt;
				pos->y += vel->y * dt;
				pos->z += vel->z * dt;

				if (pos->x > 100.0f)
					pos->x = 0.0f;
				if (pos->y > 100.0f)
					pos->y = 0.0f;
			}
		}
	}
}

BENCHMARK(BM_EntityIteration)->Range(1 << 10, 1 << 16)->Unit(benchmark::kMillisecond);

static void BM_EntityLifecycle(benchmark::State &state)
{
	for (auto _: state)
	{
		necs::World world;
		std::vector<necs::Entity> entities;

		for (auto i = 0; i < state.range(0); ++i)
		{
			auto entity = world.entity();
			world.set<Position>(entity, { static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3) });
			entities.push_back(entity);
		}

		for (auto i = 0; i < entities.size() / 2; ++i)
			world.despawn(entities[i]);

		for (auto i = 0; i < entities.size() / 2; ++i)
		{
			const auto entity = world.entity();
			world.set<Position>(entity, { 0.1f, 0.2f, 0.3f });
		}
	}
}

BENCHMARK(BM_EntityLifecycle)->Range(1 << 10, 1 << 16)->Unit(benchmark::kMillisecond);

static void BM_QueryPerformance(benchmark::State& state)
{
    necs::World world;
    std::vector<necs::Entity> entities;

    /* create a world with different component combinations */
    for (auto i = 0; i < state.range(0); ++i)
    {
        auto entity = world.entity();
        world.set<Position>(entity, { static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3) });

        /* add velocity to 75% of entities */
        if (i % 4 != 0)
            world.set<Velocity>(entity, { 0.1f, 0.2f, 0.3f });

        /* add health to 50% of entities */
        if (i % 2 == 0)
            world.set<Health>(entity, { 100, 100 });

        /* add name to 25% of entities */
        if (i % 4 == 0)
            world.set<Name>(entity, Name(std::string("Entity") + std::to_string(i)));

        entities.push_back(entity);
    }

    for (auto _ : state)
    {
        /* query all different archetype combinations */
        float total_movement = 0.0f;
        int total_health = 0;

        /* position only entities */
        auto q1 = world.query<Position>();
        benchmark::DoNotOptimize(q1);

        /* position+velocity entities (physics objects) */
        auto q2 = world.query<Position, Velocity>();
        for (auto& [entity, pos, vel] : q2)
        {
            total_movement += pos->x + vel->y;
        }

        /* entities with health (game characters) */
        auto q3 = world.query<Position, Health>();
        for (auto& [entity, pos, health] : q3)
        {
            total_health += health->current;
        }

        /* entities with names (important objects) */
        auto q4 = world.query<Position, Name>();
        benchmark::DoNotOptimize(q4);

        /* fully featured entities */
        auto q5 = world.query<Position, Velocity, Health, Name>();
        benchmark::DoNotOptimize(q5);

        benchmark::DoNotOptimize(total_movement);
        benchmark::DoNotOptimize(total_health);
    }
}

BENCHMARK(BM_QueryPerformance)->Range(1 << 10, 1 << 16)->Unit(benchmark::kMillisecond);

static void BM_HeavyQueryWorkload(benchmark::State& state)
{
    /* each benchmark iteration should create a fresh world */
    for (auto _ : state)
    {
        necs::World world;

        /* create a complex distribution of entity archetypes */
        const int entity_count = state.range(0);
        std::vector<necs::Entity> entities(entity_count);

        /* create entities with randomized component combinations */
        std::mt19937 rng(42);
        std::uniform_int_distribution dist(0, 31); /* 2^5 possible component combinations */

        for (int i = 0; i < entity_count; ++i)
        {
            auto entity = world.entity();
            entities[i] = entity;

            int pattern = dist(rng);

            /* always add position */
            world.set<Position>(entity, { static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3) });

            /* add other components based on bit pattern */
            if (pattern & 1)
                world.set<Velocity>(entity, { static_cast<float>(i % 10) * 0.1f, static_cast<float>(i % 5) * 0.2f, 0.3f });

            if (pattern & 2)
                world.set<Health>(entity, { 100 - (i % 50), 100 });

            if (pattern & 4)
            {
                /* Use fixed-size naming to avoid string allocation/reallocation issues */
                char buffer[32];
                std::snprintf(buffer, sizeof(buffer), "Entity%d", i);
                world.set<Name>(entity, Name(buffer));
            }

            if (pattern & 8)
                world.set<AI>(entity, { i % 3, static_cast<float>(i % 100), static_cast<float>(i % 100) });

            /* add custom tag to every 7th entity to further increase archetype count */
            if (i % 7 == 0)
                world.set<Tag1>(entity, {});
            if (i % 11 == 0)
                world.set<Tag2>(entity, {});
            if (i % 13 == 0)
                world.set<Tag3>(entity, {});
        }

        auto physics_sum = 0.0f;
        auto health_sum = 0;
        auto entities_processed = 0;

        /* run multiple queries to simulate different systems */
        for (int i = 0; i < 10; ++i) /* simulate 10 frames */
        {
            /* physics query - process movement */
            auto q1 = world.query<Position, Velocity>();
            for (auto& [entity, pos, vel] : q1)
            {
                pos->x += vel->x;
                pos->y += vel->y;
                pos->z += vel->z;
                physics_sum += pos->x + pos->y + pos->z;
                entities_processed++;
            }

            /* health query - process regeneration */
            auto q2 = world.query<Health>();
            for (auto& [entity, health] : q2)
            {
                if (health->current < health->max)
                    health->current += 1;
                health_sum += health->current;
                entities_processed++;
            }

            /* ai query - update ai state */
            auto q3 = world.query<AI, Position>();
            for (auto& [entity, ai, pos] : q3)
            {
                ai->state = (ai->state + 1) % 3;
                ai->target_x = pos->x + 10.0f;
                ai->target_y = pos->y + 10.0f;
                entities_processed++;
            }

            /* named entities query */
            auto q4 = world.query<Name, Position>();
            for (auto& [entity, name, pos] : q4)
            {
                /* instead of modifying the string, just measure position */
                if (pos->x > 1000.0f)
                    benchmark::DoNotOptimize(pos->x);

                entities_processed++;
            }

            /* complex query - fully featured entities */
            auto q5 = world.query<Position, Velocity, Health, AI>();
            for (auto& [entity, pos, vel, health, ai] : q5)
            {
                /* complex interaction between components */
                if (health->current < health->max / 2)
                {
                    vel->x *= 0.8f; /* slow down when injured */
                    ai->state = 2; /* escape mode */
                }
                else
                {
                    vel->x *= 1.05f; /* speed up when healthy */
                    if (ai->state != 1)
                        ai->state = 0;
                }

                entities_processed++;
            }
        }

        benchmark::DoNotOptimize(physics_sum);
        benchmark::DoNotOptimize(health_sum);
        benchmark::DoNotOptimize(entities_processed);

        for (const auto entity : entities)
            world.despawn(entity);
    }
}

BENCHMARK(BM_HeavyQueryWorkload)->Range(1 << 10, 1 << 15)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

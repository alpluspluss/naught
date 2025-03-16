/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <gtest/gtest.h>
#include <ncs/world/world.hpp>

struct Position
{
	float x, y, z;
	explicit Position(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

	bool operator==(const Position &other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

struct Velocity
{
	float x, y, z;
	explicit Velocity(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

	bool operator==(const Velocity &other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

struct Health
{
	int value;
	explicit Health(int value = 0) : value(value) {}

	bool operator==(const Health &other) const
	{
		return value == other.value;
	}
};

struct Tag1 {};

struct Tag2 {};

struct Tag3 {};

TEST(WorldTest, entity_count_iteration)
{
	ncs::World world;

	for (auto i = 0; i < 5; ++i)
	{
		const auto entity = world.entity();
		world.set<Position>(entity, Position { static_cast<float>(i), 0.0f, 0.0f });
		world.set<Velocity>(entity, Velocity { 0.0f, static_cast<float>(i), 0.0f });
	}

	auto q = world.query<Position, Velocity>();
	EXPECT_EQ(q.size(), 5);
}

TEST(WorldTest, mixed_components)
{
	ncs::World world;

	const auto e1 = world.entity();
	const auto e2 = world.entity();
	const auto e3 = world.entity();
	const auto e4 = world.entity();

	world.set<Position>(e1, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(e1, Velocity { 10.0f, 20.0f, 30.0f });

	world.set<Position>(e2, Position { 4.0f, 5.0f, 6.0f });
	world.set<Health>(e2, Health { 100 });

	world.set<Velocity>(e3, Velocity { 7.0f, 8.0f, 9.0f });
	world.set<Health>(e3, Health { 200 });

	world.set<Position>(e4, Position { 11.0f, 12.0f, 13.0f });
	world.set<Velocity>(e4, Velocity { 14.0f, 15.0f, 16.0f });
	world.set<Health>(e4, Health { 300 });

	const auto q1 = world.query<Position>();
	EXPECT_EQ(q1.size(), 3); /* e1, e2, e4 */

	const auto q2 = world.query<Position, Velocity>();
	EXPECT_EQ(q2.size(), 2); /* e1, e4 */

	const auto q3 = world.query<Position, Health>();
	EXPECT_EQ(q3.size(), 2); /* e2, e4 */

	const auto q4 = world.query<Position, Velocity, Health>();
	EXPECT_EQ(q4.size(), 1); /* only e4 */
}

TEST(WorldTest, component_data_access)
{
	ncs::World world;

	const auto entity = world.entity();
	world.set<Position>(entity, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(entity, Velocity { 10.0f, 20.0f, 30.0f });

	auto q = world.query<Position, Velocity>();
	EXPECT_EQ(q.size(), 1);

	auto& [e, pos, vel] = q[0];
	EXPECT_EQ(pos->x, 1.0f);
	EXPECT_EQ(pos->y, 2.0f);
	EXPECT_EQ(pos->z, 3.0f);
	EXPECT_EQ(vel->x, 10.0f);
	EXPECT_EQ(vel->y, 20.0f);
	EXPECT_EQ(vel->z, 30.0f);
}

TEST(WorldTest, query_after_modifications)
{
	ncs::World world;

	const auto e1 = world.entity();
	const auto e2 = world.entity();

	world.set<Position>(e1, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(e1, Velocity { 10.0f, 20.0f, 30.0f });

	world.set<Position>(e2, Position { 4.0f, 5.0f, 6.0f });

	auto q1 = world.query<Position, Velocity>();
	EXPECT_EQ(q1.size(), 1); /* e1 */

	/* add velocity to e2 */
	world.set<Velocity>(e2, Velocity { 40.0f, 50.0f, 60.0f });

	auto q2 = world.query<Position, Velocity>();
	EXPECT_EQ(q2.size(), 2); /* e1 & e2 */
	world.remove<Velocity>(e1);

	auto q3 = world.query<Position, Velocity>();
	EXPECT_EQ(q3.size(), 1); /* only e2 */
}

TEST(WorldTest, query_after_despawn)
{
	ncs::World world;

	const auto e1 = world.entity();
	const auto e2 = world.entity();

	world.set<Position>(e1, Position { 1.0f, 2.0f, 3.0f });
	world.set<Position>(e2, Position { 4.0f, 5.0f, 6.0f });

	auto q1 = world.query<Position>();
	EXPECT_EQ(q1.size(), 2);

	world.despawn(e1);

	auto q2 = world.query<Position>();
	EXPECT_EQ(q2.size(), 1);

	auto& [e, pos] = q2[0];
	EXPECT_EQ(pos->x, 4.0f);
	EXPECT_EQ(pos->y, 5.0f);
	EXPECT_EQ(pos->z, 6.0f);
}

TEST(WorldTest, query_same_archetype)
{
	ncs::World world;
	std::vector<ncs::Entity> entities;
	for (auto i = 0; i < 10; ++i)
	{
		auto e = world.entity();
		entities.push_back(e);
		world.set<Position>(e, Position { static_cast<float>(i), 0.0f, 0.0f });
		world.set<Velocity>(e, Velocity { 0.0f, static_cast<float>(i), 0.0f });
		world.set<Health>(e, Health { i * 10 });
	}

	const auto q1 = world.query<Position>();
	EXPECT_EQ(q1.size(), 10);

	const auto q2 = world.query<Velocity>();
	EXPECT_EQ(q2.size(), 10);

	const auto q3 = world.query<Health>();
	EXPECT_EQ(q3.size(), 10);

	const auto q4 = world.query<Position, Velocity>();
	EXPECT_EQ(q4.size(), 10);

	const auto q5 = world.query<Position, Health>();
	EXPECT_EQ(q5.size(), 10);

	const auto q6 = world.query<Velocity, Health>();
	EXPECT_EQ(q6.size(), 10);

	const auto q7 = world.query<Position, Velocity, Health>();
	EXPECT_EQ(q7.size(), 10);
}

TEST(WorldTest, query_empty_archetypes)
{
	ncs::World world;

	/* create an entity; then despawn */
	const auto e = world.entity();
	world.set<Position>(e, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(e, Velocity { 10.0f, 20.0f, 30.0f });
	world.despawn(e);

	/* should empty; even if archetype exists (the arch is empty) */
	const auto q = world.query<Position, Velocity>();
	EXPECT_EQ(q.size(), 0);
}

TEST(WorldTest, modify_during_iteration)
{
	ncs::World world;

	/* create entity with pos */
	std::vector<ncs::Entity> entities;
	for (auto i = 0; i < 5; ++i)
	{
		auto e = world.entity();
		entities.push_back(e);
		world.set<Position>(e, Position { static_cast<float>(i), 0.0f, 0.0f });
	}

	/* add vel during iter */
	auto positions = world.query<Position>();
	for (auto &[entity, pos]: positions)
		world.set<Velocity>(entity, Velocity { pos->x, pos->y, pos->z });

	/* should now have 5 entities */
	auto q = world.query<Position, Velocity>();
	EXPECT_EQ(q.size(), 5);
}

TEST(WorldTest, component_order_in_query)
{
	ncs::World world;

	auto e = world.entity();
	world.set<Position>(e, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(e, Velocity { 10.0f, 20.0f, 30.0f });

	/* different order; shouldn't affect anything at all */
	auto q1 = world.query<Position, Velocity>();
	auto q2 = world.query<Velocity, Position>();

	EXPECT_EQ(q1.size(), 1);
	EXPECT_EQ(q2.size(), 1);

	auto [e1, pos1, vel1] = q1[0];
	auto [e2, vel2, pos2] = q2[0];

	EXPECT_EQ(pos1->x, 1.0f);
	EXPECT_EQ(vel1->x, 10.0f);
	EXPECT_EQ(pos2->x, 1.0f);
	EXPECT_EQ(vel2->x, 10.0f);
}

TEST(WorldTest, multiple_archetypes)
{
	ncs::World world;

	/* different archetype with position */
	for (auto i = 0; i < 5; ++i)
	{
		const auto e = world.entity();
		world.set<Position>(e, Position { static_cast<float>(i), 0.0f, 0.0f });
	}

	for (auto i = 0; i < 3; ++i)
	{
		const auto e = world.entity();
		world.set<Position>(e, Position { static_cast<float>(i), 1.0f, 0.0f });
		world.set<Velocity>(e, Velocity { static_cast<float>(i), 0.0f, 0.0f });
	}

	for (auto i = 0; i < 2; ++i)
	{
		const auto e = world.entity();
		world.set<Position>(e, Position { static_cast<float>(i), 2.0f, 0.0f });
		world.set<Health>(e, Health { i * 10 });
	}

	for (auto i = 0; i < 4; ++i)
	{
		const auto e = world.entity();
		world.set<Position>(e, Position { static_cast<float>(i), 3.0f, 0.0f });
		world.set<Velocity>(e, Velocity { static_cast<float>(i), 1.0f, 0.0f });
		world.set<Health>(e, Health { i * 20 });
	}

	/* query pos */
	const auto q1 = world.query<Position>();
	EXPECT_EQ(q1.size(), 5 + 3 + 2 + 4);

	/* q pv */
	const auto q2 = world.query<Position, Velocity>();
	EXPECT_EQ(q2.size(), 3 + 4);

	/* q pos n hp */
	const auto q3 = world.query<Position, Health>();
	EXPECT_EQ(q3.size(), 2 + 4);

	/* q all */
	const auto q4 = world.query<Position, Velocity, Health>();
	EXPECT_EQ(q4.size(), 4);
}

TEST(WorldTest, large_query)
{
	ncs::World world;
	for (auto i = 0; i < 1000; ++i)
	{
		const auto e = world.entity();
		world.set<Position>(e, Position { static_cast<float>(i), 0.0f, 0.0f });

		/* every third entity also gets Velocity */
		if (i % 3 == 0)
			world.set<Velocity>(e, Velocity { 0.0f, static_cast<float>(i), 0.0f });

		/* every fith entity has health component */
		if (i % 5 == 0)
			world.set<Health>(e, Health { i });
	}

	/* query for Position (all entities) */
	const auto q1 = world.query<Position>();
	EXPECT_EQ(q1.size(), 1000);

	/* query position and velocity (every third) */
	const auto q2 = world.query<Position, Velocity>();
	EXPECT_EQ(q2.size(), 1000 / 3 + (1000 % 3 > 0 ? 1 : 0));

	/* query pos and health (every fifth) */
	const auto q3 = world.query<Position, Health>();
	EXPECT_EQ(q3.size(), 1000 / 5 + (1000 % 5 > 0 ? 1 : 0));

	/* query for all three (every fifteenth) */
	const auto q4 = world.query<Position, Velocity, Health>();
	EXPECT_EQ(q4.size(), 1000 / 15 + (1000 % 15 > 0 ? 1 : 0));
}

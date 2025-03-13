#include <gtest/gtest.h>
#include <necs/world.hpp>

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

class CRUDTest : public testing::Test
{
protected:
	necs::World world;
	necs::Entity entity = {};

	void SetUp() override
	{
		entity = world.entity();
	}
};

TEST_F(CRUDTest, ComponentSet)
{
	const Position pos { 1.0f, 2.0f, 3.0f };
	world.set<Position>(entity, pos);

	EXPECT_TRUE(world.has<Position>(entity));

	const Position new_pos { 4.0f, 5.0f, 6.0f };
	world.set<Position>(entity, new_pos);

	auto *pos_ptr = world.get<Position>(entity);
	ASSERT_NE(pos_ptr, nullptr);
	EXPECT_EQ(*pos_ptr, new_pos);

	const Velocity vel { 10.0f, 20.0f, 30.0f };
	world.set<Velocity>(entity, vel);

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_TRUE(world.has<Velocity>(entity));

	auto *vel_ptr = world.get<Velocity>(entity);
	ASSERT_NE(vel_ptr, nullptr);
	EXPECT_EQ(*vel_ptr, vel);
}

TEST_F(CRUDTest, ComponentGet)
{
	const Position pos { 1.0f, 2.0f, 3.0f };
	const Velocity vel { 10.0f, 20.0f, 30.0f };
	const Health health { 100 };

	world.set<Position>(entity, pos);
	world.set<Velocity>(entity, vel);
	world.set<Health>(entity, health);

	auto *pos_ptr = world.get<Position>(entity);
	auto *vel_ptr = world.get<Velocity>(entity);
	auto *health_ptr = world.get<Health>(entity);

	ASSERT_NE(pos_ptr, nullptr);
	ASSERT_NE(vel_ptr, nullptr);
	ASSERT_NE(health_ptr, nullptr);

	EXPECT_EQ(*pos_ptr, pos);
	EXPECT_EQ(*vel_ptr, vel);
	EXPECT_EQ(*health_ptr, health);

	struct NonExistent {};
	auto *none_ptr = world.get<NonExistent>(entity);
	EXPECT_EQ(none_ptr, nullptr);

	pos_ptr->x = 99.0f;

	auto *updated_pos = world.get<Position>(entity);
	ASSERT_NE(updated_pos, nullptr);
	EXPECT_EQ(updated_pos->x, 99.0f);
}

TEST_F(CRUDTest, ComponentHas)
{
	world.set<Position>(entity, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(entity, Velocity { 10.0f, 20.0f, 30.0f });

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_TRUE(world.has<Velocity>(entity));

	EXPECT_FALSE(world.has<Health>(entity));

	world.set<Health>(entity, Health { 100 });
	EXPECT_TRUE(world.has<Health>(entity));

	const necs::Entity invalid_entity = necs::World::encode_entity(999999, 0);
	EXPECT_FALSE(world.has<Position>(invalid_entity));

	const necs::Entity entity2 = world.entity();
	world.set<Health>(entity2, Health { 50 });

	EXPECT_TRUE(world.has<Health>(entity2));
	EXPECT_FALSE(world.has<Position>(entity2));
	EXPECT_FALSE(world.has<Velocity>(entity2));
}

TEST_F(CRUDTest, ComponentRemove)
{
	world.set<Position>(entity, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(entity, Velocity { 10.0f, 20.0f, 30.0f });
	world.set<Health>(entity, Health { 100 });

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_TRUE(world.has<Velocity>(entity));
	EXPECT_TRUE(world.has<Health>(entity));

	world.remove<Velocity>(entity);

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_FALSE(world.has<Velocity>(entity));
	EXPECT_TRUE(world.has<Health>(entity));

	auto *vel_ptr = world.get<Velocity>(entity);
	EXPECT_EQ(vel_ptr, nullptr);

	auto *pos_ptr = world.get<Position>(entity);
	auto *health_ptr = world.get<Health>(entity);
	ASSERT_NE(pos_ptr, nullptr);
	ASSERT_NE(health_ptr, nullptr);
	EXPECT_EQ(pos_ptr->x, 1.0f);
	EXPECT_EQ(health_ptr->value, 100);

	world.remove<Position>(entity);
	world.remove<Health>(entity);

	EXPECT_FALSE(world.has<Position>(entity));
	EXPECT_FALSE(world.has<Velocity>(entity));
	EXPECT_FALSE(world.has<Health>(entity));
}

TEST_F(CRUDTest, EntityLifecycle)
{
	world.set<Position>(entity, Position { 1.0f, 2.0f, 3.0f });
	world.set<Health>(entity, Health { 100 });

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_TRUE(world.has<Health>(entity));

	world.despawn(entity);

	EXPECT_FALSE(world.has<Position>(entity));
	EXPECT_FALSE(world.has<Health>(entity));

	necs::Entity new_entity = world.entity();

	world.set<Position>(new_entity, Position { 4.0f, 5.0f, 6.0f });

	EXPECT_TRUE(world.has<Position>(new_entity));
	EXPECT_FALSE(world.has<Position>(entity));
}

TEST_F(CRUDTest, MultipleEntities)
{
	const necs::Entity entity1 = entity; // Use the one from SetUp
	const necs::Entity entity2 = world.entity();
	const necs::Entity entity3 = world.entity();

	world.set<Position>(entity1, Position { 1.0f, 2.0f, 3.0f });
	world.set<Velocity>(entity1, Velocity { 10.0f, 20.0f, 30.0f });

	world.set<Position>(entity2, Position { 4.0f, 5.0f, 6.0f });
	world.set<Health>(entity2, Health { 200 });

	world.set<Velocity>(entity3, Velocity { 40.0f, 50.0f, 60.0f });
	world.set<Health>(entity3, Health { 300 });

	EXPECT_TRUE(world.has<Position>(entity1));
	EXPECT_TRUE(world.has<Velocity>(entity1));
	EXPECT_FALSE(world.has<Health>(entity1));

	EXPECT_TRUE(world.has<Position>(entity2));
	EXPECT_FALSE(world.has<Velocity>(entity2));
	EXPECT_TRUE(world.has<Health>(entity2));

	EXPECT_FALSE(world.has<Position>(entity3));
	EXPECT_TRUE(world.has<Velocity>(entity3));
	EXPECT_TRUE(world.has<Health>(entity3));

	auto *pos1 = world.get<Position>(entity1);
	ASSERT_NE(pos1, nullptr);
	EXPECT_EQ(pos1->y, 2.0f);

	auto *health2 = world.get<Health>(entity2);
	ASSERT_NE(health2, nullptr);
	EXPECT_EQ(health2->value, 200);

	auto *vel3 = world.get<Velocity>(entity3);
	ASSERT_NE(vel3, nullptr);
	EXPECT_EQ(vel3->z, 60.0f);
}

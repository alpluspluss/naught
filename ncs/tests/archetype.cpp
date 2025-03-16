/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <gtest/gtest.h>
#include <ncs/archetype/archetypes.hpp>
#include <ncs/world/world.hpp>

struct Position
{
	float x, y, z;
};

struct Velocity
{
	float x, y, z;
};

struct Health
{
	int value;
};

struct Tag {};

class ArchetypeTest : public testing::Test
{
protected:
	ncs::World world;

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(ArchetypeTest, ArchetypeCreation)
{
	std::vector<ncs::Component> components = { 1, 2, 3 };

	ncs::Archetype *arch1 = world.create_archetype(components);
	ASSERT_NE(arch1, nullptr);

	EXPECT_TRUE(arch1->has(1));
	EXPECT_TRUE(arch1->has(2));
	EXPECT_TRUE(arch1->has(3));
	EXPECT_FALSE(arch1->has(4));

	ncs::Archetype *arch2 = world.find_archetype(components);
	ASSERT_NE(arch2, nullptr);
	EXPECT_EQ(arch1, arch2);

	std::vector<ncs::Component> components2 = { 3, 1, 2 };
	ncs::Archetype *arch3 = world.find_archetype(components2);
	ASSERT_NE(arch3, nullptr);
	EXPECT_EQ(arch1, arch3);

	std::vector<ncs::Component> components3 = { 1, 2, 4 };
	ncs::Archetype *arch4 = world.find_archetype(components3);
	EXPECT_EQ(arch4, nullptr); // Should not find this archetype yet

	arch4 = world.create_archetype(components3);
	ASSERT_NE(arch4, nullptr);
	EXPECT_NE(arch1, arch4);
}

TEST_F(ArchetypeTest, EntityOperations)
{
	std::vector<ncs::Component> components = { 1, 2 };
	ncs::Archetype *arch = world.create_archetype(components);

	constexpr ncs::Entity entity1 = 1;
	constexpr ncs::Entity entity2 = 2;
	constexpr ncs::Entity entity3 = 3;

	size_t row1 = arch->append(entity1);
	EXPECT_EQ(row1, 0);
	EXPECT_EQ(arch->entity_count, 1);
	EXPECT_EQ(arch->entities[0], entity1);
	EXPECT_EQ(arch->entity_rows[entity1], 0);

	size_t row2 = arch->append(entity2);
	EXPECT_EQ(row2, 1);
	EXPECT_EQ(arch->entity_count, 2);
	EXPECT_EQ(arch->entities[1], entity2);
	EXPECT_EQ(arch->entity_rows[entity2], 1);

	arch->remove(entity1);
	EXPECT_EQ(arch->entity_count, 1);
	EXPECT_FALSE(arch->entity_rows.contains(entity1));
	EXPECT_EQ(arch->entities[0], entity2);
	EXPECT_EQ(arch->entity_rows[entity2], 0);

	size_t row3 = arch->append(entity3);
	EXPECT_EQ(row3, 1);
	EXPECT_EQ(arch->entity_count, 2);
	EXPECT_EQ(arch->entities[1], entity3);

	arch->remove(entity3);
	EXPECT_EQ(arch->entity_count, 1);
	EXPECT_FALSE(arch->entity_rows.contains(entity3));
}

TEST_F(ArchetypeTest, ArchetypeTransitions)
{
	std::vector<ncs::Component> comps1 = { 1 };
	ncs::Archetype *source = world.create_archetype(comps1);

	ncs::Archetype *dest_add = world.find_archetype_with(source, 2);
	ASSERT_NE(dest_add, nullptr);
	EXPECT_TRUE(dest_add->has(1));
	EXPECT_TRUE(dest_add->has(2));

	ncs::Archetype *dest_add2 = world.find_archetype_with(source, 2);
	EXPECT_EQ(dest_add, dest_add2);

	const std::vector<ncs::Component> comps2 = { 1, 2 };
	source = world.create_archetype(comps2);

	ncs::Archetype *dest_rem = world.find_archetype_without(source, 2);
	ASSERT_NE(dest_rem, nullptr);
	EXPECT_TRUE(dest_rem->has(1));
	EXPECT_FALSE(dest_rem->has(2));

	const std::vector<ncs::Component> single_comp = { 1 };
	ncs::Archetype *first_arch = world.find_archetype(single_comp);
	EXPECT_EQ(dest_rem, first_arch);

	ncs::Archetype *dest_rem2 = world.find_archetype_without(source, 2);
	EXPECT_EQ(dest_rem, dest_rem2);
}

TEST_F(ArchetypeTest, ArchetypeMovement)
{
	const std::vector<ncs::Component> comps1 = { 1, 2 };
	std::vector<ncs::Component> comps2 = { 1, 3 };

	ncs::Archetype *source = world.create_archetype(comps1);
	ncs::Archetype *dest = world.create_archetype(comps2);

	constexpr auto size1 = sizeof(int);
	constexpr auto size3 = sizeof(float);

	source->columns[1].size = size1;
	source->columns[1].resize(10);
	source->columns[2].size = size1;
	source->columns[2].resize(10);

	dest->columns[1].size = size1;
	dest->columns[1].resize(10);
	dest->columns[3].size = size3;
	dest->columns[3].resize(10);

	constexpr ncs::Entity entity = 1;
	const size_t row = source->append(entity);

	const auto data1 = static_cast<int *>(source->columns[1].get(row));
	const auto data2 = static_cast<int *>(source->columns[2].get(row));
	*data1 = 42;
	*data2 = 99;

	ncs::Record record;
	record.archetype = source;
	record.row = row;

	world.move_entity(entity, record, dest);

	EXPECT_EQ(source->entity_count, 0);
	EXPECT_EQ(dest->entity_count, 1);
	EXPECT_FALSE(source->entity_rows.contains(entity));
	EXPECT_TRUE(dest->entity_rows.contains(entity));
	EXPECT_EQ(record.archetype, dest);

	const int *dest_data1 = static_cast<int *>(dest->columns[1].get(record.row));
	EXPECT_EQ(*dest_data1, 42);
}


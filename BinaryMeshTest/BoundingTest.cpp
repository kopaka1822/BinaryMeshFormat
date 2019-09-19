#include "pch.h"

#define TestSuite BoundingTest

TEST(TestSuite, SphereInside)
{
	Sphere s = Sphere{ glm::vec3(0.0f), 10.0f };

	ASSERT_TRUE(s.isInside(glm::vec3(0.0f, 0.0f, 0.0f)));
	ASSERT_TRUE(s.isInside(glm::vec3(10.0f, 0.0f, 0.0f)));
	ASSERT_TRUE(s.isInside(glm::vec3(0.0f, 10.0f, 0.0f)));
	ASSERT_TRUE(s.isInside(glm::vec3(0.0f, 0.0f, 10.0f)));
	ASSERT_TRUE(s.isInside(glm::vec3(2.0f, 2.0f, 2.0f)));
	ASSERT_FALSE(s.isInside(glm::vec3(11.0f, 0.0f, 0.0f)));
	ASSERT_FALSE(s.isInside(glm::vec3(10.01f, 0.0f, 0.0f)));
}


TEST(TestSuite, SphereUnion)
{
	Sphere s = Sphere{ glm::vec3(0.0f), 1.0f };

	// point outside
	auto res = s.unionWith(glm::vec3(2.0f, 0.0f, 0.0f));
	ASSERT_NEAR(res.radius, 1.5f, 0.001f); 
	ASSERT_NEAR(res.center.x, 0.5f, 0.001f);

	// point inside
	res = s.unionWith(glm::vec3(0.5f, 0.0f, 0.0f));
	ASSERT_EQ(s, res); // should remain unchanged
}
//
// pch.h
// Header for standard system include files.
//

#pragma once

#include "gtest/gtest.h"

#define BMF_GENERATORS
#include "../include/bmf/BinaryMesh.h"

using namespace bmf;

// returns vector of identity matrices
//inline std::vector<glm::vec3> getIdentityVec(size_t numInstances)
//{
//	return std::vector<glm::vec3>(numInstances, glm::vec3(0.0f));
//}
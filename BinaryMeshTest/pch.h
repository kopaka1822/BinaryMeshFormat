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
inline std::vector<glm::mat4> getIdentityVec(size_t numInstances)
{
	return std::vector<glm::mat4>(numInstances, glm::mat4(1.0f));
}
#pragma once
#include "../libs/glm/glm.hpp"

struct Vertex2
{
	glm::vec3 position;
	glm::vec2 texCoords;
};

struct Vertex3
{
	glm::vec3 position;
	glm::vec3 normals;
	glm::vec2 texCoords;
};
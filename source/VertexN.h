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

struct VertexText
{
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 colour;
};

struct BMuv
{
	glm::vec2 topLeftUV;
	glm::vec2 topRightUV;
	glm::vec2 bottomLeftUV;
	glm::vec2 bottomRightUV;
};
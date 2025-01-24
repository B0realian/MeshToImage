#pragma once
#define GLEW_STATIC
#include "glew.h"
#include "../glm/glm.hpp"
#include <vector>
#include <string>


struct Vertex2
{
	glm::vec3 position;
	glm::vec2 texCoords;
};

class MeshObject
{
public:
	MeshObject();
	~MeshObject();
	bool LoadObj(const std::string filename);
	void DrawElements();
	void DrawTriangles();


private:
	void LoadBuffers();

	bool bLoaded = false;
	std::vector<Vertex2> vertices;
	std::vector<GLuint> indices;
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ibo = 0;
};
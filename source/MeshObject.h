#pragma once
#define GLEW_STATIC
#include "glew.h"
#include "../libs/glm/glm.hpp"
#include <fbxsdk.h>
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
	bool LoadMesh(const std::string filename);
	void DrawElements();
	void DrawTriangles();

	int triangles = 0;

private:
	void LoadBuffers();

	bool bLoaded = false;
	std::vector<Vertex2> vertices;
	std::vector<GLuint> indices;
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ibo = 0;
};
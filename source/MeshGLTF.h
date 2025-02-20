#pragma once
#define GLEW_STATIC
#include "glew.h"
#include "../libs/glm/glm.hpp"
#include "../libs/json/json.hpp"
#include <vector>
#include <string>
#include "VertexN.h"

using JSON = nlohmann::json;

class MeshGLTF
{
public:
	MeshGLTF();
	~MeshGLTF();
	bool LoadMesh(const std::string filename);
	void DrawTriangles();

	unsigned int triangles = 0;

private:
	void LoadBuffers();

	JSON json;

	bool bLoaded = false;
	std::vector<Vertex2> vertices;
	GLuint vao = 0;
	GLuint vbo = 0;
};
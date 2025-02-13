#pragma once
#define GLEW_STATIC
#include "glew.h"
#include "../libs/glm/glm.hpp"
#include <fbxsdk.h>
#include <vector>
#include <string>
#include "VertexN.h"


class MeshFBX
{
public:
	MeshFBX();
	~MeshFBX();
	bool LoadMesh(const std::string filename);
	void DrawTriangles();

	int triangles = 0;

private:
	void LoadBuffers();

	FbxManager* manager;
	FbxIOSettings* ioSettings;
	
	bool bLoaded = false;
	std::vector<Vertex2> vertices;
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ibo = 0;

};

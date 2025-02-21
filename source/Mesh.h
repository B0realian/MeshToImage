#pragma once
#define GLEW_STATIC
#include "glew.h"
#include "../libs/glm/glm.hpp"
#include "../libs/json/json.hpp"
#include <fbxsdk.h>
#include <vector>
#include <string>
#include "Enums.h"
#include "VertexN.h"

using JSON = nlohmann::json;

class Mesh
{
public:
	Mesh();
	Mesh(float scale);
	~Mesh();
	bool LoadMesh(const std::string filename, EMeshType filetype);
	void DrawTriangles();

	float fbxScale = 0.01f;		// My .fbx-files in testing were far too big. YMMV.
	int triangles = 0;

private:
	bool GltfFile(const std::string filename);
	bool ObjFile(const std::string filename);
	bool FbxFile(const std::string filename);
	void LoadBuffers();

	bool bLoaded = false;
	std::vector<Vertex2> vertices;
	GLuint vao = 0;
	GLuint vbo = 0;

};
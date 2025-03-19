#pragma once

enum class EMeshType;
struct Vertex2;

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool LoadMesh(const char* filename, const EMeshType filetype, const float in_mesh_scale);
	void DrawTriangles();

	int32_t triangles = 0;

private:

	bool GltfFile(const char* filename, const float in_mesh_scale);
	bool ObjFile(const char* filename, const float in_mesh_scale);
	bool FbxFile(const char* filename, const float in_mesh_scale);
	void LoadBuffers(const std::vector<Vertex2>& in);

	GLsizei num_vertices = 0;
	uint32_t vao = 0;
	uint32_t vbo = 0;
};

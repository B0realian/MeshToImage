#include "MeshObject.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>


MeshObject::MeshObject()
{

}

MeshObject::~MeshObject()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
}

bool MeshObject::LoadObj(const std::string filename)
{
    std::vector<unsigned int> vertexIndex, uvIndex;
    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempUVs;

    if (filename.find(".obj") != std::string::npos)
    {
        std::ifstream file(filename, std::ios::in);
        if (!file)
        {
            std::cout << "Failed to open " << filename << std::endl;
            return false;
        }

        std::cout << "Loading " << filename << std::endl;
        std::string linebuffer;
        while (std::getline(file, linebuffer))
        {
            if (linebuffer.substr(0, 2) == "v ")
            {
                std::istringstream v(linebuffer.substr(2));
                glm::vec3 vertex;
                v >> vertex.x;
                v >> vertex.y;
                v >> vertex.z;
                tempVertices.push_back(vertex);
            }
            else if (linebuffer.substr(0, 2) == "vt")
            {
                std::istringstream vt(linebuffer.substr(2));
                glm::vec2 uv;
                vt >> uv.x;
                vt >> uv.y;
                tempUVs.push_back(uv);
            }
            else if (linebuffer.substr(0, 2) == "f ")
            {
                int v1, v2, v3;
                int t1, t2, t3;
                int n1, n2, n3;
                const char* face = linebuffer.c_str();
                int match = sscanf_s(face, "f %i/%i/%i %i/%i/%i %i/%i/%i", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
                if (match != 9)
                    std::cout << "Failed to parse .obj." << std::endl;

                vertexIndex.push_back(v1 - 1);
                vertexIndex.push_back(v2 - 1);
                vertexIndex.push_back(v3 - 1);
                uvIndex.push_back(t1 - 1);
                uvIndex.push_back(t2 - 1);
                uvIndex.push_back(t3 - 1);
                // Not bothering with normals at the moment
            }
        }
        file.close();

        for (unsigned int i = 0; i < vertexIndex.size(); i++)
        {
            glm::vec3 vertex = tempVertices[vertexIndex[i]];
            glm::vec2 uv = tempUVs[uvIndex[i]];

            Vertex2 meshVertex;
            meshVertex.position = vertex;
            meshVertex.texCoords = uv;
            vertices.push_back(meshVertex);
        }

        LoadBuffers();
        return (bLoaded = true);
    }

    return false;
}

void MeshObject::DrawElements()
{
	if (!bLoaded) return;
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void MeshObject::DrawTriangles()
{
	if (!bLoaded) return;
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);
}

void MeshObject::LoadBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    //glGenBuffers(1, &ibo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex2), &vertices[0], GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (GLvoid*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}


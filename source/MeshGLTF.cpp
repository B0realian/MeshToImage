#include "MeshGLTF.h"
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>


MeshGLTF::MeshGLTF()
{

}

MeshGLTF::~MeshGLTF()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

bool MeshGLTF::LoadMesh(const std::string filename)
{
	std::ifstream file(filename);
	json = JSON::parse(file);
	std::string uri = json["buffers"][0]["uri"];
	unsigned int positionIndex = json["meshes"][0]["primitives"][0]["attributes"]["POSITION"];
	unsigned int uvIndex = json["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"];
	unsigned int indicesIndex = json["meshes"][0]["primitives"][0]["indices"];
	unsigned int positionOffset = json["bufferViews"][positionIndex]["byteOffset"];
	unsigned int positionLength = json["bufferViews"][positionIndex]["byteLength"];
	unsigned int uvOffset = json["bufferViews"][uvIndex]["byteOffset"];
	unsigned int uvLength = json["bufferViews"][uvIndex]["byteLength"];
	unsigned int indicesOffset = json["bufferViews"][indicesIndex]["byteOffset"];
	unsigned int indicesLength = json["bufferViews"][indicesIndex]["byteLength"];

	std::vector<unsigned int> vertexIndex, uvIndex, tempIndex;
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;
	
	if (uri.find(".bin") != std::string::npos)
	{
		std::string binFileName;
		if (filename.find("/") == std::string::npos)
			binFileName = uri;
		else
			binFileName = filename.substr(0, filename.find_last_of('/') + 1) + uri;

		int fileSize = std::filesystem::file_size(binFileName);
		std::vector<char> binData(fileSize);
		std::ifstream binFile(binFileName, std::ios::binary);
		binFile.read(binData.data(), fileSize);
		binFile.close();
		
		for (int i = positionOffset; i < positionOffset + positionLength; i += sizeof(float) * 3)
		{
			unsigned char xbytes[] = { binData[i], binData[i + 1], binData[i + 2], binData[i + 3] };
			unsigned char ybytes[] = { binData[i + 4], binData[i + 5], binData[i + 6], binData[i + 7] };
			unsigned char zbytes[] = { binData[i + 8], binData[i + 9], binData[i + 10], binData[i + 11]	};
			float x, y, z;
			std::memcpy(&x, xbytes, sizeof(float));
			std::memcpy(&y, ybytes, sizeof(float));
			std::memcpy(&z, zbytes, sizeof(float));
			glm::vec3 tempVec = glm::vec3(x, y, z);
			tempVertices.push_back(tempVec);
		}

		for (int i = uvOffset; i < uvOffset + uvLength; i += sizeof(float) * 2)
		{
			unsigned char xbytes[] = { binData[i], binData[i + 1], binData[i + 2], binData[i + 3] };
			unsigned char ybytes[] = { binData[i + 4], binData[i + 5], binData[i + 6], binData[i + 7] };
			float x, y;
			std::memcpy(&x, xbytes, sizeof(float));
			std::memcpy(&y, ybytes, sizeof(float));
			glm::vec2 tempVec = glm::vec2(x, y);
			tempUVs.push_back(tempVec);
		}

		for (int i = indicesOffset; i < indicesOffset + indicesLength; i += sizeof(unsigned short))
		{
			unsigned char bytes[] = { binData[i], binData[i + 1] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			tempIndex.push_back(value);
		}

		// Unravel vertices and uvs
	}
	else
	{

	}

	file.close();
}

void MeshGLTF::DrawTriangles()
{
	if (!bLoaded) return;
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);
}

void MeshGLTF::LoadBuffers()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex2), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (GLvoid*)(3 * sizeof(GLfloat)));

	glBindVertexArray(0);
}
#include "MeshGLTF.h"
#include <iostream>
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
	if (filename.find(".gltf") != std::string::npos)
	{
		std::ifstream file(filename);
		json = JSON::parse(file);
		std::string uri = json["buffers"][0]["uri"];
		unsigned int positionIndex = json["meshes"][0]["primitives"][0]["attributes"]["POSITION"];
		unsigned int txcoordIndex = json["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"];
		unsigned int indicesIndex = json["meshes"][0]["primitives"][0]["indices"];
		unsigned int positionOffset = json["bufferViews"][positionIndex]["byteOffset"];
		unsigned int positionLength = json["bufferViews"][positionIndex]["byteLength"];
		unsigned int uvOffset = json["bufferViews"][txcoordIndex]["byteOffset"];
		unsigned int uvLength = json["bufferViews"][txcoordIndex]["byteLength"];
		unsigned int indicesOffset = json["bufferViews"][indicesIndex]["byteOffset"];
		unsigned int indicesLength = json["bufferViews"][indicesIndex]["byteLength"];
		unsigned int indicesType = json["accessors"][indicesIndex]["componentType"];

		std::vector<unsigned int> vertexIndex, uvIndex;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUVs;
		std::vector<unsigned char> binData;

		if (uri.find(".bin") != std::string::npos)		// gltf with separate .bin containing mesh data
		{
			std::string binFileName = filename.substr(0, filename.find_last_of('/') + 1) + uri;
			unsigned int fileSize = static_cast<unsigned int>(std::filesystem::file_size(binFileName));
			binData.resize(fileSize);
			std::ifstream binFile(binFileName, std::ios::binary);
			binFile.read(reinterpret_cast<char*>(binData.data()), fileSize);
			binFile.close();
		}
		else											// gltf with embedded data, base64 encoded
		{
			const char b64table[64] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
										'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
										'0','1','2','3','4','5','6','7','8','9','+','/' };

			unsigned int start = static_cast<unsigned int>(uri.find("base64,") + 7);
			uri = uri.substr(start);
			int padding = uri.length() % 4;
			if (padding > 0)							// This should not be necessary. 
			{
				for (int i = 0; i < padding; i++)
					uri.append(padding, 'A');
			}

			for (int i = 0; i < uri.length(); i += 4)
			{
				std::string tempString = uri.substr(i, 4);
				unsigned char uriSub[4];
				std::copy(tempString.begin(), tempString.end(), uriSub);
				unsigned char from64[4];

				for (int c = 0; c < 4; c++)
				{
					int idx = 0;
					for (char x : b64table)
					{
						if (uriSub[c] == x)
						{
							from64[c] = idx;
							break;
						}
						idx++;
					}
				}

				unsigned char byteTriplet[3];
				byteTriplet[0] = (from64[0] << 2) + (from64[1] >> 4);
				byteTriplet[1] = (from64[1] << 4) + (from64[2] >> 2);
				byteTriplet[2] = (from64[2] << 6) + from64[3];
				binData.push_back(byteTriplet[0]);
				binData.push_back(byteTriplet[1]);
				binData.push_back(byteTriplet[2]);
			}
		}

		for (unsigned int i = positionOffset; i < positionOffset + positionLength; i += sizeof(float) * 3)
		{
			unsigned char xbytes[] = { binData[i],		binData[i + 1], binData[i + 2],	 binData[i + 3] };
			unsigned char ybytes[] = { binData[i + 4],	binData[i + 5], binData[i + 6],  binData[i + 7] };
			unsigned char zbytes[] = { binData[i + 8],	binData[i + 9], binData[i + 10], binData[i + 11] };

			float x, y, z;
			std::memcpy(&x, xbytes, sizeof(float));
			std::memcpy(&y, ybytes, sizeof(float));
			std::memcpy(&z, zbytes, sizeof(float));
			glm::vec3 tempVec = glm::vec3(x, y, z);
			tempVertices.push_back(tempVec);
		}

		for (unsigned int i = uvOffset; i < uvOffset + uvLength; i += sizeof(float) * 2)
		{
			unsigned char xbytes[] = { binData[i],		binData[i + 1], binData[i + 2], binData[i + 3] };
			unsigned char ybytes[] = { binData[i + 4],	binData[i + 5], binData[i + 6], binData[i + 7] };

			float x, y;
			std::memcpy(&x, xbytes, sizeof(float));
			std::memcpy(&y, ybytes, sizeof(float));
			glm::vec2 tempVec = glm::vec2(x, y);
			tempUVs.push_back(tempVec);
		}

		if (indicesType == 5121)
		{
			for (unsigned int i = indicesOffset; i < indicesOffset + indicesLength; i++)
			{
				unsigned int value = static_cast<unsigned int>(binData[i]);
				vertexIndex.push_back(value);
			}
		}
		else if (indicesType == 5123)
		{
			for (unsigned int i = indicesOffset; i < indicesOffset + indicesLength; i += sizeof(unsigned short))
			{
				unsigned char bytes[] = { binData[i], binData[i + 1] };
				unsigned short value;
				std::memcpy(&value, bytes, sizeof(unsigned short));
				vertexIndex.push_back(value);
			}
		}
		else if (indicesType == 5125)
		{
			for (unsigned int i = indicesOffset; i < indicesOffset + indicesLength; i += sizeof(unsigned int))
			{
				unsigned char bytes[] = { binData[i],	binData[i + 1],	binData[i + 2], binData[i + 3] };
				unsigned int value;
				std::memcpy(&value, bytes, sizeof(unsigned int));
				vertexIndex.push_back(value);
			}
		}
		
		for (unsigned int i = 0; i < vertexIndex.size(); i++)
		{
			glm::vec3 vertex = tempVertices[vertexIndex[i]];
			glm::vec2 uv = tempUVs[vertexIndex[i]];

			Vertex2 meshVertex;
			meshVertex.position = vertex;
			meshVertex.texCoords = uv;
			vertices.push_back(meshVertex);
		}

		triangles = vertexIndex.size() / 3;
		file.close();
		LoadBuffers();
		return (bLoaded = true);
	}
	
	return false;
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
#include "stdafx.h"//pre-compiled headers

#include "../libs/json/json.hpp"//header-only, so not in stdafx.h? dunno...
#include "Mesh.h"
#include "Enums.h"
#include "VertexN.h"

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Mesh::DrawTriangles()
{
	if (!num_vertices)
		return;
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	glBindVertexArray(0);
}

bool Mesh::LoadMesh(const char* in_filename, const EMeshType in_filetype, const float in_mesh_scale)
{
	switch (in_filetype)
	{
	case EMeshType::None:
		std::cout << "No mesh file found.\n";
		return false;
	case EMeshType::GLTF:
		if (GltfFile(in_filename, in_mesh_scale))
			return true;
		break;
	case EMeshType::OBJ:
		if (ObjFile(in_filename, in_mesh_scale))
			return true;
		break;
	case EMeshType::FBX:
		if (FbxFile(in_filename, in_mesh_scale))
			return true;
		break;
	default:
		std::cout << "Unexpected error obtaining mesh.\n";
		return false;
	}
	std::cout << "Error: File/enum mismatch.\n";
	return false;
}

bool Mesh::GltfFile(const char* in_filename, const float in_mesh_scale)
{
	if (::strstr(in_filename, ".gltf"))
	{
		std::cout << "Opening file " << in_filename << std::endl;
		std::ifstream file(in_filename);
		if (!file)
		{
			std::cout << "Failed to open " << in_filename << std::endl;
			return false;
		}
		std::cout << "Parsing JSON..." << std::endl;
		nlohmann::json json = nlohmann::json::parse(file);
		std::cout << "Reading common gltf-variables..." << std::endl;
		std::string uri = json["buffers"][0]["uri"];
		uint32_t positionIndex = json["meshes"][0]["primitives"][0]["attributes"]["POSITION"];
		uint32_t txcoordIndex = json["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"];
		uint32_t indicesIndex = json["meshes"][0]["primitives"][0]["indices"];
		uint32_t posBufferView = json["accessors"][positionIndex]["bufferView"];
		uint32_t txcoordBufferView = json["accessors"][txcoordIndex]["bufferView"];
		uint32_t indBufferView = json["accessors"][indicesIndex]["bufferView"];
		uint32_t indicesType = json["accessors"][indicesIndex]["componentType"];
		uint32_t positionCount = json["accessors"][positionIndex]["count"];
		uint32_t txcoordCount = json["accessors"][txcoordIndex]["count"];
		uint32_t indicesCount = json["accessors"][indicesIndex]["count"];
		//uint32_t positionLength = json["bufferViews"][posBufferView]["byteLength"];
		//uint32_t txcoordLength = json["bufferViews"][txcoordBufferView]["byteLength"];
		uint32_t indicesLength = json["bufferViews"][indBufferView]["byteLength"];

		uint32_t counter = 0;
		uint32_t posElementSize = 12;
		uint32_t txcoordElementSize = 8;
		uint32_t indElementSize = indicesLength / indicesCount;

		uint32_t positionByteStride = posElementSize;
		uint32_t positionAccessorByteOffset = 0;
		uint32_t txcoordByteStride = txcoordElementSize;
		uint32_t txcoordAccessorByteOffset = 0;
		uint32_t indicesByteStride = indElementSize;
		uint32_t indicesAccessorByteOffset = 0;

		uint32_t positionOffset = 0;
		uint32_t txcoordOffset = 0;
		uint32_t indicesOffset = 0;

		std::vector<uint32_t> vertexIndex, uvIndex;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUVs;
		std::vector<unsigned char> binData;

		std::cout << "Looking for optional gltf-variables..." << std::endl;
		if (json["bufferViews"][posBufferView].contains("byteStride"))
			positionByteStride = json["bufferViews"][posBufferView]["byteStride"];
		if (json["bufferViews"][txcoordBufferView].contains("byteStride"))
			txcoordByteStride = json["bufferViews"][txcoordBufferView]["byteStride"];
		if (json["bufferViews"][indBufferView].contains("byteStride"))
			indicesByteStride = json["bufferViews"][indBufferView]["byteStride"];
		if (json["bufferViews"][posBufferView].contains("byteOffset"))
			positionOffset = json["bufferViews"][posBufferView]["byteOffset"];
		if (json["bufferViews"][txcoordBufferView].contains("byteOffset"))
			txcoordOffset = json["bufferViews"][txcoordBufferView]["byteOffset"];
		if (json["bufferViews"][indBufferView].contains("byteOffset"))
			indicesOffset = json["bufferViews"][indBufferView]["byteOffset"];
		if (json["accessors"][positionIndex].contains("byteOffset"))
			positionAccessorByteOffset = json["accessors"][positionIndex]["byteOffset"];
		if (json["accessors"][txcoordIndex].contains("byteOffset"))
			txcoordAccessorByteOffset = json["accessors"][txcoordIndex]["byteOffset"];
		if (json["accessors"][indicesIndex].contains("byteOffset"))
			indicesAccessorByteOffset = json["accessors"][indicesIndex]["byteOffset"];


		if (uri.find(".bin") != std::string::npos)		// gltf with separate .bin containing mesh data
		{
			std::string binFileName;
			{
				std::string slask = in_filename;
				if (slask.find_last_of('/') == std::string::npos)
					binFileName = slask.substr(0, slask.find_last_of('\\') + 1) + uri;
				else
					binFileName = slask.substr(0, slask.find_last_of('/') + 1) + uri;
			}

			std::cout << "Reading separate bin-file " << binFileName << "..." << std::endl;
			std::ifstream binFile(binFileName, std::ios::binary);
			if (!binFile)
			{
				std::cout << "Failed to open " << binFileName << std::endl;
				return false;
			}
			uint32_t fileSize = static_cast<uint32_t>(std::filesystem::file_size(binFileName));
			binData.resize(fileSize);
			binFile.read(reinterpret_cast<char*>(binData.data()), fileSize);
			binFile.close();
		}
		else											// gltf with embedded data, base64 encoded
		{
			std::cout << "Decoding embedded data..." << std::endl;
			const char b64table[64] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
										'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
										'0','1','2','3','4','5','6','7','8','9','+','/' };

			uint32_t start = static_cast<uint32_t>(uri.find("base64,") + 7);
			uri = uri.substr(start);					// Assuming encoding will always be an octet stream. Check beginning of "uri" in gltf-file if decoding seems off.

			int padding = uri.length() % 4;				// This should not be necessary but is a safeguard. 
			if (padding > 0)
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
							from64[c] = (char)idx;
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


		std::cout << "Retrieving gltf position data..." << std::endl;
		counter = 0;
		for (uint32_t i = positionOffset + positionAccessorByteOffset; counter < positionCount; i += posElementSize + (positionByteStride - posElementSize))
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

			counter++;
		}

		std::cout << "Retrieving gltf uv data..." << std::endl;
		counter = 0;
		for (uint32_t i = txcoordOffset + txcoordAccessorByteOffset; counter < txcoordCount; i += txcoordElementSize + (txcoordByteStride - txcoordElementSize))
		{
			unsigned char xbytes[] = { binData[i],		binData[i + 1], binData[i + 2], binData[i + 3] };
			unsigned char ybytes[] = { binData[i + 4],	binData[i + 5], binData[i + 6], binData[i + 7] };

			float x, y;
			std::memcpy(&x, xbytes, sizeof(float));
			std::memcpy(&y, ybytes, sizeof(float));
			glm::vec2 tempVec = glm::vec2(x, y);
			tempUVs.push_back(tempVec);

			counter++;
		}

		std::cout << "Retrieving gltf indices data..." << std::endl;
		counter = 0;
		if (indicesType == 5121)
		{
			for (uint32_t i = indicesOffset + indicesAccessorByteOffset; counter < indicesCount; i += indElementSize + (indicesByteStride - indElementSize))
			{
				uint32_t value = static_cast<uint32_t>(binData[i]);
				vertexIndex.push_back(value);

				counter++;
			}
		}
		else if (indicesType == 5123)
		{
			for (uint32_t i = indicesOffset + indicesAccessorByteOffset; counter < indicesCount; i += indElementSize + (indicesByteStride - indElementSize))
			{
				unsigned char bytes[] = { binData[i], binData[i + 1] };
				unsigned short value;
				std::memcpy(&value, bytes, sizeof(unsigned short));
				vertexIndex.push_back(value);

				counter++;
			}
		}
		else if (indicesType == 5125)
		{
			for (uint32_t i = indicesOffset + indicesAccessorByteOffset; counter < indicesCount; i += indElementSize + (indicesByteStride - indElementSize))
			{
				unsigned char bytes[] = { binData[i],	binData[i + 1],	binData[i + 2], binData[i + 3] };
				uint32_t value;
				std::memcpy(&value, bytes, sizeof(uint32_t));
				vertexIndex.push_back(value);

				counter++;
			}
		}

		std::cout << "Assembling vertices from data..." << std::endl;
		std::vector<Vertex2> vertices;
		for (
			uint32_t i = 0;
			i < vertexIndex.size();
			++i
			)
		{
			glm::vec3 vertex = tempVertices[vertexIndex[i]];
			glm::vec2 uv = tempUVs[vertexIndex[i]];
			vertex *= in_mesh_scale;

			Vertex2 meshVertex;
			meshVertex.position = vertex;
			meshVertex.texCoords = uv;
			vertices.push_back(meshVertex);
		}
		num_vertices = (GLsizei)vertices.size();

		triangles = (int32_t)vertexIndex.size() / 3;
		file.close();
		LoadBuffers(vertices);
		return num_vertices;
	}

	return false;
}

bool Mesh::ObjFile(const char* in_filename, const float in_mesh_scale)
{
	if (::strstr(in_filename, ".obj"))
	{
		std::vector<uint32_t> vertexIndex, uvIndex;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUVs;

		std::ifstream file(in_filename, std::ios::in);
		if (!file)
		{
			std::cout << "Failed to open " << in_filename << std::endl;
			return false;
		}

		std::cout << "Loading " << in_filename << std::endl;
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
				// Not bothering with normals
			}
		}
		file.close();

		std::vector<Vertex2> vertices;
		for (
			uint32_t i = 0;
			i < vertexIndex.size();
			++i
			)
		{
			glm::vec3 vertex = tempVertices[vertexIndex[i]];
			glm::vec2 uv = tempUVs[uvIndex[i]];
			vertex *= in_mesh_scale;

			Vertex2 meshVertex;
			meshVertex.position = vertex;
			meshVertex.texCoords = uv;
			vertices.push_back(meshVertex);
		}
		num_vertices = (GLsizei)vertices.size();

		triangles = (int32_t)vertexIndex.size() / 3;

		LoadBuffers(vertices);
		return num_vertices;
	}

	return false;
}

#if 0
bool Mesh::FbxFile(const std::string filename, const float in_mesh_scale)
{
	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ioSettings);

	FbxImporter* importer = FbxImporter::Create(manager, "");
	importer->Initialize(filename.c_str(), -1, manager->GetIOSettings());
	FbxScene* scene = FbxScene::Create(manager, "SceneName");
	importer->Import(scene);
	importer->Destroy();
	FbxNode* node = scene->GetRootNode();

	if (node)
	{
		int childrenAmount = node->GetChildCount();
		FbxNode* childNode = 0;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUVs;
		std::vector<uint32_t> vertexIndex, uvIndex;

		for (int i = 0; i < childrenAmount; i++)
		{
			childNode = node->GetChild(i);
			FbxMesh* mesh = childNode->GetMesh();

			if (mesh != NULL)
			{
				std::cout << "Mesh found in child node " << i << std::endl;

				FbxVector4* verticeArr = mesh->GetControlPoints();
				FbxGeometryElementUV* elementUV = mesh->GetElementUV(0);		// Banking on the main texture being layer 0, or that all textures have the same UV
				int currentVertexIndex = 0;
				int currentUVIndex = 0;

				for (int controlPoints = 0; controlPoints < mesh->GetControlPointsCount(); controlPoints++)
				{
					glm::vec3 vert = glm::vec3(verticeArr[controlPoints].mData[0], verticeArr[controlPoints].mData[1], verticeArr[controlPoints].mData[2]);
					tempVertices.push_back(vert);
				}

				for (int polygon = 0; polygon < mesh->GetPolygonCount(); polygon++)
				{
					for (int posInPolygon = 0; posInPolygon < mesh->GetPolygonSize(polygon); posInPolygon++)
					{
						currentVertexIndex = mesh->GetPolygonVertex(polygon, posInPolygon);
						currentUVIndex = mesh->GetTextureUVIndex(polygon, posInPolygon);
						vertexIndex.push_back(currentVertexIndex);
						uvIndex.push_back(currentUVIndex);

						FbxVector2 uvCoord{ 0, 0 };

						switch (elementUV->GetMappingMode())
						{
						case FbxGeometryElement::eByControlPoint:
							switch (elementUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								uvCoord = elementUV->GetDirectArray().GetAt(currentVertexIndex);
								tempUVs.push_back(glm::vec2(uvCoord.mData[0], uvCoord.mData[1]));
								break;
							case FbxGeometryElement::eIndexToDirect:
								uvCoord = elementUV->GetDirectArray().GetAt(elementUV->GetIndexArray().GetAt(currentVertexIndex));
								tempUVs.push_back(glm::vec2(uvCoord.mData[0], uvCoord.mData[1]));
								break;
							default:
								std::cout << "Error reading UV element by vertex.\n";
								break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							uvCoord = elementUV->GetDirectArray().GetAt(currentUVIndex);
							tempUVs.push_back(glm::vec2(uvCoord.mData[0], uvCoord.mData[1]));
							break;
						default:
							std::cout << "Error reading UV element.\n";
							break;
						}
					}
				}

				for (int n = 0; n < vertexIndex.size(); n++)
				{
					glm::vec3 vertex = tempVertices[vertexIndex[n]];
					glm::vec2 uv = tempUVs[n];
					vertex *= in_mesh_scale;		// Vertice values turned out to be scaled up 100 times in my test megascan fbx. YMMV. 

					Vertex2 meshVertex;
					meshVertex.position = vertex;
					meshVertex.texCoords = uv;
					vertices.push_back(meshVertex);
				}

				triangles = vertices.size() / 3;

				LoadBuffers();
				return (bLoaded = true);	// This causees only one mesh to be loaded from the fbx. Should be alright for this project?
			}
			else
				std::cout << "No mesh found in fbx child node " << i << std::endl;
		}
	}
	else
		std::cout << "No root node found. Check path and filename.\n";

	return false;
}
#else
bool Mesh::FbxFile(const char* in_filename, const float in_mesh_scale)
{
	in_filename;//w4: unreferenced
	assert(0 && "temporarily disabled!");
	return false;
}
#endif

void Mesh::LoadBuffers(const std::vector<Vertex2>& in)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, in.size() * sizeof(Vertex2), &in[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (GLvoid*)(3 * sizeof(GLfloat)));

	glBindVertexArray(0);
}


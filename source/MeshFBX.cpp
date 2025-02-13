#include "MeshFBX.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#if _DEBUG
#pragma comment (lib, "C:\\CPP\\_libraries\\fbx_2020.3.7\\lib\\x64\\debug\\libfbxsdk-md.lib")
#pragma comment (lib, "C:\\CPP\\_libraries\\fbx_2020.3.7\\lib\\x64\\debug\\libxml2-md.lib")
#pragma comment (lib, "C:\\CPP\\_libraries\\fbx_2020.3.7\\lib\\x64\\debug\\zlib-md.lib")
#else
#pragma comment (lib, "C:\\CPP\\_libraries\\fbx_2020.3.7\\lib\\x64\\release\\libfbxsdk-md.lib")
#pragma comment (lib, "C:\\CPP\\_libraries\\fbx_2020.3.7\\lib\\x64\\release\\libxml2-md.lib")
#pragma comment (lib, "C:\\CPP\\_libraries\\fbx_2020.3.7\\lib\\x64\\release\\zlib-md.lib")
#endif

MeshFBX::MeshFBX()
{
	manager = FbxManager::Create();
	ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ioSettings);
}

MeshFBX::~MeshFBX()
{

}

bool MeshFBX::LoadMesh(const std::string filename)
{
	
	FbxImporter* importer = FbxImporter::Create(manager, "");
	importer->Initialize(filename.c_str(), -1, manager->GetIOSettings());
	FbxScene* scene = FbxScene::Create(manager, "SceneName");
	importer->Import(scene);
	importer->Destroy();
	FbxNode* node = scene->GetRootNode();

	if (node)
	{
		std::cout << "Root node found.\n";

		int childrenAmount = node->GetChildCount();
		FbxNode* childNode = 0;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUVs;
		std::vector<unsigned int> vertexIndex, uvIndex;

		for (int i = 0; i < childrenAmount; i++)
		{
			childNode = node->GetChild(i);
			FbxMesh* mesh = childNode->GetMesh();

			if (mesh != NULL)
			{
				std::cout << "Mesh found in child node " << i << std::endl;

				FbxVector4* verticeArr = mesh->GetControlPoints();
				FbxGeometryElementUV* elementUV = mesh->GetElementUV(0);		// Banking on the main texture being layer 0
				int currentVertexIndex = 0;
				int currentUVIndex = 0;

				for (int controlPoints = 0; controlPoints < mesh->GetControlPointsCount(); controlPoints++)
				{
					glm::vec3 vert = glm::vec3(verticeArr[controlPoints].mData[0], verticeArr[controlPoints].mData[1], verticeArr[controlPoints].mData[2]);
					tempVertices.push_back(vert);
				}

				std::cout << "Number of unique vertices in mesh: " << tempVertices.size() << std::endl;

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

				std::cout << "Number of unique uv coordinates: " << tempUVs.size() << std::endl;
				std::cout << "Number of vertex indices: " << vertexIndex.size() << std::endl;
				std::cout << "Number of uv indices: " << uvIndex.size() << std::endl;

				for (int n = 0; n < vertexIndex.size(); n++)
				{
					glm::vec3 vertex = tempVertices[vertexIndex[n]];
					glm::vec2 uv = tempUVs[uvIndex[n]];

					Vertex2 meshVertex;
					meshVertex.position = vertex;
					meshVertex.texCoords = uv;
					vertices.push_back(meshVertex);
				}

				triangles = vertexIndex.size() / 3;

				LoadBuffers();
				return (bLoaded = true);	// This causees only one mesh to be loaded from the fbx. Should be alright for this project?
			}
		}
	}
	return false;
}

void MeshFBX::DrawTriangles()
{
	if (!bLoaded) return;
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);
}

void MeshFBX::LoadBuffers()
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
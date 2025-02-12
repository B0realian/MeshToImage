#include "MeshFBX.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>


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
		int childrenAmount = node->GetChildCount();
		FbxNode* childNode = 0;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUVs;
		std::vector<unsigned int> tempVertIndex, tempUVIndex;

		for (int i = 0; i < childrenAmount; i++)
		{
			childNode = node->GetChild(i);
			FbxMesh* mesh = childNode->GetMesh();

			if (mesh != NULL)
			{
				FbxVector4* vertices = mesh->GetControlPoints();
				FbxGeometryElementUV* elementUV = mesh->GetElementUV(0);		// Banking on the main texture being layer 0

				for (int controlPoints = 0; controlPoints < mesh->GetControlPointsCount(); controlPoints++)
				{
					glm::vec3 vert = glm::vec3(vertices[controlPoints].mData[0], vertices[controlPoints].mData[1], vertices[controlPoints].mData[2]);
					tempVertices.push_back(vert);
				}

				for (int polygon = 0; polygon < mesh->GetPolygonCount(); polygon++)
				{
					for (int posInPolygon = 0; posInPolygon < mesh->GetPolygonSize(polygon); posInPolygon++)
					{
						int vertIndex = mesh->GetPolygonVertex(polygon, posInPolygon);
						int uvIndex = mesh->GetTextureUVIndex(polygon, posInPolygon);
						tempVertIndex.push_back(vertIndex);
						tempUVIndex.push_back(uvIndex);

						FbxVector2 uvCoord{ 0, 0 };

						switch (elementUV->GetMappingMode())
						{
						case FbxGeometryElement::eByControlPoint:
							switch (elementUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								uvCoord = elementUV->GetDirectArray().GetAt(vertIndex);
								tempUVs.push_back(glm::vec2(uvCoord.mData[0], uvCoord.mData[1]));
								break;
							case FbxGeometryElement::eIndexToDirect:
								uvCoord = elementUV->GetDirectArray().GetAt(elementUV->GetIndexArray().GetAt(vertIndex));
								tempUVs.push_back(glm::vec2(uvCoord.mData[0], uvCoord.mData[1]));
								break;
							default:
								std::cout << "Error reading UV element by vertex.\n";
								break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							uvCoord = elementUV->GetDirectArray().GetAt(uvIndex);
							tempUVs.push_back(glm::vec2(uvCoord.mData[0], uvCoord.mData[1]));
							break;
						default:
							std::cout << "Error reading UV element.\n";
							break;
						}
					}
				}

				// Convert to Vertex2 vector

				// LoadBuffers();
				return (bLoaded = false);	// Change to true when function is finished		// This causees only one mesh to be loaded from the fbx. Should be alright for this project?
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
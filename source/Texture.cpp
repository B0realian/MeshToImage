#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"
#include <iostream>
#include <fstream>

Texture::Texture()
{
	texture = 0;
}

Texture::~Texture()
{

}

bool Texture::LoadTexture(const std::string& filename, bool generateMipMaps)
{
	int width, height, components;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &components, STBI_rgb_alpha);
	if (imageData == NULL)
	{
		std::cout << "Failed to load texture." << std::endl;
		return false;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

	if (generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(imageData);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void Texture::Bind(GLuint texUnit)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::Unbind(GLuint texUnit)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SaveTGA(int width, int height, int fileNum)
{
	tgaHeader.width = width;
	tgaHeader.height = height;
	GLubyte* capturedColor = new GLubyte[width * height * 3];
	GLfloat* capturedDepth = new GLfloat[width * height];
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, capturedColor);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, capturedDepth);

	std::ofstream colFile("logs/objTexture.tga", fileNum, std::ios::binary);
	if (!colFile.is_open())
	{
		std::cout << "Failed to save images." << std::endl;
		return;
	}
	colFile.write((char*)&tgaHeader, sizeof(UncompTGAHeader));
	colFile.write((char*)capturedColor, (width * height * 3));
	colFile.close();

	std::ofstream heightFile("logs/objHeightMap.tga", fileNum, std::ios::binary);
	if (!heightFile.is_open())
	{
		std::cout << "Failed to save images." << std::endl;
		return;
	}
	heightFile.write((char*)&tgaHeader, sizeof(UncompTGAHeader));
	for (int i = 0; i < width * height; i++)
	{
		GLubyte tempByte = (GLubyte)(-255 * capturedDepth[i]) + 255;
		for (int j = 0; j < 3; j++)
		{
			heightFile.write((char*)&tempByte, sizeof(GLubyte));
		}
	}
	heightFile.close();
	delete[] capturedColor;
	delete[] capturedDepth;
}
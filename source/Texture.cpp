#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"
//#include "../libs/tt/tinytiffwriter.h"
#include <format>
#include <iostream>
#include <fstream>
#include <vector>

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

void Texture::SaveCaptures(int width, int height, int fileNum)
{
	tgaHeader.width = width;
	tgaHeader.height = height;
	pgmHeader.size = std::format("{} {}\n", width, height);
	std::string RGB_Filename = std::format("logs/Texture{}.tga", fileNum);
	std::string GM_Filename = std::format("logs/HeightMap{}.pgm", fileNum);
	GLfloat* capturedColor = new GLfloat[width * height * 3];
	GLfloat* capturedDepth = new GLfloat[width * height];
	glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, capturedColor);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, capturedDepth);

	std::ofstream colFile(RGB_Filename, std::ios::binary);
	if (colFile.is_open())
	{
		colFile.write((char*)&tgaHeader, sizeof(Raw_RGB_TGA_Header));
		for (int i = 0; i < width * height * 3; i += 3)
		{
			GLubyte tempR = static_cast<GLubyte>(255 * capturedColor[i + 2]);
			GLubyte tempG = static_cast<GLubyte>(255 * capturedColor[i + 1]);
			GLubyte tempB = static_cast<GLubyte>(255 * capturedColor[i]);
			colFile.write((char*)&tempR, 1);
			colFile.write((char*)&tempG, 1);
			colFile.write((char*)&tempB, 1);
		}
		colFile.close();
	}
	else
		std::cout << "Failed to save RGB-data." << std::endl;

	std::ofstream heightFile(GM_Filename);
	if (heightFile.is_open())
	{
		heightFile << pgmHeader.type << pgmHeader.size << pgmHeader.maxValue;
		for (int i = (width * height) - width; i > -1; i -= width)
		{
			for (int j = 0; j < width; j++)
			{
				GLushort tempValue = static_cast<GLushort>((-65535 * capturedDepth[j+i]) + 65535);
				heightFile << tempValue << "\n";
			}
		}
		heightFile.close();
	}
	else
		std::cout << "Failed to save height-data." << std::endl;
	
	delete[] capturedColor;
	delete[] capturedDepth;
}

void Texture::SaveTiff(int width, int height, int fileNum)
{
	std::string tif_Filename = std::format("logs/Texture{}.tif", fileNum);
	GLfloat* capturedColor = new GLfloat[width * height * 3];
	GLfloat* capturedDepth = new GLfloat[width * height];
	glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, capturedColor);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, capturedDepth);
	GLushort* RGBA_Data = new GLushort[width * height * 4];

	for (int i = 0, c = 0, d = 0; i < width * height * 4; i += 4)
	{
		RGBA_Data[i] = static_cast<GLushort>(-65535 * capturedColor[c] + 65535);
		RGBA_Data[i + 1] = static_cast<GLushort>(-65535 * capturedColor[c + 1] + 65535);
		RGBA_Data[i + 2] = static_cast<GLushort>(-65535 * capturedColor[c + 2] + 65535);
		RGBA_Data[i + 3] = static_cast<GLushort>(-65535 * capturedDepth[d] + 65535);
		c += 3;
		d++;
	}

	// Actual saving as tif code

	delete[] capturedColor;
	delete[] capturedDepth;
	delete[] RGBA_Data;
}
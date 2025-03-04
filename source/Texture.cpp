#include "stdafx.h"
#include <filesystem>

//this is included here and not in stdafx.h (pre-compiled header) in order to respect how SBT_IMAGE is doing code generation (for implementation)
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"

#include "Texture.h"

Texture::Texture()
{
	texture = 0;
}

Texture::~Texture()
{
}

bool Texture::LoadTexture(const char* in_filename, const bool in_generate_mipmaps, const bool in_flip)
{
	int32_t width;
	int32_t height;
	int32_t components;
	stbi_set_flip_vertically_on_load(in_flip);
	uint8_t* imageData = stbi_load(in_filename, &width, &height, &components, STBI_rgb_alpha);
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

	if (in_generate_mipmaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(imageData);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void Texture::Bind(const uint32_t texUnit)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::Unbind(const uint32_t texUnit)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SaveRaw(const int32_t in_width, int32_t const in_height, const int32_t in_file_num, const std::string in_filename, const std::string in_path)
{
	assert(in_width < 65536);
	tgaHeader.width = (uint16_t)in_width;
	assert(in_height < 65536);
	tgaHeader.height = (uint16_t)in_height;
	pgmHeader.size = std::format("{} {}\n", in_width, in_height);
	
	std::string path = in_path;
	if (path.back() == '/' || path.back() == '\\')
		path.pop_back();
	std::filesystem::create_directory(path);
	
	std::string filename;
	if (in_filename.find('\\') != std::string::npos)
		filename = in_filename.substr(in_filename.find_last_of('\\') + 1);
	else if (in_filename.find("/") != std::string::npos)
		filename = in_filename.substr(in_filename.find_last_of('/') + 1);
	else
		filename = in_filename;
	filename.resize(filename.size() - 4);

	std::string RGB_Filename = std::format("{}/{}_rgb{}.tga", path, filename, in_file_num);
	std::string GM_Filename = std::format("{}/{}_hm{}.pgm", path, filename, in_file_num);
	std::cout << "Filename: " + RGB_Filename << std::endl;
	float* capturedColor = new float[in_width * in_height * 3];
	float* capturedDepth = new float[in_width * in_height];
	glReadPixels(0, 0, in_width, in_height, GL_RGB, GL_FLOAT, capturedColor);
	glReadPixels(0, 0, in_width, in_height, GL_DEPTH_COMPONENT, GL_FLOAT, capturedDepth);

	std::ofstream colFile(RGB_Filename, std::ios::binary);
	if (colFile.is_open())
	{
		colFile.write((char*)&tgaHeader, sizeof(Raw_RGB_TGA_Header));
		for (
			int32_t i = 0;
			i < in_width * in_height * 3;
			i += 3
			)
		{
			GLubyte tempR = static_cast<GLubyte>(255 * capturedColor[i + 2]);
			GLubyte tempG = static_cast<GLubyte>(255 * capturedColor[i + 1]);
			GLubyte tempB = static_cast<GLubyte>(255 * capturedColor[i]);
			colFile.write(reinterpret_cast<char*>(&tempR), 1);
			colFile.write(reinterpret_cast<char*>(&tempG), 1);
			colFile.write(reinterpret_cast<char*>(&tempB), 1);
		}
		colFile.close();
	}
	else
		std::cout << "Failed to save RGB-data." << std::endl;

	float maxHeight = 1.f;
	for (int i = 0; i < (in_width * in_height); i++)
	{
		// REMEMBER: depth values are 0-1 float, 0 closest to cam (i.e. highest).
		if (maxHeight > capturedDepth[i])
			maxHeight = capturedDepth[i];
	}

	float contrastRatio = 1 / (1 - maxHeight);

	std::ofstream heightFile(GM_Filename);
	if (heightFile.is_open())
	{
		heightFile << pgmHeader.type << pgmHeader.size << pgmHeader.maxValue;
		for (
			int32_t i = (in_width * in_height) - in_width;
			i > -1;
			i -= in_width
			)
		{
			for (
				int32_t j = 0; 
				j < in_width; 
				++j
				)
			{
				uint16_t tempValue = uint16_t((-65535 * capturedDepth[j + i]) + 65535) * contrastRatio;
				if (tempValue > 65535)
					tempValue = 65535;
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

//void Texture::SaveTiff(const int32_t width, const int32_t height, const int32_t fileNum)
//{
//	std::string tif_Filename = std::format("logs/Texture{}.tif", fileNum);
//	GLfloat* capturedColor = new GLfloat[width * height * 3];
//	GLfloat* capturedDepth = new GLfloat[width * height];
//	glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, capturedColor);
//	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, capturedDepth);
//	GLushort* RGBA_Data = new GLushort[width * height * 4];
//
//	for (int32_t i = 0, c = 0, d = 0; i < width * height * 4; i += 4)
//	{
//		RGBA_Data[i] = static_cast<GLushort>(-65535 * capturedColor[c] + 65535);
//		RGBA_Data[i + 1] = static_cast<GLushort>(-65535 * capturedColor[c + 1] + 65535);
//		RGBA_Data[i + 2] = static_cast<GLushort>(-65535 * capturedColor[c + 2] + 65535);
//		RGBA_Data[i + 3] = static_cast<GLushort>(-65535 * capturedDepth[d] + 65535);
//		c += 3;
//		d++;
//	}
//
//	// Actual saving as tif code
//
//	delete[] capturedColor;
//	delete[] capturedDepth;
//	delete[] RGBA_Data;
//}
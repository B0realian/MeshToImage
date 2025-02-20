#pragma once
#define GLEW_STATIC
#include "glew.h"
#include <string>

struct Raw_RGB_TGA_Header
{
	GLubyte nil01 = 0;
	GLubyte nil02 = 0;
	GLubyte compression = 2;
	GLubyte nil04 = 0;
	GLubyte nil05 = 0;
	GLubyte nil06= 0;
	GLubyte nil07 = 0;
	GLubyte nil08 = 0;
	GLubyte nil09 = 0;
	GLubyte nil10 = 0;
	GLubyte nil11 = 0;
	GLubyte nil12 = 0;
	GLushort width = 0;
	GLushort height = 0;
	GLubyte bits = 24;
	GLubyte nil18 = 0;
};

struct Raw_16bit_PGM_Header
{
	std::string type = "P2\n";
	std::string size;
	std::string maxValue = "65535\n";
};

class Texture
{
public:
	Texture();
	virtual ~Texture();
	bool LoadTexture(const std::string& filename, bool generateMipMaps = true);
	void Bind(GLuint texUnit = 0);
	void Unbind(GLuint texUnit = 0);
	void SaveRaw(int width, int height, int fileNum);
	void SaveTiff(int width, int height, int fileNum);

private:
	GLuint texture;
	Raw_RGB_TGA_Header tgaHeader;
	Raw_16bit_PGM_Header pgmHeader;
};
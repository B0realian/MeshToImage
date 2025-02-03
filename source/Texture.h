#pragma once
#define GLEW_STATIC
#include "glew.h"
#include <string>

struct UncompTGAHeader
{
	GLubyte one = 0;
	GLubyte two = 0;
	GLubyte compression = 2;
	GLubyte four = 0;
	GLubyte five = 0;
	GLubyte six = 0;
	GLubyte seven = 0;
	GLubyte eight = 0;
	GLubyte nine = 0;
	GLubyte ten = 0;
	GLubyte eleven = 0;
	GLubyte twelve = 0;
	GLushort width = 0;
	GLushort height = 0;
	GLubyte bits = 24;
	GLubyte eighteen = 0;
};

class Texture
{
public:
	Texture();
	virtual ~Texture();
	bool LoadTexture(const std::string& filename, bool generateMipMaps = true);
	void Bind(GLuint texUnit = 0);
	void Unbind(GLuint texUnit = 0);
	void SaveTGA(int width, int height, int fileNum);
	

private:
	GLuint texture;
	UncompTGAHeader tgaHeader; // [18] {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0};
	GLushort gsh;
};
#pragma once
#define GLEW_STATIC
#include "glew.h"
#include <string>


class Texture
{
public:
	Texture();
	virtual ~Texture();
	bool LoadTexture(const std::string& filename, bool generateMipMaps = true);
	void Bind(GLuint texUnit = 0);
	void Unbind(GLuint texUnit = 0);


private:
	GLuint texture;

};
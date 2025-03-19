#include "stdafx.h"

#include "UIText.h"
#include "Enums.h"
#include "VertexN.h"
#include "Texture.h"


UIText::UIText(const uint32_t in_width, const uint32_t in_height)
{
	screenWidth = in_width;
	screenHeight = in_height;
	CreateBuffers();
}

UIText::~UIText()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void UIText::CreateBuffers()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexText), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexText), (GLvoid*)(3 * sizeof(GLfloat)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexText), (GLvoid*)(5 * sizeof(GLfloat)));

	glBindVertexArray(0);
}

void UIText::WriteLine(const std::string &text, const std::map<char, BMuv> &textmap)
{
	WriteLine(text, textmap, ETextColour::None);
}

void UIText::WriteLine(const std::string &text, const std::map<char, BMuv> &textmap, const ETextColour in_colour)
{
	glm::vec3 colour;
	switch (in_colour)
	{
	case ETextColour::None:
		colour = glm::vec3{ 1.f, 1.f, 1.f };
		break;
	case ETextColour::RED:
		colour = glm::vec3{ 1.f, 0.f, 0.f };
		break;
	case ETextColour::GREEN:
		colour = glm::vec3{ 0.f, 1.f, 0.f };
		break;
	case ETextColour::BLUE:
		colour = glm::vec3{ 0.f, 0.f, 1.f };
		break;
	case ETextColour::YELLOW:
		colour = glm::vec3{ 0.8f, 0.8f, 0.1f };
		break;
	default:
		colour = glm::vec3{ 1.f, 1.f, 1.f };
		break;
	}

	vertices.clear();
	vertices.resize(text.size() * 4);
	const float quadWidth = 20.f / screenWidth;
	const float quadHeight = 36.f / screenHeight;
	const float zValue = 0.01f;

	int pos_itr = 0;
	int v_itr = 0;
	for (char c : text)
	{
		if (static_cast<int>(c) < 32 || static_cast<int>(c) > 126)
			c = '#';

		const float lposx = pos_itr * quadWidth;
		const float rposx = (pos_itr + 1) * quadWidth;
		const float tposy = quadHeight;
		const float bposy = 0.f;
		const BMuv charUV = textmap.at(c);
		VertexText* v = vertices.data() + v_itr;

		v->position = { lposx, tposy, zValue };
		v->texCoords = charUV.topLeftUV;
		v->colour = colour;
		v++;

		v->position = { rposx, tposy, zValue };
		v->texCoords = charUV.topRightUV;
		v->colour = colour;
		v++;

		v->position = { rposx, bposy, zValue };
		v->texCoords = charUV.bottomRightUV;
		v->colour = colour;
		v++;

		v->position = { lposx, bposy, zValue };
		v->texCoords = charUV.bottomLeftUV;
		v->colour = colour;
		v++;
			
		pos_itr++;
		v_itr += 4;
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexText), &vertices[0], GL_STATIC_DRAW);
	glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(vertices.size()));
	glBindVertexArray(0);
	
}



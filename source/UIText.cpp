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

void UIText::WriteLine(const std::string text, const std::map<char, BMuv> &textmap)
{
	WriteLine(text, textmap, ETextColour::None);
}

void UIText::WriteLine(const std::string text, const std::map<char, BMuv> &textmap, const ETextColour in_colour)
{
	if (text.size() < 90)
	{
		glm::vec3 colour;
		switch (in_colour)
		{
		case ETextColour::None:
			colour = glm::vec3(1.f, 1.f, 1.f);
			break;
		case ETextColour::RED:
			colour = glm::vec3(1.f, 0.f, 0.f);
			break;
		case ETextColour::GREEN:
			colour = glm::vec3(0.f, 1.f, 0.f);
			break;
		case ETextColour::BLUE:
			colour = glm::vec3(0.f, 0.f, 1.f);
			break;
		case ETextColour::YELLOW:
			colour = glm::vec3(0.8f, 0.8f, 0.1f);
			break;
		default:
			colour = glm::vec3(1.f, 1.f, 1.f);
			break;
		}

		vertices.clear();
		vertices.resize(text.size() * 4);
		float quadWidth = 20.f / screenWidth;
		float quadHeight = 36.f / screenHeight;
		float zValue = 0.01f;

		int it = 0;
		int i = 0;
		for (char c : text)
		{
			if (static_cast<int>(c) < 32 || static_cast<int>(c) > 126)
				c = '#';

			float lposx = it * quadWidth;
			float rposx = (it + 1) * quadWidth;
			float tposy = quadHeight;
			float bposy = 0.f;
			glm::vec3 tlpos(lposx, tposy, zValue);
			glm::vec3 trpos(rposx, tposy, zValue);
			glm::vec3 brpos(rposx, bposy, zValue);
			glm::vec3 blpos(lposx, bposy, zValue);
			BMuv charUV = textmap.at(c);
			glm::vec2 tluv(charUV.topLeftU, charUV.topLeftV);
			glm::vec2 truv(charUV.topRightU, charUV.topRightV);
			glm::vec2 bruv(charUV.bottomRightU, charUV.bottomRightV);
			glm::vec2 bluv(charUV.bottomLeftU, charUV.bottomLeftV);

			VertexText vertice1{ tlpos, tluv, colour };
			vertices[i] = vertice1;
			i++;
			VertexText vertice2{ trpos, truv, colour };
			vertices[i] = vertice2;
			i++;
			VertexText vertice3{ brpos, bruv, colour };
			vertices[i] = vertice3;
			i++;
			VertexText vertice4{ blpos, bluv, colour };
			vertices[i] = vertice4;
			i++;

			it++;
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexText), &vertices[0], GL_STATIC_DRAW);
		glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(vertices.size()));
		glBindVertexArray(0);
	}
	else
		return;
}



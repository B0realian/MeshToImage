#pragma once

enum class ETextColour;
struct VertexText;
struct BMuv;

class UIText
{
public:
	UIText(const uint32_t in_width, const uint32_t in_height);
	~UIText();
	void WriteLine(const std::string text, const std::map<char, BMuv> &textmap);
	void WriteLine(const std::string text, const std::map<char, BMuv> &textmap, const ETextColour in_colour);

private:
	void CreateBuffers();

	std::vector<VertexText> vertices;
	uint32_t vao = 0;
	uint32_t vbo = 0;
	uint32_t screenWidth;
	uint32_t screenHeight;
};
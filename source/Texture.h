#pragma once

//FIXME: could be file-local to the implementation? only used during output?
struct Raw_RGB_TGA_Header
{
	uint8_t nil01 = 0;
	uint8_t nil02 = 0;
	uint8_t compression = 2;
	uint8_t nil04 = 0;
	uint8_t nil05 = 0;
	uint8_t nil06= 0;
	uint8_t nil07 = 0;
	uint8_t nil08 = 0;
	uint8_t nil09 = 0;
	uint8_t nil10 = 0;
	uint8_t nil11 = 0;
	uint8_t nil12 = 0;
	uint16_t width = 0;
	uint16_t height = 0;
	uint8_t bits = 24;
	uint8_t nil18 = 0;
};

//FIXME: could be file-local to the implementation? only used during output?
struct Raw_16bit_PGM_Header
{
	std::string type = "P2\n";
	std::string size;
	std::string maxValue = "65535\n";
};

class Texture
{
public:

	explicit Texture();
	virtual ~Texture();
	bool LoadTexture(const char* in_filename, bool in_generate_mipmaps = true, bool in_flip = true);
	void Bind(const uint32_t texUnit = 0);
	void Unbind(const uint32_t texUnit = 0);
	void SaveRaw(const int32_t in_width, int32_t const in_height, const int32_t in_file_num, const std::string filename, const std::string path);
	//void SaveTiff(const int width, const int height, const int fileNum);		// Wishlisted to be able to save all data in a single file

private:

	explicit Texture(const Texture& in_other) = delete;

	uint32_t texture;

};

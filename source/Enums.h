#pragma once

enum class EMeshType
{
	None,
	GLTF,
	OBJ,
	FBX
};

enum class EShaderType
{
	SHADER,
	PROGRAM,
	None = -1
};

enum class ETextColour
{
	None,		// Equivalent to white
	RED,
	GREEN,
	BLUE,
	YELLOW
};
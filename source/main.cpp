#include "stdafx.h"//pre-compiled headers

#include "../libs/glm/gtc/matrix_transform.hpp"//header-only, so not in stdafx.h? dunno...
#include "../libs/glm/gtc/type_ptr.hpp"//header-only, so not in stdafx.h? dunno...
#include "Enums.h"
#include "VertexN.h"
#include "Mesh.h"
#include "Texture.h"
#include "TextMap.h"
#include "UIText.h"

//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
const char* mainWindowTitle = "Mesh to Image";
const size_t BUFFER_SIZE = 190;
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE

// Requires textbuffer and textmap to be named as such.
#define WRITE_PLAIN(text, row, colour)		{ glUniform1i(UNIFORM_LINE, row);\
											snprintf(textbuffer, BUFFER_SIZE, text);\
											textline.WriteLine(textbuffer, textmap, ETextColour:: colour); }

// Requires textbuffer and textmap to be named as such. Expects a variable as second argument.
#define WRITE_VAR(text, f, row, colour)		{ glUniform1i(UNIFORM_LINE, row);\
											snprintf(textbuffer, BUFFER_SIZE, text, f);\
											textline.WriteLine(textbuffer, textmap, ETextColour:: colour); }

//MUTABLE
//MUTABLE
//MUTABLE
//MUTABLE
static struct state_t
{
	GLFWwindow* mainWindow = NULL;

	Mesh* meshptr;
	std::vector<std::pair<std::string, EMeshType>> meshFiles;
	size_t meshFile = 0;
	size_t meshFileAmount = 0;
	std::string texFile = "";
	std::string filePath = "captures";

	int32_t mainWindowHeight = 1080;
	int32_t mainWindowWidth = 1920;
	int32_t captures = 0;
	uint8_t frames = 0;

	EMeshType meshtype = EMeshType::FBX;
	float meshScale = 0.01f;

	GLuint shaderProgramMesh = 0;
	GLuint shaderProgramText = 1;
	Texture texture;
	Texture bmText;

	glm::vec3 camPosition = glm::vec3(0.f, 0.f, 10.f);
	glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 subjectPosition = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 subjectRotation = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 subjectOffset = glm::vec3(0.f, 0.f, 0.f);
	float textY = 0.05f;

	float mouseSensitivity = 0.01f;
	float orthoZoom = 5.f;
	float orthoFar = 10.f;

	bool bWireFrameMode = false;
	bool bFlipTexture = true;
	bool bOrthographic = false;
	bool bCapturing = false;
} __state;//personal style - anything file local has a __ prefix...
//MUTABLE
//MUTABLE
//MUTABLE
//MUTABLE

static bool __find_mesh_files()
{
	for (std::filesystem::directory_entry const& entry : std::filesystem::recursive_directory_iterator{ std::filesystem::current_path() })
	{
		const std::string entrypath = entry.path().string();
		const std::string entryext = entry.path().extension().string();
		if (entryext == ".gltf")
		{
			__state.meshFiles.push_back({ entrypath, EMeshType::GLTF });
		}
		else if (entryext == ".fbx")
		{
			__state.meshFiles.push_back({ entrypath, EMeshType::FBX });
		}
		else if (entryext == ".obj")
		{
			__state.meshFiles.push_back({ entrypath, EMeshType::OBJ });
		}
	}

	__state.meshFileAmount = __state.meshFiles.size();
	std::cout << "Number of files found: " << __state.meshFileAmount << std::endl;
	return (__state.meshFileAmount > 0);
}

static bool __find_texture(Mesh& in_mesh)
{
	std::cout << "Relative texture path: " << in_mesh.relative_texture_path << std::endl;
	if (in_mesh.relative_texture_path == "")
	{
		std::string meshname = __state.meshFiles[__state.meshFile].first.substr(__state.meshFiles[__state.meshFile].first.find_last_of('\\') + 1,
			__state.meshFiles[__state.meshFile].first.find('.') - 1 - __state.meshFiles[__state.meshFile].first.find_last_of('\\'));
		std::string meshpath = __state.meshFiles[__state.meshFile].first.substr(0, __state.meshFiles[__state.meshFile].first.find_last_of('\\') + 1);

		const std::filesystem::path tex_path{ meshpath };
		for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator{ tex_path })
		{
			const std::string entrypath = entry.path().string();
			if (entrypath.find(meshname) != std::string::npos)
				if (entrypath.find("jpg") != std::string::npos ||
					entrypath.find("jpeg") != std::string::npos ||
					entrypath.find("png") != std::string::npos ||
					entrypath.find("tga") != std::string::npos ||
					entrypath.find("gif") != std::string::npos)
				{
					__state.texFile = entrypath;
					break;
				}
		}
		if (__state.meshFiles[__state.meshFile].second != EMeshType::GLTF)
			__state.bFlipTexture = true;
	}
	else
	{
		if (__state.meshFiles[__state.meshFile].first.find_last_of('/') != std::string::npos)
			__state.texFile = __state.meshFiles[__state.meshFile].first.substr(0, __state.meshFiles[__state.meshFile].first.find_last_of('/') + 1) + in_mesh.relative_texture_path;
		else if (__state.meshFiles[__state.meshFile].first.find_last_of('\\') != std::string::npos)
			__state.texFile = __state.meshFiles[__state.meshFile].first.substr(0, __state.meshFiles[__state.meshFile].first.find_last_of('\\') + 1) + in_mesh.relative_texture_path;
		__state.bFlipTexture = false;
	}

	if (!__state.texture.LoadTexture(__state.texFile.c_str(), true, __state.bFlipTexture))
		return false;

	return true;
}

static void __set_window_title()
{
	std::ostringstream outs;
	outs << std::fixed << mainWindowTitle << "  -  Triangles: " << __state.meshptr->triangles;
	glfwSetWindowTitle(__state.mainWindow, outs.str().c_str());
}

static void __capture()
{
	__state.texture.SaveRaw(__state.mainWindowWidth, __state.mainWindowHeight, __state.captures, __state.meshFiles[__state.meshFile].first, __state.filePath);
	++__state.captures;
	__state.frames = 0;
	__state.bCapturing = false;
}

static void __on_frame_buffer_size(GLFWwindow* in_window, int in_width, int in_height)
{
	in_window;//w4: unreferenced

	__state.mainWindowWidth = in_width;
	__state.mainWindowHeight = in_height;
	__state.textY = 1.f / __state.mainWindowHeight * 60;
	glViewport(0, 0, __state.mainWindowWidth, __state.mainWindowHeight);
}

static void __on_key_down(GLFWwindow* in_window, int in_key, int in_scancode, int in_action, int in_mode)
{
	in_scancode;//w4: unreferenced
	in_mode;//w4: unreferenced

	if (in_key == GLFW_KEY_ESCAPE && in_action == GLFW_PRESS)
		glfwSetWindowShouldClose(in_window, GL_TRUE);
	if (in_key == GLFW_KEY_V && in_action == GLFW_PRESS)
	{
		__state.bWireFrameMode ^= 1;
		if (__state.bWireFrameMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		return;
	}
	if (in_key == GLFW_KEY_ENTER && in_action == GLFW_PRESS)
	{
		__state.bCapturing = true;
		return;
	}
	if (in_key == GLFW_KEY_SPACE && in_action == GLFW_PRESS)
	{
		if (!__state.bOrthographic)
		{
			__state.orthoFar = __state.camPosition.z + 2.f;
			__state.bOrthographic = true;
		}
		else
		{
			__state.bOrthographic = false;
			__state.orthoZoom = 5.f;
			__state.subjectOffset = glm::vec3(0.f, 0.f, 0.f);
		}
		return;
	}
	else if (in_key == GLFW_KEY_DOWN && in_action == GLFW_PRESS && __state.meshFile < (__state.meshFileAmount - 1))
	{
		__state.meshFile++;
		__state.meshptr->LoadMesh(__state.meshFiles[__state.meshFile].first.c_str(), __state.meshFiles[__state.meshFile].second, __state.meshScale);
		__find_texture(*__state.meshptr);
		__set_window_title();
	}
	else if (in_key == GLFW_KEY_UP && in_action == GLFW_PRESS && __state.meshFile > 0)
	{
		__state.meshFile--;
		__state.meshptr->LoadMesh(__state.meshFiles[__state.meshFile].first.c_str(), __state.meshFiles[__state.meshFile].second, __state.meshScale);
		__find_texture(*__state.meshptr);
		__set_window_title();
	}

	else if (
		in_key == GLFW_KEY_E &&
		__state.bOrthographic
		)
		__state.orthoZoom -= 0.1f;
	else if (
		in_key == GLFW_KEY_Q &&
		__state.bOrthographic
		)
		__state.orthoZoom += 0.1f;
	else if (
		in_key == GLFW_KEY_W &&
		__state.bOrthographic
		)
		__state.subjectOffset.y -= 0.05f;
	else if (
		in_key == GLFW_KEY_S &&
		__state.bOrthographic
		)
		__state.subjectOffset.y += 0.05f;
	else if (
		in_key == GLFW_KEY_A &&
		__state.bOrthographic
		)
		__state.subjectOffset.x -= 0.05f;
	else if (
		in_key == GLFW_KEY_D &&
		__state.bOrthographic
		)
		__state.subjectOffset.x += 0.05f;
	else if (
		in_key == GLFW_KEY_Z &&
		__state.bOrthographic
		)
		__state.orthoFar -= 0.05f;
	else if (
		in_key == GLFW_KEY_X &&
		__state.bOrthographic
		)
		__state.orthoFar += 0.05f;
}

static void __on_mouse_move(GLFWwindow* in_window, double in_pos_x, double in_pos_y)
{
	in_window;//w4: unreferenced

	static glm::vec2 lastMousePos = glm::vec2(0.f, 0.f);

	if (glfwGetMouseButton(__state.mainWindow, GLFW_MOUSE_BUTTON_LEFT) == 1)
	{
		__state.subjectRotation.y += (static_cast<float>(in_pos_x) - lastMousePos.x) * __state.mouseSensitivity;
		__state.subjectRotation.x += (static_cast<float>(in_pos_y) - lastMousePos.y) * __state.mouseSensitivity;
	}

	if (glfwGetMouseButton(__state.mainWindow, GLFW_MOUSE_BUTTON_RIGHT) == 1)
	{
		float dX = __state.mouseSensitivity * 5.f * (static_cast<float>(in_pos_x) - lastMousePos.x);
		float dY = __state.mouseSensitivity * 5.f * (static_cast<float>(in_pos_y) - lastMousePos.y);
		__state.camPosition.z += (dX - dY);
		if (__state.camPosition.z < 0.1f)
			__state.camPosition.z = 0.1f;
	}

	lastMousePos.x = static_cast<float>(in_pos_x);
	lastMousePos.y = static_cast<float>(in_pos_y);
}

static bool __init()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	if (!glfwInit())
		return false;

	__state.mainWindow = glfwCreateWindow(__state.mainWindowWidth, __state.mainWindowHeight, mainWindowTitle, NULL, NULL);
	if (!__state.mainWindow)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(__state.mainWindow);
	glfwSetWindowSizeCallback(__state.mainWindow, __on_frame_buffer_size);
	glfwSetKeyCallback(__state.mainWindow, __on_key_down);
	glfwSetCursorPosCallback(__state.mainWindow, __on_mouse_move);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return false;
	}

	glClearColor(0.f, 0.f, 0.05f, 1.f);
	glViewport(0, 0, __state.mainWindowWidth, __state.mainWindowHeight);
	glfwSwapInterval(0);
	glEnable(GL_DEPTH_TEST);
	__state.textY = 1.f / __state.mainWindowHeight * 60;

	return true;
}

static bool __shader_compilation_check(const GLuint in_shader, const EShaderType in_type)
{
	int status = 0;
	switch (in_type)
	{
	case EShaderType::SHADER:
		glGetShaderiv(in_shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			int length = 0;
			glGetShaderiv(in_shader, GL_INFO_LOG_LENGTH, &length);
			std::string errorlog(length, ' ');
			glGetShaderInfoLog(in_shader, length, &length, &errorlog[0]);
			std::cout << "Shader failed to compile. " << errorlog << std::endl;
			return false;
		}
		break;
	case EShaderType::PROGRAM:
		glGetProgramiv(in_shader, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			int length = 0;
			glGetProgramiv(in_shader, GL_INFO_LOG_LENGTH, &length);
			std::string errorlog(length, ' ');
			glGetProgramInfoLog(in_shader, length, &length, &errorlog[0]);
			std::cout << "Shader Program Linker failure. " << errorlog << std::endl;
			return false;
		}
		break;
	default:
		std::cout << "Shader Compilation Check misuse." << std::endl;
		return false;
		break;
	}

	return true;
}

static bool __compile_shaders()
{
	bool bReturnValue = true;
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	
	const char* VERTEX_SHADER_MESH = R"foo(
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.f);
	TexCoord = uv;
};
)foo";
	glShaderSource(vs, 1, &VERTEX_SHADER_MESH, NULL);
	
	glCompileShader(vs);
	if (!__shader_compilation_check(vs, EShaderType::SHADER))
		bReturnValue = false;

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		
	const char* FRAGMENT_SHADER_MESH = R"foo(
#version 330 core

in vec2 TexCoord;
out vec4 frag_color;

uniform sampler2D sampler;

void main()
{
   frag_color = texture(sampler, TexCoord);
};
)foo";
	glShaderSource(fs, 1, &FRAGMENT_SHADER_MESH, NULL);
	
	glCompileShader(fs);
	if (!__shader_compilation_check(fs, EShaderType::SHADER))
		bReturnValue = false;

	__state.shaderProgramMesh = glCreateProgram();
	glAttachShader(__state.shaderProgramMesh, vs);
	glAttachShader(__state.shaderProgramMesh, fs);
	glLinkProgram(__state.shaderProgramMesh);
	if (!__shader_compilation_check(__state.shaderProgramMesh, EShaderType::PROGRAM))
		bReturnValue = false;
	glDetachShader(__state.shaderProgramMesh, vs);
	glDetachShader(__state.shaderProgramMesh, fs);

	glDeleteShader(fs);
	glDeleteShader(vs);

	GLuint vst = glCreateShader(GL_VERTEX_SHADER);
	const char* VERTEX_SHADER_TEXT = R"raw(
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 colour;

out vec2 TexCoord;
out vec3 textColour;

uniform int line;
uniform float textY;

void main()
{
	float ypos = (1 - textY) - (line * (textY * 0.7f));
	gl_Position = vec4(pos, 1.f) + vec4(-0.99f, ypos, 0.f, 0.f);
	TexCoord = uv;
	textColour = colour;
};
)raw";
	glShaderSource(vst, 1, &VERTEX_SHADER_TEXT, NULL);
	glCompileShader(vst);
	if (!__shader_compilation_check(vst, EShaderType::SHADER))
		bReturnValue = false;

	GLuint fst = glCreateShader(GL_FRAGMENT_SHADER);
	const char* FRAGMENT_SHADER_TEXT = R"raw(
#version 330 core

in vec2 TexCoord;
in vec3 textColour;

out vec4 frag_color;

uniform sampler2D sampler;

void main()
{
	vec4 colour = texture(sampler, TexCoord) * vec4(textColour, 1);
	if (colour.a < 0.1f) discard;
	frag_color = colour;
};
)raw";
	glShaderSource(fst, 1, &FRAGMENT_SHADER_TEXT, NULL);
	glCompileShader(fst);
	if (!__shader_compilation_check(fst, EShaderType::SHADER))
		bReturnValue = false;

	__state.shaderProgramText = glCreateProgram();
	glAttachShader(__state.shaderProgramText, vst);
	glAttachShader(__state.shaderProgramText, fst);
	glLinkProgram(__state.shaderProgramText);
	if (!__shader_compilation_check(__state.shaderProgramText, EShaderType::PROGRAM))
		bReturnValue = false;
	glDetachShader(__state.shaderProgramText, vst);
	glDetachShader(__state.shaderProgramText, fst);

	glDeleteShader(fst);
	glDeleteShader(vst);

	return bReturnValue;
}

static void __camera_projection(glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
	model = glm::rotate(model, __state.subjectRotation.y * 1.5f, glm::vec3(0.f, 1.f, 0.f));
	const float xRot = cosf(__state.subjectRotation.y * 1.5f);
	const float zRot = sinf(__state.subjectRotation.y * 1.5f);
	model = glm::rotate(model, __state.subjectRotation.x * 1.5f, glm::vec3(xRot, 0.f, zRot));
	
	view = glm::lookAt(__state.camPosition, __state.subjectPosition + __state.subjectOffset, __state.camUp);
	
	const float ASPECT_RATIO = static_cast<float>(__state.mainWindowHeight) / static_cast<float>(__state.mainWindowWidth);

	if (__state.bOrthographic)
	{
		projection = glm::ortho(
			-__state.orthoZoom,
			__state.orthoZoom,
			-__state.orthoZoom * ASPECT_RATIO,
			__state.orthoZoom * ASPECT_RATIO,
			0.01f,
			__state.orthoFar);
	}
	else
		projection = glm::perspective(glm::radians(45.f), (1.f / ASPECT_RATIO), 0.01f, 100.f);
}
//END FILE LOCAL FUNCTIONS
//END FILE LOCAL FUNCTIONS
//END FILE LOCAL FUNCTIONS
//END FILE LOCAL FUNCTIONS


//ENTRYPOINT
//ENTRYPOINT
//ENTRYPOINT
//ENTRYPOINT
int main()
{
	if (!__init())
		return -1;

	if (!__find_mesh_files())
		return -2;

	Mesh mesh;
	__state.meshptr = &mesh;
	__state.meshptr->LoadMesh(__state.meshFiles[0].first.c_str(), __state.meshFiles[0].second, __state.meshScale);
	if (!__find_texture(*__state.meshptr))
		return -3;
	
	if (!__compile_shaders())
		return -4;

	std::map<char, BMuv> textmap;
	if (!__state.bmText.LoadText(CascadiaWidth(), CascadiaHeight(), CascadiaData()))
		std::cout << "Failed to load text." << std::endl;
	else
		GetCascadiaMap(textmap);
	DeleteTextData();
	UIText textline(__state.mainWindowWidth, __state.mainWindowHeight);
	char textbuffer[BUFFER_SIZE];

	__set_window_title();

	//cache uniform locations locally
	const GLint UNIFORM_MODEL = glGetUniformLocation(__state.shaderProgramMesh, "model");
	const GLint UNIFORM_VIEW = glGetUniformLocation(__state.shaderProgramMesh, "view");
	const GLint UNIFORM_PROJECTION = glGetUniformLocation(__state.shaderProgramMesh, "projection");
	const GLint UNIFORM_LINE = glGetUniformLocation(__state.shaderProgramText, "line");
	const GLint UNIFORM_TEXTY = glGetUniformLocation(__state.shaderProgramText, "textY");

	while (!glfwWindowShouldClose(__state.mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.f);
		glm::mat4 view(1.f);
		glm::mat4 projection(1.f);
		__camera_projection(model, view, projection);

		__state.texture.Bind();
		glUseProgram(__state.shaderProgramMesh);
		glUniformMatrix4fv(UNIFORM_MODEL, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(UNIFORM_VIEW, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(UNIFORM_PROJECTION, 1, GL_FALSE, glm::value_ptr(projection));
		__state.meshptr->DrawTriangles();
		__state.texture.Unbind();

		if (!__state.bCapturing)
		{
			__state.bmText.Bind();
			glUseProgram(__state.shaderProgramText);
			glUniform1f(UNIFORM_TEXTY, __state.textY);
			if (__state.bOrthographic)
				WRITE_VAR("Far render: %f", __state.orthoFar, 0, YELLOW)
			else
				WRITE_PLAIN("Far render: 100", 0, GREEN)
			WRITE_VAR("Mesh Z-position: %f", __state.camPosition.z, 1, YELLOW)
			uint16_t n = 0;
			for (uint16_t i = 0; i < 25; i++)
			{
				if (__state.meshFile > 20)
					n = __state.meshFile - 20;
				else
					n = 0;

				if ((i + n) == __state.meshFile)
					WRITE_VAR("%s", __state.meshFiles[i + n].first.c_str(), i + 3, YELLOW)
				else
					WRITE_VAR("%s", __state.meshFiles[i + n].first.c_str(), i + 3, BLUE)

				if (i + n == __state.meshFileAmount - 1)
					break;
			}
			__state.bmText.Unbind();
		}
		else
		{
			if (__state.frames == 3)
				__capture();
			__state.frames++;
		}

		glfwSwapBuffers(__state.mainWindow);
		glfwPollEvents();
	}

	glDeleteProgram(__state.shaderProgramMesh);
	glDeleteProgram(__state.shaderProgramText);
	glfwTerminate();
	return 0;
}
 
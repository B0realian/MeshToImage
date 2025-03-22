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
// DEBUGGING/TESTING STRINGS:
const char* testObj = "test/test.obj";
const char* testFbx = "test/test.fbx";
const char* testGltf = "test/test_split.gltf";
const char* testGltfEmb = "test/test_embedded.gltf";
const char* testTexture = "textures/test.jpg";
const char* devMFile = "megascans\\Sandstone_Rock.fbx";
const char* devTFile = "megascans\\Textures\\T_wftnffyva_1K_B.jpg";
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE

// Poorly implemented as it relies on textbuffer and textmap to be named as such. Macros are fun and idiotic at the same time...
#define WRITE(text, f, row, colour)			glUniform1i(UNIFORM_LINE, row);\
											snprintf(textbuffer, BUFFER_SIZE, text, f);\
											textline.WriteLine(textbuffer, textmap, ETextColour:: colour);

//MUTABLE
//MUTABLE
//MUTABLE
//MUTABLE
static struct state_t
{
	GLFWwindow* mainWindow = NULL;

	std::string meshFile = "";
	std::string texFile = "";
	std::string filePath = "captures";

	int32_t mainWindowHeight = 1080;
	int32_t mainWindowWidth = 1920;
	int32_t captures = 0;

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

	//double previousTime = 0;
} __state;//personal style - anything file local has a __ prefix...
//MUTABLE
//MUTABLE
//MUTABLE
//MUTABLE

//BEGIN FILE LOCAL FUNCTIONS
//BEGIN FILE LOCAL FUNCTIONS
//BEGIN FILE LOCAL FUNCTIONS
//BEGIN FILE LOCAL FUNCTIONS
//note that we can get away without using std::string / allocated copies
//conceptually this is also cleaner as we are reading the immutable inputs to the program
static bool __main_arguments(int in_argc, char* in_argv[])
{
	if (in_argc < 2)
	{
		std::cout << "User Instructions:\n";
		std::cout << "\n";
		std::cout << "tldr: -m mesh.gltf       (loads mesh file).\n";
		std::cout << "      -t texture.jpg     (loads texture).\n";
		std::cout << "      -s 1               (sets scale to 1).\n";
		std::cout << "      -f                 (flips texture).\n";
		std::cout << "      -p save/path       (changes save dir from default).\n";
		std::cout << "To load a mesh, you must specify both a mesh-file and a texture-file (case sensitive) in the following manner:\n";
		std::cout << "meshtoimage -m pathto/mesh.file -t pathto/texture.file\n";
		std::cout << ".obj and .gltf works with portable version, full version adds .fbx.\n";
		std::cout << "The program will, by default, scale meshes by 0.01 (i.e. 1:100) since I noticed that a lot of meshes are too big for an OpenGL renderer.\n";
		std::cout << "To change scale, add -s followed by desired scale, i.e. -s 1, or -s 0.1.\n";
		std::cout << "Depending on mesh, the texture may need flipping. If you know you have the right texture but it looks broken, add -f.\n";
		std::cout << "\n";
		std::cout << "In program navigation:\n";
		std::cout << "\n";
		std::cout << "Hold left mouse button and move mouse: rotate mesh.\n";
		std::cout << "Hold right mouse button and move mouse: zoom mesh.\n";
		std::cout << "V to toggle wireframe mode.\n";
		std::cout << "Spacebar to toggle betweem orthographic and perspective view. (Program starts in perspective view).\n";
		std::cout << "WASD to pan camera in orthographic mode.\n";
		std::cout << "Q/E for orthographic zoom in/out. (Mouse zoom only works in perspective view).\n";
		std::cout << "Z/X to limit/extend depth of field in orthographic mode. (Decreases/increases far render limit).\n";
		std::cout << "Return to take a snapshot. Image filenames will increment while program is running.\n";
		std::cout << "Please note: for best result, make sure far render limit is close to the mesh.\n";
		return false;
	}

	bool bMesh = false;
	bool bTexture = false;

	int i = 1;
	while (i < in_argc)
	{
		if (
			0 == ::strcmp(in_argv[i], "-m") &&
			in_argc > i
			)
		{
			__state.meshFile = in_argv[i + 1];
			if (__state.meshFile.find(".gltf") != std::string::npos)
				__state.meshtype = EMeshType::GLTF;
			else if (__state.meshFile.find(".obj") != std::string::npos)
				__state.meshtype = EMeshType::OBJ;
			else if (__state.meshFile.find(".fbx") != std::string::npos)
				__state.meshtype = EMeshType::FBX;
			else
			{
				std::cout << "Unsupported mesh filetype.\n";
				return false;
			}
			bMesh = true;
			i++;
		}
		else if (
			0 == ::strcmp(in_argv[i], "-t") &&
			in_argc > i
			)
		{
			__state.texFile = in_argv[i + 1];
			if (
				__state.texFile.find(".jpg") != std::string::npos ||
				__state.texFile.find(".jpeg") != std::string::npos ||
				__state.texFile.find(".png") != std::string::npos ||
				__state.texFile.find(".tga") != std::string::npos ||
				__state.texFile.find(".gif") != std::string::npos
				)
				bTexture = true;
			else
			{
				std::cout << "Unsupported texture filetype.\n";
				return false;
			}
			i++;
		}
		else if (
			0 == ::strcmp(in_argv[i], "-s") &&
			in_argc > i
			)
		{
			if (!(__state.meshScale = (float)::atof(in_argv[i + 1])))
			{
				std::cout << "Failed to convert scale argument to float.\n";
				return false;
			}
			i++;
		}
		else if (0 == ::strcmp(in_argv[i], "-p") &&
			in_argc > i)
		{
			__state.filePath = in_argv[i + 1];
			i++;
		}
		else if (0 == ::strcmp(in_argv[i], "-f"))
			__state.bFlipTexture = false;

		//	TESTING AND DEBUGGING COMMANDS: WILL NOT WORK OUTSIDE DEVELOPER ENVIRONMENT STRUCTURE
		else if (0 == ::strcmp(in_argv[i], "-to"))
		{
			__state.meshFile = testObj;
			__state.texFile = testTexture;
			__state.meshScale = 1.f;
			__state.meshtype = EMeshType::OBJ;
			return true;
		}
		else if (0 == ::strcmp(in_argv[i], "-tf"))
		{
			__state.meshFile = testFbx;
			__state.texFile = testTexture;
			__state.meshScale = 1.f;
			__state.meshtype = EMeshType::FBX;
			return true;
		}
		else if (0 == ::strcmp(in_argv[i], "-tg"))
		{
			__state.meshFile = testGltf;
			__state.texFile = testTexture;
			__state.meshScale = 1.f;
			__state.meshtype = EMeshType::GLTF;
			return true;
		}
		else if (0 == ::strcmp(in_argv[i], "-tge"))
		{
			__state.meshFile = testGltfEmb;
			__state.texFile = testTexture;
			__state.meshScale = 1.f;
			__state.meshtype = EMeshType::GLTF;
			return true;
		}

		i++;
	}

	return (bMesh && bTexture);
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
		__state.texture.SaveRaw(__state.mainWindowWidth, __state.mainWindowHeight, __state.captures, __state.meshFile, __state.filePath);
		++__state.captures;
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

static bool __find_texture(Mesh &in_mesh)
{
	if (__state.meshtype == EMeshType::GLTF)
	{
		if (__state.meshFile.find_last_of('/') != std::string::npos)
			__state.texFile = __state.meshFile.substr(0, __state.meshFile.find_last_of('/') + 1) + in_mesh.relative_texture_path;
		else if (__state.meshFile.find_last_of('\\') != std::string::npos)
			__state.texFile = __state.meshFile.substr(0, __state.meshFile.find_last_of('\\') + 1) + in_mesh.relative_texture_path;
		__state.bFlipTexture = false;
		std::cout << "Texture path: " << __state.texFile << std::endl;
	}
	else
	{
		std::string meshname = __state.meshFile.substr(__state.meshFile.find_last_of('\\') + 1, __state.meshFile.find('.') - 1 - __state.meshFile.find_last_of('\\'));
		std::string meshpath = std::filesystem::current_path().string() + '\\' + __state.meshFile.substr(0, __state.meshFile.find_last_of('\\') + 1);

		const std::filesystem::path tex_path{ meshpath };
		for (std::filesystem::directory_entry const &entry : std::filesystem::directory_iterator{ tex_path })
		{
			std::string entrypath = entry.path().string();
			if (entrypath.find(meshname) != std::string::npos)
				if (entrypath.find("jpg") != std::string::npos ||
					entrypath.find("jpeg") != std::string::npos ||
					entrypath.find("png") != std::string::npos ||
					entrypath.find("tga") != std::string::npos ||
					entrypath.find("gif") != std::string::npos)
				{
					__state.texFile = entrypath;
					std::cout << "Texture path: " << __state.texFile << std::endl;
					break;
				}
		}
	}

	if (!__state.texture.LoadTexture(__state.texFile.c_str(), true, __state.bFlipTexture))
		return false;

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
	/*double currentTime = glfwGetTime();
	static double deltaTime = 0;
	deltaTime += (currentTime - __state.previousTime);*/
	
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
		
		/*if (deltaTime >= 1.0)
		{
			std::cout << "Camera Z: " << __state.camPosition.z << " Far render: " << __state.orthoFar << std::endl;
			deltaTime = 0;
		}*/
	}
	else
		projection = glm::perspective(glm::radians(45.f), (1.f / ASPECT_RATIO), 0.01f, 100.f);

	//__state.previousTime = currentTime;
}
//END FILE LOCAL FUNCTIONS
//END FILE LOCAL FUNCTIONS
//END FILE LOCAL FUNCTIONS
//END FILE LOCAL FUNCTIONS


//ENTRYPOINT
//ENTRYPOINT
//ENTRYPOINT
//ENTRYPOINT
int main(int in_argc, char* in_argv[])
{
	if (!__main_arguments(in_argc, in_argv))
		return 0;

	//__state.meshFile = devMFile;
	////__state.meshtype = EMeshType::GLTF;
	//__state.meshScale = 0.01f;
	////__state.texFile = devTFile;
	//__state.bFlipTexture = true;

	if (!__init())
		return -1;

	Mesh mesh;
	mesh.LoadMesh(__state.meshFile.c_str(), __state.meshtype, __state.meshScale);
	if (!__find_texture(mesh))
		return -3;

	std::map<char, BMuv> textmap;
	if (!__state.bmText.LoadTexture("textures/bmtxt-cascadia.png", false, false))
		std::cout << "Failed to load text." << std::endl;
	else
		GetMap(textmap);

	UIText textline(__state.mainWindowWidth, __state.mainWindowHeight);
	
	std::ostringstream outs;
	outs << std::fixed << mainWindowTitle << "  -  Triangles: " << mesh.triangles;
	glfwSetWindowTitle(__state.mainWindow, outs.str().c_str());
	
	if (!__compile_shaders())
		return -4;

	//cache uniform locations locally
	const GLint UNIFORM_MODEL = glGetUniformLocation(__state.shaderProgramMesh, "model");
	const GLint UNIFORM_VIEW = glGetUniformLocation(__state.shaderProgramMesh, "view");
	const GLint UNIFORM_PROJECTION = glGetUniformLocation(__state.shaderProgramMesh, "projection");
	const GLint UNIFORM_LINE = glGetUniformLocation(__state.shaderProgramText, "line");
	const GLint UNIFORM_TEXTY = glGetUniformLocation(__state.shaderProgramText, "textY");

	//__state.previousTime = glfwGetTime();

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
		mesh.DrawTriangles();
		__state.texture.Unbind();

		__state.bmText.Bind();
		char textbuffer[BUFFER_SIZE];
		glUseProgram(__state.shaderProgramText);
		glUniform1f(UNIFORM_TEXTY, __state.textY);
		if (__state.bOrthographic)
			{WRITE("Far render: %f", __state.orthoFar, 0, YELLOW);	}
		else
			{WRITE("Far render: 100", NULL, 0, GREEN);	}
		WRITE("Mesh Z-position: %f", __state.camPosition.z, 1, YELLOW); 
		__state.bmText.Unbind();

		glfwSwapBuffers(__state.mainWindow);
		glfwPollEvents();
	}

	glDeleteProgram(__state.shaderProgramMesh);
	glDeleteProgram(__state.shaderProgramText);
	glfwTerminate();
	return 0;
}
 
#include "stdafx.h"//pre-compiled headers

#include "../libs/glm/gtc/matrix_transform.hpp"//header-only, so not in stdafx.h? dunno...
#include "../libs/glm/gtc/type_ptr.hpp"//header-only, so not in stdafx.h? dunno...
#include "../libs/glm/gtc/quaternion.hpp"
#include "Enums.h"
#include "Mesh.h"
#include "Texture.h"

//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
enum
{
	SHADER,
	PROGRAM,
};
const char* mainWindowTitle = "Mesh to Image";

// DEBUGGING/TESTING STRINGS:
const char* testObj = "test/test.obj";
const char* testFbx = "test/test.fbx";
const char* testGltf = "test/test_split.gltf";
const char* testGltfEmb = "test/test_embedded.gltf";
const char* testTexture = "textures/test.jpg";
const char* devMFile = "megascans/Mossy_Rock_alt.gltf";
const char* devTFile = "megascans/Mossy_Rock.jpg";
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE
//IMMUTABLE

//MUTABLE
//MUTABLE
//MUTABLE
//MUTABLE
//generally we don't want globaly accessible mutable state, but the GLFW callbacks complicate things...
//at least this is an attempt to make clear what and where all the mutable state in the program is (and it's file local as well)
static struct state_t
{
	GLFWwindow* mainWindow = NULL;

	std::string meshFile = "";
	std::string texFile = "";

	int32_t mainWindowHeight = 1080;
	int32_t mainWindowWidth = 1920;
	int32_t captures = 0;

	EMeshType meshtype = EMeshType::FBX;
	float meshScale = 0.01f;

	GLuint shaderProgram = 0;
	Texture texture;

	glm::vec3 camPosition = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);
	float camYaw = 0.f;
	float camPitch = 0.f;
	float camRadius = 10.f;

	glm::vec3 subjectPos = glm::vec3(0.f, 0.f, 10.f);
	glm::vec3 subjectOffset = glm::vec3(0.f, 0.f, 0.f);

	float mouseSensitivity = 0.01f;
	float orthoZoom = 5.f;
	float orthoFar = 10.f;

	bool bWireFrameMode = false;
	bool bFlipTexture = true;
	bool bOrthographic = false;
} __state;//personal style - anything file local has a __ prefix...
//MUTABLE
//MUTABLE
//MUTABLE
//MUTABLE

//BEGIN FILE LOCAL FUNCTIONS
//BEGIN FILE LOCAL FUNCTIONS
//BEGIN FILE LOCAL FUNCTIONS
//BEGIN FILE LOCAL FUNCTIONS

//this style is both to indicate that they are "private implementation" of this module and also to ensure that they can't be called from other modules)
//also putting them before main and ordered so that they are defined before being called - this avoids the need for forwarding function declarations

//note that we can get away without using std::string / allocated copies
//conceptually this is also cleaner as we are reading the immutable inputs to the program
static bool __main_arguments(int in_argc, char* in_argv[])
{
	if (in_argc < 2)
	{
		std::cout << "User Instructions:\n";
		std::cout << "\n";
		std::cout << "tldr: -m mesh.fbx        (loads mesh file).\n";
		std::cout << "      -t texture.jpg     (loads texture).\n";
		std::cout << "      -s 1               (sets scale to 1).\n";
		std::cout << "      -f                 (flips texture).\n";
		std::cout << "To load a mesh, you must specify both a mesh-file and a texture-file (case sensitive) in the following manner:\n";
		std::cout << "meshtoimage -m pathto/mesh.file -t pathto/texture.file\n";
		std::cout << "The program will, by default, scale meshes by 0.01 (i.e. 1:100) since I noticed that a lot of meshes are too big for an OpenGL renderer.\n";
		std::cout << "To change scale, add -s followed by desired scale, i.e. -s 1, or -s 0.1.\n";
		std::cout << "Depending on mesh, the texture may need flipping. If you know you have the right texture but it looks broken, add -f.\n";
		std::cout << "\n";
		std::cout << "In program navigation:\n";
		std::cout << "\n";
		std::cout << "Hold left mouse button and move mouse: rotate mesh. (There is currently a lot of gimbal-lock issues. Still a work in progress).\n";
		std::cout << "Hold right mouse button and move mouse: zoom mesh.\n";
		std::cout << "V to toggle wireframe mode.\n";
		std::cout << "Spacebar to toggle betweem orthographic and perspective view. (Program starts in perspective view).\n";
		std::cout << "WASD to pan camera in orthographic mode.\n";
		std::cout << "Q/E for orthographic zoom in/out. (Mouse zoom only works in perspective view).\n";
		std::cout << "Z/X to limit/extend depth of field in orthographic mode. (Decreases/increases far render limit).\n";
		std::cout << "Return to take a snapshot. Image filenames will increment while program is running.\n";
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
		__state.texture.SaveRaw(__state.mainWindowWidth, __state.mainWindowHeight, __state.captures);
		++__state.captures;
		return;
	}
	if (in_key == GLFW_KEY_SPACE && in_action == GLFW_PRESS)
	{
		if (!__state.bOrthographic)
			__state.bOrthographic = true;
		else
		{
			__state.bOrthographic = false;
			__state.orthoZoom = 5.f;
			__state.orthoFar = 10.f;
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
		__state.camYaw -= (static_cast<float>(in_pos_x) - lastMousePos.x) * __state.mouseSensitivity;
		__state.camPitch += (static_cast<float>(in_pos_y) - lastMousePos.y) * __state.mouseSensitivity;
	}

	if (glfwGetMouseButton(__state.mainWindow, GLFW_MOUSE_BUTTON_RIGHT) == 1)
	{
		float dX = 0.01f * (static_cast<float>(in_pos_x) - lastMousePos.x);
		float dY = 0.01f * (static_cast<float>(in_pos_y) - lastMousePos.y);
		__state.camRadius += dX - dY;
	}

	lastMousePos.x = (float)in_pos_x;
	lastMousePos.y = (float)in_pos_y;
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

	return true;
}

static void __shader_compiliation_check(const GLuint in_shader, const int in_type)
{
	int status = 0;
	switch (in_type)
	{
	case SHADER:
		glGetShaderiv(in_shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			int length = 0;
			glGetShaderiv(in_shader, GL_INFO_LOG_LENGTH, &length);
			std::string errorlog(length, ' ');
			glGetShaderInfoLog(in_shader, length, &length, &errorlog[0]);
			std::cout << "Shader failed to compile. " << errorlog << std::endl;
		}
		break;
	case PROGRAM:
		glGetProgramiv(in_shader, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			int length = 0;
			glGetProgramiv(in_shader, GL_INFO_LOG_LENGTH, &length);
			std::string errorlog(length, ' ');
			glGetProgramInfoLog(in_shader, length, &length, &errorlog[0]);
			std::cout << "Shader Program Linker failure. " << errorlog << std::endl;
		}
		break;
	default:
		std::cout << "Shader Compilation Check misuse." << std::endl;
		break;
	}
}

static void __compile_shaders()
{
#if 0
#if 0
	std::string vertShader = ShaderToString(in_vs_name);
	std::string fragShader = ShaderToString(in_fs_name);
	const char* vsSourcePtr = vertShader.c_str();
	const char* fsSourcePtr = fragShader.c_str();
#else
	uint8_t* vs_bytes = __file_contents(in_vs_name);
	assert(vs_bytes);
	const char* vsSourcePtr = (char*)vs_bytes;

	uint8_t* fs_bytes = __file_contents(in_fs_name);
	assert(fs_bytes);
	const char* fsSourcePtr = (char*)fs_bytes;
#endif
#endif

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	{
		//with gl it's happily trivial to inline shader source in the exe
		//also yay c++20 string literals!
		const char* VERTEX_PROGRAM = R"foo(
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
		glShaderSource(vs, 1, &VERTEX_PROGRAM, NULL);
	}
	glCompileShader(vs);
	__shader_compiliation_check(vs, SHADER);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	{
		//with gl it's happily trivial to inline shader source in the exe
		//also yay c++20 string literals!
		const char* FRAGMENT_PROGRAM = R"foo(
#version 330 core

in vec2 TexCoord;
out vec4 frag_color;

uniform sampler2D sampler;

void main()
{
   frag_color = texture(sampler, TexCoord);
};
)foo";
		glShaderSource(fs, 1, &FRAGMENT_PROGRAM, NULL);
	}
	glCompileShader(fs);
	__shader_compiliation_check(fs, SHADER);

	__state.shaderProgram = glCreateProgram();
	glAttachShader(__state.shaderProgram, vs);
	glAttachShader(__state.shaderProgram, fs);
	glLinkProgram(__state.shaderProgram);
	__shader_compiliation_check(__state.shaderProgram, PROGRAM);
	glDetachShader(__state.shaderProgram, vs);
	glDetachShader(__state.shaderProgram, fs);

	glDeleteShader(fs);
	glDeleteShader(vs);

	//delete[] fs_bytes;
	//delete[] vs_bytes;
}

static void __move_camera(const float rYaw, float rPitch)
{
	rPitch = glm::clamp(rPitch, -1.57f, 1.57f);		// 1.57: just shy of half Pi, i.e. slightly less than 90 in degrees

	__state.camPosition.x = __state.subjectPos.x + __state.camRadius * cosf(rPitch) * sinf(rYaw);
	__state.camPosition.y = __state.subjectPos.y + __state.camRadius * sinf(rPitch);
	__state.camPosition.z = __state.subjectPos.z + __state.camRadius * cosf(rPitch) * cosf(rYaw);
}

static void __camera_projection(glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
	// Model = Translation * Rotation * Scale
	model = glm::translate(model, __state.subjectPos);
	view = glm::lookAt(__state.camPosition, __state.subjectPos + __state.subjectOffset, __state.camUp);

	const float ASPECT_RATIO = (float)__state.mainWindowHeight / (float)__state.mainWindowWidth;

	if (__state.bOrthographic)
		projection = glm::ortho(
			-__state.orthoZoom,
			__state.orthoZoom,
			-__state.orthoZoom * ASPECT_RATIO,
			__state.orthoZoom * ASPECT_RATIO,
			0.01f,
			__state.orthoFar
		);
	else
		projection = glm::perspective(glm::radians(45.f), (1.f / ASPECT_RATIO), 0.1f, 100.f);
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

	/*__state.meshFile = devMFile;
	__state.meshtype = EMeshType::GLTF;
	__state.meshScale = 0.01f;
	__state.texFile = devTFile;
	__state.bFlipTexture = false;*/

	if (!__init())
		return -1;

	Mesh mesh;
	mesh.LoadMesh(__state.meshFile.c_str(), __state.meshtype, __state.meshScale);
	__state.texture.LoadTexture(__state.texFile.c_str(), true, __state.bFlipTexture);

	//set title (inlining a function that only had a single callsite and isn't too big to be annoying)
	{
		std::ostringstream outs;
		outs << std::fixed << mainWindowTitle << "  -  Triangles: " << mesh.triangles;
		glfwSetWindowTitle(__state.mainWindow, outs.str().c_str());
	}

	//FIXME: abort on failure?
	__compile_shaders();

	//cache uniform locations locally
	const GLint UNIFORM_MODEL = glGetUniformLocation(__state.shaderProgram, "model");
	const GLint UNIFORM_VIEW = glGetUniformLocation(__state.shaderProgram, "view");
	const GLint UNIFORM_PROJECTION = glGetUniformLocation(__state.shaderProgram, "projection");

	while (!glfwWindowShouldClose(__state.mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		__move_camera(__state.camYaw, __state.camPitch);

		glm::mat4 model(1.f);
		glm::mat4 view(1.f);
		glm::mat4 projection(1.f);
		__camera_projection(model, view, projection);

		__state.texture.Bind();

		glUseProgram(__state.shaderProgram);
#if 0
		SetUniform("model", model);
		SetUniform("view", view);
		SetUniform("projection", projection);
#else
		glUniformMatrix4fv(UNIFORM_MODEL, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(UNIFORM_VIEW, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(UNIFORM_PROJECTION, 1, GL_FALSE, glm::value_ptr(projection));
#endif

		mesh.DrawTriangles();

		glfwSwapBuffers(__state.mainWindow);
		glfwPollEvents();
	}

	glDeleteProgram(__state.shaderProgram);
	glfwTerminate();
	return 0;
}




#if 0
//this was just to see if there was something wrong with the loading of the shader contents (I'm unfamiliar with string stream stuff)
//FIXME: asserts are NOT error handling... :P
static uint8_t* __file_contents(const char* in_file)
{
	assert(in_file);
	::FILE* handle = nullptr;
	if (0 == ::fopen_s(&handle, in_file, "rb"))
	{
		assert(handle);

		{
			const int32_t SEEK_RESULT = ::fseek(handle, 0, SEEK_END);
			assert(0 == SEEK_RESULT);
		}

		const int32_t FILE_SIZE = ::ftell(handle);
		assert(0 < FILE_SIZE);

		{
			const int32_t SEEK_RESULT = ::fseek(handle, 0, SEEK_SET);
			assert(0 == SEEK_RESULT);
		}

		uint8_t* result = new uint8_t[FILE_SIZE + 1];//we want to null terminate!
		assert(result);
		const size_t READ_RESULT = ::fread(result, FILE_SIZE, 1, handle);
		assert(1 == READ_RESULT);

		//null terminate!
		result[FILE_SIZE] = 0;

		::fclose(handle);

		return result;
	}

	assert(0);
	return nullptr;
}
#endif



#if 0
std::string ShaderToString(const std::string& filename)
{
	std::stringstream ss;
	std::ifstream file;

	try
	{
		file.open(filename, std::ios::in);
		if (!file.fail())
			ss << file.rdbuf();
		file.close();
	}
	catch (std::exception err)
	{
		std::cerr << "Error reading shader filename." << std::endl;
	}

	return ss.str();
}
#endif


#if 0
void SetUniform(const char* name, float& variable)
{
	int element = -1;
	for (const auto pair : uniformRegisterLocation)
	{
		if (pair.first == name)
			element = pair.second;
	}
	if (element < 0)
	{
		element = glGetUniformLocation(__state.shaderProgram, name);
		uniformRegisterLocation[name] = element;
	}

	glUniform1f(element, variable);
}

void SetUniform(const char* name, glm::mat4& matrix)
{
	int element = -1;
	for (const auto pair : uniformRegisterLocation)
	{
		if (pair.first == name)
			element = pair.second;
	}
	if (element < 0)
	{
		element = glGetUniformLocation(__state.shaderProgram, name);
		uniformRegisterLocation[name] = element;
	}

	glUniformMatrix4fv(element, 1, GL_FALSE, glm::value_ptr(matrix));
}
#endif






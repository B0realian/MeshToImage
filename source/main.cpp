#define GLEW_STATIC
#include "glew.h"
#include <GLFW/glfw3.h>
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include "Enums.h"
#include "Mesh.h"
#include "Texture.h"


GLFWwindow* mainWindow = NULL;
const std::string mainWindowTitle = "Mesh to Image";
int mainWindowHeight = 1080;
int mainWindowWidth = 1920;
float aspectRatio = (float)mainWindowHeight / (float)mainWindowWidth;
glm::vec3 backgroundColor = glm::vec3(0.f, 0.f, 0.05f);
int captures = 0;

GLuint shaderProgram = 0;
const int SHADER = 0;
const int PROGRAM = 1;
std::map<std::string, int> uniformRegisterLocation;
bool bWireFrameMode = false;
std::string vShaderName = "shaders/texOrbCam.vert";
std::string fShaderName = "shaders/texture.frag";

EMeshType meshtype = EMeshType::FBX;
float meshScale = 0.01f;

Texture texture;
std::string meshFile = "";
std::string texFile = "";
bool bFlipTexture = true;

std::string testObj = "objects/test/test.obj";
std::string testFbx = "objects/test/test.fbx";
std::string testGltf = "objects/test/test_split.gltf";
std::string testGltfEmb = "objects/test/test_embedded.gltf";
std::string testTexture = "textures/test.jpg";

std::string devMFile = "objects/Mossy_Rock_alt.gltf";
std::string devTFile = "textures/Mossy_Rock.jpg";
GLuint vbo, vao, ibo;

glm::vec3 camPosition = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 subjectPos = glm::vec3(0.f, 0.f, 10.f);
glm::vec3 subjectOffset = glm::vec3(0.f, 0.f, 0.f);
float camYaw = 0.f;
float camPitch = 0.f;
float camRadius = 10.f;
float yawR;
float pitchR;
float fov = 45.f;
float orthoZoom = 5.f;
float orthoFar = 10.f;
bool bOrthographic = false;

bool Init();
void SetTitle(int triangles);
void CompileShaders(const std::string vsName, const std::string fsName);
std::string ShaderToString(const std::string& filename);
void ShaderCompilationCheck(unsigned int shader, int type);
void SetUniform(const char* name, float& variable);
void SetUniform(const char* name, glm::mat4& matrix);
void MoveCamera(float dYaw, float dPitch);
void CameraProjection(glm::mat4& model, glm::mat4& view, glm::mat4& projection);
void OnFrameBufferSize(GLFWwindow* window, int width, int height);
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode);
void OnMouseMove(GLFWwindow* window, double posX, double posY);
bool MainArguments(int args, std::vector<std::string> arg);


int main(int argc, char* argv[])
{
	std::vector<std::string> a;
	for (int i = 0; i < argc; i++)
		a.push_back(argv[i]);

	if (!MainArguments(argc, a))
		return 0;
	
	/*meshFile = devMFile;
	texFile = devTFile;
	meshtype = EMeshType::GLTF;*/

	if (!Init())
		return -1;
	
	Mesh mesh(meshScale);
	mesh.LoadMesh(meshFile, meshtype);
	texture.LoadTexture(texFile, true, bFlipTexture);
	SetTitle(mesh.triangles);

	CompileShaders(vShaderName, fShaderName);

	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 model(1.f), view(1.f), projection(1.f);
		MoveCamera(camYaw, camPitch);
		CameraProjection(model, view, projection);

		texture.Bind();

		glUseProgram(shaderProgram);
		SetUniform("model", model);
		SetUniform("view", view);
		SetUniform("projection", projection);

		mesh.DrawTriangles();

		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram);
	glfwTerminate();
	return 0;
}

bool Init()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	if (!glfwInit())
		return false;

	mainWindow = glfwCreateWindow(mainWindowWidth, mainWindowHeight, mainWindowTitle.c_str(), NULL, NULL);
	if (!mainWindow)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(mainWindow);
	glfwSetWindowSizeCallback(mainWindow, OnFrameBufferSize);
	glfwSetKeyCallback(mainWindow, OnKeyDown);
	glfwSetCursorPosCallback(mainWindow, OnMouseMove);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return false;
	}

	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.f);
	glViewport(0, 0, mainWindowWidth, mainWindowHeight);
	glfwSwapInterval(0);
	glEnable(GL_DEPTH_TEST);

	return true;
}

void SetTitle(int triangles)
{
	std::ostringstream outs;
	outs << std::fixed << mainWindowTitle << "  -  Triangles: " << triangles;
	glfwSetWindowTitle(mainWindow, outs.str().c_str());
}

void CompileShaders(const std::string vsName, const std::string fsName)
{
	std::string vertShader = ShaderToString(vsName);
	std::string fragShader = ShaderToString(fsName);
	const char* vsSourcePtr = vertShader.c_str();
	const char* fsSourcePtr = fragShader.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vs, 1, &vsSourcePtr, NULL);
	glShaderSource(fs, 1, &fsSourcePtr, NULL);
	glCompileShader(vs);
	ShaderCompilationCheck(vs, SHADER);
	glCompileShader(fs);
	ShaderCompilationCheck(fs, SHADER);
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);
	glLinkProgram(shaderProgram);
	ShaderCompilationCheck(shaderProgram, PROGRAM);
	glDeleteShader(vs);
	glDeleteShader(fs);
}

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

void ShaderCompilationCheck(unsigned int shader, int type)
{
	int status = 0;
	switch (type)
	{
		case SHADER:
			glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
			if (status == GL_FALSE)
			{
				int length = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
				std::string errorlog(length, ' ');
				glGetShaderInfoLog(shader, length, &length, &errorlog[0]);
				std::cout << "Shader failed to compile. " << errorlog << std::endl;
			}
			break;
		case PROGRAM:
			glGetProgramiv(shader, GL_LINK_STATUS, &status);
			if (status == GL_FALSE)
			{
				int length = 0;
				glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
				std::string errorlog(length, ' ');
				glGetProgramInfoLog(shader, length, &length, &errorlog[0]);
				std::cout << "Shader Program Linker failure. " << errorlog << std::endl;
			}
			break;
		default:
			std::cout << "Shader Compilation Check misuse." << std::endl;
			break;
	}
}

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
		element = glGetUniformLocation(shaderProgram, name);
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
		element = glGetUniformLocation(shaderProgram, name);
		uniformRegisterLocation[name] = element;
	}

	glUniformMatrix4fv(element, 1, GL_FALSE, glm::value_ptr(matrix));
}

void MoveCamera(float dYaw, float dPitch)
{
	dPitch = glm::clamp(dPitch, -90.f, 90.f);
	yawR = glm::radians(dYaw);
	pitchR = glm::radians(dPitch);

	camPosition.x = subjectPos.x + camRadius * cosf(pitchR) * sinf(yawR);
	camPosition.y = subjectPos.y + camRadius * sinf(pitchR);
	camPosition.z = subjectPos.z + camRadius * cosf(pitchR) * cosf(yawR);
}

void CameraProjection(glm::mat4& model, glm::mat4 &view, glm::mat4& projection)
{
	model = glm::translate(model, subjectPos);
	view = glm::lookAt(camPosition, subjectPos + subjectOffset, camUp);

	if (bOrthographic)
		projection = glm::ortho(-orthoZoom, orthoZoom, -orthoZoom * aspectRatio, orthoZoom * aspectRatio, 0.01f, orthoFar);
	else
		projection = glm::perspective(glm::radians(45.f), (1.f / aspectRatio), 0.1f, 100.f);
}

void OnFrameBufferSize(GLFWwindow* window, int width, int height)
{
	mainWindowWidth = width;
	mainWindowHeight = height;
	aspectRatio = static_cast<float>(mainWindowHeight) / static_cast<float>(mainWindowWidth);
	glViewport(0, 0, mainWindowWidth, mainWindowHeight);
}

void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		bWireFrameMode = !bWireFrameMode;
		if (bWireFrameMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		return;
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		texture.SaveRaw(mainWindowWidth, mainWindowHeight, captures);
		captures++;
		return;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		if (!bOrthographic)
			bOrthographic = true;
		else
		{
			bOrthographic = false;
			orthoZoom = 5.f;
			orthoFar = 10.f;
			subjectOffset = glm::vec3(0.f, 0.f, 0.f);
		}
		return;
	}

	else if (key == GLFW_KEY_E && bOrthographic)
		orthoZoom -= 0.25f;
	else if (key == GLFW_KEY_Q && bOrthographic)
		orthoZoom += 0.25f;
	else if (key == GLFW_KEY_W && bOrthographic)
		subjectOffset.y -= 0.1f;
	else if (key == GLFW_KEY_S && bOrthographic)
		subjectOffset.y += 0.1f;
	else if (key == GLFW_KEY_A && bOrthographic)
		subjectOffset.x += 0.1f;
	else if (key == GLFW_KEY_D && bOrthographic)
		subjectOffset.x -= 0.1f;
	else if (key == GLFW_KEY_Z && bOrthographic)
		orthoFar -= 0.05f;
	else if (key == GLFW_KEY_X && bOrthographic)
		orthoFar += 0.05f;
}

void OnMouseMove(GLFWwindow* window, double posX, double posY)
{
	static glm::vec2 lastMousePos = glm::vec2(0.f, 0.f);

	if (glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_LEFT) == 1)
	{
		camYaw -= ((float)posX - lastMousePos.x) * 0.25f;
		camPitch += ((float)posY - lastMousePos.y) * 0.25f;
	}

	if (glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_RIGHT) == 1)
	{
		float dX = 0.01f * ((float)posX - lastMousePos.x);
		float dY = 0.01f * ((float)posY - lastMousePos.y);
		camRadius += dX - dY;
	}

	lastMousePos.x = (float)posX;
	lastMousePos.y = (float)posY;
}

bool MainArguments(int args, std::vector<std::string> arg)
{
	if (args < 2)
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
	while (i < args)
	{
		if (arg[i] == "-m" && args > i)
		{
			meshFile = static_cast<std::string>(arg[i + 1]);
			if (meshFile.find(".gltf") != std::string::npos)
				meshtype = EMeshType::GLTF;
			else if (meshFile.find(".obj") != std::string::npos)
				meshtype = EMeshType::OBJ;
			else if (meshFile.find(".fbx") != std::string::npos)
				meshtype = EMeshType::FBX;
			else
			{
				std::cout << "Unsupported mesh filetype.\n";
				return false;
			}
			bMesh = true;
			i++;
		}
		else if (arg[i] == "-t" && args > i)
		{
			texFile = static_cast<std::string>(arg[i + 1]);
			if (texFile.find(".jpg") != std::string::npos ||
				texFile.find(".jpeg") != std::string::npos ||
				texFile.find(".png") != std::string::npos ||
				texFile.find(".tga") != std::string::npos ||
				texFile.find(".gif") != std::string::npos)
				bTexture = true;
			else
			{
				std::cout << "Unsupported texture filetype.\n";
				return false;
			}
			i++;
		}
		else if (arg[i] == "-s" && args > i)
		{
			if (!(meshScale = std::stof(arg[i + 1])))
			{
				std::cout << "Failed to convert scale argument to float.\n";
				return false;
			}
			i++;
		}
		else if (arg[i] == "-f")
			bFlipTexture = false;

		//	TESTING AND DEBUGGING COMMANDS: WILL NOT WORK OUTSIDE DEVELOPER ENVIRONMENT STRUCTURE
		else if (arg[i] == "-to")
		{
			meshFile = testObj;
			texFile = testTexture;
			meshScale = 1.f;
			meshtype = EMeshType::OBJ;
			return true;
		}
		else if (arg[i] == "-tf")
		{
			meshFile = testFbx;
			texFile = testTexture;
			meshScale = 1.f;
			meshtype = EMeshType::FBX;
			return true;
		}
		else if (arg[i] == "-tg")
		{
			meshFile = testGltf;
			texFile = testTexture;
			meshScale = 1.f;
			meshtype = EMeshType::GLTF;
			return true;
		}
		else if (arg[i] == "-tge")
		{
			meshFile = testGltfEmb;
			texFile = testTexture;
			meshScale = 1.f;
			meshtype = EMeshType::GLTF;
			return true;
		}

		i++;
	}

	return (bMesh && bTexture);
}
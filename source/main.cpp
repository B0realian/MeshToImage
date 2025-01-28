#define GLEW_STATIC
#include "glew.h"
#include <GLFW/glfw3.h>
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include "MeshObject.h"
#include "Texture.h"


GLFWwindow* mainWindow = NULL;
const std::string mainWindowTitle = "Mesh to Image";
int mainWindowHeight = 1080;
int mainWindowWidth = 1920;
glm::vec3 backgroundColor = glm::vec3(0.f, 0.f, 0.05f);

GLuint shaderProgram = 0;
const int SHADER = 0;
const int PROGRAM = 1;
std::map<std::string, int> uniformRegisterLocation;
bool bWireFrameMode = false;
std::string vShaderName = "shaders/texOrbCam.vert";
std::string fShaderName = "shaders/texture.frag";

MeshObject mesh;
Texture texture;
std::string objFile = "objects/mega.obj";
std::string testObj = "objects/testObject.obj";
std::string texFile = "textures/mega.jpg";
GLuint vbo, vao, ibo;

float camYaw = 0.f;
float camPitch = 0.f;
float camRadius = 10.f;
float yawR;
float pitchR;
float fov = 45.f;
glm::vec3 camPosition = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 subjectPos = glm::vec3(0.f, 0.f, 20.f);

bool Init();
void CompileShaders(const std::string vsName, const std::string fsName);
std::string ShaderToString(const std::string& filename);
void ShaderCompilationCheck(unsigned int shader, int type);
void SetUniform(const char* name, float& variable);
void SetUniform(const char* name, glm::mat4& matrix);
void MoveCamera(float dYaw, float dPitch);
void OnFrameBufferSize(GLFWwindow* window, int width, int height);
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode);
void OnMouseMove(GLFWwindow* window, double posX, double posY);
bool MainArguments(int args, std::vector<std::string> arg);


int main()
{
	// MainArguments handling

	if (!Init())
		return -1;

	mesh.LoadObj(objFile);
	texture.LoadTexture(texFile, true);

	CompileShaders(vShaderName, fShaderName);

	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 model(1.f), view(1.f), projection(1.f);
		MoveCamera(camYaw, camPitch);
		model = glm::translate(model, subjectPos);
		view = glm::lookAt(camPosition, subjectPos, camUp);
		projection = glm::perspective(glm::radians(45.f), (float)mainWindowWidth / (float)mainWindowHeight, 0.1f, 100.f);

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

void OnFrameBufferSize(GLFWwindow* window, int width, int height)
{
	mainWindowWidth = width;
	mainWindowHeight = height;
	glViewport(0, 0, mainWindowWidth, mainWindowHeight);
}

void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		bWireFrameMode = !bWireFrameMode;
		if (bWireFrameMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	/*if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
		invHS += 2.f;
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
		invHS -= 2.f;*/
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
	if (args == 1)
	{
		std::cout << "User Manual:\n";
		return false;
	}

	int i = 1;

	while (i < args)
	{
		

		i++;
	}

	return true;
}
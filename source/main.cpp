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

MeshObject mesh;
Texture texture;
std::string objFile = "objects/mega.obj";
std::string testObj = "objects/testObject.obj";
std::string texFile = "textures/mega.jpg";
GLuint vbo, vao, ibo;

glm::vec3 camPosition = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 subjectPos = glm::vec3(0.f, 0.f, 10.f);
glm::vec3 subjectOffset = glm::vec3(0.f, 0.f, 0.f);
glm::vec2 drawOffset = glm::vec2(0.f, 0.f);
float camYaw = 0.f;
float camPitch = 0.f;
float camRadius = 10.f;
float yawR;
float pitchR;
float fov = 45.f;
float orthoZoom = 5.f;
bool bOrthographic = false;

bool Init();
void SetTitle();
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


int main()
{
	// MainArguments handling
	
	if (!Init())
		return -1;

	mesh.LoadObj(objFile);
	texture.LoadTexture(texFile, true);
	SetTitle();

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

void SetTitle()
{
	std::ostringstream outs;
	outs << std::fixed << mainWindowTitle << "  -  Triangles: " << mesh.triangles;
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
		projection = glm::ortho(-orthoZoom, orthoZoom, -orthoZoom * aspectRatio, orthoZoom * aspectRatio, 0.01f, 20.f);
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
		texture.SaveCaptures(mainWindowWidth, mainWindowHeight, captures);
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
			subjectOffset = glm::vec3(0.f, 0.f, 0.f);
		}
		return;
	}

	if (key == GLFW_KEY_E)
		orthoZoom -= 0.25f;
	if (key == GLFW_KEY_Q)
		orthoZoom += 0.25f;
	if (key == GLFW_KEY_W)
		subjectOffset.y -= 0.1f;
	if (key == GLFW_KEY_S)
		subjectOffset.y += 0.1f;
	if (key == GLFW_KEY_A)
		subjectOffset.x += 0.1f;
	if (key == GLFW_KEY_D)
		subjectOffset.x -= 0.1f;
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
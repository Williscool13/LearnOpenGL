#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

std::string readShaderFile(const char* filePath) 
{
	std::string shaderCode;
	std::ifstream fileStream(filePath, std::ios::in);

	if (fileStream.is_open()) {
		std::stringstream sstr;
		sstr << fileStream.rdbuf();
		shaderCode = sstr.str();
		fileStream.close();
	}
	else {
		std::cerr << "Could not open file: " << filePath << std::endl;
	}

	return shaderCode;
}

void checkShaderCompileStatus(GLuint shader)
{
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Shader compiled successfully." << std::endl;
	}
}

void checkShaderLinkStatus(GLuint shaderProgram)
{
	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Shader linked successfully." << std::endl;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{ 
	glViewport(0, 0, width, height); 
}

/// <summary>
/// Process Inputs of the user. 
/// If the user presses the escape key, the window should close.
/// </summary>
/// <param name="window"></param>
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
		glfwSetWindowShouldClose(window, true); 
}

/// <summary>
/// Renders the scene.
/// Animates the background color of the window.
/// </summary>
void renderScene(GLuint program) {

	float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	// Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Vertex Buffer Object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// get attribute location
	GLuint pos = glGetAttribLocation(program, "pos");
	// set attribute pointer
	glEnableVertexAttribArray(pos);
	// tell OpenGL how to interpret the vertex data (per vertex attribute), it uses the previously defined VBO for its data
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// draw triangle
	glUseProgram(program); 
	glDrawArrays(GL_TRIANGLES, 0, 3); 
	glUseProgram(0);


	// animate and change background color
	float timeValue = glfwGetTime();
	float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
	float redValue = (cos(timeValue) / 2.0f) + 0.5f;
	float blueValue = (sin(timeValue) / 2.0f) + (cos(timeValue) / 2.0f);
	glClearColor(redValue, greenValue, blueValue, 1.0f);
}

int main()
{
	// initialize GLFW before calling any GLFW functions
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version 3.x
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // OpenGL version x.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window object and check if it is created successfully
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// make the context of the specified window current on the calling thread
	glfwMakeContextCurrent(window);

	// initialize GLAD before calling any OpenGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// viewport
	glViewport(0, 0, 1920, 1080);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// set background color
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);


	// Vertex Shader
	std::string vertexShaderSourceString = readShaderFile("shaders\\helloVertexShader.vert");
	const GLchar* vertexShaderSource = vertexShaderSourceString.c_str();
	//// compile vertex shader
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	//// check if vertex shader compiled successfully
	checkShaderCompileStatus(vertexShader);

	// Fragment Shader
	std::string fragmentShaderSourceString = readShaderFile("shaders\\helloFragmentShader.frag");
	const GLchar* fragmentShaderSource = fragmentShaderSourceString.c_str();
	//// compile fragment shader
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	//// check if fragment shader compiled successfully
	checkShaderCompileStatus(fragmentShader);


	// Shader Program
	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	//// check if shader program linked successfully
	checkShaderLinkStatus(shaderProgram);

	// delete shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// use shader program
	glUseProgram(shaderProgram);



	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);
		
		// rendering commands here
		renderScene(shaderProgram);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// clean up
	glfwTerminate();

	return 0;
}

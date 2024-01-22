#include <iostream>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ext/glext.h>
#include <wingdi.h>



bool first = true;
GLuint VAOs[2];

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		first = !first;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

GLuint setupShader(GLenum shaderType, const char* shaderSource) {
	// compile vertex shader
	GLuint shader;
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);
	// check if vertex shader compiled successfully
	checkShaderCompileStatus(shader);
	return shader;
}

void setupVertexArrayObjectOne(GLuint program) {
	float vertices[] = {
	-0.1,  -0.1f, 0.0f,  // top right
	-0.1f, -0.9f, 0.0f,  // bottom right
	-0.9f, -0.9f, 0.0f,  // bottom left
	-0.9f, -0.1f, 0.0f   // top left 
	};

	float colors[] = {
	 1.0f,  0.0f, 0.0f,  // top right
	 0.0f,  1.0f, 0.0f,  // bottom right
	 0.0f,  0.0f, 1.0f,  // bottom left
	 1.0f,  1.0f, 0.0f   // top left 
	};

	unsigned int indices[] = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	// Bind Vertex Array Object (VAO)
	glBindVertexArray(VAOs[0]);


	GLuint VBOs[2], EBO;
	glGenBuffers(2, VBOs);

	/// VBOs must be bound before glVertexAttribPointer is called
	// Set Up Points Vertex Buffer Object (VBO)
	glGenBuffers(1, &VBOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint pos = glGetAttribLocation(program, "pos");
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(pos);

	// Set Up Colors Vertex Buffer Object (VBO)
	glGenBuffers(1, &VBOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	GLuint color = glGetAttribLocation(program, "color");
	glVertexAttribPointer(color, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(color);


	// Element (Index) Buffer Object (EBO)
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void setupVertexArrayObjectTwo(GLuint program) {
	float verticesTwo[] = {
		-0.5f, 0.0f, 0.0f,  // bottom left
		0.5f,  0.0f, 0.0f,  // bottom right
		0.0f,  0.5f, 0.0f,  // middle top
		0.0f, -0.5f, 0.0f   // middle bottom
	};

	float colorsTwo[] = {
		1.0f,  0.0f, 0.0f,  // top right
		0.0f,  1.0f, 0.0f,  // bottom right
		0.0f,  0.0f, 1.0f,  // bottom left
		1.0f,  1.0f, 0.0f   // top left 
	};
	
	unsigned int indices[] = {  
		0, 1, 2,   // first triangle
		0, 1, 3    // second triangle
	};

	// Bind Vertex Array Object (VAO)
	glBindVertexArray(VAOs[1]);

	GLuint VBOs[2], EBO;

	/// VBOs must be bound before glVertexAttribPointer is called
	// Set Up Points Vertex Buffer Object (VBO)
	glGenBuffers(1, &VBOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTwo), verticesTwo, GL_STATIC_DRAW);

	GLuint pos = glGetAttribLocation(program, "pos");
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(pos);

	// Set Up Colors Vertex Buffer Object (VBO)
	glGenBuffers(1, &VBOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorsTwo), colorsTwo, GL_STATIC_DRAW);

	GLuint color = glGetAttribLocation(program, "color");
	glVertexAttribPointer(color, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(color);


	// Element (Index) Buffer Object (EBO)
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}


void setupProgram(GLuint program) {
	// Vertex Shader
	GLuint vertexShader = setupShader(GL_VERTEX_SHADER, readShaderFile("shaders\\helloVertexShader.vert").c_str());

	// Fragment Shader
	GLuint fragmentShader = setupShader(GL_FRAGMENT_SHADER, readShaderFile("shaders\\helloFragmentShader.frag").c_str());

	// Shader Program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	checkShaderLinkStatus(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	glGenVertexArrays(2, VAOs);
	setupVertexArrayObjectOne(program);
	setupVertexArrayObjectTwo(program);

	// Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
	glBindVertexArray(0);
}

void renderScene(GLuint program) {
	GLuint timeUniform = glGetUniformLocation(program, "time");
	
	// draw in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Choose program and vertex array object
	glUseProgram(program); 
	if (first) {
		glBindVertexArray(VAOs[0]);
	}
	else {
		glBindVertexArray(VAOs[1]);
	}
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUniform1f(timeUniform, glfwGetTime());

	glUseProgram(0);
    glBindVertexArray(0);


	// animate and change background color
	/*float timeValue = glfwGetTime();
	float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
	float redValue = (cos(timeValue) / 2.0f) + 0.5f;
	float blueValue = (sin(timeValue) / 2.0f) + (cos(timeValue) / 2.0f);
	glClearColor(redValue, greenValue, blueValue, 1.0f);*/
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


	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	setupProgram(shaderProgram);
	
	glfwSetKeyCallback(window, key_callback);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);


		// rendering commands here
		renderScene(shaderProgram);

		// poll events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// clean up
	glfwTerminate();

	return 0;
}

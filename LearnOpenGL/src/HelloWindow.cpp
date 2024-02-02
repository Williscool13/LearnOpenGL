#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

// glad/glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ext/glext.h>
#include <wingdi.h>
// cyCodeBase
#include <cyCode/cyCore.h>
#include <cyCode/cyVector.h>
#include <cyCode/cyMatrix.h>
#include <cyCode/cyGL.h>

// camera
#include <camera.h>

// image loader
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{ 
	glViewport(0, 0, width, height); 
}


// settings
const float cameraSpeed = 2.5f;
const float sensitivity = 2.5f;

// Delta time
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera camera;

// Mouse input
bool firstMouse = true;
cy::Vec2<float> lastMousePos = cy::Vec2<float>(400.0f, 300.0f);
cy::Vec2<float> mouseDelta = cy::Vec2<float>(0.0f, 0.0f);
// Keyboard input
bool wInput = false;
bool aInput = false;
bool sInput = false;
bool dInput = false;
// Camera
cy::Vec3<float> cameraPos = cy::Vec3<float>(0.0f, 0.0f, 3.0f);
cy::Vec3<float> cameraFront = cy::Vec3<float>(0.0f, 0.0f, -1.0f);
cy::Vec3<float> cameraUp = cy::Vec3<float>(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;


float vertices[] = {
	// positions          // colors           // texture coords
	 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
};

unsigned int indices[] = {
	0, 1, 2,   // first triangle
	0, 2, 3    // second triangle
};


// Program
GLuint mvpIndex = 99;
GLuint textureIndex1 = 100;
GLuint textureIndex2 = 101;
GLuint timeIndex = 102;
GLuint VAO;
cy::GLSLProgram cyProgram;
cy::GLTexture2D texture1;
cy::GLTexture2D texture2;

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
		//first = !first;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		wInput = true;
	}

	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		wInput = false;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		aInput = true;
	}

	if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		aInput = false;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		sInput = true;
	}

	if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		sInput = false;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		dInput = true;
	}

	if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		dInput = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	//camera.ProcessMouseMovement(xpos, ypos);
	if (firstMouse) {
		lastMousePos = cy::Vec2<float>(xpos, ypos);
		firstMouse = false;
	}
	mouseDelta = cy::Vec2<float>(xpos - lastMousePos.x, lastMousePos.y - ypos);
	lastMousePos = cy::Vec2<float>(xpos, ypos);

	// mouse input
	float _xoffset = mouseDelta.x * sensitivity * deltaTime;
	float _yoffset = mouseDelta.y * sensitivity * deltaTime;

	yaw += _xoffset;
	pitch += _yoffset;

	if (pitch > 89.0f) { pitch = 89.0f; }
	if (pitch < -89.0f) { pitch = -89.0f; }

	cy::Vec3<float> _front;
	_front.x = cos(yaw * cy::Pi<float>() / 180) * cos(pitch * cy::Pi<float>() / 180);
	_front.y = sin(pitch * cy::Pi<float>() / 180);
	_front.z = sin(yaw * cy::Pi<float>() / 180) * cos(pitch * cy::Pi<float>() / 180);
	cameraFront = _front.GetNormalized();

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	fov -= (float)yoffset;

	if (fov <= 1.0f) {
		fov = 1.0f;
	}
	if (fov >= 45.0f) {
		fov = 45.0f;
	}
}


void setupProgram() {
	cy::GLSLShader vertexShader;
	cy::GLSLShader fragmentShader;
	vertexShader.CompileFile("shaders\\helloVertexShader.vert", GL_VERTEX_SHADER);
	fragmentShader.CompileFile("shaders\\helloFragmentShader.frag", GL_FRAGMENT_SHADER);

	// creates program, compiles shaders, and links to program
	if (!cyProgram.Build(&vertexShader, &fragmentShader)) {
		std::cout << "Program failed to build" << std::endl;
	}

	cyProgram.RegisterUniform(timeIndex, "time");
	cyProgram.RegisterUniform(textureIndex1, "texture1");
	cyProgram.RegisterUniform(textureIndex2, "texture2");
	cyProgram.RegisterUniform(mvpIndex, "mvp");


	cyProgram.SetUniform("texture1", 0);
	cyProgram.SetUniform("texture2", 1);


	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	
	GLuint VBOs[3], EBO;
	glGenBuffers(3, VBOs);
	glGenBuffers(1, &EBO);

	// bind vertex position and color
	cyProgram.SetAttribBuffer("pos", VBOs[0], 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),  0 * sizeof(float));
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	cyProgram.SetAttribBuffer("color", VBOs[1], 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 3 * sizeof(float));
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	cyProgram.SetAttribBuffer("texCoord", VBOs[2], 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 6 * sizeof(float));
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
	glBindVertexArray(0);

}

void setupTextures() {
	texture1.Initialize();
	texture1.SetFilteringMode(GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR);
	texture1.SetWrappingMode(GL_REPEAT, GL_REPEAT);

	texture2.Initialize();
	texture2.SetFilteringMode(GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR);
	texture2.SetWrappingMode(GL_REPEAT, GL_REPEAT);

	stbi_set_flip_vertically_on_load(true);
}

cy::Matrix4f setupMVP() {
	cy::Matrix4<float> model = cy::Matrix4f();
	model.SetIdentity();
	//model.SetRotation(cy::Vec3<float>(0, 1, 0), glfwGetTime() * 0.5f);
	//model.SetRotation(cy::Vec3<float>(1.0f, 0.0f, 0.0f), glm::radians(-55.0f));
	model.AddTranslation(cy::Vec3<float>(2.0f, 0.0f, 0.0f));

	cy::Matrix4<float> view = cy::Matrix4f();
	//cy::Vec3<float> cameraPos = cy::Vec3<float>(0.0f, 0.0f, 3.0f);
	view.SetView(cameraPos, cameraPos + cameraFront, cameraUp);

	cy::Matrix4<float> project = cy::Matrix4f();
	project.SetPerspective(fov / 180.0f * cy::Pi<float>(), 800.0f / 600.0f, 0.1f, 100.0f);

	return project * view * model;
}

void parseInputs() {
	if (wInput) { camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime); }
	if (aInput) { camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime); }
	if (sInput) { camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime); }
	if (dInput) { camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime); }
}

void parseDeltatime() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void renderScene() {
	glUseProgram(cyProgram.GetID());

	float timeValue = glfwGetTime();
	cyProgram.SetUniform(timeIndex, timeValue);
	cy::Matrix4<float> mvp = setupMVP();
	cyProgram.SetUniform(mvpIndex, mvp);


	int width, height, nrChannels;
	unsigned char* data = stbi_load("textures/WoodenContainerTexture.jpg", &width, &height, &nrChannels, 0);
	if (!data) {
		std::cout << "Failed to load texture" << std::endl;
	}

	glActiveTexture(GL_TEXTURE0);
	texture1.SetImage(data, nrChannels, width, height);
	texture1.BuildMipmaps();

	unsigned char* data2 = stbi_load("textures/AwesomeFace.png", &width, &height, &nrChannels, 0);
	if (!data2) {
		std::cout << "Failed to load texture" << std::endl;
	}

	glActiveTexture(GL_TEXTURE1);
	texture2.SetImage(data2, nrChannels, width, height);
	texture2.BuildMipmaps();

	stbi_image_free(data);
	stbi_image_free(data2);
	
	//glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	glUseProgram(0);
    glBindVertexArray(0);

}

int main()
{
	// initialize GLFW before calling any GLFW functions
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version 3.x
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // OpenGL version x.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window object and check if it is created successfully
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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
	glViewport(0, 0, 800, 600);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	camera = Camera(cy::Vec3<float>(0,0,0), cy::Vec3<float>(0,1,0));

	glEnable(GL_DEPTH_TEST);

	lastFrame = glfwGetTime();

	setupTextures();
	setupProgram();
	

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		parseDeltatime();
		parseInputs();

		// rendering commands here
		renderScene();

		// poll events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// clean up
	glfwTerminate();

	return 0;
}

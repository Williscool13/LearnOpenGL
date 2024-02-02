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
#include <cyCode/cyTriMesh.h>

// camera
#include <camera.h>

// image loader
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>



// Delta time
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera camera(cy::Vec3<float>(0, 0, 20));

// Mouse Input
bool firstMouse = true;
cy::Vec2<float> lastMousePos;

// Keyboard input
cy::Vec4<bool> wasdInput = cy::Vec4<bool>(false, false, false, false);
cy::Vec2<bool> spaceCInput = cy::Vec2<bool>(false, false);
bool shiftInput = false;



unsigned int indices[] = {
	0, 1, 2,   // first triangle
	0, 2, 3    // second triangle
};

void buildShaders();
void registerUniforms();
void setupBuffers();
cy::Matrix4f setupMVP();

void parseInputs();
void parseDeltatime();

void renderScene();
void RecompileShaders();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);





struct Vertex {
	float x, y, z;
};

// Program
GLuint textureIndex1 = 100;
GLuint textureIndex2 = 101;
GLuint mvpIndex = 102;
GLuint timeIndex = 103;
GLuint colorModIndex = 104;
GLuint VAO;
cy::GLSLProgram cyProgram;
cy::GLTexture2D texture1;
cy::GLTexture2D texture2;
cy::TriMesh currMesh;
Vertex* vertices; 

cy::Vec3<float> colorMod = cy::Vec3<float>(1.0f, 0.0f, 0.0f);

void parseObj(char* objFilePath) {
	cy::TriMesh mesh;
	if (mesh.LoadFromFileObj(objFilePath)) {
		std::cout << "Model loaded successfully" << std::endl;
	}
	else {
		std::cout << "Model failed to load" << std::endl;
	}
	std::cout << "Loaded model with " << mesh.NV() << " Vertices" << std::endl;

	currMesh = mesh;
	vertices = new Vertex[mesh.NV()];
	for (int i = 0; i < mesh.NV(); i++) {
		vertices[i] = Vertex{ mesh.V(i).x, mesh.V(i).y, mesh.V(i).z };
	}
}

void RecompileShaders() {
	std::cout << "-----Recompiling Shaders-----" << std::endl;
	buildShaders();
	registerUniforms();
	std::cout << "Color Changed to green to demonstrate recompilation" << std::endl;
	//cyProgram.SetUniform(colorModIndex, cy::Vec3<float>(0.0f, 1.0f, 0.0f));
	colorMod = cy::Vec3<float>(0.0f, 1.0f, 0.0f);
}

void buildShaders() {
	cy::GLSLShader vertexShader;
	cy::GLSLShader fragmentShader;
	vertexShader.CompileFile("shaders\\helloVertexShader.vert", GL_VERTEX_SHADER);
	fragmentShader.CompileFile("shaders\\helloFragmentShader.frag", GL_FRAGMENT_SHADER);
	if (!cyProgram.Build(&vertexShader, &fragmentShader)) {
		std::cout << "Program failed to build" << std::endl;
	}
}
void registerUniforms() {
	cyProgram.RegisterUniform(timeIndex, "time");
	cyProgram.RegisterUniform(mvpIndex, "mvp");
	cyProgram.RegisterUniform(colorModIndex, "colorMod");

	std::cout << "Uniforms registered" << std::endl;
}


void setupBuffers() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);

	cyProgram.SetAttribBuffer("pos", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glBufferData(GL_ARRAY_BUFFER, currMesh.NV() * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glBindVertexArray(0);
}


float p = -90.0f;
float y = 0.0f;
float d = -10.0f;
bool ortho = false;
cy::Matrix4f setupMVP() {
	cy::Matrix4<float> model = cy::Matrix4f();
	model.SetIdentity();
	if (!currMesh.IsBoundBoxReady()) {
		currMesh.ComputeBoundingBox();
	}
	else {
		cy::Vec3<float> min = currMesh.GetBoundMin();
		cy::Vec3<float> max = currMesh.GetBoundMax();
		cy::Vec3<float> center = (min + max) * 0.5f;
		float radius = cy::Max((max - min).x, (max - min).y, (max - min).z);
		//model.SetScale(1.0f / radius);
		model.AddTranslation(-center);
	}
	//model.SetScale(cy::Vec3<float>(0.5f, 0.5f, 0.5f));
	//model.SetRotation(cy::Vec3<float>(0, 1, 0), glfwGetTime() * 0.5f);
	//model.SetRotation(cy::Vec3<float>(1.0f, 0.0f, 0.0f), glm::radians(-55.0f));
	//model.AddTranslation(cy::Vec3<float>(2.0f, 0.0f, 0.0f));

	//cy::Matrix4<float> view = camera.GetViewMatrix();
	cy::Matrix4<float> view = cy::Matrix4<float>();
	view.SetIdentity();
	view.SetRotationXYZ(y / 180.0f * cy::Pi<float>(), p / 180.0f * cy::Pi<float>(), 0.0f);
	view.AddTranslation(cy::Vec3<float>(0.0f, 0.0f, d));
	

	cy::Matrix4<float> project = cy::Matrix4f();
	if (ortho) {
		project.SetIdentity();

		float r = 1.0f;
		float l = -1.0f;
		float t = 1.0f;
		float b = -1.0f;
		float n = 0.1f;
		float f = 1000.0f;


		cy::Vec3<float> c1 = cy::Vec3<float>(2 / (r - l), 0.0f, 0.0f);
		cy::Vec3<float> c2 = cy::Vec3<float>(0.0f, 2 / (t - b), 0.0f);
		cy::Vec3<float> c3 = cy::Vec3<float>(0.0f, 0.0f, 2 / (f - n));
		cy::Vec3<float> c4 = cy::Vec3<float>((r + l) / (l - r), (t + b) / (b - t), (f + n) / (n - f));
		project = cy::Matrix4f(c1, c2, c3, c4);

		project.SetColumn(2, cy::Vec4<float>(0, 0, 1 / d, 0));
		//project.SetColumn(2, cy::Vec4<float>(0, 0, 1 / d, 0));
		//project.OrthogonalizeZ();

		//project.OrthogonalizeX();
		//project.OrthogonalizeY();
		//project = cy::Matrix4f::Scale(1.0f / d) * project;
		//project.SetScale(cy::Vec3<float>(1.0f / d, 1.0f / d, 1.0f / d));

	}
	else {
		project.SetPerspective(camera.Zoom / 180.0f * cy::Pi<float>(), 800.0f / 600.0f, 0.1f, 1000.0f);
	}

	return project * view * model;
}

void parseInputs() {
	if (wasdInput[0]) { camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime); }
	if (wasdInput[1]) { camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime); }
	if (wasdInput[2]) { camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime); }
	if (wasdInput[3]) { camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime); }
	if (spaceCInput[0]) { camera.ProcessKeyboard(Camera_Movement::UP, deltaTime); }
	if (spaceCInput[1]) { camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime); }

	if (shiftInput) { camera.MovementSpeed = 5.0f; }
	else { camera.MovementSpeed = 2.5f; }


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

	cyProgram.SetUniform(colorModIndex, colorMod);

	//glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, currMesh.NV());


	glUseProgram(0);
    glBindVertexArray(0);

}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
		return 1;  // Exit with an error code
	}

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//glEnable(GL_DEPTH_TEST);

	lastFrame = glfwGetTime();
	char* objFilePath = argv[1];

	parseObj(objFilePath);
	buildShaders();
	registerUniforms();
	setupBuffers();
	

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


bool lmb = false;
bool rmb = false;
const float cameraSpeed = 0.1f;
const float sensitivity = 0.1f;

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
		//first = !first;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		RecompileShaders();
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		ortho = !ortho;
	}


	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		wasdInput[0] = true;
	}

	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		wasdInput[0] = false;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		wasdInput[1] = true;
	}

	if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		wasdInput[1] = false;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		wasdInput[2] = true;
	}

	if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		wasdInput[2] = false;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		wasdInput[3] = true;
	}

	if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		wasdInput[3] = false;
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		spaceCInput[0] = true;
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
		spaceCInput[0] = false;
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		spaceCInput[1] = true;
	}

	if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
		spaceCInput[1] = false;
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		shiftInput = true;
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		shiftInput = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastMousePos = cy::Vec2<float>((float)xpos, (float)ypos);
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastMousePos.x;
	float yoffset = lastMousePos.y - (float)ypos; // reversed

	lastMousePos = cy::Vec2<float>((float)xpos, (float)ypos);

	camera.ProcessMouseMovement(xoffset, yoffset);

	if (lmb) {
		p += xoffset * sensitivity;
		y += yoffset * sensitivity;
	}
	if (rmb) {
		d += yoffset * cameraSpeed;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		lmb = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		lmb = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		rmb = true;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		rmb = false;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}
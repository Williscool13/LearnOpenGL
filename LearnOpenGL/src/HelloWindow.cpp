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



struct Transformation {
	cy::Matrix4<float> model;
	cy::Matrix4<float> view;
	cy::Matrix4<float> projection;
	cy::Matrix4<float> mvp;
	cy::Matrix4<float> mv;
};

struct Vertex {
	float x, y, z;
};

struct Normal {
	float x, y, z;
};

struct Indices {
	unsigned int i1, i2, i3;
};

struct VertexProperty {
	Vertex vertex;
	Normal normal;
};


void buildShaders();
void registerUniforms();
void setupBuffers();
Transformation setupMVP();

void parseInputs();
void parseDeltatime();

void renderScene();
void RecompileShaders();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);





// Program
GLuint mvpIndex = 0;
GLuint mvIndex = 1;
GLuint modelIndex = 2;
GLuint viewIndex = 3;
GLuint projectionIndex = 4;
GLuint mvNormalIndex = 5;
GLuint smoothnessIndex = 6;
GLuint diffuseColorIndex = 7;
GLuint specularColorIndex = 8;
GLuint ambientIlluminanceIndex = 9;
GLuint timeIndex = 11;
GLuint mainLightDirIndex = 12;
GLuint VAO;
cy::GLSLProgram cyProgram;
cy::GLTexture2D texture1;
cy::GLTexture2D texture2;
cy::TriMesh currMesh;

VertexProperty* verticesArray;

// Camera
/// Camera Style
bool freeCam = false;
/// Orbital Cam
float p = -90.0f;
float y = 0.0f;
float d = -40.0f;
// Perspective/Otho
bool ortho = false;

// Main Light
float azimuth = 0.0f;
float inclination = 0.0f;

VertexProperty* constructVerticesArray(cy::TriMesh mesh) {
	Vertex* vertices;
	Indices* _vertexIndices;
	Normal* normals;
	Indices* _normalIndices;

	vertices = new Vertex[currMesh.NV()];
	for (int i = 0; i < currMesh.NV(); i++) {

		vertices[i] = Vertex{ currMesh.V(i).x, currMesh.V(i).y, currMesh.V(i).z };
	}

	_vertexIndices = new Indices[currMesh.NF()];
	for (int i = 0; i < currMesh.NF(); i++) {

		_vertexIndices[i] = Indices{ currMesh.F(i).v[0], currMesh.F(i).v[1], currMesh.F(i).v[2] };
	}

	normals = new Normal[currMesh.NVN()];
	for (int i = 0; i < currMesh.NVN(); i++) {

		normals[i] = Normal{ currMesh.VN(i).x, currMesh.VN(i).y, currMesh.VN(i).z };
	}

	_normalIndices = new Indices[currMesh.NF()];
	for (int i = 0; i < currMesh.NF(); i++) {

		_normalIndices[i] = Indices{ currMesh.FN(i).v[0], currMesh.FN(i).v[1], currMesh.FN(i).v[2] };
	}

	VertexProperty* verticesArray = new VertexProperty[mesh.NF() * 3];

	for (int i = 0; i < mesh.NF(); i++) {
		verticesArray[i * 3].vertex = vertices[_vertexIndices[i].i1];
		verticesArray[i * 3].normal = normals[_normalIndices[i].i1];
		verticesArray[i * 3 + 1].vertex = vertices[_vertexIndices[i].i2];
		verticesArray[i * 3 + 1].normal = normals[_normalIndices[i].i2];
		verticesArray[i * 3 + 2].vertex = vertices[_vertexIndices[i].i3];
		verticesArray[i * 3 + 2].normal = normals[_normalIndices[i].i3];
	}

	return verticesArray;

}

cy::Matrix4<float> generateOrthoProjectionMatrix(float r, float l, float t, float b, float n, float f) {
	cy::Matrix4<float> project = cy::Matrix4f();

	cy::Vec3<float> c1 = cy::Vec3<float>(2 / (r - l), 0.0f, 0.0f);
	cy::Vec3<float> c2 = cy::Vec3<float>(0.0f, 2 / (t - b), 0.0f);
	cy::Vec3<float> c3 = cy::Vec3<float>(0.0f, 0.0f, 2 / (f - n));
	cy::Vec3<float> c4 = cy::Vec3<float>((r + l) / (l - r), (t + b) / (b - t), (f + n) / (n - f));
	project = cy::Matrix4f(c1, c2, c3, c4);

	project.SetColumn(2, cy::Vec4<float>(0, 0, 1 / -20.0f, 0));

	return project;
}

void parseObj(char* objFilePath) {
	if (currMesh.LoadFromFileObj(objFilePath)) {
		std::cout << "Model loaded successfully" << std::endl;
	}
	else {
		std::cout << "Model failed to load" << std::endl;
	}


	verticesArray = constructVerticesArray(currMesh);
}

void RecompileShaders() {
	std::cout << "-----Recompiling Shaders-----" << std::endl;
	buildShaders();
	registerUniforms();
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
	cyProgram.RegisterUniform(mvIndex, "mv");
	cyProgram.RegisterUniform(modelIndex, "model");
	cyProgram.RegisterUniform(viewIndex, "view");
	cyProgram.RegisterUniform(projectionIndex, "projection");

	cyProgram.RegisterUniform(mainLightDirIndex, "mainLightDirection");

	cyProgram.RegisterUniform(ambientIlluminanceIndex, "ambientIlluminance");

	cyProgram.RegisterUniform(diffuseColorIndex, "diffuseColor");
	cyProgram.RegisterUniform(specularColorIndex, "specularColor");
	cyProgram.RegisterUniform(smoothnessIndex, "smoothness");

	cyProgram.RegisterUniform(mvNormalIndex, "mvNormal");

	std::cout << "Uniforms registered" << std::endl;
}


void setupBuffers() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//cyProgram.SetAttribBuffer("pos", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	//glBufferData(GL_ARRAY_BUFFER, currMesh.NV() * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	cyProgram.SetAttribBuffer("pos", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0); 
	glBufferData(GL_ARRAY_BUFFER, currMesh.NF() * 3 * sizeof(VertexProperty), verticesArray, GL_STATIC_DRAW);

	std::cout << "Set up buffer with " << currMesh.NF() * 3 << " vertices" << std::endl;

	cyProgram.SetAttribBuffer("normal", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	glBufferData(GL_ARRAY_BUFFER, currMesh.NF() * 3 * sizeof(VertexProperty), verticesArray, GL_STATIC_DRAW);




	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, currMesh.NF() * sizeof(Indices), _vertexIndices, GL_STATIC_DRAW);


	glBindVertexArray(0);
}

void rotateVertex(cy::Vec3<float>& vertex, cy::Matrix4<float> rotationMatrix) {
	cy::Vec4<float> v = cy::Vec4<float>(vertex.x, vertex.y, vertex.z, 1.0f);
	v = rotationMatrix * v;
	vertex = cy::Vec3<float>(v.x, v.y, v.z);
}

Transformation setupMVP() {
	cy::Matrix4<float> model = cy::Matrix4f();
	model.SetIdentity();
	
	model = cy::Matrix4<float>::RotationY(90.0f * cy::Pi<float>() / 180.0f) * model;
	model = cy::Matrix4<float>::RotationZ(90.0f * cy::Pi<float>() / 180.0f) * model;

	if (!currMesh.IsBoundBoxReady()) {
		currMesh.ComputeBoundingBox();
	}
	else {
		cy::Vec3<float> min = currMesh.GetBoundMin();
		cy::Vec3<float> max = currMesh.GetBoundMax();
		rotateVertex(min, model);
		rotateVertex(max, model);
		cy::Vec3<float> center = (min + max) * 0.5f;
		float radius = cy::Max((max - min).x, (max - min).y, (max - min).z);
		model.AddTranslation(-center);
	}
	

	cy::Matrix4<float> view;
	if (freeCam) {
		view = camera.GetViewMatrix();
	}
	else {
		view = cy::Matrix4<float>();
		view.SetIdentity();
		view.SetRotationXYZ(y / 180.0f * cy::Pi<float>(), p / 180.0f * cy::Pi<float>(), 0.0f);
		view.AddTranslation(cy::Vec3<float>(0.0f, 0.0f, d));
	}
	
	

	cy::Matrix4<float> project = cy::Matrix4f();
	if (ortho) {
		project.SetIdentity();

		float r = 10.0f;
		float l = -10.0f;
		float t = 10.0f;
		float b = -10.0f;
		float n = 0.1f;
		float f = 1000.0f;

		project = generateOrthoProjectionMatrix(r, l, t, b, n, f);
	}
	else {
		project.SetPerspective(camera.Zoom / 180.0f * cy::Pi<float>(), 800.0f / 600.0f, 0.1f, 1000.0f);
	}

	Transformation t;
	t.model = model;
	t.view = view;
	t.projection = project;
	t.mvp = project * view * model;
	t.mv = view * model;

	return t;
}

cy::Vec3<float> lightDirection(float az, float inc) {
	float x = cos(az) * cos(inc);
	float y = sin(inc);
	float z = sin(az) * cos(inc);
	return cy::Vec3<float>(x, y, z);
}

void renderScene() {
	glUseProgram(cyProgram.GetID());

	float timeValue = glfwGetTime();
	cyProgram.SetUniform(timeIndex, timeValue);
	Transformation t = setupMVP();
	cyProgram.SetUniform(modelIndex, t.model);
	cyProgram.SetUniform(viewIndex, t.view);
	cyProgram.SetUniform(projectionIndex, t.projection);
	cyProgram.SetUniform(mvIndex, t.mv);
	cyProgram.SetUniform(mvpIndex, t.mvp);

	cyProgram.SetUniform(mvNormalIndex, t.mv.GetSubMatrix3().GetInverse().GetTranspose());
	cy::Vec3<float> mainLightDir = lightDirection(azimuth, inclination);
	cyProgram.SetUniform(mainLightDirIndex, mainLightDir.GetNormalized());
	cyProgram.SetUniform(ambientIlluminanceIndex, 0.2f);

	cyProgram.SetUniform(diffuseColorIndex, cy::Vec3<float>(1.0f, 0.0f, 0.0f));
	cyProgram.SetUniform(specularColorIndex, cy::Vec3<float>(1.0f, 1.0f, 1.0f));
	cyProgram.SetUniform(smoothnessIndex, 32.0f);


	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_TRIANGLES, currMesh.NF() * 3, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, currMesh.NF() * 3);


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

	glEnable(GL_DEPTH_TEST);

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
bool ctrl = false;
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

	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		freeCam = !freeCam;
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

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		ctrl = true;
	}

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
		ctrl = false;
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

	if (lmb && ctrl) {
		azimuth -= xoffset * sensitivity;
		inclination += yoffset * sensitivity;
	}
	else if (lmb) {
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
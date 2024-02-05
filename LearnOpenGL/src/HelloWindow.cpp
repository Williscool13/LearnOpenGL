#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <functional>
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

// png decoder
#include "lodepng.h"



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

struct Texture {
	float u, v;
};

struct VertexProperty {
	Vertex vertex;
	Normal normal;
	Texture texture;
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
GLuint ambientIlluminanceIndex = 9;
GLuint timeIndex = 11;
GLuint mainLightDirIndex = 12;

GLuint Ka = 13;
GLuint Kd = 14;
GLuint Ks = 15;

GLuint VAO;
GLuint VBO;

cy::GLSLProgram cyProgram;
cy::GLTexture2D texture1;
cy::GLTexture2D texture2;
cy::TriMesh currMesh;

VertexProperty* verticesArray;
int numVertices;

// Texture
GLuint diffuseTextureIndex = 100;
GLuint specularTextureIndex = 101;
GLuint ambientTextureIndex = 102;
cy::GLTexture2D diffuseTexture;
cy::GLTexture2D specularTexture;
cy::GLTexture2D ambientTexture;


cy::GLRenderTexture2D renderBuffer;
int renderTextureChannels = 3;
GLsizei viewportWidth = 800;
GLsizei viewportHeight = 600;
GLsizei renderTextureWidth = 800;
GLsizei renderTextureHeight = 600;


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

//std::map<std::string, cy::TriMesh::Mtl> materialMap;
std::map<int, std::vector<VertexProperty>> materialMap;
std::map<int, std::vector<unsigned char>> diffuseMaterialTextureMap;
std::map<int, std::vector<unsigned char>> specularMaterialTextureMap;
std::map<int, std::vector<unsigned char>> ambientMaterialTextureMap;


VertexProperty* generateVertexProperties(cy::TriMesh mesh) {
	//std::vector<cy::TriMesh::Str> _materialNames = new cy::TriMesh::Str*[mesh.NF()];
	int verticesPerFace = 3;
	numVertices = mesh.NF() * verticesPerFace;

	VertexProperty* verticesArray = new VertexProperty[numVertices];

	//std::map<std::string, std::vector<VertexProperty>> vertexMap;
	GLuint materialIndex = 0;
	for (int i = 0; i < mesh.NF(); i++) {
		for (int j = 0; j < verticesPerFace; j++) {
			int normalIndex = mesh.FN(i).v[j];
			cy::Vec3<float> n = mesh.VN(normalIndex);
			int vertexIndex = mesh.F(i).v[j];
			cy::Vec3<float> v = mesh.V(vertexIndex);
			int textureIndex = mesh.FT(i).v[j];
			cy::Vec3<float> t = mesh.VT(textureIndex);

			VertexProperty p = { Vertex{v.x, v.y, v.z}, Normal{n.x, n.y, n.z}, Texture{t.x, t.y} };
			verticesArray[i * verticesPerFace + j] = p;

			int materialIndex = mesh.GetMaterialIndex(i);
			auto it = materialMap.find(materialIndex);
			if (it != materialMap.end()) {
				materialMap[materialIndex].push_back(p);
			}
			else {
				std::vector<VertexProperty> v;
				v.push_back(p);
				materialMap.insert(std::pair<int, std::vector<VertexProperty>>(materialIndex, v));
			}
		}
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


	verticesArray = generateVertexProperties(currMesh);
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
	glUseProgram(cyProgram.GetID());
	cyProgram.RegisterUniform(timeIndex, "time");
	cyProgram.RegisterUniform(mvpIndex, "mvp");
	cyProgram.RegisterUniform(mvIndex, "mv");
	cyProgram.RegisterUniform(modelIndex, "model");
	cyProgram.RegisterUniform(viewIndex, "view");
	cyProgram.RegisterUniform(projectionIndex, "projection");
	cyProgram.RegisterUniform(mvNormalIndex, "mvNormal");

	cyProgram.RegisterUniform(mainLightDirIndex, "mainLightDirection");
	cyProgram.RegisterUniform(ambientIlluminanceIndex, "ambientIlluminance");



	// Material Properties
	cyProgram.RegisterUniform(diffuseTextureIndex, "fragTexture");
	cyProgram.RegisterUniform(specularTextureIndex, "specularTexture");
	cyProgram.RegisterUniform(ambientTextureIndex, "ambientTexture");
	cyProgram.RegisterUniform(Ka, "Ka");
	cyProgram.RegisterUniform(Kd, "Kd");
	cyProgram.RegisterUniform(Ks, "Ks");
	cyProgram.RegisterUniform(smoothnessIndex, "specularExponent");



	/// Diffuse Texture
	diffuseTexture = cy::GLTexture2D();
	diffuseTexture.Bind(0);
	diffuseTexture.Initialize(); // gen, bind, set filtering and wrapping
	diffuseTexture.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR); // override default filtering mode
	diffuseTexture.SetWrappingMode(GL_REPEAT, GL_REPEAT); // override default wrapping mode
	// Specular Texture
	specularTexture = cy::GLTexture2D();
	specularTexture.Bind(1);
	specularTexture.Initialize();
	specularTexture.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	specularTexture.SetWrappingMode(GL_REPEAT, GL_REPEAT);
	// Ambient Texture
	ambientTexture = cy::GLTexture2D();
	ambientTexture.Bind(2);
	ambientTexture.Initialize();
	ambientTexture.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	ambientTexture.SetWrappingMode(GL_REPEAT, GL_REPEAT);

	// Render to Texture
	// init frame buffer
	renderBuffer = cy::GLRenderTexture2D();
	renderBuffer.Initialize(true, renderTextureChannels, renderTextureWidth, renderTextureHeight);
	renderBuffer.Bind();





	// initial set of any uniform variables (DONT FORGET TO USE PROGRAM)
	cyProgram.SetUniform(diffuseTextureIndex, 0);
	cyProgram.SetUniform(specularTextureIndex, 1);
	cyProgram.SetUniform(ambientTextureIndex, 2);

	/*diffuseTexture.Bind(0);
	specularTexture.Bind(1);
	ambientTexture.Bind(2);*/
	glUseProgram(0);


	std::cout << "Uniforms registered" << std::endl;
}


void setupBuffers() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// doing dynamic draws in render loop
	//glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexProperty), verticesArray, GL_STATIC_DRAW);

	cyProgram.SetAttribBuffer("pos", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0); 
	cyProgram.SetAttribBuffer("normal", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	cyProgram.SetAttribBuffer("texCoord", VBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void rotateVertex(cy::Vec3<float>& vertex, cy::Matrix4<float> rotationMatrix) {
	cy::Vec4<float> v = cy::Vec4<float>(vertex.x, vertex.y, vertex.z, 1.0f);
	v = rotationMatrix * v;
	vertex = cy::Vec3<float>(v.x, v.y, v.z);
}

Transformation setupMVP() {
	cy::Matrix4<float> model = cy::Matrix4f();
	model.SetIdentity();
	
	//model = cy::Matrix4<float>::RotationY(90.0f * cy::Pi<float>() / 180.0f) * model;
	//model = cy::Matrix4<float>::RotationZ(90.0f * cy::Pi<float>() / 180.0f) * model;

	//model.SetScale(0.05f);


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

enum textureType {
	DIFFUSE,
	SPECULAR,
	AMBIENT,
};

std::vector<unsigned char> getTexture(int materialIndex, textureType t, unsigned* width, unsigned* height) {
	if (t == DIFFUSE) {
		auto contained = diffuseMaterialTextureMap.find(materialIndex);
		if (contained != diffuseMaterialTextureMap.end()) {
			return contained->second;
		}
	}
	else if (t == SPECULAR) {
		auto contained = specularMaterialTextureMap.find(materialIndex);
		if (contained != specularMaterialTextureMap.end()) {
			return contained->second;
		}
	}
	else if (t == AMBIENT) {
		auto contained = ambientMaterialTextureMap.find(materialIndex);
		if (contained != ambientMaterialTextureMap.end()) {
			return contained->second;
		}
	}
	
	cy::TriMesh::Mtl mtl = currMesh.M(materialIndex);
	std::vector<unsigned char> image;
	unsigned error;
	unsigned char* map;
	char* pathToTexture;// = mtl.map_Kd.data;
	switch (t) {
		case DIFFUSE:
			pathToTexture = mtl.map_Kd.data;
			break;
		case SPECULAR:
			pathToTexture = mtl.map_Ks.data;
			break;
		case AMBIENT:
			pathToTexture = mtl.map_Ka.data;
			break;
		default:
			pathToTexture = nullptr;
	}

	// no texture
	if (pathToTexture == nullptr) { return std::vector<unsigned char> {}; }

	std::string _pathToTexture = "textures\\" + std::string(pathToTexture);

	error = lodepng::decode(image, *width, *height, _pathToTexture);
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	switch (t) {
		case DIFFUSE:
			diffuseMaterialTextureMap.insert(std::pair<int, std::vector<unsigned char>>(materialIndex, image));
			break;
		case SPECULAR:
			specularMaterialTextureMap.insert(std::pair<int, std::vector<unsigned char>>(materialIndex, image));
			break;
		case AMBIENT:
			ambientMaterialTextureMap.insert(std::pair<int, std::vector<unsigned char>>(materialIndex, image));
			break;
	}
	return image;
}


void renderScene() {
	glUseProgram(cyProgram.GetID());

	float timeValue = (float)glfwGetTime();
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

	//cyProgram.SetUniform(smoothnessIndex, 32.0f);

	glBindVertexArray(VAO);

	for (auto const& x : materialMap) {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		cy::TriMesh::Mtl mtl = currMesh.M(x.first);
		std::vector<VertexProperty> vertexProperties = x.second;

		unsigned width, height;
		std::vector<unsigned char> diffMap = getTexture(x.first, DIFFUSE, &width, &height);

		if (diffMap.size() != 0) {
			diffuseTexture.SetImage(diffMap.data(), 4, width, height);
			diffuseTexture.BuildMipmaps();
			cyProgram.SetUniform(Kd, cy::Vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
			cyProgram.SetUniform(Kd, cy::Vec4<float>(mtl.Kd[0], mtl.Kd[1], mtl.Kd[2], 1.0f));

		}

		std::vector<unsigned char> specMap = getTexture(x.first, SPECULAR, &width, &height);
		if (specMap.size() != 0) {
			specularTexture.SetImage(specMap.data(), 4, width, height);
			specularTexture.BuildMipmaps();
			cyProgram.SetUniform(Ks, cy::Vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
		}
		else {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
			cyProgram.SetUniform(Ks, cy::Vec4<float>(mtl.Ks[0], mtl.Ks[1], mtl.Ks[2], 1.0f));
		}

		std::vector<unsigned char> ambMap = getTexture(x.first, AMBIENT, &width, &height);
		if (ambMap.size() != 0) {
			ambientTexture.SetImage(ambMap.data(), 4, width, height);
			ambientTexture.BuildMipmaps();
			cyProgram.SetUniform(Ka, cy::Vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
		}
		else {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
			cyProgram.SetUniform(Ka, cy::Vec4<float>(mtl.Ka[0], mtl.Ka[1], mtl.Ka[2], 1.0f));
		}


		cyProgram.SetUniform(smoothnessIndex, mtl.Ns);
		
		glBufferData(GL_ARRAY_BUFFER, vertexProperties.size() * sizeof(VertexProperty), vertexProperties.data(), GL_DYNAMIC_DRAW);

		GLint _odfb;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_odfb);
	
		// draw first to frame buffer
		renderBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, vertexProperties.size());
		renderBuffer.BuildTextureMipmaps(); // generate mipmaps for the render texture. mipmaps not always necessary. Depends on the use case.
		renderBuffer.Unbind();


		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, viewportWidth, viewportHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		renderBuffer.BindTexture(3); // bind texture to one of the texture units
		
		glDrawArrays(GL_TRIANGLES, 0, vertexProperties.size()); // draw final image to back buffer and swap later


		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

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
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	viewportWidth = width;
	viewportHeight = height;
	glViewport(0, 0, viewportWidth, viewportHeight);
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


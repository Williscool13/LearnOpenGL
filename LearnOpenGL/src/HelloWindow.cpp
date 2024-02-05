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
#include <gameobject.h>
#include <utilities.h>
#include <inputs.h>
#include <deltatime.h>
#include <windowProperties.h>

// png decoder
#include "lodepng.h"




void processInputs(GLFWwindow* window);
cy::GLSLProgram CreateObjectProgram();
void setupBuffers(cy::GLSLProgram& program, GameObject gameObject);
void renderObject(cy::GLSLProgram& program, GameObject gameobject, const GLuint vao);
void rotateObject(GameObject& gameObject);


int renderTextureChannels = 3;
GLsizei renderTextureWidth = 800;
GLsizei renderTextureHeight = 600;


// Camera
/// Camera Style
bool freeCam = false;
// Perspective/Otho
bool ortho = false;

/// Main Object Rotation and Translation
float y = 0.0f;
float p = 0.0f;
float d = -20.0f;
/// Plane Model Rotation and Translation
float plane_p = 45.0f;
float plane_y = 45.0f;
float plane_d = -20.0f;

// Main Light
float azimuth = 0.0f;
float inclination = 0.0f;

//cy::GLTexture2D diffuseTexture;
//cy::GLTexture2D specularTexture;
//cy::GLTexture2D ambientTexture;
//cy::GLRenderTexture2D renderTexture;
//
//cy::GLSLProgram objectProgram;
//cy::GLSLProgram _planeShader;
//
//GLuint objectVAO;
//GLuint objectVBO;
//GLuint planeVAO;
//GLuint planeVBO;

std::map<int, std::vector<unsigned char>> diffuseMaterialTextureMap;
std::map<int, std::vector<unsigned char>> specularMaterialTextureMap;
std::map<int, std::vector<unsigned char>> ambientMaterialTextureMap;




//cy::GLSLProgram setupObjectShader() {
//	cy::GLSLProgram objectShader = compileShaders("shaders\\helloVertexShader.vert", "shaders\\helloFragmentShader.frag");
//
//
//	// Uniforms
//	int baseindex = 200;
//	// time
//	objectShader.RegisterUniform(baseindex, "time");
//	baseindex += 1;
//	// transforms
//	 const char* transforms = "mvp mv model view projection mvNormal";
//	objectShader.RegisterUniforms(transforms, baseindex);
//	baseindex += 6;
//	// light properties
//	const char* light = "mainLightDirection ambientIlluminance";
//	objectShader.RegisterUniforms(light, baseindex);
//	baseindex += 2;
//	// material properties
//	const char* material = "fragTexture specularTexture ambientTexture Ka Kd Ks specularExponent";
//	objectShader.RegisterUniforms(material, baseindex);
//	baseindex += 6;
//
//	objectShader.Bind();
//
//	objectShader.SetUniform("fragTexture", 0);
//	objectShader.SetUniform("specularTexture", 1);
//	objectShader.SetUniform("ambientTexture", 2);
//	
//
//	return objectShader;	
//}



//void setupTextureUnits() {
//	objectProgram.Bind();
//	// Texture Units
//	diffuseTexture = cy::GLTexture2D();
//	diffuseTexture.Bind(0);
//	diffuseTexture.Initialize();
//	specularTexture = cy::GLTexture2D();
//	specularTexture.Bind(1);
//	specularTexture.Initialize();
//	ambientTexture = cy::GLTexture2D();
//	ambientTexture.Bind(2);
//	ambientTexture.Initialize();
//	renderTexture.BindTexture(3);
//	renderTexture.Initialize(true, 3, 800, 600);
//
//	glUseProgram(0);
//}





//void setupBuffers() {
//	objectProgram.Bind();
//	// object 
//	glGenVertexArrays(1, &objectVAO);
//	glBindVertexArray(objectVAO);
//
//	glGenBuffers(1, &objectVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
//
//	objectProgram.SetAttribBuffer("pos", objectVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
//	objectProgram.SetAttribBuffer("normal", objectVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
//	objectProgram.SetAttribBuffer("texCoord", objectVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));
//
//	std::cout << "Object vertex array object created" << std::endl;
//
//	glBindVertexArray(0);
//	// plane
//	/*_planeShader.Bind();
//	glGenVertexArrays(1, &planeVAO);
//	glBindVertexArray(planeVAO);
//
//	glGenBuffers(1, &planeVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
//
//	_planeShader.SetAttribBuffer("pos", planeVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
//	_planeShader.SetAttribBuffer("normal", planeVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
//	_planeShader.SetAttribBuffer("texCoord", planeVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));*/
//
//
//	//glBindVertexArray(0);
//}






enum textureType {
	DIFFUSE,
	SPECULAR,
	AMBIENT,
};

/// <summary>
/// Texture Caching
/// </summary>
/// <param name="materialIndex"></param>
/// <param name="t"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <returns></returns>
std::vector<unsigned char> getTexture(int materialIndex, textureType t, unsigned* width, unsigned* height, cy::TriMesh::Mtl material) {
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
	
	cy::TriMesh::Mtl mtl = material;
	std::vector<unsigned char> image;
	unsigned error;
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
	std::cout << "Loading texture: " << _pathToTexture << std::endl;

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





unsigned int VBO, VAO;
int main(int argc, char* argv[])
{
	#pragma region Initialization
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
		return 1;  // Exit with an error code
	}

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(viewportWidth, viewportHeight, "Look Mom, TRIANGLES!", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// initialize GLAD before calling any OpenGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// viewport
	glViewport(0, 0, viewportWidth, viewportHeight);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// all callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	#pragma endregion 


	//glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.8f, 1.0f);

	char* objFilePath = argv[1];
	std::cout << "Supplied obj file: " << objFilePath << std::endl;


	cy::GLSLProgram program = CreateObjectProgram();

	GameObject mainObject(objFilePath);
	mainObject.ApplyScale(cy::Vec3f(0.3f));
	setupBuffers(program, mainObject);


	// simple texture to send to the shader

	while (!glfwWindowShouldClose(window))
	{

		// do stuff with inputs
		processInputs(window);
		rotateObject(mainObject);
		resetDeltas();

		calculateDeltaTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw our first triangle
		renderObject(program, mainObject, VAO);

		//std::cout << "Object's position is " << mainObject.Position.x << ", " << mainObject.Position.y << ", " << mainObject.Position.z << std::endl;
		//renderScene(program, VAO, 6);
		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}
bool center = true;
void renderObject(cy::GLSLProgram& program, GameObject gameobject, const GLuint vao) {
	program.Bind();
	glBindVertexArray(vao);

	// set transforms (as it is the same for all materials under the same object)
	Transformation t = setupMVP(gameobject, center);
	program.SetUniform("mvp", t.mvp);
	program.SetUniform("mv", t.mv);
	program.SetUniform("m", t.model);
	program.SetUniform("v", t.view);
	program.SetUniform("p", t.projection);
	program.SetUniform("mvN", t.mv.GetSubMatrix3().GetInverse().GetTranspose());

	int cummulativeVertexIndex = 0;
	for (int i = 0; i < gameobject.getMaterialCount(); i++) {
		int materialFaceCount = gameobject.getMaterialFaceCount(i) * gameobject.getVerticesPerFace();
		// set material properties
		glDrawArrays(GL_TRIANGLES, cummulativeVertexIndex, materialFaceCount);
		cummulativeVertexIndex += materialFaceCount;

		//std::cout << "Drew " << cummulativeVertexIndex << " vertices" << std::endl;
	}
}


void setupBuffers(cy::GLSLProgram& program, GameObject gameObject) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, gameObject.getTotalVertices() * sizeof(VertexProperty), gameObject.vertices.data(), GL_STATIC_DRAW);

	program.SetAttribBuffer("pos", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	program.SetAttribBuffer("normal", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	program.SetAttribBuffer("texCoord", VBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));
}


void processInputs(GLFWwindow* window) {
	//if (wasdInput[0]) { camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime); }
	//if (wasdInput[1]) { camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime); }
	//if (wasdInput[2]) { camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime); }
	//if (wasdInput[3]) { camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime); }
	//if (spaceCInput[0]) { camera.ProcessKeyboard(Camera_Movement::UP, deltaTime); }
	//if (spaceCInput[1]) { camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime); }

	if (numberInput[0]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
	if (numberInput[1]) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
	if (numberInput[2]) { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); }

	/*//camera.ProcessMouseScroll(scrollDelta);

	//camera.ProcessMouseMovement(xoffset, yoffset);
	if (lmb && alt) {
		plane_p += xoffset * sensitivity;
		plane_y += yoffset * sensitivity;
	}
	else if (lmb && ctrl) {
		azimuth -= xoffset * sensitivity;
		inclination += yoffset * sensitivity;
	}
	else if (lmb) {
		p += xoffset * sensitivity;
		y += yoffset * sensitivity;
	}

	if (rmb && alt) {
		plane_d += yoffset * cameraSpeed;
	}
	else if (rmb) {
		d += yoffset * cameraSpeed;
	}
	if (shiftInput) { camera.MovementSpeed = 5.0f; }
	else { camera.MovementSpeed = 2.5f; }*/

	// do stuff with the inputs here
}

void rotateObject(GameObject& gameObject) {
	if (lmbPressed) {
		gameObject.ApplyRotation(mouseDeltaY, -mouseDeltaX);
		// move the camera lol
	}
	else if (rmbPressed) {
		gameObject.ApplyTranslation(cy::Vec3f(0,0, mouseDeltaY));
		//std::cout << "RMB pressed" << std::endl;
	}

	if (pPressed) {
		center = !center;
	}

}


cy::GLSLProgram CreateObjectProgram() {
	cy::GLSLProgram program;
	cy::GLSLShader vertexShader;
	cy::GLSLShader fragmentShader;
	const char* vertPath = "shaders\\helloVertexShader.vert";
	const char* fragPath = "shaders\\helloFragmentShader.frag";
	vertexShader.CompileFile(vertPath, GL_VERTEX_SHADER);
	fragmentShader.CompileFile(fragPath, GL_FRAGMENT_SHADER);
	if (program.Build(&vertexShader, &fragmentShader)) {
		std::cout << "Program built successfully" << std::endl;
		std::cout << "With vertex shader: " << vertPath << std::endl;
		std::cout << "With fragment shader: " << fragPath << std::endl;
	}
	else {
		std::cout << "Program failed to build" << std::endl;
	}



	// Uniforms
	int baseIndex = 0;
	const char* transforms = "mvp mv model view projection mvNormal";
	program.RegisterUniforms(transforms, baseIndex);
	baseIndex += 6;





	return program;

}

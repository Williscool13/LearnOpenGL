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
cy::GLSLProgram CreatePlaneProgram();
void setupBuffers(cy::GLSLProgram& program, GameObject& gameObject);
void renderObject(cy::GLSLProgram& program, GameObject& gameobject, const GLuint& vao);
void renderPlane(cy::GLSLProgram& program, const GLuint& vao);
void rotate(GameObject& gameObject);


int renderTextureChannels = 3;
GLsizei renderTextureWidth = 800;
GLsizei renderTextureHeight = 600;


// Camera
/// Camera Style
bool freeCam = false;
// Perspective/Otho
bool ortho = false;

/// Main Object Rotation and Translation
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraDistance = -30.0f;
/// Plane Model Rotation and Translation
//float plane_p = 45.0f;
//float plane_y = 45.0f;
//float plane_d = -20.0f;

// Main Light
float lightYaw = 0.0f;
float lightPitch = 0.0f;

cy::GLTexture2D diffuseTexture;
cy::GLTexture2D specularTexture;
cy::GLTexture2D ambientTexture;

cy::GLRenderTexture2D renderTexture;





VertexProperty* planeVertices = new VertexProperty[6]{
	{Vertex{-1.0f, -1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 0.0f}},
	{Vertex{1.0f, -1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 0.0f}},
	{Vertex{1.0f, 1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 1.0f}},
	{Vertex{1.0f, 1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 1.0f}},
	{Vertex{-1.0f, 1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 1.0f}},
	{Vertex{-1.0f, -1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 0.0f}}
};






GLuint VBO, VAO;
GLuint planeVBO, planeVAO;
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


	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.8f, 1.0f);

	char* objFilePath = argv[1];
	std::cout << "Supplied obj file: " << objFilePath << std::endl;


	diffuseTexture = cy::GLTexture2D();
	specularTexture = cy::GLTexture2D();
	ambientTexture = cy::GLTexture2D();
	renderTexture = cy::GLRenderTexture2D();

	glActiveTexture(GL_TEXTURE0);
	diffuseTexture.Initialize();
	glActiveTexture(GL_TEXTURE1);
	specularTexture.Initialize();
	glActiveTexture(GL_TEXTURE2);
	ambientTexture.Initialize();
	glActiveTexture(GL_TEXTURE3);
	renderTexture.Initialize(true, 3, viewportWidth, viewportHeight);



	cy::GLSLProgram objectProgram = CreateObjectProgram();
	cy::GLSLProgram planeProgram = CreatePlaneProgram();

	GameObject mainObject = GameObject(objFilePath);
	mainObject.setYaw(-90.0f);
	setupBuffers(objectProgram, mainObject);

	// simple texture to send to the shader

	while (!glfwWindowShouldClose(window))
	{

		// do stuff with inputs
		processInputs(window);
		rotate(mainObject);
		resetDeltas();

		calculateDeltaTime();

		// draw our first triangle
		//renderTexture.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderObject(objectProgram, mainObject, VAO);
		//renderTexture.Unbind();
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//renderPlane(planeProgram, planeVAO);
		


		//std::cout << "Object's position is " << mainObject.Position.x << ", " << mainObject.Position.y << ", " << mainObject.Position.z << std::endl;
		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}
void renderObject(cy::GLSLProgram& program, GameObject& gameobject, const GLuint& vao) {
	program.Bind();
	glBindVertexArray(vao);

	// set transforms (as it is the same for all materials under the same object)
	cy::Matrix4<float> model = gameobject.GetModelMatrix(true);

	cy::Matrix4f view = cy::Matrix4f::Identity();
	view.SetRotationXYZ(degToRad(cameraYaw), degToRad(cameraPitch), 0.0f);
	view.AddTranslation(cy::Vec3<float>(0.0f, 0.0f, cameraDistance));

	cy::Matrix4f projection = cy::Matrix4f::Perspective(degToRad(45), 800.0f / 600.0f, 0.1f, 1000.0f);
	
	cy::Matrix4f mvp = projection * view * model;
	cy::Matrix4f mv = view * model;

	program.SetUniform("time", (float)glfwGetTime());
	program.SetUniform("mvp", mvp);
	program.SetUniform("mv", mv);
	program.SetUniform("m", model);
	program.SetUniform("v", view);
	program.SetUniform("p", projection);
	program.SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

	cy::Vec3f _lightDirection = lightDirection(lightYaw, lightPitch);
	program.SetUniform("mainLightDirectionView", (view * cy::Vec4f(_lightDirection.GetNormalized(), 0)).XYZ());
	program.SetUniform("ambientIlluminance", 0.2f);
	program.SetUniform("mainLightIlluminance", 1.0f);

	int cummulativeVertexIndex = 0;
	for (int i = 0; i < gameobject.getMaterialCount(); i++) {
		int materialFaceCount = gameobject.getMaterialFaceCount(i) * gameobject.getVerticesPerFace();


		// set per material properties
		cy::TriMesh::Mtl _m = gameobject.getMaterial(i);
		program.SetUniform("Ka", cy::Vec3f(_m.Ka[0], _m.Ka[1], _m.Ka[2]));
		program.SetUniform("Kd", cy::Vec3f(_m.Kd[0], _m.Kd[1], _m.Kd[2]));
		program.SetUniform("Ks", cy::Vec3f(_m.Ks[0], _m.Ks[1], _m.Ks[2]));
		program.SetUniform("specularExponent", _m.Ns);

		
		diffuseTexture.SetImage(gameobject.GetDiffuseTexture(i).texture.data(), 4, gameobject.GetDiffuseTexture(i).width, gameobject.GetDiffuseTexture(i).height);
		specularTexture.SetImage(gameobject.GetSpecularTexture(i).texture.data(), 4, gameobject.GetSpecularTexture(i).width, gameobject.GetSpecularTexture(i).height);
		ambientTexture.SetImage(gameobject.GetAmbientTexture(i).texture.data(), 4, gameobject.GetAmbientTexture(i).width, gameobject.GetAmbientTexture(i).height);

		glDrawArrays(GL_TRIANGLES, cummulativeVertexIndex, materialFaceCount);
		cummulativeVertexIndex += materialFaceCount;

		//std::cout << "Drew " << cummulativeVertexIndex << " vertices" << std::endl;
	}
}

void renderPlane(cy::GLSLProgram& program, const GLuint& vao) {
	program.Bind();
	glBindVertexArray(vao);

	cy::Matrix4f model = cy::Matrix4f::Identity();

	cy::Matrix4f view = cy::Matrix4f::Identity();
	view.SetRotationXYZ(degToRad(cameraYaw), degToRad(cameraPitch), 0.0f);
	view.AddTranslation(cy::Vec3<float>(0.0f, 0.0f, cameraDistance));
	
	cy::Matrix4f projection = cy::Matrix4f::Perspective(degToRad(45), 800.0f / 600.0f, 0.1f, 1000.0f);

	cy::Matrix4f mvp = projection * view * model;
	cy::Matrix4f mv = view * model;

	program.SetUniform("time", (float)glfwGetTime());
	program.SetUniform("mvp", mvp);
	program.SetUniform("mv", mv);
	program.SetUniform("m", model);
	program.SetUniform("v", view);
	program.SetUniform("p", projection);
	program.SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void setupBuffers(cy::GLSLProgram& program, GameObject& gameObject) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, gameObject.getTotalVertices() * sizeof(VertexProperty), gameObject.vertices.data(), GL_STATIC_DRAW);

	program.SetAttribBuffer("pos", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	program.SetAttribBuffer("normal", VBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	program.SetAttribBuffer("texCoord", VBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);

	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(VertexProperty), planeVertices, GL_STATIC_DRAW);

	program.SetAttribBuffer("pos", planeVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	program.SetAttribBuffer("normal", planeVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	program.SetAttribBuffer("texCoord", planeVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));

	glBindVertexArray(0);
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

	/*camera.ProcessMouseScroll(scrollDelta);

	camera.ProcessMouseMovement(xoffset, yoffset);
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

void rotate(GameObject& gameObject) {
	if (lmbPressed && ctrlPressed) {
		lightPitch += mouseDeltaY * sensitivity;
		lightYaw += -mouseDeltaX * sensitivity;
	}
	else if (lmbPressed) {
		//gameObject.ApplyRotation(mouseDeltaX, 0, mouseDeltaY);
		cameraPitch += mouseDeltaX * sensitivity;
		cameraYaw -= mouseDeltaY * sensitivity;
	}
	else if (rmbPressed) {
		cameraDistance += mouseDeltaY * 0.1f;
		//gameObject.ApplyTranslation(cy::Vec3f(0,0, mouseDeltaY));
		//std::cout << "RMB pressed" << std::endl;
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



	// Texture Units
	program.Bind();
	
	// Teture Unit Bindings
	program.SetUniform("diffuseTexture", 0);
	program.SetUniform("specularTexture", 1);
	program.SetUniform("ambientTexture", 2);


	return program;
}

cy::GLSLProgram CreatePlaneProgram() {
	cy::GLSLProgram program;
	cy::GLSLShader vertexShader;
	cy::GLSLShader fragmentShader;
	const char* vertPath = "shaders\\planeVertexShader.vert";
	const char* fragPath = "shaders\\planeFragmentShader.frag";
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


	// Texture Units
	program.Bind();
	program.SetUniform("renderTexture", 3);
	std::cout << "Render texture location in second is" << glGetUniformLocation(program.GetID(), "renderTexture") << std::endl;
	return program;
}

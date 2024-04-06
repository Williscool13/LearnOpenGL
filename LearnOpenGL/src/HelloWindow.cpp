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
#include <environment.h>
#include <environmentCube.h>
#include <lights.h>
#include <transforms.h>
#include <reflection.h>


// png decoder
#include "lodepng.h"




void processInputs(GLFWwindow* window, OrbitCamera& camera, DirectionalLight& light);
cy::GLSLProgram CreateObjectProgram();
cy::GLSLProgram CreatePlaneProgram();
void setupBuffers(cy::GLSLProgram& program, GameObject& gameObject);

void setupReflectiveObject(const char* path);
void renderReflectiveObject(cy::Matrix4f view, cy::Matrix4f project);
void recompileReflectiveObject();

void setupReflectivePlane();
void renderReflectivePlane(cy::Matrix4f view, cy::Matrix4f project);
void recompileReflectivePlane();

void recompileShaders();
void renderObject(cy::GLSLProgram& program, GameObject& gameobject, const GLuint& vao, cy::Matrix4f view, cy::Matrix4f proj);
void renderPlane(cy::GLSLProgram& program, const GLuint& vao);




// Camera
float planeYaw = 0.0f;
float planePitch = 0.0f;
float planeDistance = -30.0f;

// Main Light
DirectionalLight mainLight = DirectionalLight();

// Objects
GLuint VBO, VAO;
// Plane
GLuint planeVBO, planeVAO;
std::vector<VertexProperty> planeVertices{
	{Vertex{-1.0f, -1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 0.0f}},
	{Vertex{1.0f, -1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 0.0f}},
	{Vertex{1.0f, 1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 1.0f}},
	{Vertex{1.0f, 1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 1.0f}},
	{Vertex{-1.0f, 1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 1.0f}},
	{Vertex{-1.0f, -1.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 0.0f}}
};

// Reflective Object
bool reflectiveObjectActive = false;
GLuint reflectiveObjectVAO, reflectiveObjectVBO;
cy::GLSLProgram reflectiveObjectProgram;
cy::Matrix4f reflectiveObjectModel = cy::Matrix4f::Identity();
int reflectiveObjectVertexCount;


// Reflective Plane
bool reflectivePlaneActive = false;
GLuint reflectivePlaneVAO, reflectivePlaneVBO;
cy::GLSLProgram reflectivePlaneProgram;
cy::Matrix4f reflectivePlaneModel = cy::Matrix4f::Identity();
std::vector<VertexProperty> reflectivePlaneVertices{
	{Vertex{-50.0f, -50.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 0.0f}},
	{Vertex{50.0f, -50.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 0.0f}},
	{Vertex{50.0f, 50.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 1.0f}},
	{Vertex{50.0f, 50.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{1.0f, 1.0f}},
	{Vertex{-50.0f, 50.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 1.0f}},
	{Vertex{-50.0f, -50.0f, 0.0f}, Normal{0.0f, 0.0f, 1.0f}, Texture{0.0f, 0.0f}}
};


cy::GLTexture2D diffuseTexture;
cy::GLTexture2D specularTexture;
cy::GLTexture2D ambientTexture;

cy::GLRenderTexture2D renderTexture;








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

	GLFWwindow* window = glfwCreateWindow(viewportWidth, viewportHeight, "Williscool.", NULL, NULL);
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

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	char* objFilePath = argv[1];
	std::cout << "Supplied obj file: " << objFilePath << std::endl;

	OrbitCamera camera = OrbitCamera();

	diffuseTexture = cy::GLTexture2D();
	specularTexture = cy::GLTexture2D();
	ambientTexture = cy::GLTexture2D();
	renderTexture = cy::GLRenderTexture2D();

	diffuseTexture.Initialize();
	specularTexture.Initialize();
	ambientTexture.Initialize();

	renderTexture.Initialize(true, renderTextureChannels, viewportWidth, viewportHeight);
	renderTexture.BindTexture(3);

	//initializeEnvironmentMap(4);
	initializeCubeEnvironmentMap(5);
	setupReflectiveObject(objFilePath);
	setupReflectivePlane();


	//cy::GLSLProgram objectProgram = CreateObjectProgram();
	//cy::GLSLProgram planeProgram = CreatePlaneProgram();

	//GameObject mainObject = GameObject(objFilePath);
	//mainObject.setYaw(-90.0f);
	//setupBuffers(objectProgram, mainObject);

	// simple texture to send to the shader

	while (!glfwWindowShouldClose(window))
	{
		// do stuff with inputs
		processInputs(window, camera, mainLight);

		calculateDeltaTime();

		// Render to Texture
		renderTexture.Bind();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cy::Matrix4f reflectionMatrix = generateReflectionMatrix(
			reflectivePlaneVertices[0].vertex.GetVec3(), // random vertex (dot w/ normal is the same for all vertices)
			reflectivePlaneVertices[0].normal.GetVec3(), // random normal (plane has same normal at all points)
			reflectivePlaneModel // plane model matrix (model -> world)
		);

		renderReflectiveObject(camera.GetViewMatrix() * reflectionMatrix.GetTranspose(), camera.GetProjectionMatrix());
		renderTexture.Unbind();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// cube has to be done before objects to avoid z-fighting
		renderCubeEnvironmentMap(camera.GetViewMatrix(), camera.GetProjectionMatrix(), camera.GetCameraPosition());

		//renderObject(objectProgram, mainObject, VAO, camera.GetViewMatrix(), camera.GetProjectionMatrix());
		renderReflectiveObject(camera.GetViewMatrix(), camera.GetProjectionMatrix());
		renderReflectivePlane(camera.GetViewMatrix(), camera.GetProjectionMatrix());

		// render background
		//renderEnvironmentMap(camera.GetViewMatrix(), camera.GetProjectionMatrix());



		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

void renderObject(cy::GLSLProgram& program, GameObject& gameobject, const GLuint& vao, cy::Matrix4f view, cy::Matrix4f proj) {
	program.Bind();
	glBindVertexArray(vao);

	cy::Matrix4f model = gameobject.GetModelMatrix(true);
	cy::Matrix4f mvp = proj * view * model;
	cy::Matrix4f mv = view * model;

	program.SetUniform("time", (float)glfwGetTime());
	program.SetUniform("mvp", mvp);
	program.SetUniform("mv", mv);
	program.SetUniform("m", model);
	program.SetUniform("v", view);
	program.SetUniform("p", proj);
	program.SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

	cy::Vec3f _lightDirection = mainLight.GetLightDirection();
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


		diffuseTexture.Bind(0);
		diffuseTexture.SetImage(gameobject.GetDiffuseTexture(i).texture.data(), 4, gameobject.GetDiffuseTexture(i).width, gameobject.GetDiffuseTexture(i).height);
		specularTexture.Bind(1);
		specularTexture.SetImage(gameobject.GetSpecularTexture(i).texture.data(), 4, gameobject.GetSpecularTexture(i).width, gameobject.GetSpecularTexture(i).height);
		ambientTexture.Bind(2);
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
	model = cy::Matrix4f::Scale(10.0f, 10.0f, 10.0f) * model;

	cy::Matrix4f view = cy::Matrix4f::Identity();
	view.SetRotationXYZ(degToRad(planeYaw), degToRad(planePitch), 0.0f);
	view.AddTranslation(cy::Vec3<float>(0.0f, 0.0f, planeDistance));

	cy::Matrix4f projection = cy::Matrix4f::Perspective(degToRad(45),
		800.0f / 600.0f, 0.1f, 1000.0f);

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
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(VertexProperty), planeVertices.data(), GL_STATIC_DRAW);

	program.SetAttribBuffer("pos", planeVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	program.SetAttribBuffer("normal", planeVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	program.SetAttribBuffer("texCoord", planeVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal));

	glBindVertexArray(0);
}


#pragma region Reflective Object Rendering
void setupReflectiveObject(const char* path) {
	reflectiveObjectActive = true;
	cy::TriMesh reflectiveObjectMesh;
	std::vector<VertexProperty> reflectiveObjectVertices{};

	reflectiveObjectMesh.LoadFromFileObj(path);
	for (int i = 0; i < reflectiveObjectMesh.NF(); i++) {
		cy::TriMesh::TriFace v_f = reflectiveObjectMesh.F(i);
		cy::TriMesh::TriFace n_f = reflectiveObjectMesh.FN(i);
		cy::TriMesh::TriFace t_f = reflectiveObjectMesh.FT(i);
		for (int j = 0; j < 3; j++) {
			cy::Vec3f vertex = reflectiveObjectMesh.V(v_f.v[j]);
			cy::Vec3f normal = reflectiveObjectMesh.VN(n_f.v[j]);
			cy::Vec3f texCoord = reflectiveObjectMesh.VT(t_f.v[j]);

			reflectiveObjectVertices.push_back(VertexProperty{
				Vertex{vertex.x, vertex.y, vertex.z},
				Normal{normal.x, normal.y, normal.z},
				Texture{texCoord.x, texCoord.y} }
			);
		}
	}


	reflectiveObjectMesh.ComputeBoundingBox();
	cy::Vec3<float> min = reflectiveObjectMesh.GetBoundMin();
	cy::Vec3<float> max = reflectiveObjectMesh.GetBoundMax();
	cy::Vec3<float> center = (min + max) * 0.5f;
	float radius = cy::Max((max - min).x, (max - min).y, (max - min).z);
	reflectiveObjectModel = cy::Matrix4f::Translation(-center);


	glGenVertexArrays(1, &reflectiveObjectVAO);
	glGenBuffers(1, &reflectiveObjectVBO);
	glBindVertexArray(reflectiveObjectVAO);

	reflectiveObjectProgram.BuildFiles("shaders\\reflections\\reflect.vert", "shaders\\reflections\\reflect.frag");
	reflectiveObjectProgram.SetUniform("environmentTexture", 5);

	glBindBuffer(GL_ARRAY_BUFFER, reflectiveObjectVBO);
	glBufferData(GL_ARRAY_BUFFER, reflectiveObjectVertices.size() * sizeof(VertexProperty), reflectiveObjectVertices.data(), GL_STATIC_DRAW);
	reflectiveObjectProgram.SetAttribBuffer("pos", reflectiveObjectVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	reflectiveObjectProgram.SetAttribBuffer("normal", reflectiveObjectVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	reflectiveObjectProgram.SetAttribBuffer("texCoord", reflectiveObjectVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty),
		sizeof(Vertex) + sizeof(Normal));

	reflectiveObjectVertexCount = reflectiveObjectVertices.size();
	std::cout << "Reflective Object Finished Initialization" << std::endl;
}

void renderReflectiveObject(cy::Matrix4f view, cy::Matrix4f project) {
	reflectiveObjectProgram.Bind();
	glBindVertexArray(reflectiveObjectVAO);
	reflectiveObjectProgram.SetUniform("model", reflectiveObjectModel);
	reflectiveObjectProgram.SetUniform("view", view);
	reflectiveObjectProgram.SetUniform("proj", project);
	cy::Matrix4f mv = view * reflectiveObjectModel;
	reflectiveObjectProgram.SetUniform("mv", mv);
	reflectiveObjectProgram.SetUniform("mvp", project * mv);
	reflectiveObjectProgram.SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

	reflectiveObjectProgram.SetUniform("i_model", reflectiveObjectModel.GetInverse());
	reflectiveObjectProgram.SetUniform("i_view", view.GetInverse());
	reflectiveObjectProgram.SetUniform("i_proj", project.GetInverse());

	cy::Vec3f _lightDirection = mainLight.GetLightDirection();
	reflectiveObjectProgram.SetUniform("mainLightDirectionView", (view * cy::Vec4f(_lightDirection.GetNormalized(), 0)).XYZ());
	reflectiveObjectProgram.SetUniform("specularExponent", 32.0f);


	//glDrawArrays(GL_TRIANGLES, 0, reflectiveObjectVertices.size());
	glDrawArrays(GL_TRIANGLES, 0, reflectiveObjectVertexCount);

	glBindVertexArray(0);
}

void recompileReflectiveObject() {
	reflectiveObjectProgram.BuildFiles("shaders\\reflections\\reflect.vert", "shaders\\reflections\\reflect.frag");
	reflectiveObjectProgram.SetUniform("environmentTexture", 5);

	reflectiveObjectProgram.SetAttribBuffer("pos", reflectiveObjectVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	reflectiveObjectProgram.SetAttribBuffer("normal", reflectiveObjectVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	reflectiveObjectProgram.SetAttribBuffer("texCoord", reflectiveObjectVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty),
		sizeof(Vertex) + sizeof(Normal));


	std::cout << "Recompiled Reflective Object Shaders" << std::endl;
}
#pragma endregion

#pragma region Reflective Plane Rendering
void setupReflectivePlane() {
	reflectivePlaneActive = true;
	reflectivePlaneModel = cy::Matrix4f::Identity();
	reflectivePlaneModel = cy::Matrix4f::RotationX(degToRad(-90.0f)) * reflectivePlaneModel;
	reflectivePlaneModel = cy::Matrix4f::Translation(cy::Vec3f(0.0f, -7.88f, 0.0f)) * reflectivePlaneModel;
	
	glGenVertexArrays(1, &reflectivePlaneVAO);
	glGenBuffers(1, &reflectivePlaneVBO);
	glBindVertexArray(reflectivePlaneVAO);

	reflectivePlaneProgram.BuildFiles("shaders\\reflections\\reflectPlane.vert", "shaders\\reflections\\reflectPlane.frag");
	reflectivePlaneProgram.SetUniform("renderTexture", 3);
	reflectivePlaneProgram.SetUniform("environmentTexture", 5);

	diffuseTexture.Bind(0);

	glBindBuffer(GL_ARRAY_BUFFER, reflectivePlaneVBO);
	glBufferData(GL_ARRAY_BUFFER, reflectivePlaneVertices.size() * sizeof(VertexProperty), reflectivePlaneVertices.data(), GL_STATIC_DRAW);
	reflectivePlaneProgram.SetAttribBuffer("pos", reflectivePlaneVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	reflectivePlaneProgram.SetAttribBuffer("normal", reflectivePlaneVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	reflectivePlaneProgram.SetAttribBuffer("texCoord", reflectivePlaneVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty),
		sizeof(Vertex) + sizeof(Normal));


	std::cout << "Reflective Plane Finished Initialization" << std::endl;
}

void renderReflectivePlane(cy::Matrix4f view, cy::Matrix4f project) {
	reflectivePlaneProgram.Bind();
	glBindVertexArray(reflectivePlaneVAO);
	reflectivePlaneProgram.SetUniform("model", reflectivePlaneModel);
	reflectivePlaneProgram.SetUniform("view", view);
	reflectivePlaneProgram.SetUniform("proj", project);
	cy::Matrix4f mv = view * reflectivePlaneModel;
	reflectivePlaneProgram.SetUniform("mv", mv);
	reflectivePlaneProgram.SetUniform("mvp", project * mv);
	reflectivePlaneProgram.SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

	reflectivePlaneProgram.SetUniform("i_model", reflectivePlaneModel.GetInverse());
	reflectivePlaneProgram.SetUniform("i_view", view.GetInverse());
	reflectivePlaneProgram.SetUniform("i_proj", project.GetInverse());

	cy::Vec3f _lightDirection = mainLight.GetLightDirection();
	reflectivePlaneProgram.SetUniform("mainLightDirectionView", (view * cy::Vec4f(_lightDirection.GetNormalized(), 0)).XYZ());
	reflectivePlaneProgram.SetUniform("specularExponent", 32.0f);

	glDrawArrays(GL_TRIANGLES, 0, reflectivePlaneVertices.size());
	glBindVertexArray(0);
}

void recompileReflectivePlane() {
	reflectivePlaneProgram.BuildFiles("shaders\\reflections\\reflectPlane.vert", "shaders\\reflections\\reflectPlane.frag");
	reflectivePlaneProgram.SetUniform("renderTexture", 3);
	reflectivePlaneProgram.SetUniform("environmentTexture", 5);

	reflectivePlaneProgram.SetAttribBuffer("pos", reflectivePlaneVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
	reflectivePlaneProgram.SetAttribBuffer("normal", reflectivePlaneVBO, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex));
	reflectivePlaneProgram.SetAttribBuffer("texCoord", reflectivePlaneVBO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty),
		sizeof(Vertex) + sizeof(Normal));

	std::cout << "Recompiled Reflective Plane Shaders" << std::endl;
}
#pragma endregion

void recompileShaders() {
	if (reflectiveObjectActive) recompileReflectiveObject();
	if (reflectivePlaneActive) recompileReflectivePlane();
}

cy::GLSLProgram CreateObjectProgram() {

	cy::GLSLProgram program;
	program.BuildFiles("shaders\\helloVertexShader.vert", "shaders\\helloFragmentShader.frag");

	// Teture Unit Bindings
	program.SetUniform("diffuseTexture", 0);
	program.SetUniform("specularTexture", 1);
	program.SetUniform("ambientTexture", 2);
	program.SetUniform("renderTexture", 3);


	return program;
}

cy::GLSLProgram CreatePlaneProgram() {
	cy::GLSLProgram _planeProgram;
	_planeProgram.BuildFiles("shaders\\planeVertexShader.vert", "shaders\\planeFragmentShader.frag");

	// Texture Units
	_planeProgram.SetUniform("renderTexture", 3);

	return _planeProgram;
}



void processInputs(GLFWwindow* window, OrbitCamera& camera, DirectionalLight& light) {
	if (numberInput[0]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
	if (numberInput[1]) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
	if (numberInput[2]) { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); }

	if (oPressed) {
		recompileShaders();
	}

	if (ctrlPressed) {
		if (lmbPressed) {
			light.SetYaw(light.GetYaw() - degToRad(mouseDeltaX));
			light.SetPitch(light.GetPitch() + degToRad(mouseDeltaY));
		}
	}
	else {
		if (lmbPressed) { camera.Rotate(mouseDeltaX, -mouseDeltaY); }
		else if (rmbPressed) { camera.Move(mouseDeltaY); }
	}


	if (wasdInput[2]) { camera.SetFOV(camera.GetFOV() + degToRad(1.0f)); }
	if (wasdInput[0]) { camera.SetFOV(camera.GetFOV() - degToRad(1.0f)); }

	resetDeltas();
}
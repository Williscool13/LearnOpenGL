#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <functional>
#include <windows.h>

#include <string_view>
using namespace std::string_view_literals;
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
void setupBuffers(GameObject& gob);

void recompileShaders();
void renderGameObject(GameObject& gob, cy::Matrix4f view, cy::Matrix4f proj);



// Main Light
DirectionalLight mainLight = DirectionalLight();


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

    OrbitCamera camera{ };

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


    GameObject mainObject{ objFilePath
        //, "shaders\\reflections\\reflect.vert"sv
        //, "shaders\\reflections\\reflect.frag"sv };
        , "shaders\\helloShader.vert"sv
        , "shaders\\helloShader.frag"sv };



    GameObject reflectivePlane{ "models\\plane.obj"
           , "shaders\\reflections\\reflectPlane.vert"sv
           , "shaders\\reflections\\reflectPlane.frag"sv };


    GameObject lightRepresentation{ "models\\sphere.obj"
           , "shaders\\basic\\basic.vert"sv
           , "shaders\\basic\\basic.frag"sv };

    setupBuffers(mainObject);
    setupBuffers(reflectivePlane);
    setupBuffers(lightRepresentation);



    float translationDist = (mainObject.getBoundMax().y - mainObject.getBoundMin().y) / 2.0f;
    reflectivePlane.applyTranslation(cy::Vec3f(0.0f, -translationDist, 0.0f));
    reflectivePlane.applyRotation(-90.0f, 0.0f, 0.0f);

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
            reflectivePlane.getVertex(0), // random vertex (dot w/ normal is the same for all vertices)
            reflectivePlane.getNormal(0), // random normal (plane has same normal at all points)
            reflectivePlane.getModelMatrix() // plane model matrix (model -> world)
        );

        renderGameObject(mainObject, camera.GetViewMatrix() * reflectionMatrix.GetTranspose(), camera.GetProjectionMatrix());
        renderTexture.Unbind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // cube has to be done before objects to avoid z-fighting
        renderCubeEnvironmentMap(camera.GetViewMatrix(), camera.GetProjectionMatrix(), camera.GetCameraPosition());

        renderGameObject(reflectivePlane, camera.GetViewMatrix(), camera.GetProjectionMatrix());
        renderGameObject(mainObject, camera.GetViewMatrix(), camera.GetProjectionMatrix());

        lightRepresentation.setTranslation(mainLight.GetLightPosition());
        renderGameObject(lightRepresentation, camera.GetViewMatrix(), camera.GetProjectionMatrix());

        // render background
        //renderEnvironmentMap(camera.GetViewMatrix(), camera.GetProjectionMatrix());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}

void setupBuffers(GameObject& gob) {
    glGenVertexArrays(1, &gob.getVAO());
    glGenBuffers(1, &gob.getVBO());
    glBindVertexArray(gob.getVAO());

    glBindBuffer(GL_ARRAY_BUFFER, gob.getVBO());
    glBufferData(GL_ARRAY_BUFFER, gob.getVertices().size() * sizeof(VertexProperty), gob.getVertices().data(), GL_STATIC_DRAW);

    gob.getProgram().SetAttribBuffer("pos", gob.getVBO(), 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), 0);
    if (gob.hasNormals()) { gob.getProgram().SetAttribBuffer("normal", gob.getVBO(), 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex)); }
    if (gob.hasTextureVertices()) { gob.getProgram().SetAttribBuffer("texCoord", gob.getVBO(), 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperty), sizeof(Vertex) + sizeof(Normal)); }
}

void renderGameObject(GameObject& gob, cy::Matrix4f view, cy::Matrix4f proj) {
    gob.getProgram().Bind();
    glBindVertexArray(gob.getVAO());

    cy::Matrix4f model = gob.getModelMatrix();
    cy::Matrix4f mvp = proj * view * model;
    cy::Matrix4f mv = view * model;

    gob.getProgram().SetUniform("time", (float)glfwGetTime());
    gob.getProgram().SetUniform("mvp", mvp);
    gob.getProgram().SetUniform("mv", mv);
    gob.getProgram().SetUniform("m", model);
    gob.getProgram().SetUniform("v", view);
    gob.getProgram().SetUniform("p", proj);
    gob.getProgram().SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

    gob.getProgram().SetUniform("i_m", model.GetInverse());
    gob.getProgram().SetUniform("i_v", view.GetInverse());
    gob.getProgram().SetUniform("i_p", proj.GetInverse());

    cy::Vec3f _lightDirection = mainLight.GetLightDirection();
    gob.getProgram().SetUniform("mainLightDirectionView", (view * cy::Vec4f(_lightDirection.GetNormalized(), 0)).XYZ());
    gob.getProgram().SetUniform("specularExponent", 32.0f);


    if (gob.getMaterialCount() == 0) {
        // no materials, just draw all vertices
        glDrawArrays(GL_TRIANGLES, 0, gob.getVertices().size());
    }
    else {
        int cummulativeVertexIndex = 0;
        for (int i = 0; i < gob.getMaterialCount(); i++) {
            int materialFaceCount = gob.getMaterialFaceCount(i) * gob.getVerticesPerFace();


            // set per material properties
            cy::TriMesh::Mtl _m = gob.getMaterial(i);
            gob.getProgram().SetUniform("Ka", cy::Vec3f(_m.Ka[0], _m.Ka[1], _m.Ka[2]));
            gob.getProgram().SetUniform("Kd", cy::Vec3f(_m.Kd[0], _m.Kd[1], _m.Kd[2]));
            gob.getProgram().SetUniform("Ks", cy::Vec3f(_m.Ks[0], _m.Ks[1], _m.Ks[2]));
            gob.getProgram().SetUniform("specularExponent", _m.Ns);


            diffuseTexture.Bind(0);
            diffuseTexture.SetImage(gob.getDiffuseTexture(i).texture.data(), 4, gob.getDiffuseTexture(i).width, gob.getDiffuseTexture(i).height);
            specularTexture.Bind(1);
            specularTexture.SetImage(gob.getSpecularTexture(i).texture.data(), 4, gob.getSpecularTexture(i).width, gob.getSpecularTexture(i).height);
            ambientTexture.Bind(2);
            ambientTexture.SetImage(gob.getAmbientTexture(i).texture.data(), 4, gob.getAmbientTexture(i).width, gob.getAmbientTexture(i).height);

            glDrawArrays(GL_TRIANGLES, cummulativeVertexIndex, materialFaceCount);
            cummulativeVertexIndex += materialFaceCount;

            //std::cout << "Drew " << cummulativeVertexIndex << " vertices" << std::endl;
        }
    }

    glBindVertexArray(0);
}


void recompileShaders() {

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
            light.SetPitch(light.GetPitch() - degToRad(mouseDeltaY));
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
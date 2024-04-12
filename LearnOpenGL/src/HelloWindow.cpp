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




void processInputs(GLFWwindow* window, OrbitCamera& camera, DirectionalLight& light, PointLight& pointLight);
void setupBuffers(GameObject& gob);

void recompileShaders();
void renderGameObject(
    GameObject& gob
    , cy::GLTexture2D& diffuseTexture
    , cy::GLTexture2D& specularTexture
    , cy::GLTexture2D& ambientTexture
    , DirectionalLight& mainLight
    , PointLight& pointLight
    , OrbitCamera& camera
);
void depthRenderGameObject(GameObject& gob, cy::GLSLProgram& depthProgram, cy::Matrix4f lightView, cy::Matrix4f lightProj);
void cubeDepthRenderGameObject(GameObject& gob, cy::GLSLProgram& cubeDepthProgram, PointLight& pointLight);




// Main Light

std::vector<GameObject> gameObjects{};


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
    DirectionalLight mainLight{ };
    PointLight pointLight{};

    cy::GLTexture2D diffuseTexture{};
    cy::GLTexture2D specularTexture{};
    cy::GLTexture2D ambientTexture{};
    cy::GLTexture2D normalTexture{};
    cy::GLRenderTexture2D renderTexture{};
    diffuseTexture.Initialize();
    specularTexture.Initialize();
    ambientTexture.Initialize();
    normalTexture.Initialize();
    normalTexture.Bind(7);
    TextureProperty teapotNormal = loadTexture("teapot_normal.png");
    normalTexture.SetImage(teapotNormal.texture.data(), 4, teapotNormal.width, teapotNormal.height);

    renderTexture.BindTexture(3);
    renderTexture.Initialize(true, renderTextureChannels, viewportWidth, viewportHeight);

    //initializeEnvironmentMap(4);
    //initializeCubeEnvironmentMap(5);

    // Shadow Mapping
    //  Point Light
    cy::GLRenderDepthCube cubeRenderDepth{};
    cubeRenderDepth.BindTexture(4);
    cubeRenderDepth.Initialize(false, 4096, 4096);
    cubeRenderDepth.SetTextureFilteringMode(GL_NEAREST, GL_NEAREST);
    //cubeRenderDepth.SetTextureWrappingMode(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

    //  Directional Light
    cy::GLRenderDepth2D renderDepth{};
    renderDepth.BindTexture(6);
    renderDepth.Initialize(true, 1024, 1024);
    renderDepth.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
    renderDepth.SetTextureWrappingMode(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    float borderColor[]{ 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);



    GameObject mainObject{ objFilePath
        //, "shaders\\reflections\\reflect.vert"sv
        //, "shaders\\reflections\\reflect.frag"sv };
        , "shaders\\basic\\basicShaded.vert"sv
        , "shaders\\basic\\basicShaded.frag"sv
        , "shaders\\basic\\basicShaded.geom"sv
    };

    GameObject groundPlane{ "models\\plane.obj"
        , "shaders\\basic\\basicShaded.vert"sv
        , "shaders\\basic\\basicShaded.frag"sv
        , "shaders\\basic\\basicShaded.geom"sv
    };


    GameObject lightRepresentation{ "models\\light.obj"
        , "shaders\\basic\\basicShaded.vert"sv
        , "shaders\\basic\\basicShaded.frag"sv
        , "shaders\\basic\\basicShaded.geom"sv
    };

    GameObject pointLightRepresentation{ "models\\sphere.obj"
        , "shaders\\basic\\basic.vert"sv
        , "shaders\\basic\\basic.frag"sv
        , "shaders\\basic\\basic.geom"sv
    };

    GameObject renTexDebug{ "models\\plane.obj"
        , "shaders\\basic\\texture.vert"sv
        , "shaders\\basic\\texture.frag"sv
    };


    GameObject planeNormalMapping { "models\\plane.obj"
        , "shaders\\advancedMapping.vert"sv
        , "shaders\\advancedMapping.frag"sv
    };


    gameObjects.push_back(mainObject);
    //gameObjects.push_back(reflectivePlane);
    //gameObjects.push_back(lightRepresentation);



    setupBuffers(mainObject);
    setupBuffers(groundPlane);
    setupBuffers(lightRepresentation);
    setupBuffers(pointLightRepresentation);
    setupBuffers(renTexDebug);
    setupBuffers(planeNormalMapping);

    renTexDebug.applyTranslation(cy::Vec3f(0.0f, -10.0f, 0.0f));
    renTexDebug.applyRotation(degToRad(-90.0f), 0.0f, 0.0f);

    lightRepresentation.applyScale(cy::Vec3f(3.0f));
    pointLightRepresentation.applyScale(cy::Vec3f(0.1f));

    float translationDist = (mainObject.getBoundMax().y - mainObject.getBoundMin().y) / 2.0f;
    groundPlane.applyTranslation(cy::Vec3f(0.0f, -translationDist, 0.0f));
    groundPlane.applyRotation(degToRad(-90.0f), 0.0f, 0.0f);

    //planeNormalMapping.applyTranslation(cy::Vec3f(0.0f, -translationDist, 0.0f));
    planeNormalMapping.applyRotation(degToRad(-90.0f), 0.0f, 0.0f);

    cy::GLSLProgram depthProgram{};
    depthProgram.BuildFiles("shaders\\basic\\depth.vert", "shaders\\basic\\depth.frag");

    cy::GLSLProgram cubeDepthProgram{};
    cubeDepthProgram.BuildFiles("shaders\\basic\\depthCube.vert", "shaders\\basic\\depthCube.frag"
        , "shaders\\basic\\depthCube.geom");



    while (!glfwWindowShouldClose(window))
    {
        // do stuff with inputs
        processInputs(window, camera, mainLight, pointLight);
        calculateDeltaTime();

#pragma region Render Texture Reflections
        //renderTexture.Bind();
        //glClearColor(0, 0, 0, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //cy::Matrix4f reflectionMatrix = generateReflectionMatrix(
        //    reflectivePlane.getVertex(0), // random vertex (dot w/ normal is the same for all vertices)
        //    reflectivePlane.getNormal(0), // random normal (plane has same normal at all points)
        //    reflectivePlane.getModelMatrix() // plane model matrix (model -> world)
        //);

        ///*renderGameObject(mainObject, camera.GetViewMatrix() * reflectionMatrix.GetTranspose(), camera.GetProjectionMatrix()
        //    , diffuseTexture, specularTexture, ambientTexture, mainLight);*/

        //renderTexture.Unbind();

#pragma endregion

#pragma region Render Depth Shadows
    // sets viewport and binds framebuffer
        renderDepth.Bind();
        glCullFace(GL_FRONT);
        glClear(GL_DEPTH_BUFFER_BIT);

        // render gameobject w/ light view and projection
        // depthRenderGameObject(mainObject, depthProgram, mainLight.getViewMatrix(), mainLight.getProjectionMatrix());

        glCullFace(GL_BACK);
        renderDepth.Unbind();




        // Point Light
        cubeRenderDepth.Bind();
        // use this if rendering to faces indi
        //  cubeRenderDepth.SetTarget(0);
        glCullFace(GL_FRONT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeRenderDepth.GetTextureID(), 0);
        cubeDepthRenderGameObject(mainObject, cubeDepthProgram, pointLight);


        glCullFace(GL_BACK);
        cubeRenderDepth.Unbind();
#pragma endregion


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // cube has to be done before objects to avoid z-fighting
        //renderCubeEnvironmentMap(camera.GetViewMatrix(), camera.GetProjectionMatrix(), camera.getCameraPosition());


        /*lightRepresentation.setTranslation(mainLight.getPosition());
        lightRepresentation.setYaw(-mainLight.getYaw() + degToRad(-90.0f));
        lightRepresentation.setPitch(mainLight.getPitch());
        renderGameObject(lightRepresentation, diffuseTexture, specularTexture, ambientTexture
            , mainLight, pointLight, camera);
        */
        pointLightRepresentation.setTranslation(pointLight.getPosition());
        renderGameObject(pointLightRepresentation, diffuseTexture, specularTexture, ambientTexture
            , mainLight, pointLight, camera);


        /*renderGameObject(mainObject, diffuseTexture, specularTexture, ambientTexture
            , mainLight, pointLight, camera);
        renderGameObject(groundPlane, diffuseTexture, specularTexture, ambientTexture
            , mainLight, pointLight, camera);
        */


        renderGameObject(planeNormalMapping, diffuseTexture, specularTexture, ambientTexture
            , mainLight, pointLight, camera);


        //renderGameObject(renTexDebug, camera.GetViewMatrix(), camera.GetProjectionMatrix()
            //, diffuseTexture, specularTexture, ambientTexture, mainLight);


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
    if (gob.hasNormals()) {
        gob.getProgram().SetAttribBuffer(
            "normal"
            , gob.getVBO()
            , 3
            , GL_FLOAT
            , GL_FALSE
            , sizeof(VertexProperty)
            , sizeof(Vertex)
        );
    }
    if (gob.hasNormals() && gob.hasTextureVertices()) {
        gob.getProgram().SetAttribBuffer(
            "tangent"
            , gob.getVBO()
            , 3
            , GL_FLOAT, GL_FALSE
            , sizeof(VertexProperty)
            , sizeof(Vertex) + sizeof(Normal)
        );

    }
    if (gob.hasTextureVertices()) {
        gob.getProgram().SetAttribBuffer(
            "texCoord"
            , gob.getVBO()
            , 2
            , GL_FLOAT, GL_FALSE
            , sizeof(VertexProperty)
            , sizeof(Vertex) + sizeof(Normal) + sizeof(Tangent)
        );
    }


}

void renderGameObject(
    GameObject& gob
    , cy::GLTexture2D& diffuseTexture
    , cy::GLTexture2D& specularTexture
    , cy::GLTexture2D& ambientTexture
    , DirectionalLight& mainLight
    , PointLight& pointLight
    , OrbitCamera& camera
) {
    gob.getProgram().Bind();
    glBindVertexArray(gob.getVAO());


    gob.getProgram().SetUniform("diffuseTexture", 0);
    gob.getProgram().SetUniform("specularTexture", 1);
    gob.getProgram().SetUniform("ambientTexture", 2);
    gob.getProgram().SetUniform("renderTexture", 3);
    gob.getProgram().SetUniform("shadowCubeTexture", 4);
    gob.getProgram().SetUniform("environmentTexture", 5);
    gob.getProgram().SetUniform("shadowTexture", 6);
    gob.getProgram().SetUniform("normalTexture", 7);

    cy::Matrix4f model = gob.getModelMatrix();
    cy::Matrix4f view = camera.GetViewMatrix();
    cy::Matrix4f proj = camera.GetProjectionMatrix();
    cy::Matrix4f mvp = proj * view * model;
    cy::Matrix4f mv = view * model;

    gob.getProgram().SetUniform("time", (float)glfwGetTime());
    gob.getProgram().SetUniform("mvp", mvp);
    gob.getProgram().SetUniform("mv", mv);
    gob.getProgram().SetUniform("m", model);
    gob.getProgram().SetUniform("v", view);
    gob.getProgram().SetUniform("p", proj);
    gob.getProgram().SetUniform("mN", model.GetSubMatrix3().GetInverse().GetTranspose());
    gob.getProgram().SetUniform("mvN", mv.GetSubMatrix3().GetInverse().GetTranspose());

    gob.getProgram().SetUniform("i_m", model.GetInverse());
    gob.getProgram().SetUniform("i_v", view.GetInverse());
    gob.getProgram().SetUniform("i_p", proj.GetInverse());

    // -1 -> 1 to 0 -> 1 remapping
    cy::Matrix4f shadowvp = mainLight.getViewProjectionMatrix();
    shadowvp = cy::Matrix4f::Scale(cy::Vec3f(0.5f, 0.5f, 0.5f)) * shadowvp;
    shadowvp = cy::Matrix4f::Translation(cy::Vec3f(0.5f, 0.5f, 0.5f)) * shadowvp;
    gob.getProgram().SetUniform("shadowvp", shadowvp);

    gob.getProgram().SetUniform("pointLightPos", pointLight.getPosition());
    gob.getProgram().SetUniform("shadowCubeFarPlane", pointLight.getFarPlane());

    gob.getProgram().SetUniform("cameraPos", camera.getCameraPosition());


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

/// <summary>
/// Use a custom depth shader to render the gameobject at a lower cost
/// </summary>
/// <param name="gob"></param>
/// <param name="depthProgram"></param>
/// <param name="lightView"></param>
/// <param name="lightProj"></param>
void depthRenderGameObject(
    GameObject& gob,
    cy::GLSLProgram& depthProgram,
    cy::Matrix4f lightView,
    cy::Matrix4f lightProj
) {
    depthProgram.Bind();
    glBindVertexArray(gob.getVAO());

    depthProgram.SetUniform("m", gob.getModelMatrix());
    depthProgram.SetUniform("vp", lightProj * lightView);


    glDrawArrays(GL_TRIANGLES, 0, gob.getVertices().size());

    glBindVertexArray(0);
}

void cubeDepthRenderGameObject(
    GameObject& gob,
    cy::GLSLProgram& cubeDepthProgram,
    PointLight& pointLight
) {
    cubeDepthProgram.Bind();
    glBindVertexArray(gob.getVAO());

    std::vector<cy::Matrix4f> shadowTransorms = pointLight.getMatrices();
    cy::Vec3f lightPos = pointLight.getPosition();
    float farPlane = pointLight.getFarPlane();

    cubeDepthProgram.SetUniform("m", gob.getModelMatrix());

    cubeDepthProgram.SetUniform("farPlane", farPlane);
    cubeDepthProgram.SetUniform("lightPos", lightPos);

    cubeDepthProgram.SetUniform("shadowMatrices", shadowTransorms.data(), 6);

    glDrawArrays(GL_TRIANGLES, 0, gob.getVertices().size());
    glBindVertexArray(0);

}

void recompileShaders() {
    for (auto& GameObject : gameObjects) {
        GameObject.rebuildProgram();
    }
    std::cout << "Recompiled Shaders" << std::endl;
}



void processInputs(GLFWwindow* window, OrbitCamera& camera
    , DirectionalLight& light, PointLight& pointLight) {
    if (numberInput[0]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
    if (numberInput[1]) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    if (numberInput[2]) { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); }

    if (oPressed) {
        recompileShaders();
        oPressed = false;
    }

    if (pPressed) {
        pointLight.setPosition(camera.getCameraPosition());
    }
    if (ctrlPressed) {
        if (lmbPressed) {
            light.setYaw(light.getYaw() - degToRad(mouseDeltaX));
            light.setPitch(light.getPitch() - degToRad(mouseDeltaY));
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
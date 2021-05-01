/*
 * OpenGL version 3.3 project.
 */
#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "camera.hpp"
#include "obj.hpp"
#include "shader.hpp"
#include "text.hpp"

using namespace cg;

struct VertNorm
{
    Vertex vertex;
    Vertex normal;
    VertNorm(GLfloat x_, GLfloat y_, GLfloat z_, GLfloat nx_, GLfloat ny_, GLfloat nz_) : vertex{x_, y_, z_}, normal{nx_, ny_, nz_} {}
    VertNorm(const Vertex& vert, const Vertex& norm) : vertex(vert), normal(norm) {}
};

constexpr const char* const OBJ_FILE = "eight.uniform.obj";

constexpr glm::vec3 GLM_UP(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 GLM_RIGHT(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 GLM_DOWN = -GLM_UP;
constexpr glm::vec3 GLM_LEFT = -GLM_RIGHT;

constexpr GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

// window settings
int screenWidth = 1280;
int screenHeight = 960;

bool keys[1024]{false};

constexpr float COLOR_SPEED = 0.5f;
constexpr float LIGHT_MOVE_SPEED = 0.5f;
constexpr float LAMP_SCALE = 0.2f;

glm::vec3 lightPos(0.0f, 1.5f, 0.0f);
glm::vec3 lightColor{1.0f, 1.0f, 1.0f};
glm::vec3 materialColor{0.5, 1, 0.8};
glm::vec3 textColor{0.8f, 0.7f, 0.3f};
int useFaceNormal = 0;
bool showText = true;

Camera camera(glm::vec3{0, 3, 3}, glm::vec3{0, -1, -1}, 2);

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);
void changeLighting(GLfloat deltaTime);
int bindData(GLuint VAO, GLuint VBO, const Obj& obj, const std::vector<Vertex>& normal);

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Yifei Li - Assignment 6", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Error creating window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// use newly created window as context
	glfwMakeContextCurrent(window);

	// register callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

	// ---------------------------------------------------------------

	// Connect GLAD to GLFW by registerring glfwGetProcAddress() as GLAD loader function,
	// this must be done after setting current context

	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cerr << "Error registerring gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return -2;
	}

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

    // ---------------------------------------------------------------

	// Install GLSL Shader programs
	auto shaderProgram = Shader::Create("face.vert", "face.frag");
	if (shaderProgram == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    auto lightingShader= Shader::Create("material.vert", "material.frag");
    if (lightingShader == nullptr) {
        std::cerr << "Error creating Shader Program" << std::endl;
        glfwTerminate();
        return -3;
    }

    auto lampShader = Shader::Create("lamp.vert", "lamp.frag");
    if (lampShader == nullptr) {
        std::cerr << "Error creating Shader Program" << std::endl;
        glfwTerminate();
        return -3;
    }

    Text arial;
    if (!arial.LoadFont("arial.ttf")) {
        std::cerr << "Error loading font '" << "arial.ttf" << "'" << std::endl;
        glfwTerminate();
        return -5;
    }

    if (!arial.LoadShaders("text.vert", "text.frag")) {
        std::cerr << "Error creating text shaders" << std::endl;
        glfwTerminate();
        return -5;
    }

    // ---------------------------------------------------------------
    // load model
    std::ifstream objfile;
    Obj my_obj;

    // ensures ifstream objects can throw exceptions:
    objfile.exceptions(std::ifstream::badbit);
    try {
        objfile.open(OBJ_FILE, std::ios::in);
        objfile >> my_obj;
        objfile.close();
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "Load obj file '" << OBJ_FILE << "' error: " << e.what() << std::endl;
        glfwTerminate();
        return -4;;
    }

    // ---------------------------------------------------------------

    // compute normals

    std::vector<Vertex> avgNormals;
    std::vector<Vertex> faceNormals;
    avgNormals.resize(my_obj.vertices.size());
    faceNormals.resize(my_obj.vertices.size());

    int nf = my_obj.faces.size();
    for (const TriFace& face : my_obj.faces) {
        glm::vec3 p0(my_obj.vertices[face[0]]);
        glm::vec3 p1(my_obj.vertices[face[1]]);
        glm::vec3 p2(my_obj.vertices[face[2]]);
        glm::vec3 a = p0 - p1, b = p1 - p2;
        Vertex facenormal = Vertex(glm::cross(a, b));
        // face normal
        faceNormals[face[0]] = facenormal;
        faceNormals[face[1]] = facenormal;
        faceNormals[face[2]] = facenormal;
        // average normal
        avgNormals[face[0]] += facenormal;
        avgNormals[face[1]] += facenormal;
        avgNormals[face[2]] += facenormal;
    }

	// ---------------------------------------------------------------

	// Set up vertex data (and buffer(s)) and attribute pointers

	// bind VAO
	GLuint avgVAO, faceVAO;
    glGenVertexArrays(1, &avgVAO);
    glGenVertexArrays(1, &faceVAO);

	// bind VBO, buffer data to it
    GLuint avgVBO, faceVBO;
    glGenBuffers(1, &avgVBO);
    glGenBuffers(1, &faceVBO);
    
    int numAvgVertices = bindData(avgVAO, avgVBO, my_obj, avgNormals);
    int numFaceVertices = bindData(faceVAO, faceVBO, my_obj, faceNormals);

    GLuint lampVAO;
    glGenVertexArrays(1, &lampVAO);
    glBindVertexArray(lampVAO);

    // bind VBO, buffer data to it
    GLuint lampVBO;
    glGenBuffers(1, &lampVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set the vertex attributes (only position data for the lamp))
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the normal vectors
    glEnableVertexAttribArray(0);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

    /*auto model = glm::rotate(
        glm::rotate(glm::mat4(1.0f), glm::radians(50.0f), GLM_UP),
        glm::radians(70.0f), GLM_RIGHT);*/
    auto model = glm::mat4(1.0f);

	// Update loop
    GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
    GLfloat lastFrame = 0.0f;    // Time of last frame

	while (glfwWindowShouldClose(window) == 0) {
        // Calculate deltatime of current frame
        GLfloat currentFrame = GLfloat(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		// check event queue
		glfwPollEvents();

		/* your update code here */
        moveCamera(deltaTime);
        changeLighting(deltaTime);

        auto projection = glm::perspective(glm::radians(camera.Zoom()), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);
        auto lampModel = glm::scale(
            glm::translate(glm::mat4(1.0f), lightPos),
            glm::vec3(LAMP_SCALE)
        );

		// draw background
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader->Use();
        GLint lightPosLoc = glGetUniformLocation(lightingShader->Program(), "light.position");
        GLint viewPosLoc = glGetUniformLocation(lightingShader->Program(), "viewPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.Position()));

        // light properties
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.15f); // low influence
        GLint lightAmbientLoc = glGetUniformLocation(lightingShader->Program(), "light.ambient");
        GLint lightDiffuseLoc = glGetUniformLocation(lightingShader->Program(), "light.diffuse");
        GLint lightSpecularLoc = glGetUniformLocation(lightingShader->Program(), "light.specular");
        glUniform3fv(lightAmbientLoc, 1, glm::value_ptr(ambientColor));
        glUniform3fv(lightDiffuseLoc, 1, glm::value_ptr(diffuseColor));
        glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);

        // material properties
        GLint matAmbientLoc = glGetUniformLocation(lightingShader->Program(), "material.ambient");
        GLint matDiffuseLoc = glGetUniformLocation(lightingShader->Program(), "material.diffuse");
        GLint matSpecularLoc = glGetUniformLocation(lightingShader->Program(), "material.specular");
        GLint matShineLoc = glGetUniformLocation(lightingShader->Program(), "material.shininess");
        glUniform3fv(matAmbientLoc, 1, glm::value_ptr(materialColor));
        glUniform3fv(matDiffuseLoc, 1, glm::value_ptr(materialColor));
        glUniform3f(matSpecularLoc, 0.5f, 0.5f, 0.5f);
        glUniform1f(matShineLoc, 32.0f);

        // Create camera transformations
        glUniformMatrix4fv(glGetUniformLocation(lightingShader->Program(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader->Program(), "view"), 1, GL_FALSE, glm::value_ptr(camera.ViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(glGetUniformLocation(lightingShader->Program(), "useFaceNormal"), useFaceNormal);

        // draw flat color for each face instead of interpolating
        if (useFaceNormal) {
            //glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
            glBindVertexArray(faceVAO);
            glDrawArrays(GL_TRIANGLES, 0, numFaceVertices);
            glBindVertexArray(0);
        } else {
            glBindVertexArray(avgVAO);
            glDrawArrays(GL_TRIANGLES, 0, numAvgVertices);
            glBindVertexArray(0);
        }

        // draw lamp
        lampShader->Use();

        // pass uniform values to shader
        glUniformMatrix4fv(glGetUniformLocation(lampShader->Program(), "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
        glUniformMatrix4fv(glGetUniformLocation(lampShader->Program(), "view"), 1, GL_FALSE, glm::value_ptr(camera.ViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(lampShader->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(lampShader->Program(), "lightColor"), 1, glm::value_ptr(lightColor));

        //glUniform3fv(glGetUniformLocation(shaderProgram->Program(), "myColor"), 1, glm::value_ptr(color));
        //glUniform1i(glGetUniformLocation(shaderProgram->Program(), "useFlat"), flatColor);
        glBindVertexArray(lampVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        if (showText) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glm::mat4 UIprojection = glm::ortho(
                -GLfloat(screenWidth) / 2,
                GLfloat(screenWidth) / 2,
                -GLfloat(screenHeight) / 2,
                GLfloat(screenHeight) / 2,
                -1.0f, 1000.0f
            );

            auto screenOrigin = glm::vec2{-static_cast<GLfloat>(screenWidth) / 2, -static_cast<GLfloat>(screenHeight) / 2};
            arial.RenderText("Use W/A/S/D and mouse to control the camera.", screenOrigin.x + 25, screenOrigin.y + 235, 0.5, UIprojection, textColor);
            arial.RenderText("Use UP/DOWN/LEFT/RIGHT ARROWs and X/Z to change the position of the lamp.", screenOrigin.x + 25, screenOrigin.y + 205, 0.5, UIprojection, textColor);
            arial.RenderText("Press R/T to change the R value of material color.", screenOrigin.x + 25, screenOrigin.y + 175, 0.5, UIprojection, textColor);
            arial.RenderText("Press G/H to change the G value of material color.", screenOrigin.x + 25, screenOrigin.y + 145, 0.5, UIprojection, textColor);
            arial.RenderText("Press B/N to change the B value of material color.", screenOrigin.x + 25, screenOrigin.y + 115, 0.5, UIprojection, textColor);
            arial.RenderText("Current material RGB: (" + std::to_string(materialColor.r) + "," + std::to_string(materialColor.g) + "," + std::to_string(materialColor.b) + ").", screenOrigin.x + 25, screenOrigin.y + 85, 0.5, UIprojection, textColor);
            arial.RenderText("Press CTRL to switch between average normals and face normals.", screenOrigin.x + 25, screenOrigin.y + 55, 0.5, UIprojection, textColor);
            arial.RenderText("Press ALT to turn on/off showing this message.", screenOrigin.x + 25, screenOrigin.y + 25, 0.5, UIprojection, textColor);
        }

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
    glDeleteVertexArrays(1, &faceVAO);
    glDeleteVertexArrays(1, &avgVAO);
    glDeleteVertexArrays(1, &lampVAO);
    glDeleteBuffers(1, &faceVBO);
    glDeleteBuffers(1, &avgVBO);
    glDeleteBuffers(1, &lampVBO);

	glfwTerminate();
	return 0;
}

/* ======================== helper functions ======================== */

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // exit when pressing ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if ((key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) && action == GLFW_PRESS) {
        useFaceNormal = 1 - useFaceNormal;
    } else if ((key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) && action == GLFW_PRESS) {
        showText = !showText;
    } else if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

void moveCamera(GLfloat deltaTime)
{
    // Camera controls
    if (keys[GLFW_KEY_W]) {
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    }
    if (keys[GLFW_KEY_S]) {
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    }
    if (keys[GLFW_KEY_A]) {
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    }
    if (keys[GLFW_KEY_D]) {
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    }
}

void changeLighting(GLfloat deltaTime)
{
    // material controls
    if (keys[GLFW_KEY_R]) {
        materialColor.r += COLOR_SPEED * deltaTime;
        if (materialColor.r > 1.0f) {
            materialColor.r = 1.0f;
        }
    }
    if (keys[GLFW_KEY_T]) {
        materialColor.r -= COLOR_SPEED * deltaTime;
        if (materialColor.r < 0.0f) {
            materialColor.r = 0.0f;
        }
    }
    if (keys[GLFW_KEY_G]) {
        materialColor.g += COLOR_SPEED * deltaTime;
        if (materialColor.g > 1.0f) {
            materialColor.g = 1.0f;
        }
    }
    if (keys[GLFW_KEY_H]) {
        materialColor.g -= COLOR_SPEED * deltaTime;
        if (materialColor.g < 0.0f) {
            materialColor.g = 0.0f;
        }
    }
    if (keys[GLFW_KEY_B]) {
        materialColor.b += COLOR_SPEED * deltaTime;
        if (materialColor.b > 1.0f) {
            materialColor.b = 1.0f;
        }
    }
    if (keys[GLFW_KEY_N]) {
        materialColor.b -= COLOR_SPEED * deltaTime;
        if (materialColor.b < 0.0f) {
            materialColor.b = 0.0f;
        }
    }

    // lamp controls
    if (keys[GLFW_KEY_UP]) {
        lightPos.z -= LIGHT_MOVE_SPEED * deltaTime;
    }
    if (keys[GLFW_KEY_DOWN]) {
        lightPos.z += LIGHT_MOVE_SPEED * deltaTime;
    }
    if (keys[GLFW_KEY_RIGHT]) {
        lightPos.x += LIGHT_MOVE_SPEED * deltaTime;
    }
    if (keys[GLFW_KEY_LEFT]) {
        lightPos.x -= LIGHT_MOVE_SPEED * deltaTime;
    }
    if (keys[GLFW_KEY_Z]) {
        lightPos.y += LIGHT_MOVE_SPEED * deltaTime;
    }
    if (keys[GLFW_KEY_X]) {
        lightPos.y -= LIGHT_MOVE_SPEED * deltaTime;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    static GLfloat lastX = GLfloat(screenWidth) / 2;
    static GLfloat lastY = GLfloat(screenHeight) / 2;

    GLfloat xoffset = GLfloat(xpos) - lastX;
    GLfloat yoffset = lastY - GLfloat(ypos); // Reversed since y-coordinates go from bottom to left
    lastX = GLfloat(xpos);
    lastY = GLfloat(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(GLfloat(yoffset));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    // resize window
    glViewport(0, 0, screenWidth, screenHeight);
}

int bindData(GLuint VAO, GLuint VBO, const Obj& obj, const std::vector<Vertex>& normal)
{
    std::vector<VertNorm> data;
    for (const auto& face : obj.faces) {
        for (int i = 0; i < 3; i++) {
            int vid = face[i];
            data.emplace_back(obj.vertices[vid], normal[vid]);
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertNorm) * data.size(), &data.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertNorm), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertNorm), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return int(data.size());
}

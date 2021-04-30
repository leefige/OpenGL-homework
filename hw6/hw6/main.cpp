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
#include <trimesh2/TriMesh.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "camera.hpp"
#include "shader.hpp"
#include "obj.hpp"

using namespace cg;

struct VertNorm
{
    GLfloat x, y, z;
    GLfloat nx, ny, nz;
    VertNorm(GLfloat x_, GLfloat y_, GLfloat z_, GLfloat nx_, GLfloat ny_, GLfloat nz_) : x(x_), y(y_), z(z_), nx(nx_), ny(ny_), nz(nz_) {}
};

// window settings
int screenWidth = 1280;
int screenHeight = 960;

bool keys[1024]{false};

constexpr glm::vec3 GLM_UP(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 GLM_RIGHT(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 GLM_DOWN = -GLM_UP;
constexpr glm::vec3 GLM_LEFT = -GLM_RIGHT;

glm::vec3 lightColor{1.5f, 1.5f, 1.5f};
glm::vec3 materialColor{0.5, 1, 0.8};
int flatColor = 1;

glm::vec3 lightPos(0.0f, 2.0f, 0.0f);


const char* const OBJ_FILE = "eight.uniform.obj";

Camera camera(glm::vec3{0, 3, 3}, glm::vec3{0, -1, -1}, 3);

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

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "My OpenGL project", nullptr, nullptr);
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

    // ---------------------------------------------------------------
    //// load model
    //std::ifstream objfile;
    //Obj my_obj;

    //// ensures ifstream objects can throw exceptions:
    //objfile.exceptions(std::ifstream::badbit);
    //try {
    //    objfile.open(OBJ_FILE, std::ios::in);
    //    objfile >> my_obj;
    //    objfile.close();
    //}
    //catch (const std::ifstream::failure& e) {
    //    std::cerr << "Load obj file '" << OBJ_FILE << "' error: " << e.what() << std::endl;
    //    glfwTerminate();
    //    return -4;;
    //}

    std::unique_ptr<trimesh::TriMesh> m(trimesh::TriMesh::read(OBJ_FILE));
    if (m == nullptr) {
        std::cerr << "Load obj file '" << OBJ_FILE << "' error." << std::endl;
        glfwTerminate();
        return -4;;
    }

    std::cout << "There are " << m->vertices.size() << " vertices" << std::endl;
    std::cout << "Vertex 0 is at " << m->vertices[0] << std::endl;
    std::cout << "Face 0 has vertices " << m->faces[0][0] << ", "
        << m->faces[0][1] << ", and " << m->faces[0][2] << std::endl;
    m->need_normals(false);
    std::cout << "Vertex 0 has normal " << m->normals[0] << std::endl;


    // ---------------------------------------------------------------

    std::vector<trimesh::point> normals;
    normals.resize(m->vertices.size());
    int nf = m->faces.size();
    for (int i = 0; i < nf; i++) {
        const trimesh::point& p0 = m->vertices[m->faces[i][0]];
        const trimesh::point& p1 = m->vertices[m->faces[i][1]];
        const trimesh::point& p2 = m->vertices[m->faces[i][2]];
        trimesh::vec a = p0 - p1, b = p1 - p2;
        trimesh::vec facenormal = a CROSS b;
        normals[m->faces[i][0]] = facenormal;
        normals[m->faces[i][1]] = facenormal;
        normals[m->faces[i][2]] = facenormal;
    }

	// ---------------------------------------------------------------

	// Set up vertex data (and buffer(s)) and attribute pointers

	// bind VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// bind VBO, buffer data to it
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::vector<VertNorm> data;
    for (const auto& face : m->faces) {
        for (int i = 0; i < 3; i++) {
            int vid = face[i];
            //data.emplace_back(m->vertices[vid][0], m->vertices[vid][1], m->vertices[vid][2], m->normals[vid][0], m->normals[vid][1], m->normals[vid][2]);
            data.emplace_back(m->vertices[vid][0], m->vertices[vid][1], m->vertices[vid][2], normals[vid][0], normals[vid][1], normals[vid][2]);
        }
    }
    int numVertices = int(data.size());

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

    auto lampModel = glm::scale(
        glm::translate(glm::mat4(1.0f), lightPos),
        glm::vec3(0.2f));

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
        auto projection = glm::perspective(glm::radians(camera.Zoom()), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

		// draw background
		GLfloat red = 0.2f;
		GLfloat green = 0.3f;
		GLfloat blue = 0.3f;
		glClearColor(red, green, blue, 1.0f);
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

        glBindVertexArray(VAO);
        // draw flat color for each face instead of interpolating
        if (flatColor) {
            glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
        }
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
        glBindVertexArray(0);


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

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

/* ======================== helper functions ======================== */

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // exit when pressing ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        flatColor = 1 - flatColor;
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

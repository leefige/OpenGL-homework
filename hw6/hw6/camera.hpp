#ifndef CG_CAMERA_H_
#define CG_CAMERA_H_

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg
{
/* An abstract camera class that processes input and calculates the corresponding 
 * Eular Angles, Vectors and Matrices for use in OpenGL
*/
class Camera
{
public:
    // Defines several possible options for camera movement.
    enum class Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    // Constructor with vectors
    Camera(glm::vec3 position, glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
           GLfloat speed = 3.0f, GLfloat mouseSensitivity = 0.25f,
           GLfloat yaw = -90.0f, GLfloat pitch = 0.0f, GLfloat zoom = 45.0f, 
           glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f)) :
        front(front), movementSpeed(speed), mouseSensitivity(mouseSensitivity), zoom(zoom),
        position(position), worldUp(worldUp), yaw(yaw), pitch(pitch)
    {
        this->UpdateCameraCoord();
    }

    /* Getters */
    glm::vec3 Position() const { return this->position; }
    glm::vec3 WorldUp() const { return this->worldUp; }
    GLfloat Speed() const { return this->movementSpeed; }
    GLfloat MouseSensitivity() const { return this->mouseSensitivity; }
    GLfloat Yaw() const { return this->yaw; }
    GLfloat Pitch() const { return this->pitch; }
    GLfloat Zoom() const { return this->zoom; }
    glm::vec3 Front() const { return this->front; }
    glm::vec3 Up() const { return this->up; }
    glm::vec3 Right() const { return this->right; }
    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 ViewMatrix() const { return glm::lookAt(this->position, this->position + this->front, this->up); }

    /* Setters */
    void SetSpeed(GLfloat newSpeed) { this->movementSpeed = newSpeed; }
    void SetMouseSensitivity(GLfloat newSensitivity) { this->mouseSensitivity= newSensitivity; }
    void SetWorldUp(const glm::vec3& newWorldUp) { 
        this->worldUp = newWorldUp;
        this->UpdateCameraCoord();
    }

    /* Callbacks */
    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->movementSpeed * deltaTime;
        if (direction == Movement::FORWARD)
            this->position += this->front * velocity;
        if (direction == Movement::BACKWARD)
            this->position -= this->front * velocity;
        if (direction == Movement::LEFT)
            this->position -= this->right * velocity;
        if (direction == Movement::RIGHT)
            this->position += this->right * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->mouseSensitivity;
        yoffset *= this->mouseSensitivity;

        this->yaw += xoffset;
        this->pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            if (this->pitch > 89.0f) {
                this->pitch = 89.0f;
            } else if (this->pitch < -89.0f) {
                this->pitch = -89.0f;
            }
        }

        // Update front, right and up vectors using the updated Eular angles
        this->UpdateCameraCoord();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset)
    {
        this->zoom -= yoffset;
        // constrains
        if (this->zoom < 15.0f) {
            this->zoom = 15.0f;
        } else if (this->zoom > 45.0f) {
            this->zoom = 45.0f;
        }
    }

private:
    // Camera Attributes
    glm::vec3 worldUp;
    glm::vec3 position;

    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;

    // Eular Angles
    GLfloat yaw;
    GLfloat pitch;

    // Camera options
    GLfloat movementSpeed;
    GLfloat mouseSensitivity;
    GLfloat zoom;

    // Calculates the front vector from the Camera's (updated) Eular Angles
    void UpdateCameraCoord()
    {
        // calculate the new front vector
        glm::vec3 front(cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch)),
                        sin(glm::radians(this->pitch)),
                        sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch)));
        this->front = glm::normalize(front);
        // also update the right and up vector
        // normalize the vector
        this->right = glm::normalize(glm::cross(this->front, this->worldUp));
        this->up = glm::normalize(glm::cross(this->right, this->front));
    }
};

} /* namespace cg */

#endif /* CG_CAMERA_H_ */

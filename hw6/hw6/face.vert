/*
 * GLSL Vertex Shader code for OpenGL version 3.3
 */

#version 330 core

// input vertex attributes
layout (location = 0) in vec3 position;

flat out vec3 flatColor;
out vec3 pureColor;

uniform vec3 myColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
    flatColor = myColor;
    pureColor = myColor;
}

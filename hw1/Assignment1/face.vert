/*
 * GLSL Vertex Shader code for OpenGL version 3.3
 */

#version 330 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

flat out vec3 vertColor;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	vertColor = color;
}

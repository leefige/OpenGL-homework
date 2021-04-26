/*
 * GLSL Vertex Shader code for OpenGL version 4.6
 */

#version 460 core

// input vertex attributes
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
}

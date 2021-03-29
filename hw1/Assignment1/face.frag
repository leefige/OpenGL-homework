/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

flat in vec3 vertColor;

out vec4 color;

void main()
{
	color = vec4(vertColor, 1.0f);
}

/*
 * GLSL Vertex Shader code for OpenGL version 3.3
 */

#version 330 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec2 mapCoord;

uniform mat4 pvm;

void main()
{
	gl_Position = pvm * vec4(position, 1.0);
    mapCoord = texCoord;
}

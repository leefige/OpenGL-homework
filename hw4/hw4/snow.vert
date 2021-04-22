/*
 * GLSL Vertex Shader code for OpenGL version 3.3
 */

#version 330 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 mapCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float scale;

void main()
{
    vec4 scaled = vec4(position.x * scale, position.y * scale, position.z, 1.0);
	gl_Position = projection * view * model * scaled;
	mapCoord = texCoord;
}

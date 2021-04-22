/*
 * GLSL Vertex Shader code for OpenGL version 3.3
 */

#version 330 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 mapCoord;

uniform vec2 offset;

uniform mat4 projection;
uniform mat4 view;

uniform float scale;

void main()
{
	gl_Position = projection * view * vec4((position.xy * scale) + offset, 0, 1.0);
	mapCoord = vec2(texCoord.x, texCoord.y);
}

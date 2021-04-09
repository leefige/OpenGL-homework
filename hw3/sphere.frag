/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

in vec2 mapCoord;

out vec4 color;

uniform sampler2D texMap;

void main()
{
	color = texture(texMap, mapCoord);
}

/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

in vec2 mapCoord;

out vec4 color;

uniform sampler2D texMap;
uniform float alpha;

void main()
{
	color = texture(texMap, mapCoord);
    if (color.r + color.g + color.b >= 2.7f) {
        discard;
    }
    color.a = alpha;
}

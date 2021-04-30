/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

out vec4 color;

flat in vec3 flatColor;
in vec3 pureColor;

uniform int useFlat;

void main()
{
    if (useFlat != 0) {
	    color = vec4(flatColor, 1.0f);
    } else {
	    color = vec4(pureColor, 1.0f);
    }
}

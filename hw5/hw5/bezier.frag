/*
 * GLSL Fragment Shader code for OpenGL version 4.6
 */

#version 460 core

in vec2 texCoord;
out vec4 color;

uniform sampler2D texMap;
uniform bool useTexture;

void main()
{
    if (useTexture) {
	    color = texture(texMap, texCoord);
    } else {
	    color = vec4(0.4f, 0.9f, 0.3f, 1.0f);
    }
}

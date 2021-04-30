/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    FragColor = vec4(lightColor, 1.0f); // set alle 4 vector values to 1.0
}

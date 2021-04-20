/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

in vec2 TexCoords;
in vec4 ParticleColor;

out vec4 color;

uniform sampler2D sprite;

void main(){
    color = texture(sprite, TexCoords) * ParticleColor;
}


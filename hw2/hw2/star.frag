/*
 * GLSL Fragment Shader code for OpenGL version 3.3
 */

#version 330 core

in vec2 TexCoords;
in vec4 ParticleColor;

out vec4 color;

uniform sampler2D sprite;

void main(){
    vec4 texColor = texture(sprite, TexCoords);
    if ((texColor.r + texColor.g + texColor.b) < 0.5) {
        discard;
        texColor.r = 0.0f;
        texColor.g = 0.0f;
        texColor.b = 0.0f;
    }
    color = texColor * ParticleColor;
}


/*
 * GLSL Vertex Shader code for OpenGL version 3.3
 */

#version 330 core

// <vec2 position, vec2 texCoords>
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;

// particle scale
uniform float scale;
// particle position
uniform vec2 offset;
// particle color
uniform vec4 color;

void main(){
    TexCoords = vertex.zw;
    ParticleColor = color;
    // transform by hand
    gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);
}


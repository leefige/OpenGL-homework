#version 460 core

layout(quads, equal_spacing, ccw) in;

out vec2 texCoord;

void main() {

    vec4 p[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            p[j][i] = gl_in[i * 5 + j].gl_Position;
        }
    }

	float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    texCoord = vec2(u, v);

    // Computing the Position, given a u and v
	// the basis functions:
    float bu[5];
    bu[0] = (1.-u) * (1.-u) * (1.-u) * (1.-u);
    bu[1] = 4. * u * (1.-u) * (1.-u) * (1.-u);
    bu[2] = 6. * u * u * (1.-u) * (1.-u);
    bu[3] = 4. * u * u * u * (1.-u);
    bu[4] = u * u * u * u;

    float bv[5];
    bv[0] = (1.-v) * (1.-v) * (1.-v) * (1.-v);
    bv[1] = 4. * v * (1.-v) * (1.-v) * (1.-v);
    bv[2] = 6. * v * v * (1.-v) * (1.-v);
    bv[3] = 4. * v * v * v * (1.-v);
    bv[4] = v * v * v * v;

    // finally, we get to compute something:
    vec4 res = vec4(0., 0., 0., 0.);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            res += bu[i] * bv[j] * p[i][j];
        }
    }
    gl_Position = res;
}

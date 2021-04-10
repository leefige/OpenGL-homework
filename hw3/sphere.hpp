#ifndef CG_SPHERE_H_
#define CG_SPHERE_H_

#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg
{

struct Vertex
{
    // position
    GLfloat x, y, z;
    // normal
    GLfloat nx, ny, nz;
    // texture coord
    GLfloat s, t;

    Vertex(GLfloat x_, GLfloat y_, GLfloat z_, GLfloat nx_, GLfloat ny_, GLfloat nz_, GLfloat s_, GLfloat t_) :
        x(x_), y(y_), z(z_), nx(nx_), ny(ny_), nz(nz_), s(s_), t(t_)
    {
    }
};

struct TriFace
{
    int idx[3];

    TriFace(int f1, int f2, int f3) : idx{f1, f2, f3} {}
};

class UnitSphere
{
public:
    static constexpr double PI = 3.14159265358979;

    std::vector<Vertex> vertices;
    std::vector<TriFace> faces;

    UnitSphere(int sectorCount, int stackCount)
    {
        // Reference: http://www.songho.ca/opengl/gl_sphere.html
        float x, y, z, xz;                              // vertex position
        float s, t;                                     // vertex texCoord

        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xz = cosf(stackAngle);             // r * cos(u)
            y = sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xz * cosf(sectorAngle);             // r * cos(u) * cos(v)
                z = xz * sinf(sectorAngle);             // r * cos(u) * sin(v)
                s = 1 - (float)j / sectorCount;
                t = 1 - (float)i / stackCount;
                vertices.emplace_back(x, y, z, x, y, z, s, t);
            }
        }

        int k1, k2;
        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0) {
                    faces.emplace_back(k1, k2, k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1)) {
                    faces.emplace_back(k1 + 1, k2, k2 + 1);
                }
            }
        }
    }
};

class Sphere
{
public:
	Sphere(float radius, const UnitSphere& unitSphere) :
        Sphere(radius)
	{
        bindUnitSphere(unitSphere);
	}

    Sphere(float radius, int sectorCount, int stackCount) :
        Sphere(radius, UnitSphere(sectorCount, stackCount)) { }

    virtual ~Sphere()
    {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
    }

    const glm::mat4& BaseModel() const { return baseModel_; }

    void DrawSphere() const
    {
        glBindVertexArray(VAO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glDrawElements(GL_TRIANGLES, numIdxSphere_, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

private:
    float radius_;
	GLuint VAO_;
	GLuint VBO_;
	GLuint EBO_;
    int numIdxSphere_;
    glm::mat4 baseModel_;

    Sphere(float radius) :
        numIdxSphere_(0), radius_(radius),
        baseModel_{glm::scale(glm::mat4(1.0f), {radius, radius, radius})}
    {
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);
    }

    void bindUnitSphere(const UnitSphere& unitSphere)
    {
        glBindVertexArray(VAO_);

        // buffer VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, unitSphere.vertices.size() * sizeof(Vertex), &unitSphere.vertices.front(), GL_STATIC_DRAW);

        // buffer EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, unitSphere.faces.size() * sizeof(TriFace), &unitSphere.faces.front(), GL_STATIC_DRAW);

        // set vertex attribute pointers
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        // normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        // texCoord attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        // unbind
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        numIdxSphere_ =  unitSphere.faces.size() * sizeof(TriFace);
    }

};

} /* namespace cg */

#endif // CG_SPHERE_H_

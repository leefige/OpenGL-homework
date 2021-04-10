#ifndef CG_SPHERE_H_
#define CG_SPHERE_H_

#include <vector>
#include <memory>

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

        bindUnitSphere();
    }

    virtual ~UnitSphere()
    {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
    }

    void Draw() const
    {
        glBindVertexArray(VAO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glDrawElements(GL_TRIANGLES, numIdxSphere_, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

private:
    GLuint VAO_;
    GLuint VBO_;
    GLuint EBO_;
    int numIdxSphere_;

    void bindUnitSphere()
    {
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);

        glBindVertexArray(VAO_);

        // buffer VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

        // buffer EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(TriFace), &faces.front(), GL_STATIC_DRAW);

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

        numIdxSphere_ = int(faces.size() * sizeof(TriFace));
    }
};

class Sphere
{
public:
	Sphere(float radius, const std::shared_ptr<UnitSphere>& unitSphere) :
        radius_(radius),
        unitSphere_(unitSphere),
        baseModel_{glm::scale(glm::mat4(1.0f), {radius, radius, radius})} { }

    Sphere(float radius, int sectorCount, int stackCount) :
        Sphere(radius, std::make_shared<UnitSphere>(sectorCount, stackCount)) { }

    const glm::mat4& Model() const { return baseModel_; }

    float Radius() const { return radius_; }

    const std::shared_ptr<UnitSphere>& GetUnitSphere() const { return unitSphere_; }

    void Draw() const { unitSphere_->Draw(); }

private:
    float radius_;
    glm::mat4 baseModel_;
    const std::shared_ptr<UnitSphere> unitSphere_;
};

} /* namespace cg */

#endif // CG_SPHERE_H_

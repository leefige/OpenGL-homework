#ifndef CG_OBJ_H_
#define CG_OBJ_H_

#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

#include <glad/glad.h>

namespace cg
{

template<typename T>
struct Vec3
{
	T v[3];

	Vec3() : v{0, 0, 0} {}
	Vec3(T v1, T v2, T v3) : v{v1, v2, v3} {}
	explicit Vec3(glm::vec3 vec) : v{vec.x, vec.y, vec.z} {}

	Vec3<T>& operator+=(const Vec3& other)
	{
		v[0] += other[0];
		v[1] += other[1];
		v[2] += other[2];
		return *this;
	}

	Vec3<T> operator+(const Vec3& other)
	{
		Vec3 res(*this);
		return res += other;
	}

	T& operator[](int idx)
	{
		return v[idx];
	}

	const T& operator[](int idx) const
	{
		return v[idx];
	}

	operator glm::vec3() const
	{
		return glm::vec3{v[0], v[1], v[2]};
	}
};

typedef Vec3<GLfloat> Vertex;

typedef Vec3<int> TriFace;

class Obj
{
public:
	std::vector<Vertex> vertices;
	std::vector<TriFace> faces;

	int numTriangles() const { return int(faces.size()); }
	int numVertices() const { return int(vertices.size()); }

	friend std::istream& operator>>(std::istream& in, Obj& obj);
};

std::istream& operator>>(std::istream& in, Obj& obj)
{
	std::string buf;
	std::string cmd;
	in.exceptions(std::istream::badbit);
	while (!in.eof()) 	{
		try {
			std::getline(in, buf);
			std::istringstream iss(buf);
			iss >> cmd;

			if (cmd.size() == 0 || cmd[0] == '#' || cmd[0] == '\n') {
				continue;
			}
			else if (cmd.compare("v") == 0) {
				
				GLfloat x, y, z;
				iss >> x >> y >> z;
				obj.vertices.emplace_back(x, y, z);
			}
			else if (cmd.compare("f") == 0) {
				int f[3];
				iss >> f[0] >> f[1] >> f[2];
				obj.faces.emplace_back(f[0] - 1, f[1] - 1, f[2] - 1);
			}
			else {
				std::cerr << "Warning: unsupported line type starting with '" << buf[0] << "'" << std::endl;
				break;
			}
		}
		catch (const std::istream::failure& e) {
			std::cerr << "Reading obj file error: " << e.what() << std::endl;
			break;
		}
	}
	return in;
}

}

#endif /* CG_OBJ_H_ */

#ifndef CG_OBJ_H_
#define CG_OBJ_H_

#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

#include <glad/glad.h>

namespace cg
{

class Obj
{
public:

	struct Tri
	{
		int v[3];
		Tri(int f0, int f1, int f2) { v[0] = f0; v[1] = f1; v[2] = f2; }

		int& operator[](int idx)
		{
			return v[idx];
		}

		const int& operator[](int idx) const
		{
			return v[idx];
		}
	};

	struct Vertex
	{
		GLfloat x, y, z;
		Vertex(GLfloat x_, GLfloat y_, GLfloat z_) : x(x_), y(y_), z(z_) {}
	};

	void bufferData(GLenum target, GLenum usage) const
	{
		std::vector<Vertex> data;
		for (const Tri& face : faces) {
			for (int i = 0; i < 3; i++) {
				int vid = face[i] - 1;
				data.push_back(vertices[vid]);
			}
		}
		glBufferData(target, sizeof(Vertex) * data.size(), &data.front(), usage);
		return;
	}

	int numTriangles() const { return int(faces.size()); }

	int numVertices() const { return int(vertices.size()); }

	friend std::istream& operator>>(std::istream& in, Obj& obj);

private:
	std::vector<Vertex> vertices;
	std::vector<Tri> faces;
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

			if (cmd[0] == '#' || buf[0] == '\n' || buf[0] == '\0') {
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
				obj.faces.emplace_back(f[0], f[1], f[2]);
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

#endif // CG_OBJ_H_

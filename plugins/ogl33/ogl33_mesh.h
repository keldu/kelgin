#pragma once

#include "ogl33_bindings.h"

#include <array>

namespace gin {
class Ogl33Mesh {
private:
	GLuint vao;
	std::array<GLuint,2> ids;
	size_t indices;
	
public:
	Ogl33Mesh();
	Ogl33Mesh(GLuint vao, std::array<GLuint,2>&&, size_t ind);
	~Ogl33Mesh();
	Ogl33Mesh(Ogl33Mesh&&);

	void bindVertexArray() const;

	void bindAttribute() const;
	void bindIndex() const;

	void setData(const MeshData& data);

	size_t indexCount() const;
};

class Ogl33Mesh3d {
private:
	GLuint vao;
	std::array<GLuint, 2> ids;
	size_t indices;

public:
	Ogl33Mesh3d();
	Ogl33Mesh3d(GLuint, std::array<GLuint, 2>&&, size_t);
	~Ogl33Mesh3d();
	Ogl33Mesh3d(Ogl33Mesh3d&&);

	void bindAttribute() const;
	void bindIndex() const;

	void setData(const Mesh3dData& data);

	size_t indexCount() const;
};
}
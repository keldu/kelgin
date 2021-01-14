#include "ogl33_render.h"

namespace gin {

Ogl33Mesh::Ogl33Mesh():
	ids{0,0,0},
	indices{0}
{
}

Ogl33Mesh::Ogl33Mesh(std::array<GLuint, 3>&& i, size_t ind):
	ids{std::move(i)},
	indices{ind}
{}

Ogl33Mesh::~Ogl33Mesh(){
	if(ids[0] > 0){
		glDeleteBuffers(3, &ids[0]);
	}
}

Ogl33Mesh::Ogl33Mesh(Ogl33Mesh&& rhs):
	ids{std::move(rhs.ids)},
	indices{rhs.indices}
{
	rhs.ids = {0,0,0};
	rhs.indices = 0;
}

void Ogl33Mesh::bindVertex() const{
	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
}

void Ogl33Mesh::bindUV() const{
	glBindBuffer(GL_ARRAY_BUFFER, ids[1]);
}

void Ogl33Mesh::bindIndex() const{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[2]);
}

void Ogl33Mesh::setData(const MeshData& data){
	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(float), data.vertices.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, ids[1]);
	glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(float), data.uvs.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_DYNAMIC_DRAW);

// This is unnecessary in a correct renderer impl
#ifndef NDEBUG
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
	indices = data.indices.size();
}

size_t Ogl33Mesh::indexCount() const {
	return indices;
}


Ogl33Mesh3d::Ogl33Mesh3d():
	ids{0,0},
	indices{0}
{}

Ogl33Mesh3d::Ogl33Mesh3d(std::array<GLuint,2>&& i, size_t ind):
	ids{std::move(i)},
	indices{ind}
{}

Ogl33Mesh3d::Ogl33Mesh3d(Ogl33Mesh3d&& rhs):
	ids{std::move(rhs.ids)},
	indices{rhs.indices}
{
	rhs.ids = {0,0};
	rhs.indices = 0;
}

void Ogl33Mesh3d::bindAttribute() const {
	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
}

void Ogl33Mesh3d::bindIndex() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[1]);
}

size_t Ogl33Mesh3d::indexCount() const {
	return indices;
}

Ogl33Mesh3d::~Ogl33Mesh3d(){
	if(ids[0] > 0){
		glDeleteBuffers(2, &ids[0]);
	}
}
}
#include "ogl33_render.h"

namespace gin {

Ogl33Mesh::Ogl33Mesh():
	vao{0},
	ids{0,0},
	indices{0}
{
}

Ogl33Mesh::Ogl33Mesh(GLuint vao, std::array<GLuint, 2>&& i, size_t ind):
	vao{vao},
	ids{std::move(i)},
	indices{ind}
{}

Ogl33Mesh::~Ogl33Mesh(){
	if(ids[0] > 0){
		glDeleteBuffers(2, &ids[0]);
	}
}

Ogl33Mesh::Ogl33Mesh(Ogl33Mesh&& rhs):
	vao{rhs.vao},
	ids{std::move(rhs.ids)},
	indices{rhs.indices}
{
	rhs.vao = 0;
	rhs.ids = {0,0};
	rhs.indices = 0;
}
void Ogl33Mesh::bindVertexArray() const{
	glBindVertexArray(vao);
}

void Ogl33Mesh::bindAttribute() const {
	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
}

void Ogl33Mesh::bindIndex() const{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[1]);
}

void Ogl33Mesh::setData(const MeshData& data){
	bindVertexArray();
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(MeshData::Vertex), data.vertices.data(), GL_DYNAMIC_DRAW);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MeshData::Vertex), nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshData::Vertex), reinterpret_cast<void*>(offsetof(MeshData::Vertex, uvs)));

	#ifndef NDEBUG
		glBindVertexArray(0);
	#endif

	indices = data.indices.size();
}

size_t Ogl33Mesh::indexCount() const {
	return indices;
}


Ogl33Mesh3d::Ogl33Mesh3d():
	vao{0},
	ids{0,0},
	indices{0}
{}

Ogl33Mesh3d::Ogl33Mesh3d(GLuint vao, std::array<GLuint,2>&& i, size_t ind):
	vao{vao},
	ids{std::move(i)},
	indices{ind}
{}


Ogl33Mesh3d::~Ogl33Mesh3d(){
	if(vao){
		glDeleteVertexArrays(1,&vao);
		glDeleteBuffers(2,&ids[0]);
	}
}

Ogl33Mesh3d::Ogl33Mesh3d(Ogl33Mesh3d&& rhs):
	vao{rhs.vao},
	ids{std::move(rhs.ids)},
	indices{rhs.indices}
{
	rhs.vao = 0;
	rhs.ids = {0,0};
	rhs.indices = 0;
}

void Ogl33Mesh3d::bindAttribute() const {
	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
}

void Ogl33Mesh3d::bindIndex() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[1]);
}

void Ogl33Mesh3d::setData(const Mesh3dData& data){
	glBindVertexArray(vao);
	bindAttribute();
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Mesh3dData::Vertex), data.vertices.data(), GL_DYNAMIC_DRAW);

	bindIndex();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh3dData::Vertex), nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh3dData::Vertex), reinterpret_cast<void*>(offsetof(Mesh3dData::Vertex, normals)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh3dData::Vertex), reinterpret_cast<void*>(offsetof(Mesh3dData::Vertex, uvs)));

	#ifndef NDEBUG
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	#endif

	indices = data.indices.size();
}

size_t Ogl33Mesh3d::indexCount() const {
	return indices;
}
}
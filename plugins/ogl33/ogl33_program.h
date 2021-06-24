#pragma once

#include "ogl33_bindings.h"

#include "common/math.h"

namespace gin {
class Ogl33Texture;
class Ogl33Mesh3d;
class Ogl33Program3d {
private:
	GLuint program_id;

	GLuint texture_uniform;
	GLuint mvp_uniform;
public:
	Ogl33Program3d();
	Ogl33Program3d(GLuint program, GLuint texture, GLuint mvp);
	~Ogl33Program3d();

	void setTexture(const Ogl33Texture& texture_id);
	void setMvp(const Matrix<float, 4, 4>& mvp);
	void setMesh(const Ogl33Mesh3d& mesh_id);

	void use();
};

class Ogl33Mesh;
class Ogl33Program {
private:
	GLuint program_id;

	GLuint texture_uniform;
	GLuint mvp_uniform;
	GLuint layer_uniform;
public:
	Ogl33Program();
	Ogl33Program(GLuint, GLuint, GLuint, GLuint);
	~Ogl33Program();

	Ogl33Program(Ogl33Program&&);

	void setTexture(const Ogl33Texture&);
	void setMvp(const Matrix<float,3,3>&);
	void setMesh(const Ogl33Mesh&);
	void setLayer(float);
	void setLayer(int16_t);

	void use();
};
}
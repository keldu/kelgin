#pragma once

#include "ogl33_bindings.h"

#include "common/shapes.h"

#include <vector>

namespace gin {
class Ogl33Texture {
private:
	GLuint tex_id;

	std::vector<Rectangle<float>> sub_textures;
public:
	Ogl33Texture();
	Ogl33Texture(GLuint tex_id);
	~Ogl33Texture();
	Ogl33Texture(Ogl33Texture&&);

	void bind() const;
};
}
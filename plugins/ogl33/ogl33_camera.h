#pragma once

#include "ogl33_bindings.h"

#include "common/math.h"

#include <array>
#include <complex>

namespace gin {

class Ogl33Viewport {
private:
	float x;
	float y;
	float width;
	float height;
public:
	Ogl33Viewport(float x, float y,float w, float h);

	void use();
};

class Ogl33Camera {
private:
	Matrix<float, 3, 3> projection_matrix;

	std::array<float, 2> position = {{0.f, 0.f}};
	std::complex<float> angle = std::polar(1.f, 0.f);

	std::array<float, 2> old_position = {{0.f, 0.f}};
	std::complex<float> old_angle = std::polar(1.f, 0.f);
public:
	Ogl33Camera();

	void setViewPosition(float x, float y);
	void setViewRotation(float angle);

	void updateState(float relative_tp);

	void setOrtho(float left, float right, float top, float bot);

	Matrix<float, 3,3> view(float relative_tp) const;
	const Matrix<float, 3,3>& projection() const;
};

class Ogl33Camera3d {
private:
	Matrix<float, 4, 4> projection_matrix;
	Matrix<float, 4, 4> view_matrix;
public:
	Ogl33Camera3d();

	void setOrtho(float left, float right, float top, float bottom, float near, float far);
	void setViewPosition(float x, float y, float z);
	void setViewRotation(float alpha, float beta, float gamma);

	const Matrix<float, 4, 4>& projection() const;
	const Matrix<float, 4, 4>& view() const;
};
}
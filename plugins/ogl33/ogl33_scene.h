#pragma once

#include "ogl33_bindings.h"

#include "render/render.h"

#include <array>
#include <complex>

namespace gin {
class Ogl33Camera;
class Ogl33Scene {
public:
	struct RenderObject {
		RenderPropertyId id = 0;

		std::array<float,2> pos{{0.f, 0.f}};
		std::complex<float> angle = std::polar(1.f, 0.f);

		std::array<float,2> old_pos{{0.f, 0.f}};
		std::complex<float> old_angle = std::polar(1.f, 0.f);

		float layer = 0.f;
		bool visible = true;
	};
private:
	std::unordered_map<RenderObjectId, RenderObject> objects;
public:

	ErrorOr<RenderObjectId> createObject(const RenderPropertyId& id) noexcept;
	void destroyObject(const RenderObjectId& id) noexcept;
	Error setObjectPosition(const RenderObjectId& id, float x, float y, bool interpolate) noexcept;
	Error setObjectRotation(const RenderObjectId& id, float a, bool interpolate) noexcept;
	Error setObjectVisibility(const RenderObjectId& id, bool v) noexcept;
	Error setObjectLayer(const RenderObjectId& id, float l) noexcept;

	void visit(const Ogl33Camera&, std::vector<RenderObject*>&);

	void updateState(float interval);
};

class Ogl33Camera3d;
class Ogl33Scene3d {
public:
	struct RenderObject {
		RenderProperty3dId id = 0;

		std::array<float, 3> pos{{0.f, 0.f, 0.f}};
		std::array<float, 3> rot{{0.f, 0.f, 0.f}};

		std::array<float, 3> old_pos{{0.f, 0.f, 0.f}};
		std::array<float, 3> old_rot{{0.f, 0.f, 0.f}};

		bool visible = true;

		RenderObject(const RenderProperty3dId& p_id):id{p_id}{}
	};
private:
	std::unordered_map<RenderObject3dId, RenderObject> objects;
public:
	ErrorOr<RenderObject3dId> createObject(const RenderProperty3dId&) noexcept;
	void destroyObject(const RenderObject3dId&) noexcept;

	Error setObjectPosition(const RenderObject3dId&, float, float, float) noexcept;
	Error setObjectRotation(const RenderObject3dId&, float, float, float) noexcept;
	Error setObjectVisibility(const RenderObject3dId&, bool) noexcept;

	void visit(const Ogl33Camera3d&, std::vector<RenderObject>&);

	void updateState();
};
}
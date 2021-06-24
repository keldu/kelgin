#pragma once

#include "../common/id.h"
#include "../common/shapes.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

#include <chrono>
#include <variant>

namespace gin {
using TextureId = ResourceId;
using RenderTargetId = ResourceId;
using RenderTextureId = RenderTargetId;
using RenderWindowId = RenderTargetId;
using RenderViewportId = ResourceId;

using MeshId = ResourceId;
using ProgramId = ResourceId;
using RenderCameraId = ResourceId;
using RenderPropertyId = ResourceId;
using RenderObjectId = ResourceId;
using RenderSceneId = ResourceId;
using RenderStageId = ResourceId;
using RenderAnimationId = ResourceId;

using Mesh3dId = ResourceId;
using Program3dId = ResourceId;
using RenderCamera3dId = ResourceId;
using RenderProperty3dId = ResourceId;
using RenderObject3dId = ResourceId;
using RenderScene3dId = ResourceId;
using RenderStage3dId = ResourceId;
// using RenderAnimation3dId = ResourceId;

class MeshData {
public:
	struct Vertex {
		std::array<float, 2> position;
		std::array<float, 2> uvs;
	};

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class RenderAnimationData2D {
public:
	bool repeat = false;

	struct Frame{
		size_t key_frame;
		std::chrono::milliseconds length;
	};

	std::vector<Frame> frames;
};

class Mesh3dData {
public:
	struct Vertex {
		std::array<float, 3> position;
		std::array<float, 3> normals;
		std::array<float, 2> uvs;
	};

	static_assert(sizeof(Vertex) == sizeof(float)*8, "Mesh3dData::Vertex is not continuously set");

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

/// @todo build a valid image on default or at least don't leave it
/// undefined
class Image {
public:
	size_t width = 0, height = 0;
	std::vector<uint8_t> pixels;
	uint8_t channels = 0;

	std::vector<Rectangle<float>> sub_images;
};

struct RenderEvent {
	struct Keyboard {
		uint32_t key_code;
		bool pressed;
		bool repeat;
	};

	struct Resize {
		size_t width;
		size_t height;
	};

	struct Mouse {
		uint16_t button;
		bool pressed;
	};

	struct MouseMove {
		uint32_t x;
		uint32_t y;
	};

	using Events = std::variant<Keyboard, Resize, Mouse, MouseMove>;
};

struct RenderVideoMode {
	size_t width;
	size_t height;
};

/// @todo Change from Error returns to Conveyor

/// @todo Add a better timer. The sleeping call needs to know when it should wake up
/// And Render should know when one of its windows needs to render.
class LowLevelRender2D {
protected:
	~LowLevelRender2D() = default;
public:
	// Mesh Operations
	virtual Conveyor<MeshId> createMesh(const MeshData&) noexcept = 0;
	virtual Conveyor<void> setMeshData(const MeshId&, const MeshData&) noexcept = 0;
	virtual Conveyor<void> destroyMesh(const MeshId&) noexcept = 0;

	// Program Operations
	virtual Conveyor<ProgramId> createProgram(const std::string& vertex_src, const std::string& fragment_src) noexcept = 0;
	virtual Conveyor<ProgramId> createProgram() noexcept = 0;
	virtual Conveyor<void> destroyProgram(const ProgramId&) noexcept = 0;

	// Camera Operations
	virtual Conveyor<RenderCameraId> createCamera() noexcept = 0;
	virtual Conveyor<void> setCameraPosition(const RenderCameraId&, float x, float y) noexcept = 0;
	virtual Conveyor<void> setCameraRotation(const RenderCameraId&, float alpha) noexcept = 0;
	virtual Conveyor<void> setCameraOrthographic(const RenderCameraId&, float, float, float, float) noexcept = 0;
	virtual Conveyor<void> destroyCamera(const RenderCameraId&) noexcept = 0;

	// Property Operations
	virtual Conveyor<RenderPropertyId> createProperty(const MeshId&, const TextureId&) noexcept = 0;
	virtual Conveyor<void> setPropertyMesh(const RenderPropertyId&, const MeshId& id) noexcept = 0;
	virtual Conveyor<void> setPropertyTexture(const RenderPropertyId&, const TextureId& id) noexcept = 0;
	virtual Conveyor<void> destroyProperty(const RenderPropertyId&) noexcept = 0;

	// Scene and Object Operations
	virtual Conveyor<RenderSceneId> createScene() noexcept = 0;
	virtual Conveyor<RenderObjectId> createObject(const RenderSceneId&, const RenderPropertyId&) noexcept = 0;
	virtual Conveyor<void> destroyObject(const RenderSceneId&, const RenderObjectId&) noexcept = 0;
	virtual Conveyor<void> setObjectPosition(const RenderSceneId&, const RenderObjectId&, float, float, bool interpolate = true) noexcept = 0;
	virtual Conveyor<void> setObjectRotation(const RenderSceneId&, const RenderObjectId&, float, bool interpolate = true) noexcept = 0;
	virtual Conveyor<void> setObjectVisibility(const RenderSceneId&, const RenderObjectId&, bool) noexcept = 0;
	virtual Conveyor<void> setObjectLayer(const RenderSceneId& id, const RenderObjectId&, float) noexcept = 0;
	virtual Conveyor<void> destroyScene(const RenderSceneId&) noexcept = 0;

	// Stage Operations
	virtual Conveyor<RenderStageId> createStage(const RenderTargetId& id, const RenderViewportId&, const RenderSceneId&, const RenderCameraId&, const ProgramId&) noexcept = 0;
	virtual Conveyor<void> destroyStage(const RenderStageId&) noexcept = 0;

	// Animation Operations
	virtual Conveyor<RenderAnimationId> createAnimation(const RenderAnimationData2D& data) noexcept = 0;
	virtual Conveyor<void> destroyAnimation(const RenderAnimationId&) noexcept = 0;
	virtual Conveyor<void> playAnimation(const RenderSceneId& id, const RenderObjectId& obj, const RenderAnimationId&) noexcept = 0;
};

class LowLevelRender3D {
protected:
	~LowLevelRender3D() = default;
public:
	// Mesh3d Operations
	virtual Conveyor<Mesh3dId> createMesh3d(const Mesh3dData&) noexcept = 0;
	virtual Conveyor<void> destroyMesh3d(const Mesh3dId&) noexcept = 0;

	// Property3d Operations
	virtual Conveyor<RenderProperty3dId> createProperty3d(const Mesh3dId&, const TextureId&) noexcept = 0;
	virtual Conveyor<void> destroyProperty3d(const RenderProperty3dId&) noexcept = 0;

	// Program3d Operations
	virtual Conveyor<Program3dId> createProgram3d(const std::string& vertex_src, const std::string& fragment_src) noexcept = 0;
	virtual Conveyor<Program3dId> createProgram3d() noexcept = 0;
	virtual Conveyor<void> destroyProgram3d(const Program3dId&) noexcept = 0;
	
	// Camera3d Operations
	virtual Conveyor<RenderCamera3dId> createCamera3d() noexcept = 0;
	virtual Conveyor<void> setCamera3dPosition(const RenderCamera3dId&, float, float, float) noexcept = 0;
	virtual Conveyor<void> setCamera3dOrthographic(const RenderCamera3dId&, float, float, float, float, float, float) noexcept = 0;
	virtual Conveyor<void> destroyCamera3d(const RenderCamera3dId&) noexcept = 0;
	
	// Scene3d and Object3d Operations
	virtual Conveyor<RenderScene3dId> createScene3d() noexcept = 0;
	virtual Conveyor<RenderObject3dId> createObject3d(const RenderScene3dId&, const RenderProperty3dId&) noexcept = 0;
	virtual Conveyor<void> destroyObject3d(const RenderScene3dId&, const RenderObject3dId&) noexcept = 0;
	virtual Conveyor<void> destroyScene3d(const RenderScene3dId&) noexcept = 0;

	// Stage3d Operations
	virtual Conveyor<RenderStage3dId> createStage3d(const RenderTargetId&, const RenderViewportId&, const RenderScene3dId&, const RenderCamera3dId&, const Program3dId&) noexcept = 0;
	virtual Conveyor<void> destroyStage3d(const RenderStage3dId&) noexcept = 0;

	// Animation3d Operations
	// virtual Conveyor<RenderAnimation3d>
};


class LowLevelRender {
protected:
public:
	virtual ~LowLevelRender() = default;

	virtual LowLevelRender2D* interface2D() noexcept = 0;
	virtual LowLevelRender3D* interface3D() noexcept = 0;

	// Texture Operations
	virtual Conveyor<TextureId> createTexture(const Image&) noexcept = 0;
	virtual Conveyor<void> destroyTexture(const TextureId&) noexcept = 0;

	// Window Operations
	virtual Conveyor<RenderWindowId> createWindow(const RenderVideoMode&, const std::string& title) noexcept = 0;
	virtual Conveyor<void> setWindowDesiredFPS(const RenderWindowId&, float fps) noexcept = 0;
	virtual Conveyor<void> setWindowVisibility(const RenderWindowId& id, bool show) noexcept = 0;
	virtual Conveyor<void> destroyWindow(const RenderWindowId& id) noexcept = 0;

	virtual Conveyor<RenderEvent::Events> listenToWindowEvents(const RenderWindowId&) noexcept = 0;

	// Viewport Operations
	virtual Conveyor<RenderViewportId> createViewport() noexcept = 0;
	virtual Conveyor<void> setViewportRect(const RenderViewportId&, float, float, float, float) noexcept = 0;
	virtual Conveyor<void> destroyViewport(const RenderViewportId&) noexcept = 0;

	/// @todo change time_point to microseconds and independent to steady_clock type
	virtual void step(const std::chrono::steady_clock::time_point&) noexcept = 0;
	virtual void flush() noexcept = 0;

	virtual void updateTime(const std::chrono::steady_clock::time_point& new_old_time_point, const std::chrono::steady_clock::time_point& new_time_point) noexcept = 0;
};
}

/*
* Don't implement this here
* This is meant for plugins
*/
extern "C" {
gin::LowLevelRender* createRenderer(gin::IoProvider& io_provider);
void destroyRenderer(gin::LowLevelRender* render);
}

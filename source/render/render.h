#pragma once

#include "../common/id.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

#include <chrono>
#include <variant>

namespace gin {
using MeshId = ResourceId;
using TextureId = ResourceId;
using ProgramId = ResourceId;
using RenderPropertyId = ResourceId;
using RenderObjectId = ResourceId;
using RenderWorldId = ResourceId;
using RenderSceneId = ResourceId;
using RenderTargetId = ResourceId;
using RenderTextureId = RenderTargetId;
using RenderWindowId = RenderTargetId;

using RenderCameraId = ResourceId;
using RenderStageId = ResourceId;
using RenderViewportId = ResourceId;

using Mesh3dId = ResourceId;
using Program3dId = ResourceId;
using RenderCamera3dId = ResourceId;
using RenderStage3dId = ResourceId;
using RenderProperty3dId = ResourceId;
using RenderScene3dId = ResourceId;
using RenderObject3dId = ResourceId;

class MeshData {
public:
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;
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

/// @todo Add a better timer. The sleeping call needs to know when it should wake up
/// And Render should know when one of its windows needs to render.
class LowLevelRender {
protected:
public:
	virtual ~LowLevelRender() = default;

	// Mesh Operations
	virtual ErrorOr<MeshId> createMesh(const MeshData&) noexcept = 0;
	virtual Error setMeshData(const MeshId&, const MeshData&) noexcept = 0;
	virtual Error destroyMesh(const MeshId&) noexcept = 0;

	// Texture Operations
	virtual ErrorOr<TextureId> createTexture(const Image&) noexcept = 0;
	virtual Error destroyTexture(const TextureId&) noexcept = 0;

	// Window Operations
	virtual ErrorOr<RenderWindowId> createWindow(const RenderVideoMode&, const std::string& title) noexcept = 0;
	virtual Error setWindowDesiredFPS(const RenderWindowId&, float fps) noexcept = 0;
	virtual Error setWindowVisibility(const RenderWindowId& id, bool show) noexcept = 0;
	virtual Error destroyWindow(const RenderWindowId& id) noexcept = 0;

	virtual Conveyor<RenderEvent::Events> listenToWindowEvents(const RenderWindowId&) noexcept = 0;

	// Program Operations
	virtual ErrorOr<ProgramId> createProgram(const std::string& vertex_src, const std::string& fragment_src) noexcept = 0;
	virtual ErrorOr<ProgramId> createProgram() noexcept = 0;
	virtual Error destroyProgram(const ProgramId&) noexcept = 0;

	// Camera Operations
	virtual ErrorOr<RenderCameraId> createCamera() noexcept = 0;
	virtual Error setCameraPosition(const RenderCameraId&, float x, float y) noexcept = 0;
	virtual Error setCameraRotation(const RenderCameraId&, float alpha) noexcept = 0;
	virtual Error setCameraOrthographic(const RenderCameraId&, float, float, float, float) noexcept = 0;
	virtual Error destroyCamera(const RenderCameraId&) noexcept = 0;

	// Stage Operations
	virtual ErrorOr<RenderStageId> createStage(const RenderTargetId& id, const RenderSceneId&, const RenderCameraId&, const ProgramId&) noexcept = 0;
	virtual Error destroyStage(const RenderStageId&) noexcept = 0;

	// Viewport Operations
	virtual ErrorOr<RenderViewportId> createViewport() noexcept = 0;
	virtual Error setViewportRect(const RenderViewportId&, float, float, float, float) noexcept = 0;
	virtual Error destroyViewport(const RenderViewportId&) noexcept = 0;

	// Property Operations
	virtual ErrorOr<RenderPropertyId> createProperty(const MeshId&, const TextureId&) noexcept = 0;
	virtual Error setPropertyMesh(const RenderPropertyId&, const MeshId& id) noexcept = 0;
	virtual Error setPropertyTexture(const RenderPropertyId&, const TextureId& id) noexcept = 0;
	virtual Error destroyProperty(const RenderPropertyId&) noexcept = 0;

	// Scene and Object Operations
	virtual ErrorOr<RenderSceneId> createScene() noexcept = 0;
	virtual ErrorOr<RenderObjectId> createObject(const RenderSceneId&, const RenderPropertyId&) noexcept = 0;
	virtual Error destroyObject(const RenderSceneId&, const RenderObjectId&) noexcept = 0;
	virtual Error setObjectPosition(const RenderSceneId&, const RenderObjectId&, float, float) noexcept = 0;
	virtual Error setObjectRotation(const RenderSceneId&, const RenderObjectId&, float) noexcept = 0;
	virtual Error setObjectVisibility(const RenderSceneId&, const RenderObjectId&, bool) noexcept = 0;
	virtual Error destroyScene(const RenderSceneId&) noexcept = 0;

	//
	// 3D Interface
	//
	// Mesh3d Operations
	virtual ErrorOr<Mesh3dId> createMesh3d(const Mesh3dData&) noexcept = 0;
	virtual Error destroyMesh3d(const Mesh3dId&) noexcept = 0;
	
	// Property3d Operations
	virtual ErrorOr<RenderProperty3dId> createProperty3d(const Mesh3dId&, const TextureId&) noexcept = 0;
	virtual Error destroyProperty3d(const RenderProperty3dId&) noexcept = 0;
	
	// Program3d Operations
	virtual ErrorOr<Program3dId> createProgram3d(const std::string& vertex_src, const std::string& fragment_src) noexcept = 0;
	virtual ErrorOr<Program3dId> createProgram3d() noexcept = 0;
	virtual Error destroyProgram3d(const Program3dId&) noexcept = 0;
	
	// Camera3d Operations
	virtual ErrorOr<RenderCamera3dId> createCamera3d() noexcept = 0;
	virtual Error setCamera3dPosition(const RenderCamera3dId&, float, float, float) noexcept = 0;
	virtual Error setCamera3dOrthographic(const RenderCamera3dId&, float, float, float, float, float, float) noexcept = 0;
	virtual Error destroyCamera3d(const RenderCamera3dId&) noexcept = 0;
	
	// Scene3d and Object3d Operations
	virtual ErrorOr<RenderScene3dId> createScene3d() noexcept = 0;
	virtual ErrorOr<RenderObject3dId> createObject3d(const RenderScene3dId&, const RenderProperty3dId&) noexcept = 0;
	virtual Error destroyObject3d(const RenderScene3dId&, const RenderObject3dId&) noexcept = 0;
	virtual Error destroyScene3d(const RenderScene3dId&) noexcept = 0;
	//

	virtual ErrorOr<RenderStage3dId> createStage3d(const RenderTargetId&, const RenderScene3dId&, const RenderCamera3dId&, const Program3dId&) noexcept = 0;
	virtual Error destroyStage3d(const RenderStage3dId&) noexcept = 0;

	/// @todo change time_point to microseconds and independent to steady_clock type
	virtual void step(const std::chrono::steady_clock::time_point&) noexcept = 0;
	virtual void flush() noexcept = 0;

	virtual void updateTime(const std::chrono::steady_clock::time_point&) noexcept = 0;
};
}

/*
* Don't implement this here
* This is meant for plugins
*/
extern "C" {
gin::LowLevelRender* createRenderer(gin::AsyncIoProvider& io_provider);
void destroyRenderer(gin::LowLevelRender* render);
}

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
	virtual ErrorOr<MeshId> createMesh(const MeshData&) = 0;
	virtual Error setMeshData(const MeshId&, const MeshData&) = 0;
	virtual Error destroyMesh(const MeshId&) = 0;

	// Texture Operations
	virtual ErrorOr<TextureId> createTexture(const Image&) = 0;
	virtual Error destroyTexture(const TextureId&) = 0;

	// Window Operations
	virtual ErrorOr<RenderWindowId> createWindow(const RenderVideoMode&, const std::string& title) = 0;
	virtual Error setWindowDesiredFPS(const RenderWindowId&, float fps) = 0;
	virtual Error setWindowVisibility(const RenderWindowId& id, bool show) = 0;
	virtual Conveyor<RenderEvent::Events> listenToWindowEvents(const RenderWindowId&) = 0;
	virtual Error destroyWindow(const RenderWindowId& id) = 0;

	// Program Operations
	virtual ErrorOr<ProgramId> createProgram(const std::string& vertex_src, const std::string& fragment_src) = 0;
	virtual ErrorOr<ProgramId> createProgram() = 0;
	virtual Error destroyProgram(const ProgramId&) = 0;

	// Camera Operations
	virtual ErrorOr<RenderCameraId> createCamera() = 0;
	virtual Error setCameraPosition(const RenderCameraId&, float x, float y) = 0;
	virtual Error setCameraRotation(const RenderCameraId&, float alpha) = 0;
	virtual Error setCameraOrthographic(const RenderCameraId&, float, float, float, float) = 0;
	virtual Error destroyCamera(const RenderCameraId&) = 0;

	// Stage Operations
	virtual ErrorOr<RenderStageId> createStage(const RenderTargetId& id, const RenderSceneId&, const RenderCameraId&, const ProgramId&) = 0;
	virtual Error destroyStage(const RenderStageId&) = 0;

	// Viewport Operations
	virtual ErrorOr<RenderViewportId> createViewport() = 0;
	virtual Error setViewportRect(const RenderViewportId&, float, float, float, float) = 0;
	virtual Error destroyViewport(const RenderViewportId&) = 0;

	// Property Operations
	virtual ErrorOr<RenderPropertyId> createProperty(const MeshId&, const TextureId&) = 0;
	virtual Error setPropertyMesh(const RenderPropertyId&, const MeshId& id) = 0;
	virtual Error setPropertyTexture(const RenderPropertyId&, const TextureId& id) = 0;
	virtual Error destroyProperty(const RenderPropertyId&) = 0;

	// Scene and Object Operations
	virtual ErrorOr<RenderSceneId> createScene() = 0;
	virtual ErrorOr<RenderObjectId> createObject(const RenderSceneId&, const RenderPropertyId&) = 0;
	virtual Error destroyObject(const RenderSceneId&, const RenderObjectId&) = 0;
	virtual Error setObjectPosition(const RenderSceneId&, const RenderObjectId&, float, float) = 0;
	virtual Error setObjectRotation(const RenderSceneId&, const RenderObjectId&, float) = 0;
	virtual Error setObjectVisibility(const RenderSceneId&, const RenderObjectId&, bool) = 0;
	virtual Error destroyScene(const RenderSceneId&) = 0;

	//
	// 3D Interface
	//
	// Mesh3d Operations
	virtual Mesh3dId createMesh3d(const Mesh3dData&) = 0;
	virtual Error destroyMesh3d(const Mesh3dId&) = 0;
	
	// Property3d Operations
	virtual ErrorOr<RenderProperty3dId> createProperty3d(const Mesh3dId&, const TextureId&) = 0;
	virtual Error destroyProperty3d(const RenderProperty3dId&) = 0;
	
	// Program3d Operations
	virtual ErrorOr<Program3dId> createProgram3d(const std::string& vertex_src, const std::string& fragment_src) = 0;
	virtual ErrorOr<Program3dId> createProgram3d() = 0;
	virtual Error destroyProgram3d(const Program3dId&) = 0;
	
	// Camera3d Operations
	virtual ErrorOr<RenderCamera3dId> createCamera3d() = 0;
	virtual Error setCamera3dPosition(const RenderCamera3dId&, float, float, float) = 0;
	virtual Error setCamera3dOrthographic(const RenderCamera3dId&, float, float, float, float, float, float) = 0;
	virtual Error destroyCamera3d(const RenderCamera3dId&) = 0;
	
	// Scene3d and Object3d Operations
	virtual ErrorOr<RenderScene3dId> createScene3d() = 0;
	virtual ErrorOr<RenderObject3dId> createObject3d(const RenderScene3dId&, const RenderProperty3dId&) = 0;
	virtual Error destroyObject3d(const RenderScene3dId&, const RenderObject3dId&) = 0;
	virtual Error destroyScene3d(const RenderScene3dId&) = 0;
	//

	virtual ErrorOr<RenderStage3dId> createStage3d(const RenderTargetId&, const RenderScene3dId&, const RenderCamera3dId&, const Program3dId&) = 0;
	virtual Error destroyStage3d(const RenderStage3dId&) = 0;

	/// @todo change time_point to microseconds and independent to steady_clock type
	virtual void step(const std::chrono::steady_clock::time_point&) = 0;
	virtual void flush() = 0;

	virtual void updateTime(const std::chrono::steady_clock::time_point&) = 0;
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

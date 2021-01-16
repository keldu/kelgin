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

class MeshData {
public:
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;
};

class Mesh3dData {
public:
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;
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
	virtual MeshId createMesh(const MeshData&) = 0;
	virtual void setMeshData(const MeshId&, const MeshData&) = 0;
	virtual void destroyMesh(const MeshId&) = 0;

	// Texture Operations
	virtual TextureId createTexture(const Image&) = 0;
	virtual void destroyTexture(const TextureId&) = 0;

	// Window Operations
	virtual RenderWindowId createWindow(const RenderVideoMode&, const std::string& title) = 0;
	virtual void setWindowDesiredFPS(const RenderWindowId&, float fps) = 0;
	virtual void setWindowVisibility(const RenderWindowId& id, bool show) = 0;
	virtual Conveyor<RenderEvent::Events> listenToWindowEvents(const RenderWindowId&) = 0;
	virtual void destroyWindow(const RenderWindowId& id) = 0;

	// Program Operations
	virtual ProgramId createProgram(const std::string& vertex_src, const std::string& fragment_src) = 0;
	virtual ProgramId createProgram() = 0;
	virtual void destroyProgram(const ProgramId&) = 0;

	// Camera Operations
	virtual RenderCameraId createCamera() = 0;
	virtual void setCameraPosition(const RenderCameraId&, float x, float y) = 0;
	virtual void setCameraRotation(const RenderCameraId&, float alpha) = 0;
	virtual void setCameraOrthographic(const RenderCameraId&, float, float, float, float) = 0;
	virtual void destroyCamera(const RenderCameraId&) = 0;

	// Stage Operations
	virtual RenderStageId createStage(const RenderTargetId& id, const RenderSceneId&, const RenderCameraId&, const ProgramId&) = 0;
	virtual void destroyStage(const RenderStageId&) = 0;

	// Viewport Operations
	virtual RenderViewportId createViewport() = 0;
	virtual void setViewportRect(const RenderViewportId&, float, float, float, float) = 0;
	virtual void destroyViewport(const RenderViewportId&) = 0;

	// Property Operations
	virtual RenderPropertyId createProperty(const MeshId&, const TextureId&) = 0;
	virtual void setPropertyMesh(const RenderPropertyId&, const MeshId& id) = 0;
	virtual void setPropertyTexture(const RenderPropertyId&, const TextureId& id) = 0;
	virtual void destroyProperty(const RenderPropertyId&) = 0;

	// Scene and Object Operations
	virtual RenderSceneId createScene() = 0;
	virtual RenderObjectId createObject(const RenderSceneId&, const RenderPropertyId&) = 0;
	virtual void destroyObject(const RenderSceneId&, const RenderObjectId&) = 0;
	virtual void setObjectPosition(const RenderSceneId&, const RenderObjectId&, float, float) = 0;
	virtual void setObjectRotation(const RenderSceneId&, const RenderObjectId&, float) = 0;
	virtual void destroyScene(const RenderSceneId&) = 0;

	// Mesh3d Operations
	virtual Mesh3dId createMesh3d(const Mesh3dData&) = 0;
	virtual void destroyMesh3d(const Mesh3dId&) = 0;
	
	// Camera3d Operations
	// virtual RenderCamera3dId createCamera3d() = 0;
	// virtual void setCameraPosition(const RenderCamera3dId&, float, float, float) = 0;
	// virtual void setCameraOrthographic(const RenderCamera3dId&, float, float, float, float, float, float) = 0;
	// virtual void destroyCamera3d(const RenderCamera3dId&) = 0;
	
	// Program3d Operations
	//
	//
	
	// Property3d Operations
	//
	//
	
	// Scene3d and Object3d Operations
	//
	//

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

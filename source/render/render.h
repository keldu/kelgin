#pragma once

#include "common/id.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

#include <chrono>

namespace gin {
using MeshId = ResourceId;
using TextureId = ResourceId;
using ProgramId = ResourceId;
using RenderObjectId = ResourceId;
using RenderWorldId = ResourceId;
using RenderSceneId = ResourceId;
using RenderTargetId = ResourceId;
using RenderTextureId = RenderTargetId;
using RenderWindowId = RenderTargetId;

using RenderCameraId = ResourceId;
using RenderStageId = ResourceId;
using RenderViewportId = ResourceId;


class MeshData {
public:
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<int> indices;
};

class Image {
public:
	size_t width, height;
	std::vector<float> pixels;
	uint8_t channels;
};

class RenderScene {
public:
	virtual ~RenderScene() = default;

	virtual void attachObjectToScene(const RenderObjectId&) = 0;
	virtual void detachObjectFromScene(const RenderObjectId&) = 0;
	virtual void setObjectPosition(const RenderObjectId&, float, float) = 0;
	virtual void setObjectRotation(const RenderObjectId&, float) = 0;
};

class RenderWorld {
public:
	virtual ~RenderWorld() = default;

	virtual RenderObjectId createObject(const MeshId&, const TextureId&) = 0;
	virtual void destroyObject(const RenderObjectId&) = 0;

	virtual Own<RenderScene> createScene() = 0;
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

	virtual MeshId createMesh(const MeshData&) = 0;
	virtual void destroyMesh(const MeshId&) = 0;

	virtual TextureId createTexture(const Image&) = 0;
	virtual void destroyTexture(const TextureId&) = 0;

	virtual Own<RenderWorld> createWorld() = 0;
	// virtual void destroyRenderWorld(const RenderWorld&) = 0;

	virtual RenderWindowId createWindow(const RenderVideoMode&, const std::string& title) = 0;
	virtual void setWindowDesiredFPS(const RenderWindowId&, float fps) = 0;
	virtual void setWindowVisibility(const RenderWindowId& id, bool show) = 0;
	virtual void destroyWindow(const RenderWindowId& id) = 0;

	virtual ProgramId createProgram(const std::string& vertex_src, const std::string& fragment_src) = 0;
	virtual void destroyProgram(const ProgramId&) = 0;

	virtual RenderCameraId createCamera() = 0;
	virtual void setCameraPosition(const RenderCameraId&, float x, float y) = 0;
	virtual void setCameraRotation(const RenderCameraId&, float alpha) = 0;
	virtual void destroyCamera(const RenderCameraId&) = 0;

	virtual RenderStageId createStage(const RenderTargetId& id, const RenderSceneId&, const RenderCameraId&) = 0;
	virtual void destroyStage(const RenderStageId&) = 0;

	virtual RenderViewportId createViewport() = 0;
	virtual void setViewportRect(const RenderViewportId&, float, float, float, float) = 0;
	virtual void destroyViewport(const RenderViewportId&) = 0;

	virtual void step(const std::chrono::steady_clock::time_point&) = 0;
	virtual void flush() = 0;
};
}

/*
* Don't implement this here
* This is meant for plugins
*/
extern "C" {
gin::Render* createRenderer(gin::AsyncIoProvider& io_provider);
void destroyRenderer(gin::Render* render);
}

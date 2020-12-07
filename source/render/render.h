#pragma once

#include "common/id.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

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

class Render;
class RenderScene {

};

class MeshData {
public:
	std::vector<float> meshes, uvs;
	std::vector<int> indices;
};

class Image {
public:
	size_t width, height;
	std::vector<float> pixels;
	uint8_t channels;
};

class RenderWorld {
public:
	virtual ~RenderWorld() = default;

	virtual RenderObjectId createObject(const MeshId&, const TextureId&) = 0;
	virtual void destroyObject(const RenderObjectId&) = 0;
};

class Render {
protected:
public:
	virtual ~Render() = default;

	virtual MeshId createMesh(const MeshData&) = 0;
	virtual void destroyMesh(const MeshId&) = 0;

	virtual TextureId createTexture(const Image&) = 0;
	virtual void destroyTexture(const TextureId&) = 0;

	virtual Own<RenderWorld> createWorld() = 0;
	// virtual void destroyRenderWorld(const RenderWorld&) = 0;

	virtual RenderWindowId createWindow() = 0;
	virtual void destroyWindow(const RenderWindowId& id) = 0;

	virtual void setWindowVisibility(const RenderWindowId& id, bool show) = 0;

	virtual void step() = 0;
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

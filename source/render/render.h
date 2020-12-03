#pragma once

#include "common/id.h"

#include <kelgin/async.h>

namespace gin {
using MeshId = ResourceId;
using TextureId = ResourceId;
using ProgramId = ResourceId;
using RenderObjectId = ResourceId;
using RenderWorldId = ResourceId;
using RenderSceneId = ResourceId;
using RenderTargetId = ResourceId;

class MeshRef {
private:
	MeshId mesh_id;
public:
};

class TextureRef {
private:
	TextureId tex_id;
public:
};

class RenderObjectRef {
private:
	RenderObjectId obj_id;
public:
};

class RenderSceneRef {
public:
};

class RenderWorldRef {
public:
};

class Render;
class RenderScene {

};

class RenderWorld {
public:
	virtual ~RenderWorld() = default;
};

class Render {
protected:
public:
	virtual ~Render() = default;

	//virtual RenderObjectId createRenderObject(const RenderWorldId&, const MeshId&, const TextureId&) = 0;
	//virtual void destroyRenderObject(const RenderObjectId&) = 0;

	virtual Own<RenderWorld> createWorld() = 0;
	// virtual void destroyRenderWorld(const RenderWorld&) = 0;

	virtual RenderWindowId
};
}

/*
* Don't implement this here
* This is meant for plugins
*/
extern "C" {
gin::Render* createRenderer();
void destroyRenderer(gin::Render* render);
}
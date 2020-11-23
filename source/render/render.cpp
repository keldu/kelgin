#include "render.h"

namespace gin {
RenderWorld::RenderWorld(Render& r):
	renderer{&r}
{}

RenderWorld::~RenderWorld(){}

void RenderWorld::destroyRenderWorld(const RenderWorld& rw){

}
}
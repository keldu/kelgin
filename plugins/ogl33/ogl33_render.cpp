#include "ogl33_render.h"

#include <iostream>
#include <cassert>

namespace gin {
Ogl33Resource::Ogl33Resource(GLuint rid):
	rid{rid}
{}

GLuint Ogl33Resource::id() const {
	return rid;
}

Ogl33RenderWorld::Ogl33RenderWorld(Ogl33Render& render):
	renderer{&render}
{}

Ogl33RenderWorld::~Ogl33RenderWorld(){
	assert(renderer);
	if(renderer){
		renderer->destroyedRenderWorld(*this);
	}
}

void Ogl33RenderWorld::destroyedRender(){
	assert(renderer);
	renderer = nullptr;
}

Own<RenderWorld> Ogl33Render::createWorld(){
	Own<Ogl33RenderWorld> world = heap<Ogl33RenderWorld>(*this);

	render_worlds.insert(world.get());

	return world;
}

void Ogl33Render::destroyedRenderWorld(Ogl33RenderWorld& rw){
	render_worlds.erase(&rw);
}

Ogl33Render::~Ogl33Render(){
	assert(render_worlds.empty());
	for(auto& world : render_worlds){
		world->destroyedRender();
	}
}
}

extern "C" gin::Render* createRenderer(){
	std::cout<<"Creating ogl33 plugin"<<std::endl;
	return new gin::Ogl33Render;
}

extern "C" void destroyRenderer(gin::Render* render){
	if(!render){
		return;
	}
	std::cout<<"Destroying ogl33 plugin"<<std::endl;
	delete render;
}

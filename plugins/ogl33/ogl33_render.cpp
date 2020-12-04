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

RenderTextureId Ogl33RenderTargetStorage::insert(Ogl33RenderTexture&& rt){
	RenderTargetId id;
	if(free_ids.empty()){
		id = max_free_id;
		++max_free_id;
	}else{
		id = free_ids.top();
		free_ids.pop();
	}

	render_textures.insert(std::make_pair(id, std::move(rt)));
	return static_cast<RenderTextureId>(id);
}

RenderWindowId Ogl33RenderTargetStorage::insert(Ogl33Window&& rw){
	RenderTargetId id;
	if(free_ids.empty()){
		id = max_free_id;
		++max_free_id;
	}else{
		id = free_ids.top();
		free_ids.pop();
	}

	windows.insert(std::make_pair(id, std::move(rw)));
	return static_cast<RenderWindowId>(id);
}

void Ogl33RenderTargetStorage::erase(const RenderTargetId& id){
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		render_textures.erase(rt_find);
		return;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		windows.erase(rw_find);
	}
}

bool Ogl33RenderTargetStorage::exists(const RenderTargetId& id) const {
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		return true;
	}

	return windows.find(id) != windows.end();
}

Ogl33RenderTarget* Ogl33RenderTargetStorage::operator[](const RenderTargetId& id){
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		return &rt_find->second;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		return &rw_find->second;
	}

	assert(false);
	return nullptr;
}

const Ogl33RenderTarget* Ogl33RenderTargetStorage::operator[](const RenderTargetId& id)const{
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		return &rt_find->second;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		return &rw_find->second;
	}

	assert(false);
	return nullptr;
}

Own<RenderWorld> Ogl33Render::createWorld(){
	Own<Ogl33RenderWorld> world = heap<Ogl33RenderWorld>(*this);

	render_worlds.insert(world.get());

	return world;
}

RenderWindowId Ogl33Render::createWindow() {
	return 0;
}

void Ogl33Render::destroyedRenderWorld(Ogl33RenderWorld& rw){
	render_worlds.erase(&rw);
}

Ogl33Render::Ogl33Render(Own<GlContext>&& ctx):
	context{std::move(ctx)}
{}

Ogl33Render::~Ogl33Render(){
	assert(render_worlds.empty());
	for(auto& world : render_worlds){
		world->destroyedRender();
	}
}
}

extern "C" gin::Render* createRenderer(gin::AsyncIoProvider& io_provider){
	gin::Own<gin::GlContext> context = gin::createGlContext(io_provider, gin::GlSettings{});
	if(!context){
		return nullptr;
	}

	std::cout<<"Creating ogl33 plugin"<<std::endl;
	return new gin::Ogl33Render(std::move(context));
}

extern "C" void destroyRenderer(gin::Render* render){
	if(!render){
		return;
	}

	std::cout<<"Destroying ogl33 plugin"<<std::endl;
	delete render;
}

#include "graphics.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

#include <cassert>
#include <iostream>
#include <algorithm>

namespace gin {
RenderPlugins::Plugin::Plugin(DynamicLibrary&& dl, std::function<LowLevelRender*(AsyncIoProvider&)>&& cr, std::function<void(LowLevelRender*)>&& dr):
	handle{std::move(dl)},
	create_render{std::move(cr)},
	destroy_render{std::move(dr)}
{
}

RenderPlugins::RenderPlugins(std::filesystem::path&& fp, std::map<std::string, RenderPlugins::Plugin>&& p):
	directory{std::move(fp)},
	render_plugins{std::move(p)}
{}

RenderPlugins::Plugin* RenderPlugins::getHandle(const std::string& name){
	auto find = render_plugins.find(name);
	if(find != render_plugins.end()){
		return &find->second;
	}
	return nullptr;
}

RenderPlugins loadAllRenderPluginsIn(const std::filesystem::path& dir){
	if(!std::filesystem::is_directory(dir)){
		return RenderPlugins{};
	}

	std::map<std::string, RenderPlugins::Plugin> plugins;

	for(auto& file: std::filesystem::directory_iterator(dir)){
		std::filesystem::path path = file.path();
		ErrorOr<DynamicLibrary> lib = loadDynamicLibrary(path);
		if(!lib.isValue()){
			std::cerr<<"Cannot load library at "<<file<<std::endl;
			continue;
		}

		std::function<LowLevelRender*(AsyncIoProvider&)> create_render = reinterpret_cast<LowLevelRender*(*)(AsyncIoProvider&)>(lib.value().symbol("createRenderer"));
		assert(create_render);
		if(!create_render){
			std::cerr<<"Cannot load function create_render"<<std::endl;
			continue;
		}

		std::function<void(LowLevelRender*)> destroy_render = reinterpret_cast<void(*)(LowLevelRender*)>(lib.value().symbol("destroyRenderer"));
		assert(destroy_render);
		if(!destroy_render){
			std::cerr<<"Cannot load function destroy_render"<<std::endl;
			continue;
		}

		std::string path_string = path.filename().string();
		std::string_view ps_view{path_string};
		auto ps_dot = ps_view.find('.');
		if(ps_dot != ps_view.npos){
			ps_view.remove_suffix(ps_view.size() - ps_dot);
		}

		auto lib_view = ps_view.substr(0,3);
		if(lib_view == "lib"){
			ps_view.remove_prefix(std::min(static_cast<size_t>(3), ps_view.size()));
		}

		path_string = ps_view;

		plugins.insert(std::make_pair(std::move(path_string), RenderPlugins::Plugin{std::move(lib.value()), std::move(create_render), std::move(destroy_render)}));
	}

	return RenderPlugins{std::filesystem::path{dir}, std::move(plugins)};
}

Graphics::Graphics(RenderPlugins&& rp):
	render_plugins{std::move(rp)}
{}

Graphics::~Graphics(){
	for(auto iter = renderers.begin(); iter != renderers.end(); iter = renderers.erase(iter)){
		iter->second.first->destroy_render(iter->second.second);
	}
}

LowLevelRender* Graphics::getRenderer(AsyncIoProvider& provider, const std::string& name){
	auto r_find = renderers.find(name);
	if(r_find != renderers.end()){
		return r_find->second.second;
	}

	auto handle = render_plugins.getHandle(name);
	if(!handle){
		return nullptr;
	}

	assert(handle->create_render && handle->destroy_render);
	LowLevelRender* render = handle->create_render(provider);
	if(!render){
		return nullptr;
	}

	renderers.insert(std::make_pair(name, std::make_pair(handle,render)));

	return render;
}

GraphicsService::GraphicsService(Graphics&& gr):graphics{std::move(gr)}{}

void GraphicsService::run(AsyncIoContext& async){
	while(running){
		async.wait_scope.wait(std::chrono::seconds{1});
	}
}

int GraphicsService::rcRun(AsyncIoContext& async){
	run(async);
	return return_code;
}

void GraphicsService::stop(){
	running = false;
}
}
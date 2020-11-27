#include "graphics.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

#include <cassert>
#include <iostream>

namespace gin {
RenderPlugins::Handles::Handles(DynamicLibrary&& dl, Render& r, std::function<void(Render*)>&& dr):
	handle{std::move(dl)},
	render{&r},
	destroy_render{std::move(dr)}
{
}

RenderPlugins::Handles::~Handles(){
	if(destroy_render){
		destroy_render(render);
	}
}

RenderPlugins::RenderPlugins(std::filesystem::path&& fp, std::map<std::string, Handles>&& p):
	directory{std::move(fp)},
	render_plugins{std::move(p)}
{}

RenderPlugins loadAllRenderPluginsIn(const std::filesystem::path& dir){
	if(!std::filesystem::is_directory(dir)){
		return RenderPlugins{};
	}

	std::map<std::string, RenderPlugins::Handles> plugins;

	for(auto& file: std::filesystem::directory_iterator(dir)){
		std::filesystem::path path = file.path();
		ErrorOr<DynamicLibrary> lib = loadDynamicLibrary(path);
		if(!lib.isValue()){
			continue;
		}

		Render*(*create_render)() = reinterpret_cast<Render*(*)()>(lib.value().symbol("createRenderer"));
		if(!create_render){
			continue;
		}
		Render* renderer = create_render();
		if(!renderer){
			continue;
		}

		std::function<void(Render*)> destroy_render = reinterpret_cast<void(*)(Render*)>(lib.value().symbol("destroyRenderer"));
		assert(destroy_render);
		if(!destroy_render){
			continue;
		}

		std::string path_string = path.filename().string();
		std::string_view ps_view{path_string};
		plugins.insert(std::make_pair(std::move(path_string), RenderPlugins::Handles{std::move(lib.value()), *renderer, std::move(destroy_render)}));
	}

	return RenderPlugins{std::filesystem::path{dir}, std::move(plugins)};
}

Graphics::Graphics(RenderPlugins&& rp):
	render_plugins{std::move(rp)}
{}

Render* Graphics::getRenderer(const std::string& name){
	return nullptr;
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
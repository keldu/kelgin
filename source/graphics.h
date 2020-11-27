#pragma once

#include "service.h"
#include "dynamic.h"
#include "render/render.h"

#include <kelgin/common.h>
#include <kelgin/error.h>

#include <map>
#include <string>
#include <functional>
#include <filesystem>

namespace gin {
/*
* Service since it is the core class running the main
* aspects
*/
class RenderPlugins {
public:
	struct Handles {
	public:
		Handles(DynamicLibrary&&, Render&, std::function<void(Render*)>&&);
		~Handles();

		Handles(Handles&&) = default;
		Handles& operator=(Handles&&) = default;

		DynamicLibrary handle;
		Render* render;
		std::function<void(Render*)> destroy_render;
	};
private:
	std::filesystem::path directory;

	/**
	* The key part uses the plugin name
	*/
	std::map<std::string, Handles> render_plugins; 
public:
	RenderPlugins() = default;
	RenderPlugins(std::filesystem::path&&,std::map<std::string, Handles>&&);

	/**
	* Returns a renderer if it already exists. If it is not loaded yet
	* it will try to find the plugin within the directory.
	* If this plugin is not found, the return value is `nullptr`
	*
	* @param name search for the plugin by using the filename part without the extension
	*/
	Render* getRenderer(const std::string& name)
};

RenderPlugins loadAllRenderPluginsIn(const std::filesystem::path& dir);

class Graphics {
private:
	RenderPlugins render_plugins;
public:
	Graphics(RenderPlugins&& rp);

	/**
	* Returns a renderer if it already exists. If it is not loaded yet
	* it will try to find the plugin within the parameters provided to RenderPlugins.
	* If this plugin is not found, the return value is `nullptr`
	*
	* @param name search for the plugin by using the filename part without the extension
	*/
	Render* getRenderer(const std::string& name);
};

class GraphicsService final : public Service {
private:
	int return_code = 0;
	bool running = true;

	Graphics graphics;
public:	
	GraphicsService(Graphics&& gr);

	int rcRun(AsyncIoContext& async);

	void run(AsyncIoContext& async) override;
	void stop() override;
};
}
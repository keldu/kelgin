#pragma once

#include "./service.h"
#include "./dynamic.h"
#include "./render/render.h"

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
	struct Plugin {
	public:
		Plugin(DynamicLibrary&&, std::function<LowLevelRender*(AsyncIoProvider&)>&&, std::function<void(LowLevelRender*)>&&);

		Plugin(Plugin&&) = default;
		Plugin& operator=(Plugin&&) = default;

		DynamicLibrary handle;
		std::function<LowLevelRender*(AsyncIoProvider&)> create_render;
		std::function<void(LowLevelRender*)> destroy_render;
	};
private:
	std::filesystem::path directory;

	/**
	* The key part uses the plugin name
	*/
	std::map<std::string, RenderPlugins::Plugin> render_plugins;
public:
	RenderPlugins() = default;
	RenderPlugins(std::filesystem::path&&,std::map<std::string, RenderPlugins::Plugin>&&);

	RenderPlugins(RenderPlugins&&) = default;
	RenderPlugins& operator=(RenderPlugins&&) = default;
	/**
	* Returns a renderer if it already exists. If it is not loaded yet
	* it will try to find the plugin within the directory.
	* If this plugin is not found, the return value is `nullptr`
	*
	* @param name search for the plugin by using the filename part without the extension
	*/
	Plugin* getHandle(const std::string& name);
};

RenderPlugins loadAllRenderPluginsIn(const std::filesystem::path& dir);

class Graphics {
private:
	RenderPlugins render_plugins;
	std::map<std::string, std::pair<RenderPlugins::Plugin*, LowLevelRender*>> renderers;
public:
	Graphics(RenderPlugins&& rp);
	~Graphics();

	Graphics(Graphics&&) = default;
	Graphics& operator=(Graphics&&) = default;
	/**
	* Returns a renderer if it already exists. If it is not loaded yet
	* it will try to find the plugin within the parameters provided to RenderPlugins.
	* If this plugin is not found, the return value is `nullptr`
	*
	* @param provider This is a wrapper for listening to outside events
	* @param name search for the plugin by using the filename part without the extension
	*/
	LowLevelRender* getRenderer(AsyncIoProvider& provider, const std::string& name);
};

class GraphicsService final : public Service {
private:
	int return_code = 0;
	bool running = true;

	Graphics graphics;
public:	
	GraphicsService(Graphics&& gr);

	int rcRun(AsyncIoContext& async, WaitScope& wait_scope);

	void run(AsyncIoContext& async, WaitScope& wait_scope) override;
	void stop() override;
};
}

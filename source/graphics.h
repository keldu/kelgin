#pragma once

#include "./dynamic.h"
#include "./render/render.h"
#include "./service.h"

#include <kelgin/common.h>
#include <kelgin/error.h>

#include <filesystem>
#include <functional>
#include <map>
#include <string>

namespace gin {
/*
 * Service since it is the core class running the main
 * aspects
 */

class RenderProvider {
public:
	virtual ~RenderProvider() = default;

	/// @todo build interface, because plugins aren't always available
	virtual LowLevelRender *getRenderer(IoProvider &provider,
										const std::string &name) = 0;
};

class RenderPlugins : public RenderProvider {
public:
	struct Plugin {
	public:
		Plugin(DynamicLibrary &&,
			   std::function<LowLevelRender *(IoProvider &)> &&,
			   std::function<void(LowLevelRender *)> &&);

		Plugin(Plugin &&) = default;
		Plugin &operator=(Plugin &&) = default;

		DynamicLibrary handle;
		std::function<LowLevelRender *(IoProvider &)> create_render;
		std::function<void(LowLevelRender *)> destroy_render;
	};

	struct RenderPlugin {
		RenderPlugins::Plugin plugin;
		LowLevelRender *render;

		RenderPlugin() = delete;

		RenderPlugin(RenderPlugins::Plugin &&p)
			: plugin{std::move(p)}, render{nullptr} {}
	};

private:
	std::filesystem::path directory;

	/**
	 * The key part uses the plugin name
	 */
	std::map<std::string, RenderPlugins::RenderPlugin> render_plugins;

public:
	RenderPlugins() = default;
	RenderPlugins(std::filesystem::path &&,
				  std::map<std::string, RenderPlugins::RenderPlugin> &&);

	~RenderPlugins();

	RenderPlugins(RenderPlugins &&) = default;
	RenderPlugins &operator=(RenderPlugins &&) = default;

	/**
	 * Returns a renderer if it already exists. If it is not loaded yet
	 * it will try to find the plugin within the directory.
	 * If this plugin is not found, the return value is `nullptr`
	 *
	 * @param name search for the plugin by using the filename part without the
	 * extension
	 */

	LowLevelRender *getRenderer(IoProvider &provider,
								const std::string &name) override;
};

Own<RenderProvider> loadAllRenderPluginsIn(const std::filesystem::path &dir);

class Graphics {
private:
	Own<RenderProvider> render_provider;

public:
	Graphics(Own<RenderProvider> &&rp);
	~Graphics();

	Graphics(Graphics &&) = default;
	Graphics &operator=(Graphics &&) = default;

	/**
	 * Returns a renderer if it already exists. If it is not loaded yet
	 * it will try to find the plugin within the parameters provided to
	 * RenderPlugins. If this plugin is not found, the return value is `nullptr`
	 *
	 * @param provider This is a wrapper for listening to outside events
	 * @param name search for the plugin by using the filename part without the
	 * extension
	 */
	LowLevelRender *getRenderer(IoProvider &provider,
								const std::string &name);
};

class GraphicsService final : public Service {
private:
	int return_code = 0;
	bool running = true;

	Graphics graphics;

public:
	GraphicsService(Graphics &&gr);

	int rcRun(AsyncIoContext &async, WaitScope &wait_scope);

	void run(AsyncIoContext &async, WaitScope &wait_scope) override;
	void stop() override;
};
} // namespace gin

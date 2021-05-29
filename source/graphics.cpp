#include "graphics.h"

#include <kelgin/async.h>
#include <kelgin/io.h>

#include <algorithm>
#include <cassert>
#include <iostream>

namespace gin {
RenderPlugins::Plugin::Plugin(
	DynamicLibrary &&dl,
	std::function<LowLevelRender *(AsyncIoProvider &)> &&cr,
	std::function<void(LowLevelRender *)> &&dr)
	: handle{std::move(dl)}, create_render{std::move(cr)}, destroy_render{
															   std::move(dr)} {}

RenderPlugins::RenderPlugins(
	std::filesystem::path &&fp,
	std::map<std::string, RenderPlugins::RenderPlugin> &&p)
	: directory{std::move(fp)}, render_plugins{std::move(p)} {}

RenderPlugins::~RenderPlugins() {
	for (auto &iter : render_plugins) {
		if (iter.second.render) {
			iter.second.plugin.destroy_render(iter.second.render);
			iter.second.render = nullptr;
		}
	}
}

LowLevelRender *RenderPlugins::getRenderer(AsyncIoProvider &provider,
										   const std::string &name) {
	auto find = render_plugins.find(name);
	if (find != render_plugins.end()) {
		if (find->second.render) {
			return find->second.render;
		}
		RenderPlugins::RenderPlugin &rp = find->second;

		assert(rp.plugin.create_render && rp.plugin.destroy_render);
		if (rp.plugin.create_render && rp.plugin.destroy_render) {
			rp.render = rp.plugin.create_render(provider);
			if (rp.render) {
				return rp.render;
			}
		} else {
			render_plugins.erase(find);
			return nullptr;
		}
	}

	return nullptr;
}

Own<RenderProvider> loadAllRenderPluginsIn(const std::filesystem::path &dir) {
	if (!std::filesystem::is_directory(dir)) {
		return heap<RenderPlugins>();
	}

	std::map<std::string, RenderPlugins::RenderPlugin> plugins;

	for (auto &file : std::filesystem::directory_iterator(dir)) {
		std::filesystem::path path = file.path();
		ErrorOr<DynamicLibrary> lib = loadDynamicLibrary(path);
		if (!lib.isValue()) {
			std::cerr << "Cannot load library at " << file << std::endl;
			continue;
		}

		std::function<LowLevelRender *(AsyncIoProvider &)> create_render =
			reinterpret_cast<LowLevelRender *(*)(AsyncIoProvider &)>(
				lib.value().symbol("createRenderer"));
		assert(create_render);
		if (!create_render) {
			std::cerr << "Cannot load function create_render" << std::endl;
			continue;
		}

		std::function<void(LowLevelRender *)> destroy_render =
			reinterpret_cast<void (*)(LowLevelRender *)>(
				lib.value().symbol("destroyRenderer"));
		assert(destroy_render);
		if (!destroy_render) {
			std::cerr << "Cannot load function destroy_render" << std::endl;
			continue;
		}

		std::string path_string = path.filename().string();
		std::string_view ps_view{path_string};
		auto ps_dot = ps_view.find('.');
		if (ps_dot != ps_view.npos) {
			ps_view.remove_suffix(ps_view.size() - ps_dot);
		}

		auto lib_view = ps_view.substr(0, 3);
		if (lib_view == "lib") {
			ps_view.remove_prefix(
				std::min(static_cast<size_t>(3), ps_view.size()));
		}

		path_string = ps_view;

		plugins.insert(
			std::make_pair(std::move(path_string),
						   RenderPlugins::RenderPlugin{RenderPlugins::Plugin{
							   std::move(lib.value()), std::move(create_render),
							   std::move(destroy_render)}}));
	}

	return heap<RenderPlugins>(std::filesystem::path{dir}, std::move(plugins));
}

Graphics::Graphics(Own<RenderProvider> &&rp) : render_provider{std::move(rp)} {}

Graphics::~Graphics() {}

LowLevelRender *Graphics::getRenderer(AsyncIoProvider &provider,
									  const std::string &name) {
	assert(render_provider);
	if (!render_provider) {
		return nullptr;
	}

	LowLevelRender *render = render_provider->getRenderer(provider, name);

	return render;
}

GraphicsService::GraphicsService(Graphics &&gr) : graphics{std::move(gr)} {}

void GraphicsService::run(AsyncIoContext &async, WaitScope &wait_scope) {
	while (running) {
		wait_scope.wait(std::chrono::seconds{1});
	}
}

int GraphicsService::rcRun(AsyncIoContext &async, WaitScope &wait_scope) {
	run(async, wait_scope);
	return return_code;
}

void GraphicsService::stop() { running = false; }
} // namespace gin

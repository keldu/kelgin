#include "graphics.h"

#include <iostream>

int main() {
	using namespace gin;

	AsyncIoContext async = setupAsyncIo();

	bool running = true;
	async.event_port.onSignal(Signal::Terminate)
		.then([&running]() { running = false; })
		.detach([](const Error &error) { return error; });

	Graphics graphics{loadAllRenderPluginsIn("./bin/plugins/")};
	Render *render = graphics.getRenderer(*async.io, "ogl33");
	if (!render) {
		std::cerr << "No ogl33 renderer present" << std::endl;
		return -1;
	}

	// Have this as a 2D object
	auto render_world = render->createWorld();

	//
	RenderWindowId win_id = render->createWindow({600,400}, "Kelgin Setup Example");

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 10.0f);
	render->flush();

	while (running) {
		auto time = std::chrono::steady_clock::now();
		render->step(time);



		async.wait_scope.wait(std::chrono::milliseconds{10});
	}

	return 0;
}
#include "graphics.h"

#include <iostream>

#include "./shaders.h"
#include "./mesh_data.h"

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

	//
	RenderWindowId win_id = render->createWindow({600,400}, "Kelgin Setup Example");
	render->flush();

	ProgramId program_id = render->createProgram(default_vertex_shader, default_fragment_shader);
	std::cout <<"Program id: "<<std::to_string(program_id)<<std::endl;

	// Have this as a 2D object
	auto render_world = render->createWorld();

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 10.0f);
	render->flush();

	while (running) {
		auto time = std::chrono::steady_clock::now();
		render->step(time);


		render->flush();
		async.wait_scope.wait(std::chrono::milliseconds{10});
	}

	//render->destroyProgram(program_id);


	return 0;
}
#include "graphics.h"

#include <iostream>

#include "./mesh_data.h"
#include "./shaders.h"
#include "./texture_data.h"

#include "stb_image.h"
#include <array>
#include <cstring>

int main(){
	using namespace gin;

	ErrorOr<AsyncIoContext> err_async = setupAsyncIo();
	AsyncIoContext& async = err_async.value();
	WaitScope wait_scope{async.event_loop};

	bool running = true;
	async.event_port.onSignal(Signal::Terminate).then([&running](){running = false;})
	.detach();

	Graphics graphics{loadAllRenderPluginsIn("./bin/plugins/")};
	LowLevelRender *render = graphics.getRenderer(*async.io, "ogl33");

	if(!render){
		std::cerr<< "No ogl33 renderer present" << std::endl;
		return -1;
	}

	RenderWindowId win_id = render->createWindow({600,400}, "Kelgin Setup Example").value();
	render->flush();
	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 60.0f);


	auto old_time = std::chrono::steady_clock::now();
	while (running) {
		auto time = std::chrono::steady_clock::now();

		render->step(time);

		render->flush();
		wait_scope.wait(std::chrono::milliseconds{5});
		old_time = time;
	}

	render->destroyWindow(win_id);

	return 0;
}
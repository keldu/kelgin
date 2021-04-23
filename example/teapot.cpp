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
	RenderWindowId win_id = 0;

	render->createWindow({600,400}, "Kelgin Setup Example").then([&render, &win_id](RenderWindowId id){
		render->flush();

		render->setWindowVisibility(id, true).then([&render,id](){
			render->setWindowDesiredFPS(id, 60.f).detach();
		}).detach();

		win_id = id;
	}).detach();

	wait_scope.poll();

	Program3dId program_id = render->createProgram3d().take().value();

	// Mesh3dId mesh_id = render->createMesh3d().value();

	RenderScene3dId scene_id = render->createScene3d().take().value();

	// RenderProperty3dId rp_id = render->createProperty3d(

	RenderCamera3dId camera_id = render->createCamera3d().take().value();
	float aspect = 600.f / 400.f;
	float zoom = 10.f;

	float near = 0.1f;
	float far = 50.f;

	RenderStage3dId stage_id = render->createStage(program_id, win_id, scene_id, camera_id).take().value();

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
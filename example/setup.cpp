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
	MeshId mesh_id = render->createMesh(default_mesh);

	Own<RenderWorld> world = render->createWorld();
	if(!world){
		std::cerr<<"Couldn't create RenderWorld"<<std::endl;
		return -1;
	}

	Own<RenderScene> scene = world->createScene();
	if(!scene){
		std::cerr<<"Couldn't create RenderScene" <<std::endl;
	}

	RenderObjectId ro_id = world->createRenderObject(mesh_id, 0);

	scene->attachObjectToScene(ro_id);
	scene->setObjectPosition(0.f, 0.f);

	RenderCameraId camera_id = world->createCamera();

	RenderStageId stage_id = world->createStage(win_id, scene_id, camera_id);

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 10.0f);
	render->flush();

	while (running) {
		auto time = std::chrono::steady_clock::now();
		render->step(time);

		render->flush();
		async.wait_scope.wait(std::chrono::milliseconds{10});
	}

	scene = nullptr;

	world->destroyRenderObject(ro_id);
	world = nullptr;

	render->destroyMesh(mesh_id);
	render->destroyProgram(program_id);

	return 0;
}
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
	LowLevelRender *render = graphics.getRenderer(*async.io, "ogl33");
	if (!render) {
		std::cerr << "No ogl33 renderer present" << std::endl;
		return -1;
	}

	//
	RenderWindowId win_id = render->createWindow({600,400}, "Kelgin Setup Example");
	render->flush();

	ProgramId program_id = render->createProgram(default_vertex_shader, default_fragment_shader);
	MeshId mesh_id = render->createMesh(default_mesh);

	RenderSceneId scene_id = render->createScene();

	RenderPropertyId rp_id = render->createProperty(mesh_id, 0);

	RenderObjectId ro_id = render->createObject(scene_id, rp_id);
	render->setObjectPosition(scene_id, ro_id, -0.1f, -0.05f);

	RenderCameraId camera_id = render->createCamera();

	RenderStageId stage_id = render->createStage(win_id, scene_id, camera_id);

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 10.0f);
	render->flush();

	while (running) {
		auto time = std::chrono::steady_clock::now();
		render->step(time);

		render->flush();
		async.wait_scope.wait(std::chrono::milliseconds{10});
	}

	// Stuff gets cleaned up anyway
	render->destroyScene(scene_id);
	render->destroyMesh(mesh_id);
	render->destroyProgram(program_id);

	return 0;
}
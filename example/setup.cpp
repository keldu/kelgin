#include "graphics.h"

#include <iostream>

#include "./shaders.h"
#include "./mesh_data.h"
#include "./texture_data.h"

#include "stb_image.h"
#include <cstring>

gin::Image loadFromFile(const std::string& path){
	gin::Image image;

	int32_t cmps;
	int32_t width;
	int32_t height;
	uint8_t* data = stbi_load(path.c_str(), &width, &height, &cmps, 4);

	if(cmps <= 0 || cmps > 255 || width <= 0 || height <= 0){
		stbi_image_free(data);
		return image;
	}
	image.channels = 4;
	image.width = static_cast<size_t>(width);
	image.height = static_cast<size_t>(height);
	
	image.pixels.resize(image.width * image.height * image.channels);

	memcpy(&image.pixels[0], data, sizeof(uint8_t) * image.width * image.height * image.channels);
	stbi_image_free(data);

	return image;
}

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
	TextureId texture_id = render->createTexture(loadFromFile("test.png"));

	RenderSceneId scene_id = render->createScene();

	RenderPropertyId rp_id = render->createProperty(mesh_id, texture_id);

	RenderObjectId ro_id = render->createObject(scene_id, rp_id);

	RenderCameraId camera_id = render->createCamera();
	render->setCameraOrthographic(camera_id, -2.0f, 2.0f, -2.0f, 2.0f, -1.0f, 1.0f);

	RenderStageId stage_id = render->createStage(program_id, win_id, scene_id, camera_id);

	auto events = render->listenToWindowEvents(win_id).then([&render, camera_id](RenderEvent::Events&& event){
		std::visit([&render, camera_id](auto&& arg){
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, RenderEvent::Resize>){
				float aspect = static_cast<float>(arg.width) / static_cast<float>(arg.height);
				render->setCameraOrthographic(camera_id, - 2.0f * aspect, 2.0f * aspect,  -2.0f, 2.0f, -1.0f, 1.0f);
				std::cout<<"Resize: "<<arg.width<<" "<<arg.height<<std::endl;
			}else if constexpr(std::is_same_v<T, RenderEvent::Keyboard>){
				std::cout<<"Keypress: "<<arg.key_code<<" "<<arg.pressed<<std::endl;
			}else if constexpr(std::is_same_v<T, RenderEvent::Mouse>){
				std::cout<<"Mousepress: "<<arg.button<<" "<<arg.pressed<<std::endl;
			}
		},event);
	}).sink([](const Error& error){return error;});

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 10.0f);
	render->flush();

	float x = 0.f;
	float angle = 0.f;

	auto old_time = std::chrono::steady_clock::now();
	while (running) {
		auto time = std::chrono::steady_clock::now();

		std::chrono::duration<float> fs = time - old_time;

		x += 0.0001f * fs.count();
		angle += 0.9f * fs.count();
		render->setObjectPosition(scene_id, ro_id, x, 0.f);
		render->setObjectRotation(scene_id, ro_id, angle);

		render->step(time);

		render->flush();
		async.wait_scope.wait(std::chrono::milliseconds{10});
		old_time = time;
	}

	// Stuff gets cleaned up anyway
	render->destroyScene(scene_id);
	render->destroyMesh(mesh_id);
	render->destroyProgram(program_id);

	return 0;
}

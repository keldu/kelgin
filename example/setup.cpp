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
	TextureId green_square_tex_id = render->createTexture(default_image);

	RenderSceneId scene_id = render->createScene();

	RenderPropertyId rp_id = render->createProperty(mesh_id, texture_id);
	RenderPropertyId gsq_rp_id = render->createProperty(mesh_id, green_square_tex_id);

	RenderObjectId ro_id = render->createObject(scene_id, gsq_rp_id);

	RenderCameraId camera_id = render->createCamera();
	float aspect = 600.f / 400.f;
	float zoom = 10.f;
	render->setCameraOrthographic(camera_id, -2.0f * aspect * zoom, 2.0f * aspect * zoom, -2.0f * aspect * zoom, 2.0f * aspect * zoom, -1.0f, 1.0f);

	RenderStageId stage_id = render->createStage(program_id, win_id, scene_id, camera_id);
	
	float x = 0.f;
	float y = 0.f;
	float vx = 0.f;
	float vy = 0.f;
	float ax = 0.f;
	float ay = -9.81f;

	auto events = render->listenToWindowEvents(win_id).then([&](RenderEvent::Events&& event){
		std::visit([&](auto&& arg){
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, RenderEvent::Resize>){
				aspect = static_cast<float>(arg.width) / static_cast<float>(arg.height);
				render->setCameraOrthographic(camera_id, - 2.0f * aspect*zoom, 2.0f * aspect*zoom,  -2.0f*zoom, 2.0f*zoom, -1.0f, 1.0f);
				std::cout<<"Resize: "<<arg.width<<" "<<arg.height<<std::endl;
			}else if constexpr(std::is_same_v<T, RenderEvent::Keyboard>){
				std::cout<<"Keypress: "<<arg.key_code<<" "<<arg.pressed<<std::endl;
				switch(arg.key_code){
				case 9:
					if(!arg.pressed) running = false;
				break;
				case 65:
					if(arg.pressed && y < 1e-5f) vy = 10.f;
				break;
				case 40:
					if(arg.pressed) ax = 15.f;
					else ax = 0.f;
				break;
				case 38:
					if(arg.pressed) ax = -15.f;
					else ax = 0.f;
				break;
				default:
				break;
				}
			}else if constexpr(std::is_same_v<T, RenderEvent::Mouse>){
				std::cout<<"Mousepress: "<<arg.button<<" "<<arg.pressed<<std::endl;
			}
		},event);
	}).sink([](const Error& error){return error;});

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 60.0f);
	render->flush();

	auto old_time = std::chrono::steady_clock::now();
	while (running) {
		auto time = std::chrono::steady_clock::now();

		std::chrono::duration<float> fs = time - old_time;

		if ( y <= 1e-5f){
			float friction = 9.81 * ((0.f < vx ) - (vx < 0.f))*fs.count();
			if(std::abs(vx) < std::abs(friction)){
				vx -= 0.f;
			}else {
				vx -= friction;
			}
		}
		vx += ax * fs.count();
		vy += ay * fs.count();

		x += vx * fs.count();
		y += vy * fs.count();

		y = y < 0 ? 0.f : y;

		render->setObjectPosition(scene_id, ro_id, x, y);

		render->step(time);

		render->flush();
		async.wait_scope.wait(std::chrono::milliseconds{5});
		old_time = time;
	}

	// Stuff gets cleaned up anyway
	render->destroyScene(scene_id);
	render->destroyMesh(mesh_id);
	render->destroyProgram(program_id);

	return 0;
}

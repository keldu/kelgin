#include "graphics.h"

#include <iostream>

#include "./mesh_data.h"
#include "./shaders.h"
#include "./texture_data.h"

#include "stb_image.h"
#include <array>
#include <cstring>

gin::Image loadFromFile(const std::string &path) {
	gin::Image image;

	int32_t cmps = 0;
	int32_t width = 0;
	int32_t height = 0;
	uint8_t *data = stbi_load(path.c_str(), &width, &height, &cmps, 4);

	if (!data) {
		return image;
	}
	if (cmps <= 0 || cmps > 255 || width <= 0 || height <= 0) {
		stbi_image_free(data);
		return image;
	}

	image.channels = 4;
	image.width = static_cast<size_t>(width);
	image.height = static_cast<size_t>(height);

	image.pixels.resize(image.width * image.height * image.channels);

	memcpy(&image.pixels[0], data,
		   sizeof(uint8_t) * image.width * image.height * image.channels);
	stbi_image_free(data);

	return image;
}

int main() {
	using namespace gin;

	AsyncIoContext async = setupAsyncIo();
	WaitScope wait_scope{async.event_loop};

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

	//	========================= Render Windows =============================
	RenderWindowId win_id =
		render->createWindow({600, 400}, "Kelgin Setup Example");
	render->flush();

	//	=========================== Programs =================================
	ProgramId program_id =
		render->createProgram(default_vertex_shader, default_fragment_shader);

	//	============================ Meshes ==================================
	MeshId mesh_id = render->createMesh(default_mesh);
	MeshId bg_mesh_id = render->createMesh(bg_mesh);

	//  =========================== Textures =================================
	TextureId texture_id = render->createTexture(loadFromFile("test.png"));
	TextureId green_square_tex_id = render->createTexture(default_image);
	TextureId bg_tex_id = render->createTexture(loadFromFile("bg.png"));

	//	============================ Scenes ==================================
	RenderSceneId scene_id = render->createScene();

	//	===================== Render Properties ==============================
	RenderPropertyId rp_id = render->createProperty(mesh_id, texture_id);
	RenderPropertyId gsq_rp_id =
		render->createProperty(mesh_id, green_square_tex_id);
	RenderPropertyId bg_rp_id = render->createProperty(bg_mesh_id, bg_tex_id);

	//	======================= Render Objects ===============================
	RenderObjectId ro_id = render->createObject(scene_id, gsq_rp_id);
	std::array<std::array<RenderObjectId, 3>, 3> bg_ro_ids = {
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id),
		render->createObject(scene_id, bg_rp_id)};

	for (size_t i = 0; i < 3; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			render->setObjectPosition(scene_id, bg_ro_ids[i][j],
									  i * 80.f - 80.f, j * 80.f - 80.f);
		}
	}

	RenderCameraId camera_id = render->createCamera();
	float aspect = 600.f / 400.f;
	float zoom = 10.f;
	render->setCameraOrthographic(camera_id, -2.0f * aspect * zoom,
								  2.0f * aspect * zoom, -2.0f * zoom,
								  2.0f * zoom);

	RenderStageId stage_id =
		render->createStage(program_id, win_id, scene_id, camera_id);

	int dx = 0;
	int dy = 0;

	float x = 0.f;
	float y = 0.f;
	float vx = 0.f;
	float vy = 0.f;

	auto events =
		render->listenToWindowEvents(win_id)
			.then([&](RenderEvent::Events &&event) {
				std::visit(
					[&](auto &&arg) {
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, RenderEvent::Resize>) {
							std::cout << "Resize: " << arg.width << " "
									  << arg.height << std::endl;
							aspect = static_cast<float>(arg.width) /
									 static_cast<float>(arg.height);
							render->setCameraOrthographic(
								camera_id, -2.0f * aspect * zoom,
								2.0f * aspect * zoom, -2.0f * zoom,
								2.0f * zoom);
						} else if constexpr (std::is_same_v<
												 T, RenderEvent::Keyboard>) {
							std::cout << "Keypress: " << arg.key_code << " "
									  << arg.pressed << " " << arg.repeat
									  << std::endl;
							switch (arg.key_code) {
							case 9:
								if (!arg.pressed)
									running = false;
								break;
							case 40:
								if (arg.pressed)
									dx = 1;
								else
									dx = 0;
								break;
							case 38:
								if (arg.pressed)
									dx = -1;
								else
									dx = 0;
								break;
							case 39:
								if (arg.pressed)
									dy = -1;
								else
									dy = 0;
								break;
							case 25:
								if (arg.pressed)
									dy = 1;
								else
									dy = 0;
								break;
							default:
								break;
							}
						} else if constexpr (std::is_same_v<
												 T, RenderEvent::Mouse>) {
							std::cout << "Mousepress: " << arg.button << " "
									  << arg.pressed << std::endl;
						} else if constexpr (std::is_same_v<
												 T, RenderEvent::MouseMove>) {
							std::cout << "Mouse move: " << arg.x << " " << arg.y
									  << std::endl;
						}
					},
					event);
			})
			.sink([](const Error &error) { return error; });

	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 60.0f);
	render->flush();

	auto old_time = std::chrono::steady_clock::now();
	while (running) {
		auto time = std::chrono::steady_clock::now();

		std::chrono::duration<float> fs = time - old_time;

		if (y <= 1e-5f) {
			float friction = 9.81 * ((0.f < vx) - (vx < 0.f)) * fs.count();
			if (std::abs(vx) < std::abs(friction)) {
				vx -= 0.f;
			} else {
				vx -= friction;
			}
		}
		vx = dx * 15.f;
		vy = dy * 15.f;

		x += vx * fs.count();
		y += vy * fs.count();

		render->setCameraPosition(camera_id, x, y);

		int64_t ix = static_cast<int64_t>(x / 80.f);
		int64_t iy = static_cast<int64_t>(y / 80.f);
		for (size_t i = 0; i < 3; ++i) {
			for (size_t j = 0; j < 3; ++j) {
				render->setObjectPosition(
					scene_id, bg_ro_ids[i][j],
					(static_cast<int64_t>(i) + ix - 1) * 80.f,
					(static_cast<int64_t>(j) + iy - 1) * 80.f);
			}
		}

		render->setObjectPosition(scene_id, ro_id, x, y);

		render->step(time);

		render->flush();
		wait_scope.wait(std::chrono::milliseconds{5});
		old_time = time;
	}

	// Stuff gets cleaned up anyway
	render->destroyScene(scene_id);
	render->destroyMesh(mesh_id);
	render->destroyProgram(program_id);

	return 0;
}

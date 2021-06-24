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

	ErrorOr<AsyncIoContext> err_async = setupAsyncIo();
	if (err_async.isError()) {
		std::cerr << "Couldn't load AsyncIoContext" << std::endl;
		return -1;
	}

	AsyncIoContext &async = err_async.value();
	WaitScope wait_scope{async.event_loop};

	bool running = true;
	async.event_port.onSignal(Signal::Terminate)
		.then([&running]() { running = false; })
		.detach();

	Graphics graphics{loadAllRenderPluginsIn("./bin/plugins/")};
	LowLevelRender *render = graphics.getRenderer(*async.io, "ogl33");
	if (!render) {
		std::cerr << "No ogl33 renderer present" << std::endl;
		return -1;
	}

	gin::LowLevelRender2D *render_2d = render->interface2D();
	if (!render_2d) {
		std::cout << "Missing 2D interface" << std::endl;
		return -1;
	}

	//	========================= Render Windows =============================
	RenderWindowId win_id = render->createWindow({600, 400}, "Kelgin Setup Example").value();
	render->flush();
	render->setWindowVisibility(win_id, true);
	render->setWindowDesiredFPS(win_id, 60.f);

	//	=========================== Programs =================================
	ProgramId program_id =
		render_2d->createProgram(default_vertex_shader, default_fragment_shader)
			.value();

	//	============================ Meshes ==================================
	MeshId mesh_id = render_2d->createMesh(default_mesh).value();
	MeshId bg_mesh_id = render_2d->createMesh(bg_mesh).value();

	//  =========================== Textures =================================
	TextureId texture_id =
		render->createTexture(loadFromFile("test.png")).value();
	TextureId green_square_tex_id =
		render->createTexture(default_image).value();
	TextureId bg_tex_id =
		render->createTexture(loadFromFile("bg.png")).value();

	//	============================ Scenes ==================================
	RenderSceneId scene_id = render_2d->createScene().value();

	//	===================== Render Properties ==============================
	RenderPropertyId rp_id =
		render_2d->createProperty(mesh_id, texture_id).value();
	RenderPropertyId gsq_rp_id =
		render_2d->createProperty(mesh_id, green_square_tex_id).value();
	RenderPropertyId bg_rp_id =
		render_2d->createProperty(bg_mesh_id, bg_tex_id).value();

	//	======================= Render Objects ===============================
	RenderObjectId ro_id =
		render_2d->createObject(scene_id, gsq_rp_id).value();

	RenderObjectId ro_spin_id =
		render_2d->createObject(scene_id, gsq_rp_id).value();

	render_2d->setObjectLayer(scene_id, ro_id, -0.1f);
	render_2d->setObjectLayer(scene_id, ro_spin_id, -0.1f);

	std::array<std::array<RenderObjectId, 3>, 3> bg_ro_ids = {
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value(),
		render_2d->createObject(scene_id, bg_rp_id).value()};

	for (size_t i = 0; i < 3; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			render_2d->setObjectPosition(scene_id, bg_ro_ids[i][j],
										 i * 80.f - 80.f, j * 80.f - 80.f);
			render_2d->setObjectLayer(scene_id, bg_ro_ids[i][j], 0.2f);
		}
	}
	render_2d->setObjectPosition(scene_id, ro_spin_id, 5.f, 0.f);

	RenderCameraId camera_id = render_2d->createCamera().value();
	float aspect = 600.f / 400.f;
	float zoom = 10.f;
	render_2d->setCameraOrthographic(camera_id, -2.0f * aspect * zoom,
									 2.0f * aspect * zoom, -2.0f * zoom,
									 2.0f * zoom);

	RenderViewportId viewport_id = render->createViewport().value();

	RenderStageId stage_id =
		render_2d
			->createStage(program_id, viewport_id, win_id, scene_id, camera_id)
			
			.value();

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
							render_2d->setCameraOrthographic(
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
			.sink();

	render->flush();

	auto old_time = std::chrono::steady_clock::now();
	auto next_phys_time = old_time;

	float phys_time_delta = 1.f / 10.f;

	float angle = 0.f;
	float fps = 0.f;
	float kalman = 0.99f;

	while (running) {
		auto time = std::chrono::steady_clock::now();

		if (time >= next_phys_time) {
			next_phys_time +=
				std::chrono::duration_cast<std::chrono::steady_clock::duration>(
					std::chrono::duration<float>{phys_time_delta});
			render->updateTime(
				time, next_phys_time +
						  std::chrono::duration_cast<
							  std::chrono::steady_clock::duration>(
							  std::chrono::duration<float>{phys_time_delta}));

			if (y <= 1e-5f) {
				float friction =
					9.81 * ((0.f < vx) - (vx < 0.f)) * phys_time_delta;
				if (std::abs(vx) < std::abs(friction)) {
					vx -= 0.f;
				} else {
					vx -= friction;
				}
			}
			vx = dx * 15.f;
			vy = dy * 15.f;

			angle += phys_time_delta;

			x += vx * phys_time_delta;
			y += vy * phys_time_delta;

			render_2d->setCameraPosition(camera_id, x, y);

			render_2d->setObjectRotation(scene_id, ro_spin_id, angle);

			int64_t ix = static_cast<int64_t>(x / 80.f);
			int64_t iy = static_cast<int64_t>(y / 80.f);
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					render_2d->setObjectPosition(
						scene_id, bg_ro_ids[i][j],
						(static_cast<int64_t>(i) + ix - 1) * 80.f,
						(static_cast<int64_t>(j) + iy - 1) * 80.f, false);
				}
			}

			render_2d->setObjectPosition(scene_id, ro_id, x, y);
		}

		render->step(time);

		render->flush();
		wait_scope.wait(std::chrono::milliseconds{1});

		fps = fps * kalman + (1.f-kalman) / std::chrono::duration<float>{time-old_time}.count();

		// std::cout<<"FPS: "<<fps<<std::endl;

		old_time = time;
	}

	// Stuff gets cleaned up anyway
	render_2d->destroyScene(scene_id);
	render_2d->destroyMesh(mesh_id);
	render_2d->destroyProgram(program_id);

	return 0;
}

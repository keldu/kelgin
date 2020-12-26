#pragma once

#include "render/render.h"

#include <queue>
#include <set>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <cassert>

#include <iostream>

#include "ogl33_bindings.h"

#include <kelgin/window/gl/gl_context.h>
#include <kelgin/common.h>

#include "common/math.h"

namespace gin {
/**
* Helper parent
*/
class Ogl33Resource {
protected:
public:
};
class Ogl33Render;
class Ogl33RenderProperty;
class Ogl33RenderObject;

class Ogl33Camera {
private:
	Matrix<float, 3, 3> projection_matrix;
	Matrix<float, 3, 3> view_matrix;
public:
	Ogl33Camera();

	void setViewPosition(float x, float y);
	void setViewRotation(float angle);

	void setOrtho(float left, float right, float top, float bot, float near, float far);

	const Matrix<float, 3,3>& view() const;
	const Matrix<float, 3,3>& projection() const;
};

class Ogl33Viewport {
private:
	float x;
	float y;
	float width;
	float height;
public:
	Ogl33Viewport(float x, float y,float w, float h);

	void use();
};

class Ogl33Mesh final : public Ogl33Resource {
private:
	std::array<GLuint,3> ids;
public:
	Ogl33Mesh();
	Ogl33Mesh(std::array<GLuint,3>&&);
	~Ogl33Mesh();
	Ogl33Mesh(Ogl33Mesh&&);

	void bindVertex() const;
	void bindUV() const;
	void bindIndex() const;
};

class Ogl33Texture final : public Ogl33Resource {
private:
	GLuint tex_id;
public:
	Ogl33Texture();
	Ogl33Texture(GLuint tex_id);
	~Ogl33Texture();
	Ogl33Texture(Ogl33Texture&&);

	void bind() const;
};

class Ogl33Program final : public Ogl33Resource {
private:
	GLuint program_id;

	GLuint texture_uniform;
	GLuint mvp_uniform;
public:
	Ogl33Program();
	Ogl33Program(GLuint, GLuint, GLuint);
	~Ogl33Program();

	Ogl33Program(Ogl33Program&&);

	void setTexture(const Ogl33Texture&);
	void setMvp(const Matrix<float,3,3>&);
	void setMesh(const Ogl33Mesh&);

	void use();
};

class Ogl33RenderTarget {
protected:
	~Ogl33RenderTarget() = default;

	std::array<float, 4> clear_colour = {0.f, 0.f, 0.f, 1.f};
public:
	virtual void beginRender() = 0;
	virtual void endRender() = 0;

	void setClearColour(const std::array<float, 4>& colour);

	virtual void bind() = 0;

	virtual size_t width() const = 0;
	virtual size_t height() const = 0;
};

class Ogl33Window final : public Ogl33RenderTarget {
private:
	Own<GlWindow> window;
public:
	Ogl33Window(Own<GlWindow>&&);

	void show();
	void hide();

	Conveyor<RenderEvent::Events> listenToWindowEvents();

	void beginRender() override;
	void endRender() override;

	/**
	* This binds the window as the current 0 framebuffer in its context
	* This is different to glBindFramebuffer(0);
	*/
	void bindAsMain();

	/**
	*
	*/
	void bind() override;

	size_t width() const override;
	size_t height() const override;
};

class Ogl33RenderTexture final : public Ogl33RenderTarget {
private:
public:
	~Ogl33RenderTexture();

	void beginRender() override;
	void endRender() override;

	void bind() override;

	size_t width() const override;
	size_t height() const override;
};

/// @todo this storage kinda feels hacky
/// An idea would be to use evenly numbered IDs for RenderWindows
/// and odd numbered IDs for RenderTextures
class Ogl33RenderTargetStorage {
private:
	std::map<RenderTargetId, Ogl33RenderTexture> render_textures;
	std::map<RenderTargetId, Ogl33Window> windows;

	RenderTargetId max_free_id = 1;
	std::priority_queue<RenderTargetId, std::vector<RenderTargetId>, std::greater<RenderTargetId>> free_ids;
public:
	RenderTextureId insert(Ogl33RenderTexture&& render_texture);
	RenderWindowId insert(Ogl33Window&& render_window);
	void erase(const RenderTargetId& id);

	bool exists(const RenderTargetId& id) const;
	Ogl33RenderTarget* operator[](const RenderTargetId& id);
	const Ogl33RenderTarget* operator[](const RenderTargetId& id) const;

	Ogl33Window* getWindow(const RenderWindowId&);
	Ogl33RenderTexture* getRenderTexture(const RenderTextureId&);
};

struct Ogl33RenderProperty {
	MeshId mesh_id;
	TextureId texture_id;
};

class Ogl33Scene {
public:
	struct RenderObject {
		RenderPropertyId id;
		float x;
		float y;
		float angle;
	};
private:
	std::unordered_map<RenderObjectId, RenderObject> objects;
public:

	RenderObjectId createObject(const RenderPropertyId&);
	void destroyObject(const RenderObjectId&);
	void setObjectPosition(const RenderObjectId&, float, float);
	void setObjectRotation(const RenderObjectId&, float);

	void visit(const Ogl33Camera&, std::vector<RenderObject*>&);
};

class Ogl33RenderStage {
private:
	void renderOne(Ogl33Program& program, Ogl33RenderProperty& property, Ogl33Scene::RenderObject& object, Ogl33Mesh& mesh, Ogl33Texture&, Matrix<float, 3, 3>& vp);
public:
	RenderTargetId target_id;
	RenderSceneId scene_id;
	RenderCameraId camera_id;
	ProgramId program_id;

	void render(Ogl33Render& render);
};

class Ogl33Render final : public LowLevelRender {
private:
	Own<GlContext> context;

	Ogl33RenderTargetStorage render_targets;
	bool loaded_glad = false;
	GLuint vao = 0;

	std::unordered_map<MeshId, Ogl33Mesh> meshes;
	std::unordered_map<TextureId, Ogl33Texture> textures;
	std::unordered_map<ProgramId, Ogl33Program> programs;
	std::unordered_map<RenderCameraId, Ogl33Camera> cameras;
	std::unordered_map<RenderStageId, Ogl33RenderStage> render_stages;
	std::unordered_map<RenderViewportId, Ogl33Viewport> viewports;
	std::unordered_map<RenderPropertyId, Ogl33RenderProperty> render_properties;
	std::unordered_map<RenderSceneId, Own<Ogl33Scene>> scenes;

	std::unordered_multimap<RenderTargetId, RenderStageId> render_target_stages;

	struct RenderTargetUpdate {
		std::chrono::steady_clock::duration seconds_per_frame;
		std::chrono::steady_clock::time_point next_update;
	};
	std::unordered_map<RenderTargetId, RenderTargetUpdate> render_target_times;	

	void stepRenderTargetTimes(const std::chrono::steady_clock::time_point&);

	std::queue<RenderTargetId> render_target_draw_tasks;
public:
	Ogl33Render(Own<GlContext>&&);
	~Ogl33Render();

	Ogl33Scene* getScene(const RenderSceneId&);
	Ogl33Camera* getCamera(const RenderCameraId&);
	Ogl33Program* getProgram(const ProgramId&);
	Ogl33RenderProperty* getProperty(const RenderPropertyId&);
	Ogl33Mesh* getMesh(const MeshId&);
	Ogl33Texture* getTexture(const TextureId&);

	MeshId createMesh(const MeshData&) override;
	void destroyMesh(const MeshId&) override;

	TextureId createTexture(const Image&) override;
	void destroyTexture(const TextureId&) override;

	RenderWindowId createWindow(const RenderVideoMode&, const std::string& title) override;
	void setWindowDesiredFPS(const RenderWindowId&, float fps) override;
	void setWindowVisibility(const RenderWindowId& id, bool show) override;
	void destroyWindow(const RenderWindowId& id) override;

	ProgramId createProgram(const std::string& vertex_src, const std::string& fragment_src) override;
	void destroyProgram(const ProgramId&) override;

	RenderCameraId createCamera() override;
	void setCameraPosition(const RenderCameraId&, float x, float y) override;
	void setCameraRotation(const RenderCameraId&, float alpha) override;
	void setCameraOrthographic(const RenderCameraId&, float, float, float, float, float, float) override;
	Conveyor<RenderEvent::Events> listenToWindowEvents(const RenderWindowId&) override;
	void destroyCamera(const RenderCameraId&) override;

	RenderStageId createStage(const RenderTargetId& id, const RenderSceneId&, const RenderCameraId&, const ProgramId&) override;
	void destroyStage(const RenderStageId&) override;

	RenderViewportId createViewport() override;
	void setViewportRect(const RenderViewportId&, float, float, float, float) override;
	void destroyViewport(const RenderViewportId&) override;

	RenderPropertyId createProperty(const MeshId&, const TextureId&) override;
	void destroyProperty(const RenderPropertyId&) override;

	RenderSceneId createScene() override;
	RenderObjectId createObject(const RenderSceneId&, const RenderPropertyId&) override;
	void destroyObject(const RenderSceneId&, const RenderObjectId&) override;
	void setObjectPosition(const RenderSceneId&, const RenderObjectId&, float, float) override;
	void setObjectRotation(const RenderSceneId&, const RenderObjectId&, float) override;
	void destroyScene(const RenderSceneId&) override;

	void step(const std::chrono::steady_clock::time_point&) override;
	void flush() override;
};
}

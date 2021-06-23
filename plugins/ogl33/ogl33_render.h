#pragma once

#include "render/render.h"

#include <queue>
#include <set>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <cassert>
#include <complex>

#include <iostream>

#include "ogl33_bindings.h"

#include <kelgin/window/gl/gl_context.h>
#include <kelgin/common.h>

#include "common/math.h"

namespace gin {
class Ogl33Render;
class Ogl33RenderProperty;
class Ogl33RenderObject;

class Ogl33Camera {
private:
	Matrix<float, 3, 3> projection_matrix;

	std::array<float, 2> position = {{0.f, 0.f}};
	std::complex<float> angle = std::polar(1.f, 0.f);

	std::array<float, 2> old_position = {{0.f, 0.f}};
	std::complex<float> old_angle = std::polar(1.f, 0.f);
public:
	Ogl33Camera();

	void setViewPosition(float x, float y);
	void setViewRotation(float angle);

	void updateState(float relative_tp);

	void setOrtho(float left, float right, float top, float bot);

	Matrix<float, 3,3> view(float relative_tp) const;
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

/// @todo Implement usage of VertexArrayObjects
class Ogl33Mesh {
private:
	GLuint vao;
	std::array<GLuint,2> ids;
	size_t indices;
	
public:
	Ogl33Mesh();
	Ogl33Mesh(GLuint vao, std::array<GLuint,2>&&, size_t ind);
	~Ogl33Mesh();
	Ogl33Mesh(Ogl33Mesh&&);

	void bindVertexArray() const;

	// void bindAttribute() const;

	void bindAttribute() const;
	void bindIndex() const;

	void setData(const MeshData& data);

	size_t indexCount() const;
};

class Ogl33Mesh3d {
private:
	GLuint vao;
	std::array<GLuint, 2> ids;
	size_t indices;

public:
	Ogl33Mesh3d();
	Ogl33Mesh3d(GLuint, std::array<GLuint, 2>&&, size_t);
	~Ogl33Mesh3d();
	Ogl33Mesh3d(Ogl33Mesh3d&&);

	void bindAttribute() const;
	void bindIndex() const;

	void setData(const Mesh3dData& data);

	size_t indexCount() const;
};

class Ogl33Camera3d {
private:
	Matrix<float, 4, 4> projection_matrix;
	Matrix<float, 4, 4> view_matrix;
public:
	Ogl33Camera3d();

	void setOrtho(float left, float right, float top, float bottom, float near, float far);
	void setViewPosition(float x, float y, float z);
	void setViewRotation(float alpha, float beta, float gamma);

	const Matrix<float, 4, 4>& projection() const;
	const Matrix<float, 4, 4>& view() const;
};

class Ogl33Texture {
private:
	GLuint tex_id;
public:
	Ogl33Texture();
	Ogl33Texture(GLuint tex_id);
	~Ogl33Texture();
	Ogl33Texture(Ogl33Texture&&);

	void bind() const;
};

class Ogl33Program3d {
private:
	GLuint program_id;

	GLuint texture_uniform;
	GLuint mvp_uniform;
public:
	Ogl33Program3d();
	Ogl33Program3d(GLuint program, GLuint texture, GLuint mvp);
	~Ogl33Program3d();

	void setTexture(const Ogl33Texture& texture_id);
	void setMvp(const Matrix<float, 4, 4>& mvp);
	void setMesh(const Ogl33Mesh3d& mesh_id);

	void use();
};

class Ogl33Program {
private:
	GLuint program_id;

	GLuint texture_uniform;
	GLuint mvp_uniform;
	GLuint layer_uniform;
public:
	Ogl33Program();
	Ogl33Program(GLuint, GLuint, GLuint, GLuint);
	~Ogl33Program();

	Ogl33Program(Ogl33Program&&);

	void setTexture(const Ogl33Texture&);
	void setMvp(const Matrix<float,3,3>&);
	void setMesh(const Ogl33Mesh&);
	void setLayer(float);
	void setLayer(int16_t);

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

class Ogl33RenderProperty {
public:
	MeshId mesh_id;
	TextureId texture_id;
};

class Ogl33RenderProperty3d {
public:
	Mesh3dId mesh_id;
	TextureId texture_id;
};

class Ogl33Scene {
public:
	struct RenderObject {
		RenderPropertyId id = 0;

		std::array<float,2> pos{{0.f, 0.f}};
		std::complex<float> angle = std::polar(1.f, 0.f);

		std::array<float,2> old_pos{{0.f, 0.f}};
		std::complex<float> old_angle = std::polar(1.f, 0.f);

		float layer = 0.f;
		bool visible = true;
	};
private:
	std::unordered_map<RenderObjectId, RenderObject> objects;
public:

	ErrorOr<RenderObjectId> createObject(const RenderPropertyId& id) noexcept;
	void destroyObject(const RenderObjectId& id) noexcept;
	Error setObjectPosition(const RenderObjectId& id, float x, float y, bool interpolate) noexcept;
	Error setObjectRotation(const RenderObjectId& id, float a, bool interpolate) noexcept;
	Error setObjectVisibility(const RenderObjectId& id, bool v) noexcept;
	Error setObjectLayer(const RenderObjectId& id, float l) noexcept;

	void visit(const Ogl33Camera&, std::vector<RenderObject*>&);

	void updateState(float interval);
};

class Ogl33Scene3d {
public:
	struct RenderObject {
		RenderProperty3dId id = 0;

		std::array<float, 3> pos{{0.f, 0.f, 0.f}};
		std::array<float, 3> rot{{0.f, 0.f, 0.f}};

		std::array<float, 3> old_pos{{0.f, 0.f, 0.f}};
		std::array<float, 3> old_rot{{0.f, 0.f, 0.f}};

		bool visible = true;

		RenderObject(const RenderProperty3dId& p_id):id{p_id}{}
	};
private:
	std::unordered_map<RenderObject3dId, RenderObject> objects;
public:

	ErrorOr<RenderObject3dId> createObject(const RenderProperty3dId&) noexcept;
	void destroyObject(const RenderObject3dId&) noexcept;

	Error setObjectPosition(const RenderObject3dId&, float, float, float) noexcept;
	Error setObjectRotation(const RenderObject3dId&, float, float, float) noexcept;
	Error setObjectVisibility(const RenderObject3dId&, bool) noexcept;

	void visit(const Ogl33Camera3d&, std::vector<RenderObject>&);

	void updateState();
};

class Ogl33RenderStage {
private:
	void renderOne(Ogl33Program& program, Ogl33RenderProperty& property, Ogl33Scene::RenderObject& object, Ogl33Mesh& mesh, Ogl33Texture&, Matrix<float, 3, 3>& vp, float time_interval);
public:
	RenderTargetId target_id;
	RenderViewportId viewport_id;
	RenderSceneId scene_id;
	RenderCameraId camera_id;
	ProgramId program_id;

	void render(Ogl33Render& render, float time_interval);
};

class Ogl33RenderStage3d {
private:
	void renderOne(Ogl33Program3d& program, Ogl33RenderProperty3d& property, Ogl33Scene3d::RenderObject& object, Ogl33Mesh3d& mesh, Ogl33Texture& texture, Matrix<float, 4, 4>& vp);
public:
	RenderTargetId target_id;
	RenderViewportId viewport_id;
	RenderScene3dId scene_id;
	RenderCamera3dId camera_id;
	Program3dId program_id;

	void render(Ogl33Render& render);
};

class Ogl33Render final : public LowLevelRender, public LowLevelRender2D, public LowLevelRender3D {
private:
	Own<GlContext> context;

	Ogl33RenderTargetStorage render_targets;
	bool loaded_glad = false;

	// General Resource Storage
	std::unordered_map<TextureId, Ogl33Texture> textures;
	std::unordered_map<RenderViewportId, Ogl33Viewport> viewports;

	// 2D Resource Storage
	std::unordered_map<MeshId, Ogl33Mesh> meshes;
	std::unordered_map<ProgramId, Ogl33Program> programs;
	std::unordered_map<RenderCameraId, Ogl33Camera> cameras;
	std::unordered_map<RenderPropertyId, Ogl33RenderProperty> render_properties;
	std::unordered_map<RenderSceneId, Ogl33Scene> scenes;
	std::unordered_map<RenderStageId, Ogl33RenderStage> render_stages;

	std::unordered_map<RenderAnimationId, RenderAnimationData2D> animation_data;

	// Stages listening  to RenderTarget changes
	std::unordered_multimap<RenderTargetId, RenderStageId> render_target_stages;

	// 3D Resource Storage
	std::unordered_map<Mesh3dId, Ogl33Mesh3d> meshes_3d;
	std::unordered_map<Program3dId, Ogl33Program3d> programs_3d;
	std::unordered_map<RenderCamera3dId, Ogl33Camera3d> cameras_3d;
	std::unordered_map<RenderProperty3dId, Ogl33RenderProperty3d> render_properties_3d;
	std::unordered_map<RenderScene3dId, Ogl33Scene3d> scenes_3d;
	std::unordered_map<RenderStage3dId, Ogl33RenderStage3d> render_stages_3d;

	// Stages listening  to RenderTarget changes
	std::unordered_multimap<RenderTargetId, RenderStage3dId> render_target_stages_3d;

	struct RenderTargetUpdate {
		std::chrono::steady_clock::duration seconds_per_frame;
		std::chrono::steady_clock::time_point next_update;
	};
	std::unordered_map<RenderTargetId, RenderTargetUpdate> render_target_times;	

	void stepRenderTargetTimes(const std::chrono::steady_clock::time_point&);

	std::queue<RenderTargetId> render_target_draw_tasks;

	std::chrono::steady_clock::time_point old_time_point;
	std::chrono::steady_clock::time_point time_point;
public:
	Ogl33Render(Own<GlContext>&&);
	~Ogl33Render();

	Ogl33Scene* getScene(const RenderSceneId&) noexcept;
	Ogl33Camera* getCamera(const RenderCameraId&) noexcept;
	Ogl33Program* getProgram(const ProgramId&) noexcept;
	Ogl33RenderProperty* getProperty(const RenderPropertyId&) noexcept;
	Ogl33Mesh* getMesh(const MeshId&) noexcept;
	Ogl33Texture* getTexture(const TextureId&) noexcept;

	Ogl33Scene3d* getScene3d(const RenderScene3dId&) noexcept;
	Ogl33Camera3d* getCamera3d(const RenderCamera3dId&) noexcept;
	Ogl33Program3d* getProgram3d(const Program3dId&) noexcept;
	Ogl33RenderProperty3d* getRenderProperty3d(const RenderProperty3dId&) noexcept;
	Ogl33Mesh3d* getMesh3d(const Mesh3dId& ) noexcept;

	LowLevelRender2D* interface2D() noexcept override {return this;}
	LowLevelRender3D* interface3D() noexcept override {return this;}
 
	Conveyor<MeshId> createMesh(const MeshData&) noexcept override;
	Conveyor<void> setMeshData(const MeshId&, const MeshData&) noexcept override;
	Conveyor<void> destroyMesh(const MeshId&) noexcept override;

	Conveyor<TextureId> createTexture(const Image&) noexcept override;
	Conveyor<void> destroyTexture(const TextureId&) noexcept override;

	Conveyor<RenderWindowId> createWindow(const RenderVideoMode&, const std::string& title) noexcept override;
	Conveyor<void> setWindowDesiredFPS(const RenderWindowId&, float fps) noexcept override;
	Conveyor<void> setWindowVisibility(const RenderWindowId& id, bool show) noexcept override;
	Conveyor<void> destroyWindow(const RenderWindowId& id) noexcept override;

	Conveyor<RenderEvent::Events> listenToWindowEvents(const RenderWindowId&) noexcept override;

	// 2D

	Conveyor<ProgramId> createProgram(const std::string& vertex_src, const std::string& fragment_src) noexcept override;
	Conveyor<ProgramId> createProgram() noexcept override;
	Conveyor<void> destroyProgram(const ProgramId&) noexcept override;

	Conveyor<RenderCameraId> createCamera() noexcept override;
	Conveyor<void> setCameraPosition(const RenderCameraId&, float x, float y) noexcept override;
	Conveyor<void> setCameraRotation(const RenderCameraId&, float alpha) noexcept override;
	Conveyor<void> setCameraOrthographic(const RenderCameraId&, float, float, float, float) noexcept override;
	Conveyor<void> destroyCamera(const RenderCameraId&) noexcept override;
	
	Conveyor<RenderStageId> createStage(const RenderTargetId& id, const RenderViewportId&, const RenderSceneId&, const RenderCameraId&, const ProgramId&) noexcept override;
	Conveyor<void> destroyStage(const RenderStageId&) noexcept override;

	Conveyor<RenderViewportId> createViewport() noexcept override;
	Conveyor<void> setViewportRect(const RenderViewportId&, float, float, float, float) noexcept override;
	Conveyor<void> destroyViewport(const RenderViewportId&) noexcept override;

	Conveyor<RenderPropertyId> createProperty(const MeshId&, const TextureId&) noexcept override;
	Conveyor<void> setPropertyMesh(const RenderPropertyId&, const MeshId& id) noexcept override;
	Conveyor<void> setPropertyTexture(const RenderPropertyId&, const TextureId& id) noexcept override;
	Conveyor<void> destroyProperty(const RenderPropertyId&) noexcept override;

	Conveyor<RenderSceneId> createScene() noexcept override;
	Conveyor<RenderObjectId> createObject(const RenderSceneId&, const RenderPropertyId&) noexcept override;
	Conveyor<void> setObjectPosition(const RenderSceneId&, const RenderObjectId&, float, float, bool interpolate = true) noexcept override;
	Conveyor<void> setObjectRotation(const RenderSceneId&, const RenderObjectId&, float, bool interpolate = true) noexcept override;
	Conveyor<void> setObjectVisibility(const RenderSceneId&, const RenderObjectId&, bool) noexcept override;
	Conveyor<void> setObjectLayer(const RenderSceneId& id, const RenderObjectId&, float) noexcept override;
	Conveyor<void> destroyObject(const RenderSceneId&, const RenderObjectId&) noexcept override;
	Conveyor<void> destroyScene(const RenderSceneId&) noexcept override;

	Conveyor<RenderAnimationId> createAnimation(const RenderAnimationData2D& data) noexcept override;
	Conveyor<void> destroyAnimation(const RenderAnimationId&) noexcept override;
	Conveyor<void> playAnimation(const RenderSceneId& id, const RenderObjectId& obj, const RenderAnimationId&) noexcept override;

	// 3D
	Conveyor<Mesh3dId> createMesh3d(const Mesh3dData&) noexcept override;
	Conveyor<void> destroyMesh3d(const Mesh3dId&) noexcept override;

	/// @todo layout api ideas
	Conveyor<RenderProperty3dId> createProperty3d(const Mesh3dId&, const TextureId&) noexcept override;
	Conveyor<void> destroyProperty3d(const RenderProperty3dId&) noexcept override;

	Conveyor<Program3dId> createProgram3d(const std::string& vertex_src, const std::string& fragment_src) noexcept override;
	Conveyor<Program3dId> createProgram3d() noexcept override;
	Conveyor<void> destroyProgram3d(const Program3dId&) noexcept override;

	Conveyor<RenderScene3dId> createScene3d() noexcept override;
	Conveyor<RenderObject3dId> createObject3d(const RenderScene3dId&, const RenderProperty3dId&) noexcept override;
	Conveyor<void> destroyObject3d(const RenderScene3dId&, const RenderObject3dId&) noexcept override;
	Conveyor<void> destroyScene3d(const RenderScene3dId&) noexcept override;

	Conveyor<RenderCamera3dId> createCamera3d() noexcept override;
	Conveyor<void> setCamera3dPosition(const RenderCamera3dId&, float, float, float) noexcept override;
	Conveyor<void> setCamera3dOrthographic(const RenderCamera3dId&, float, float, float, float, float, float) noexcept override;
	Conveyor<void> destroyCamera3d(const RenderCamera3dId&) noexcept override;

	Conveyor<RenderStage3dId> createStage3d(const RenderTargetId&, const RenderViewportId&, const RenderScene3dId&, const RenderCamera3dId&, const Program3dId&) noexcept override;
	Conveyor<void> destroyStage3d(const RenderStage3dId&) noexcept override;

	void step(const std::chrono::steady_clock::time_point&) noexcept override;
	void flush() noexcept override;

	void updateTime(const std::chrono::steady_clock::time_point& new_old_time_point, const std::chrono::steady_clock::time_point& new_time_point) noexcept override;
};
}

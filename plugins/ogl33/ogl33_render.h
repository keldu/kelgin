#pragma once

#include "render/render.h"

#include <queue>
#include <set>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <cassert>

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

class Ogl33Mesh final : public Ogl33Resource {
private:
	std::array<GLuint,3> ids;
public:
	Ogl33Mesh();
	Ogl33Mesh(std::array<GLuint,3>&&);
	~Ogl33Mesh();
};

class Ogl33Texture final : public Ogl33Resource {
private:
	GLuint tex_id;
public:
	Ogl33Texture();
	Ogl33Texture(GLuint tex_id);
	~Ogl33Texture();
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
};

class Ogl33Window final : public Ogl33RenderTarget {
private:
	Own<GlWindow> window;
public:
	Ogl33Window(Own<GlWindow>&&);

	void show();
	void hide();

	void beginRender() override;
	void endRender() override;
};

class Ogl33RenderTexture final : public Ogl33RenderTarget {
private:
public:
	~Ogl33RenderTexture();

	void beginRender() override;
	void endRender() override;
};

/// @todo maybe do this with deque?
/// I need a sorted deque, because i need to check the lowest and the highest elements in
/// the sorted queue :/
/// STL doesn't provide this.... Need something like std::priority_deque instead of std::priority_queue
template<typename I, typename T>
class Ogl33RenderResourceVector {
private:
	struct RefCountedRes {
		size_t references = 0;
		T resource;
	};
	std::vector<RefCountedRes> resources;
	std::priority_queue<I, std::vector<I>, std::greater<I>> free_ids;
public:
	Ogl33RenderResourceVector(){
		static_assert(std::is_base_of<Ogl33Resource, T>::value, "Type needs to be a child of Ogl33Resource");
	}

	I insert(T&& data){
		I id;
		if(free_ids.empty()){
			id = resources.size();
			resources.push_back({1,std::move(data)});
		}else{
			id = free_ids.top();
			free_ids.pop();
			if(id < resources.size()){
				resources[id] = {1,std::move(data)};
			}else{
				while(!free_ids.empty()){
					free_ids.pop();
				}
				resources.push_back({1,std::move(data)});
			}

		}

		return id;
	}

	void erase(const I& id){
		/// @unsure Should I handle this differently?
		assert(id < resources.size());
		if( (id+1) == resources.size()){
			resources.pop_back();
			while(!resources.empty() && resources.back().references == 0){
				resources.pop_back();
			}
			if(free_ids.top() >= resources.size()){
				while(!free_ids.empty()){
					free_ids.pop();
				}
			}
		}else{
			resources[id] = RefCountedRes{0,{}};
			free_ids.push(id);
		}
	}

	T& operator[](const I& id){
		return resources[id];
	}

	const T& operator[](const I& id)const{
		return resources[id];
	}

	/// @todo implement iterator with find and so on
};

class Ogl33Render;
class Ogl33RenderWorld final : public RenderWorld {
private:
	Ogl33Render* renderer;
public:
	Ogl33RenderWorld(Ogl33Render&);
	~Ogl33RenderWorld();

	void destroyedRender();
};

/// @todo this storage kinda feels hacky
/// An idea would be to use evenly numbered IDs for RenderWindows
/// and odd numbered IDs for RenderTextures
class Ogl33RenderTargetStorage {
private:
	std::map<RenderTargetId, Ogl33RenderTexture> render_textures;
	std::map<RenderTargetId, Ogl33Window> windows;

	RenderTargetId max_free_id = 0;
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

class Ogl33Render final : public Render {
private:
	Own<GlContext> context;

	Ogl33RenderResourceVector<MeshId, Ogl33Mesh> meshes;
	Ogl33RenderResourceVector<TextureId, Ogl33Texture> textures;
	Ogl33RenderResourceVector<ProgramId, Ogl33Program> programs;

	Ogl33RenderTargetStorage render_targets;

	std::set<Ogl33RenderWorld*> render_worlds;

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

	MeshId createMesh(const MeshData&) override;
	void destroyMesh(const MeshId&) override;

	TextureId createTexture(const Image&) override;
	void destroyTexture(const TextureId&) override;

	Own<RenderWorld> createWorld() override;

	RenderWindowId createWindow(const RenderVideoMode&, const std::string&) override;
	void setWindowDesiredFPS(const RenderWindowId&, float fps) override;
	void destroyWindow(const RenderWindowId& id) override;

	ProgramId createProgram(const std::string& vertex_src, const std::string& fragment_src) override;
	void destroyProgram(const ProgramId&) override;

	void setWindowVisibility(const RenderWindowId& id, bool show) override;

	void destroyedRenderWorld(Ogl33RenderWorld& rw);

	void flush() override;
	void step(const std::chrono::steady_clock::time_point&) override;
};
}

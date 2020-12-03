#pragma once

#include "render/render.h"

#include <queue>
#include <set>
#include <vector>
#include <cassert>

#include "ogl33_bindings.h"

#include <kelgin/window/gl/gl_context.h>
#include <kelgin/common.h>

namespace gin {
/**
* Helper parent
*/
class Ogl33Resource {
private:
	GLuint rid;
protected:
	~Ogl33Resource() = default;
public:
	Ogl33Resource(GLuint);

	GLuint id() const;
};

class Ogl33Mesh final : public Ogl33Resource {
public:
	using Ogl33Resource::Ogl33Resource;
	~Ogl33Mesh();
};

class Ogl33Texture final : public Ogl33Resource {
public:
	using Ogl33Resource::Ogl33Resource;
	~Ogl33Texture();
};

class Ogl33Program final : public Ogl33Resource {
public:
	using Ogl33Resource::Ogl33Resource;
	~Ogl33Program();
};

class Ogl33Window {
private:
	Own<GlWindow> window;
public:
	Ogl33Window(GLuint, Own<GlWindow>&&);
};

class Ogl33RenderTexture {
private:
public:
	~Ogl33RenderTexture();
};

/// @todo maybe do this with deque?
/// I need a sorted deque, because i need to check the lowest and the highest elements in
/// the sorted queue :/
/// STL doesn't provide this.... Need something like std::priority_deque instead of std::priority_queue
template<typename I, typename T>
class Ogl33RenderResourceVector {
private:
	std::vector<T> resources;
	std::priority_queue<I, std::vector<I>, std::greater<I>> free_ids;
public:
	Ogl33RenderResourceVector(){
		static_assert(std::is_base_of<Ogl33Resource, T>::value, "Type needs to be a child of Ogl33Resource");
	}

	I insert(T&& data){
		if(free_ids.empty()){
			resources.push_back(std::move(data));
		}else{
			I id = free_ids.top();
			free_ids.pop();
			if(id < resources.size()){
				resources[id] = std::move(data);
			}else{
				free_ids.clear();
				resources.push_back(std::move(data));
			}

		}
	}

	void erase(const I& id){
		/// @unsure Should I handle this differently?
		assert(id < resources.size());
		if( (id+1) == resources.size()){
			resources.pop_back();
			while(!resources.empty() && resources.back().id() == 0){
				resources.pop_back();
			}
			if(free_ids.top() >= resources.size()){
				free_ids.clear();
			}
		}else{
			resources[id] = T{0};
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

struct Ogl33RenderTargetStorage {
	std::map<RenderTextureId, Ogl33RenderTexture> render_textures;
	std::map<RenderWindowId, Ogl33Window> windows;

	size_t max_free_id = 0;
	std::priority_queue<RenderTargetId, std::vector<RenderTargetId>, std::greater<RenderTargetId>> free_ids;
};

class Ogl33Render final : public Render {
private:
	Own<GlContext> context;

	Ogl33RenderResourceVector<MeshId, Ogl33Mesh> meshes;
	Ogl33RenderResourceVector<TextureId, Ogl33Texture> textures;
	Ogl33RenderResourceVector<ProgramId, Ogl33Program> programs;

	std::map<RenderTextureId, Ogl33RenderTexture> render_textures;
	std::map<RenderWindowId, Ogl33Window> windows;

	std::set<Ogl33RenderWorld*> render_worlds;
public:
	Ogl33Render(Own<GlContext>&&);
	~Ogl33Render();

	Own<RenderWorld> createWorld() override;

	void destroyedRenderWorld(Ogl33RenderWorld& rw);
};
}

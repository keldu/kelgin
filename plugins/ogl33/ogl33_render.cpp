#include "ogl33_render.h"

#include <iostream>
#include <cassert>

namespace gin {
Ogl33Mesh::Ogl33Mesh():
	ids{0,0,0}
{
}

Ogl33Mesh::Ogl33Mesh(std::array<GLuint, 3>&& i):
	ids{std::move(i)}
{}

Ogl33Mesh::~Ogl33Mesh(){
	glDeleteBuffers(3, &ids[0]);
}

Ogl33Texture::Ogl33Texture():
	Ogl33Texture(0)
{}

Ogl33Texture::Ogl33Texture(GLuint tex_id):
	tex_id{tex_id}
{}

Ogl33Texture::~Ogl33Texture(){
	glDeleteTextures(1, &tex_id);
}

Ogl33Program::Ogl33Program(GLuint p_id, GLuint tex_id, GLuint mvp_id):
	program_id{p_id},
	texture_uniform{tex_id},
	mvp_uniform{mvp_id}
{}

Ogl33Program::Ogl33Program():
	Ogl33Program(0,0,0)
{}

Ogl33Program::~Ogl33Program(){
	glDeleteProgram(program_id);
}

void Ogl33Program::setTexture(const Ogl33Texture& tex){
	glUniform1i(texture_uniform, 0);
	glActiveTexture(GL_TEXTURE0);
}

void Ogl33Program::setMvp(const Matrix<float,3,3>& mvp){
	glUniformMatrix3fv(mvp_uniform, 1, GL_TRUE, &mvp(0, 0));
}

void Ogl33Program::setMesh(const Ogl33Mesh& mesh){
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
}

void Ogl33Program::use(){
	glUseProgram(program_id);
}

void Ogl33RenderTarget::setClearColour(const std::array<float,4>& colour){
	clear_colour = colour;
}

Ogl33Window::Ogl33Window(Own<GlWindow>&& win):
	window{std::move(win)}
{}

void Ogl33Window::show(){
	if(window){
		window->bind();
		window->show();
	}
}

void Ogl33Window::hide(){
	if(window){
		window->bind();
		window->hide();
	}
}


void Ogl33Window::beginRender(){
	assert(window);
	if(!window){
		return;
	}
	window->bind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Ogl33Window::endRender(){
	assert(window);
	if(!window){
		return;
	}

	window->swap();
}

Ogl33RenderTexture::~Ogl33RenderTexture(){
}

void Ogl33RenderTexture::beginRender(){

}

void Ogl33RenderTexture::endRender(){

}

Ogl33RenderWorld::Ogl33RenderWorld(Ogl33Render& render):
	renderer{&render}
{}

Ogl33RenderWorld::~Ogl33RenderWorld(){
	assert(renderer);
	if(renderer){
		renderer->destroyedRenderWorld(*this);
	}
}

void Ogl33RenderWorld::destroyedRender(){
	assert(renderer);
	renderer = nullptr;
}

RenderTextureId Ogl33RenderTargetStorage::insert(Ogl33RenderTexture&& rt){
	RenderTargetId id;
	if(free_ids.empty()){
		id = max_free_id;
		++max_free_id;
	}else{
		id = free_ids.top();
		free_ids.pop();
	}

	render_textures.insert(std::make_pair(id, std::move(rt)));
	return static_cast<RenderTextureId>(id);
}

RenderWindowId Ogl33RenderTargetStorage::insert(Ogl33Window&& rw){
	RenderTargetId id;
	if(free_ids.empty()){
		id = max_free_id;
		++max_free_id;
	}else{
		id = free_ids.top();
		free_ids.pop();
	}

	windows.insert(std::make_pair(id, std::move(rw)));
	return static_cast<RenderWindowId>(id);
}

void Ogl33RenderTargetStorage::erase(const RenderTargetId& id){
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		render_textures.erase(rt_find);
		return;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		windows.erase(rw_find);
	}
}

bool Ogl33RenderTargetStorage::exists(const RenderTargetId& id) const {
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		return true;
	}

	return windows.find(id) != windows.end();
}

Ogl33RenderTarget* Ogl33RenderTargetStorage::operator[](const RenderTargetId& id){
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		return &rt_find->second;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		return &rw_find->second;
	}

	assert(false);
	return nullptr;
}

const Ogl33RenderTarget* Ogl33RenderTargetStorage::operator[](const RenderTargetId& id)const{
	auto rt_find = render_textures.find(id);
	if(rt_find != render_textures.end()){
		return &rt_find->second;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		return &rw_find->second;
	}

	assert(false);
	return nullptr;
}

Ogl33Window* Ogl33RenderTargetStorage::getWindow(const RenderWindowId& id){
	auto find = windows.find(static_cast<RenderTargetId>(id));
	if(find != windows.end()){
		return &find->second;
	}

	return nullptr;
}

Ogl33RenderTexture* Ogl33RenderTargetStorage::getRenderTexture(const RenderTextureId& id){
	auto find = render_textures.find(static_cast<RenderTextureId>(id));
	if(find != render_textures.end()){
		return &find->second;
	}

	return nullptr;
}

MeshId Ogl33Render::createMesh(const MeshData& data){
	std::array<GLuint,3> ids;

	/// @todo ensure that the current render context is bound

	glGenBuffers(3, &ids[0]);

	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(float), data.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, ids[1]);
	glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(float), data.uvs.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

// This is unnecessary in a correct renderer impl
#ifndef NDEBUG
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif

	return meshes.insert(Ogl33Mesh{std::move(ids)});
}

void Ogl33Render::destroyMesh(const MeshId& id){
	meshes.erase(id);
}

TextureId Ogl33Render::createTexture(const Image& image){
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);



	glBindTexture(GL_TEXTURE_2D, 0);

	return textures.insert(Ogl33Texture{texture_id});
}

void Ogl33Render::destroyTexture(const TextureId& id){
	textures.erase(id);
}

Own<RenderWorld> Ogl33Render::createWorld(){
	Own<Ogl33RenderWorld> world = heap<Ogl33RenderWorld>(*this);

	render_worlds.insert(world.get());

	return world;
}

RenderWindowId Ogl33Render::createWindow(const RenderVideoMode& mode, const std::string& title) {
	auto gl_win = context->createWindow(VideoMode{mode.width,mode.height}, title);
	if(!gl_win){
		return 0;
	}

	return render_targets.insert(Ogl33Window{std::move(gl_win)});
}

void Ogl33Render::setWindowDesiredFPS(const RenderWindowId& id, float fps){
	Ogl33Window* window = render_targets.getWindow(id);
	if(!window){
		return;
	}

	auto& update = render_target_times[static_cast<RenderTargetId>(id)];

	std::chrono::duration<float, std::ratio<1,1>> fps_chrono{1.0f / fps};
	update.next_update = std::chrono::steady_clock::now();
	update.seconds_per_frame = std::chrono::duration_cast<std::chrono::steady_clock::duration>(fps_chrono);
}

void Ogl33Render::destroyWindow(const RenderWindowId& id){
	render_targets.erase(static_cast<RenderTargetId>(id));
}

void Ogl33Render::setWindowVisibility(const RenderWindowId& id, bool show){
	Ogl33Window* window = render_targets.getWindow(id);
	if(!window){
		return;
	}

	if(show){
		window->show();
	}else{
		window->hide();
	}
}

namespace {
GLuint createShader(const std::string &source, GLenum type) {
	GLuint id = glCreateShader(type);
	if (id == 0) {
		std::cerr<<"Failed to create shader"<<error_msg<<std::endl;
		//log_error("Failed to create shader");
		return id;
	}

	GLint result = GL_FALSE;
	int info_length;

	const char *source_data = source.c_str();
	glShaderSource(id, 1, &source_data, nullptr);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_length);
	if (info_length > 1 || result == GL_FALSE) {
		std::string error_msg;
		error_msg.resize(info_length);
		glGetShaderInfoLog(id, info_length, nullptr, &error_msg[0]);

		std::cerr<<"Failed to compile "<<error_msg<<std::endl;
		// log_error(std::string{"Failed to compile "} + error_msg);
	}

	return id;
}
}

ProgramId Ogl33Render::createProgram(const std::string& vertex_src, const std::string& fragment_src){
	GLuint vertex_shader_id =
		createShader(vertex_src, GL_VERTEX_SHADER);
	GLuint fragment_shader_id =
		createShader(vertex_src, GL_FRAGMENT_SHADER);

	if (vertex_shader_id == 0 || fragment_shader_id == 0) {
		if(vertex_shader_id != 0){
			glDeleteShader(vertex_shader_id);
		}
		if(fragment_shader_id != 0){
			glDeleteShader(fragment_shader_id);
		}
		return 0;
	}

	GLuint p_id = glCreateProgram();

	if (p_id == 0) {
		glDeleteShader(vertex_shader_id);
		glDeleteShader(fragment_shader_id);
		return 0;
	}

	glAttachShader(p_id, vertex_shader_id);
	glAttachShader(p_id, fragment_shader_id);
	glLinkProgram(p_id);
	glDetachShader(p_id, vertex_shader_id);
	glDetachShader(p_id, fragment_shader_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	GLuint mvp_id = glGetUniformLocation(p_id, "mvp");
	GLuint texture_sampler_id = glGetUniformLocation(p_id, "texture_sampler");

	return programs.insert(Ogl33Program{p_id, texture_sampler_id, mvp_id});
}

void Ogl33Render::destroyProgram(const ProgramId&){

}

void Ogl33Render::destroyedRenderWorld(Ogl33RenderWorld& rw){
	render_worlds.erase(&rw);
}

Ogl33Render::Ogl33Render(Own<GlContext>&& ctx):
	context{std::move(ctx)}
{}

Ogl33Render::~Ogl33Render(){
	assert(render_worlds.empty());
	for(auto& world : render_worlds){
		world->destroyedRender();
	}
}

void Ogl33Render::stepRenderTargetTimes(const std::chrono::steady_clock::time_point& tp){
	for(auto& iter : render_target_times){
		if(iter.second.next_update <= tp){
			iter.second.next_update += iter.second.seconds_per_frame;
			render_target_draw_tasks.push(iter.first);
			while(iter.second.next_update <= tp){
				iter.second.next_update += iter.second.seconds_per_frame;
			}
		}
	}
}

void Ogl33Render::flush(){
	assert(context);
	if(context){
		context->flush();
	}
}

void Ogl33Render::step(const std::chrono::steady_clock::time_point& tp){
	assert(context);
	if(!context){
		return;
	}

	stepRenderTargetTimes(tp);

	for(;!render_target_draw_tasks.empty(); render_target_draw_tasks.pop()){
		auto front = render_target_draw_tasks.front();

		Ogl33RenderTarget* target = render_targets[front];
		assert(target);
		if(!target){
			continue;
		}

		target->beginRender();

		target->endRender();
	}
}
}

extern "C" gin::Render* createRenderer(gin::AsyncIoProvider& io_provider){
	gin::Own<gin::GlContext> context = gin::createGlContext(io_provider, gin::GlSettings{});
	if(!context){
		return nullptr;
	}

	std::cout<<"Creating ogl33 plugin"<<std::endl;
	return new gin::Ogl33Render(std::move(context));
}

extern "C" void destroyRenderer(gin::Render* render){
	if(!render){
		return;
	}

	std::cout<<"Destroying ogl33 plugin"<<std::endl;
	delete render;
}

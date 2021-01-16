#include "ogl33_render.h"

#include <iostream>
#include <cassert>

namespace gin {
namespace {
/** 
* @todo optimize with a custom container
* Fast implementation so it works
*/
template<typename Id, typename T>
Id searchForFreeId(std::unordered_map<Id, T>& map){
	Id id = 1;
	for(; true; ++id){
		auto find = map.find(id);
		if(find == map.end()){
			return id;
		}
	}
	return id;
}
}

Ogl33Camera::Ogl33Camera()
{
	for(size_t i = 0; i < 3; ++i){
		projection_matrix(i,i) = 1.0f;
		view_matrix(i,i) = 1.0f;
	}
}

void Ogl33Camera::setOrtho(float left, float right, float bottom, float top){
	projection_matrix(0,0) = 2.0 / (right - left);
	projection_matrix(0,1) = 0.f;
	projection_matrix(0,2) = -(right+left) / (right-left);

	projection_matrix(1,0) = 0.f;
	projection_matrix(1,1) = 2.0f / (top-bottom);
	projection_matrix(1,2) = -(top+bottom)/ (top-bottom);

	projection_matrix(2,0) = 0.f;
	projection_matrix(2,1) = 0.f;
	projection_matrix(2,2) = 1.0f;
}

void Ogl33Camera::setViewPosition(float x, float y){
	view_matrix(0, 2) = -x;
	view_matrix(1, 2) = -y;
}

void Ogl33Camera::setViewRotation(float angle){
	view_matrix(0,0) = cos(angle);
	view_matrix(0,1) = sin(angle);
	view_matrix(1,0) = -sin(angle);
	view_matrix(1,1) = cos(angle);
}

const Matrix<float, 3,3>& Ogl33Camera::view() const {
	return view_matrix;
}

const Matrix<float, 3,3>& Ogl33Camera::projection() const {
	return projection_matrix;
}

Ogl33Viewport::Ogl33Viewport(float x, float y, float width, float height):
	x{x},
	y{y},
	width{width},
	height{height}
{}

void Ogl33Viewport::use(){
	glViewport(x, y, width, height);
}

Ogl33Texture::Ogl33Texture():
	Ogl33Texture(0)
{}

Ogl33Texture::Ogl33Texture(GLuint tex_id):
	tex_id{tex_id}
{}

Ogl33Texture::~Ogl33Texture(){
	if(tex_id > 0){
		glDeleteTextures(1, &tex_id);
	}
}

Ogl33Texture::Ogl33Texture(Ogl33Texture&& rhs):
	tex_id{rhs.tex_id}
{
	rhs.tex_id = 0;
}

void Ogl33Texture::bind() const{
	glBindTexture(GL_TEXTURE_2D, tex_id);
}

Ogl33Program::Ogl33Program(GLuint p_id, GLuint tex_id, GLuint mvp_id, GLuint layer_id):
	program_id{p_id},
	texture_uniform{tex_id},
	mvp_uniform{mvp_id},
	layer_uniform{layer_id}
{
}

Ogl33Program::Ogl33Program():
	Ogl33Program(0,0,0,0)
{}

Ogl33Program::~Ogl33Program(){
	if(program_id > 0){
		glDeleteProgram(program_id);
	}
}

Ogl33Program::Ogl33Program(Ogl33Program&& rhs):
	program_id{rhs.program_id},
	texture_uniform{rhs.texture_uniform},
	mvp_uniform{rhs.mvp_uniform},
	layer_uniform{rhs.layer_uniform}
{
	rhs.program_id = 0;
	rhs.texture_uniform = 0;
	rhs.mvp_uniform = 0;
	rhs.layer_uniform = 0;
}

void Ogl33Program::setTexture(const Ogl33Texture& tex){
	tex.bind();
}

void Ogl33Program::setMvp(const Matrix<float,3,3>& mvp){
	glUniformMatrix3fv(mvp_uniform, 1, GL_TRUE, &mvp(0, 0));
}

void Ogl33Program::setMesh(const Ogl33Mesh& mesh){
	glEnableVertexAttribArray(0);
	mesh.bindVertex();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));

	glEnableVertexAttribArray(1);
	mesh.bindUV();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));

	mesh.bindIndex();
}

void Ogl33Program::setLayer(float layer){
	glUniform1f(layer_uniform, layer);
}

void Ogl33Program::setLayer(int16_t layer){
	setLayer(static_cast<float>(layer) / INT16_MAX);
}

void Ogl33Program::use(){
	glUseProgram(program_id);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texture_uniform, 0);
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
	assert(window);
	if(window){
		window->bind();
		window->hide();
	}
}

namespace {
template<class> inline constexpr bool always_false_v = false;
}

Conveyor<RenderEvent::Events> Ogl33Window::listenToWindowEvents(){
	assert(window);
	if(!window){
		return Conveyor<RenderEvent::Events>{nullptr, nullptr};
	}
	/*
	 * Convert anonymous window events from kelgin-window to kelgin-graphics events
	 * This needs to be translated, because the interface doesn't know kelgin-window headers
	 * and shouldn't know it. kelgin-window is a driver abstraction library for creating windows
	 * and currently also gl contexts. kelgin-graphics is a plugin based render engine.
	 */
	return window->onEvent().then([](Window::VariantEvent&& event){
		return std::visit([](auto&& arg) -> RenderEvent::Events {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Window::Event::Resize>){
				return RenderEvent::Resize{arg.width, arg.height};
			}else if constexpr(std::is_same_v<T, Window::Event::Keyboard>){
				return RenderEvent::Keyboard{arg.key, arg.pressed, arg.repeat};
			}else if constexpr(std::is_same_v<T, Window::Event::Mouse>){
				return RenderEvent::Mouse{arg.button_mask, arg.pressed};
			}else if constexpr(std::is_same_v<T, Window::Event::MouseMove>){
				return RenderEvent::MouseMove{arg.x, arg.y};
			}else{
				static_assert(always_false_v<T>, "Type in Visitor not exhausted");
			}
		}, event);
	});
}

void Ogl33Window::beginRender(){
	assert(window);
	if(!window){
		return;
	}
	window->bind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0,width(),height());
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

void Ogl33Window::bindAsMain(){
	assert(window);
	if(!window){
		return;
	}

	window->bind();
}

void Ogl33Window::bind(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

size_t Ogl33Window::width() const {
	assert(window);
	if(!window){
		return 0;
	}
	const VideoMode& mode = window->videoMode();
	return mode.width;
}

size_t Ogl33Window::height() const {
	assert(window);
	if(!window){
		return 0;
	}
	const VideoMode& mode = window->videoMode();
	return mode.height;
}

Ogl33RenderTexture::~Ogl33RenderTexture(){
}

void Ogl33RenderTexture::beginRender(){

}

void Ogl33RenderTexture::endRender(){

}

void Ogl33RenderTexture::bind(){

}

size_t Ogl33RenderTexture::width() const {
	return 0;
}

size_t Ogl33RenderTexture::height() const {
	return 0;
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
		if(( rt_find->first+1) == max_free_id){
			--max_free_id;
		}else{
			free_ids.push(rt_find->first);
		}
		render_textures.erase(rt_find);
		return;
	}

	auto rw_find = windows.find(id);
	if(rw_find != windows.end()){
		if(( rt_find->first+1) == max_free_id){
			--max_free_id;
		}else{
			free_ids.push(rt_find->first);
		}
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

RenderObjectId Ogl33Scene::createObject(const RenderPropertyId& rp_id){
	RenderObjectId id = searchForFreeId(objects);
	objects.insert(std::make_pair(id, RenderObject{rp_id, 0.f, 0.f, 0.f, 0.f}));
	return id;
}

void Ogl33Scene::destroyObject(const RenderObjectId& id){
	objects.erase(id);
}

void Ogl33Scene::setObjectPosition(const RenderObjectId& id, float x, float y){
	auto find = objects.find(id);
	if(find != objects.end()){
		find->second.x = x;
		find->second.y = y;
	}
}

void Ogl33Scene::setObjectRotation(const RenderObjectId& id, float angle){
	auto find = objects.find(id);
	if(find != objects.end()){
		find->second.angle = angle;
	}
}

/**
* @todo design better interface and check occlusion
*/
void Ogl33Scene::visit(const Ogl33Camera&, std::vector<RenderObject*>& render_queue){
	render_queue.reserve(objects.size());
	for(auto& iter: objects){
		render_queue.push_back(&iter.second);
	}
}

void Ogl33RenderStage::renderOne(Ogl33Program& program, Ogl33RenderProperty& property, Ogl33Scene::RenderObject& object, Ogl33Mesh& mesh, Ogl33Texture& texture, Matrix<float, 3, 3>& vp){
	program.setMesh(mesh);
	program.setTexture(texture);
	program.setLayer(object.layer);
	Matrix<float, 3, 3> mvp;
	float cos_a = cos(object.angle);
	float sin_a = sin(object.angle);
	mvp(0,0) = cos_a;
	mvp(1,1) = cos_a;
	mvp(0,1) = -sin_a;
	mvp(1,0) = sin_a;
	mvp(0,2) = object.x;
	mvp(1,2) = object.y;
	mvp(2,2) = 1.f;

	mvp = vp * mvp;

	program.setMvp(mvp);

	glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, 0L);
}

void Ogl33RenderStage::render(Ogl33Render& render){
	std::vector<Ogl33Scene::RenderObject*> draw_queue;

	Ogl33Scene* scene = render.getScene(scene_id);
	assert(scene);
	if(!scene){
		return;
	}
	Ogl33Camera* camera = render.getCamera(camera_id);
	assert(camera);
	if(!camera){
		return;
	}
	Ogl33Program* program = render.getProgram(program_id);
	assert(program);
	if(!program){
		return;
	}

	scene->visit(*camera, draw_queue);

	program->use();

	Matrix<float, 3, 3> vp = camera->projection()*camera->view();

	for(size_t i = 0; i < 3; ++i){
		for(size_t j = 0; j < 3; ++j){

		}
	}

	for(auto& iter : draw_queue){
		Ogl33RenderProperty* property = render.getProperty(iter->id);
		assert(property);
		if(!property){
			continue;
		}
		Ogl33Mesh* mesh = render.getMesh(property->mesh_id);
		assert(mesh);
		if(!mesh){
			continue;
		}
		Ogl33Texture* texture = render.getTexture(property->texture_id);
		assert(texture);
		if(!texture){
			continue;
		}
		
		renderOne(*program, *property, *iter, *mesh, *texture, vp);		
	}
}

Ogl33Render::Ogl33Render(Own<GlContext>&& ctx):
	context{std::move(ctx)}
{
}

Ogl33Render::~Ogl33Render(){
	if(loaded_glad && vao > 0){
		glDeleteVertexArrays(1,&vao);
	}
}

Ogl33Scene* Ogl33Render::getScene(const RenderSceneId& id){
	auto iter = scenes.find(id);
	if(iter != scenes.end()){
		return iter->second.get();
	}
	return nullptr;
}

Ogl33Camera* Ogl33Render::getCamera(const RenderCameraId& id){
	auto iter = cameras.find(id);
	if(iter != cameras.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Program* Ogl33Render::getProgram(const ProgramId& id){
	auto iter = programs.find(id);
	if(iter != programs.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33RenderProperty* Ogl33Render::getProperty(const RenderPropertyId& id){
	auto iter = render_properties.find(id);
	if(iter != render_properties.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Mesh* Ogl33Render::getMesh(const MeshId& id){
	auto iter = meshes.find(id);
	if(iter != meshes.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Texture* Ogl33Render::getTexture(const TextureId& id){
	auto iter = textures.find(id);
	if(iter != textures.end()){
		return &iter->second;
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

	MeshId m_id = searchForFreeId(meshes);

	meshes.insert(std::make_pair(m_id,Ogl33Mesh{std::move(ids), data.indices.size()}));
	return m_id;
}

void Ogl33Render::setMeshData(const MeshId& id, const MeshData& data){
	auto find = meshes.find(id);
	if(find == meshes.end()){
		return;
	}

	find->second.setData(data);
}

void Ogl33Render::destroyMesh(const MeshId& id){
	meshes.erase(id);
}

namespace {
/// @todo get correct type. Not sure if it's GLint or GLuint
GLint translateImageChannel(uint8_t channels){
	std::array<GLint, 4> data{GL_RED, GL_RG, GL_RGB, GL_RGBA};
	if(channels > 4 || channels == 0){
		return GL_RGBA;
	}
	return data[channels-1];
}
}

TextureId Ogl33Render::createTexture(const Image& image){
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());

	glBindTexture(GL_TEXTURE_2D, 0);
	
	TextureId t_id = searchForFreeId(textures);

	textures.insert(std::make_pair(t_id, Ogl33Texture{texture_id}));
	return t_id;
}

void Ogl33Render::destroyTexture(const TextureId& id){
	textures.erase(id);
}

RenderWindowId Ogl33Render::createWindow(const RenderVideoMode& mode, const std::string& title) {
	auto gl_win = context->createWindow(VideoMode{mode.width,mode.height}, title);
	if(!gl_win){
		return 0;
	}

	gl_win->bind();
	if(!loaded_glad){
		if(!gladLoadGL()){
			std::cout<<"Failed to load glad"<<std::endl;
			return 0;
		}
		glGenVertexArrays(1,&vao);
		glBindVertexArray(vao);
		loaded_glad = true;
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
	render_target_stages.erase(static_cast<RenderTargetId>(id));
	render_target_times.erase(static_cast<RenderTargetId>(id));
}

Conveyor<RenderEvent::Events> Ogl33Render::listenToWindowEvents(const RenderWindowId& id) {
	Ogl33Window* window = render_targets.getWindow(id);
	if(!window){
		return Conveyor<RenderEvent::Events>{nullptr, nullptr};
	}

	return window->listenToWindowEvents();
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
		std::cerr<<"Failed to create shader"<<std::endl;
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
	// context->bind();

	GLuint vertex_shader_id =
		createShader(vertex_src, GL_VERTEX_SHADER);
	GLuint fragment_shader_id =
		createShader(fragment_src, GL_FRAGMENT_SHADER);

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

	GLint result = GL_FALSE;
	int info_length;
	glGetProgramiv(p_id, GL_LINK_STATUS, &result);
	glGetProgramiv(p_id, GL_INFO_LOG_LENGTH, &info_length);
	if (info_length > 1 || result == GL_FALSE) {
		std::string error_msg;
		error_msg.resize(info_length);
		glGetProgramInfoLog(p_id, info_length, nullptr, &error_msg[0]);

		std::cerr<<"Failed to link "<<error_msg<<std::endl;
		// log_error(std::string{"Failed to compile "} + error_msg);
		
		

		return 0;
	}

	glDetachShader(p_id, vertex_shader_id);
	glDetachShader(p_id, fragment_shader_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	GLuint mvp_id = glGetUniformLocation(p_id, "mvp");
	GLuint texture_sampler_id = glGetUniformLocation(p_id, "texture_sampler");
	GLuint layer_id = glGetUniformLocation(p_id, "layer");

	ProgramId id = searchForFreeId(programs);

	programs.insert(std::make_pair(id, Ogl33Program{p_id, texture_sampler_id, mvp_id, layer_id}));

	return id;
}

namespace {
const std::string default_vertex_shader_program = R"(#version 330 core

layout (location = 0) in vec2 vertices;
layout (location = 1) in vec2 uvs;

out vec2 tex_coord;

uniform float layer;
uniform mat3 mvp;

void main(){
	vec3 transformed = mvp * vec3(vertices, 1.0);
	gl_Position.xyz = vec3(transformed.x, transformed.y, layer);
	gl_Position.w = transformed.z;
	tex_coord = uvs;
}
)";
const std::string default_fragment_shader_program = R"(#version 330 core

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D texture_sampler;

void main(){
	vec4 tex_colour = texture(texture_sampler, tex_coord);
	colour = tex_colour;
}
)";
}

ProgramId Ogl33Render::createProgram(){
	return createProgram(default_vertex_shader_program, default_fragment_shader_program);
}

void Ogl33Render::destroyProgram(const ProgramId& id){
	programs.erase(id);
}

RenderCameraId Ogl33Render::createCamera(){

	RenderCameraId id = searchForFreeId(cameras);

	cameras.insert(std::make_pair(id, Ogl33Camera{}));
	return id;
}

void Ogl33Render::setCameraOrthographic(const RenderCameraId& id, float l, float r, float t, float b){
	auto find = cameras.find(id);
	if(find != cameras.end()){
		find->second.setOrtho(l, r, t , b);
	}
}

void Ogl33Render::setCameraPosition(const RenderCameraId& id, float x, float y){
	auto find = cameras.find(id);
	if(find != cameras.end()){
		find->second.setViewPosition(x,y);
	}
}

void Ogl33Render::setCameraRotation(const RenderCameraId& id, float angle){
	auto find = cameras.find(id);
	if(find != cameras.end()){
		find->second.setViewRotation(angle);
	}
}

void Ogl33Render::destroyCamera(const RenderCameraId& id){
	cameras.erase(id);
}

RenderStageId Ogl33Render::createStage(const RenderTargetId& target_id, const RenderSceneId& scene, const RenderCameraId& cam, const ProgramId& program_id){
	
	RenderStageId id = searchForFreeId(render_stages);
	render_stages.insert(std::make_pair(id, Ogl33RenderStage{target_id, scene, cam, program_id}));
	
	render_target_stages.insert(std::make_pair(target_id, id));
	return id;
}

void Ogl33Render::destroyStage(const RenderStageId& id){
	auto find = render_stages.find(id);
	if(find != render_stages.end()){
		auto range = render_target_stages.equal_range(find->second.target_id);
		for(auto iter = range.first; iter != range.second; ){
			if(iter->second == id){
				iter = render_target_stages.erase(iter);
			}else{
				++iter;
			}
		}

		render_stages.erase(find);
	}
}

RenderViewportId Ogl33Render::createViewport(){
	RenderViewportId id = searchForFreeId(viewports);
	viewports.insert(std::make_pair(id, Ogl33Viewport{0.f,0.f,0.f,0.f}));
	return id;
}

void Ogl33Render::setViewportRect(const RenderViewportId& id, float x, float y, float width, float height){
	auto find = viewports.find(id);
	if(find != viewports.end()){
		find->second = Ogl33Viewport{x, y, width, height};
	}
}

void Ogl33Render::destroyViewport(const RenderViewportId& id){
	viewports.erase(id);
}

RenderPropertyId Ogl33Render::createProperty(const MeshId& mesh, const TextureId& texture){
	RenderPropertyId id = searchForFreeId(render_properties);
	render_properties.insert(std::make_pair(id, Ogl33RenderProperty{mesh, texture}));
	return id;
}

void Ogl33Render::setPropertyMesh(const RenderPropertyId& id, const MeshId& mesh_id) {
	auto find = render_properties.find(id);
	if(find != render_properties.end()){
		find->second.mesh_id = mesh_id;
	}
}

void Ogl33Render::setPropertyTexture(const RenderPropertyId& id, const TextureId& texture_id) {
	auto find = render_properties.find(id);
	if(find != render_properties.end()){
		find->second.texture_id = texture_id;
	}
}

void Ogl33Render::destroyProperty(const RenderPropertyId& id){
	render_properties.erase(id);
}

RenderSceneId Ogl33Render::createScene(){
	RenderSceneId id = searchForFreeId(scenes);
	scenes.insert(std::make_pair(id, heap<Ogl33Scene>()));
	return id;
}

RenderObjectId Ogl33Render::createObject(const RenderSceneId& scene, const RenderPropertyId& prop){
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		return find->second->createObject(prop);
	}else{
		return 0;
	}
}

void Ogl33Render::destroyObject(const RenderSceneId& scene, const RenderObjectId& obj){
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second->destroyObject(obj);
	}
}

void Ogl33Render::setObjectPosition(const RenderSceneId& scene, const RenderObjectId& obj, float x, float y){
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second->setObjectPosition(obj, x, y);
	}
}

void Ogl33Render::setObjectRotation(const RenderSceneId& scene, const RenderObjectId& obj, float angle){
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second->setObjectRotation(obj, angle);
	}
}

void Ogl33Render::destroyScene(const RenderSceneId& id){
	scenes.erase(id);
}

Mesh3dId Ogl33Render::createMesh3d(const Mesh3dData& data){
	Mesh3dId id = searchForFreeId(meshes_3d);
	std::array<GLuint,2> ids;

	/// @todo ensure that the current render context is bound

	if( data.vertices.size() != data.normals.size() || data.vertices.size() != data.uvs.size()){
		return 0;
	}

	std::vector<float> packed_mesh;
	packed_mesh.reserve(3 * data.vertices.size());
	for(size_t i = 0; i < data.vertices.size(); ++i){
		packed_mesh.push_back(data.vertices.at(i));
		packed_mesh.push_back(data.uvs.at(i));
		packed_mesh.push_back(data.normals.at(i));
	}

	glGenBuffers(2, &ids[0]);

	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
	glBufferData(GL_ARRAY_BUFFER, packed_mesh.size() * sizeof(float), packed_mesh.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

// This is unnecessary in a correct renderer impl
#ifndef NDEBUG
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif

	meshes_3d.insert(std::make_pair(id, Ogl33Mesh3d{std::move(ids), data.indices.size()}));
	return id;
}

void Ogl33Render::destroyMesh3d(const Mesh3dId& id){
	meshes_3d.erase(id);
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

		auto range = render_target_stages.equal_range(front);

		for(auto iter = range.first; iter != range.second; ++iter){
			auto stage_iter = render_stages.find(iter->second);
			if(stage_iter != render_stages.end()){
				stage_iter->second.render(*this);
			}
		}

		target->endRender();
	}
}

void Ogl33Render::updateTime(const std::chrono::steady_clock::time_point& ){
	/// @todo implement interpolation
}
}

extern "C" gin::LowLevelRender* createRenderer(gin::AsyncIoProvider& io_provider){
	gin::Own<gin::GlContext> context = gin::createGlContext(io_provider, gin::GlSettings{});
	if(!context){
		return nullptr;
	}
	
	/*
	if(!gladLoadGL()){
		std::cout<<"Failed to load glad"<<std::endl;
		return nullptr;
	}
	*/

	std::cout<<"Creating ogl33 plugin"<<std::endl;
	return new gin::Ogl33Render(std::move(context));
}

extern "C" void destroyRenderer(gin::LowLevelRender* render){
	if(!render){
		return;
	}

	std::cout<<"Destroying ogl33 plugin"<<std::endl;
	delete render;
}

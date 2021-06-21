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

void Ogl33Camera::updateState(float relative_tp){
	old_position[0] = position[0] * relative_tp + old_position[0] * ( 1.f - relative_tp);
	old_position[1] = position[1] * relative_tp + old_position[1] * ( 1.f - relative_tp);

	old_angle = slerp2D( old_angle, angle, relative_tp );
}

void Ogl33Camera::setViewPosition(float x, float y){
	position[0] = x;
	position[1] = y;
}

void Ogl33Camera::setViewRotation(float angle){
	this->angle = angle;
}

Matrix<float, 3,3> Ogl33Camera::view(float interpol) const {
	Matrix<float, 3, 3> view_matrix;

	view_matrix(0,2) = - (interpol * position[0] + (1.f - interpol) * old_position[0]);
	view_matrix(1,2) = - (interpol * position[1] + (1.f - interpol) * old_position[1]);

	std::complex<float> interpol_angle = slerp2D<float>(old_angle, angle, interpol);

	view_matrix(0,0) = std::real(interpol_angle);
	view_matrix(0,1) = std::imag(interpol_angle);
	view_matrix(1,0) = -std::imag(interpol_angle);
	view_matrix(1,1) = std::real(interpol_angle);

	view_matrix(2,2) = 1.f;
	view_matrix(2,0) = 0.f;
	view_matrix(2,1) = 0.f;

	return view_matrix;
}

const Matrix<float, 3,3>& Ogl33Camera::projection() const {
	return projection_matrix;
}

Ogl33Camera3d::Ogl33Camera3d(){
	for(size_t i = 0; i < 4; ++i){
		projection_matrix(i,i) = 1.0f;
		view_matrix(i,i) = 1.0f;
	}
}

void Ogl33Camera3d::setOrtho(float left, float right, float top, float bottom, float near, float far){
	/// @todo set orthographic projection
}

void Ogl33Camera3d::setViewPosition(float x, float y, float z){
	view_matrix(0, 3) = -x;
	view_matrix(1, 3) = -y;
	view_matrix(2, 3) = -z;
}

void Ogl33Camera3d::setViewRotation(float alpha, float beta, float gamma){
	/// @todo set rotation matrix in [0-2]x[0-2]
}

const Matrix<float, 4, 4>& Ogl33Camera3d::projection() const {
	return projection_matrix;
}

const Matrix<float, 4, 4>& Ogl33Camera3d::view() const {
	return view_matrix;
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

ErrorOr<RenderObjectId> Ogl33Scene::createObject(const RenderPropertyId& rp_id)noexcept{
	RenderObjectId id = searchForFreeId(objects);

	try{
		objects.insert(std::make_pair(id, RenderObject{rp_id}));
	}catch(const std::bad_alloc&){
		return criticalError("Out of memory");
	}
	return id;
}

void Ogl33Scene::destroyObject(const RenderObjectId& id)noexcept{
	objects.erase(id);
}

Error Ogl33Scene::setObjectPosition(const RenderObjectId& id, float x, float y)noexcept{
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}

	find->second.pos = {{x,y}};

	return noError();
}

Error Ogl33Scene::setObjectRotation(const RenderObjectId& id, float angle)noexcept{
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}
	find->second.angle = std::polar(1.f, angle);
	return noError();
}

Error Ogl33Scene::setObjectVisibility(const RenderObjectId& id, bool visible)noexcept{
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}
	
	find->second.visible = visible;
	return noError();
}

Error Ogl33Scene::setObjectLayer(const RenderObjectId& id, float l) noexcept {
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}

	find->second.layer = l;
	return noError();
}

/**
* @todo design better interface and check occlusion
*/
void Ogl33Scene::visit(const Ogl33Camera&, std::vector<RenderObject*>& render_queue){
	render_queue.reserve(objects.size());
	for(auto& iter: objects){
		if(iter.second.visible){
			render_queue.push_back(&iter.second);
		}
	}
}

void Ogl33Scene::updateState(float interval){
	for(auto& iter : objects){
		iter.second.old_pos[0] = iter.second.pos[0] * interval + iter.second.old_pos[0] * (1.f - interval);
		iter.second.old_pos[1] = iter.second.pos[1] * interval + iter.second.old_pos[1] * (1.f - interval);

		iter.second.old_angle = slerp2D<float>(iter.second.old_angle, iter.second.angle, interval);
	}
}

ErrorOr<RenderObject3dId> Ogl33Scene3d::createObject(const RenderProperty3dId& id) noexcept {
	RenderObject3dId o_id = searchForFreeId(objects);

	try{
		objects.insert(std::make_pair(o_id, Ogl33Scene3d::RenderObject{id}));
	}catch(const std::bad_alloc&){
		return criticalError("Out of memory");
	}

	return o_id;
}

void Ogl33Scene3d::destroyObject(const RenderObject3dId& id) noexcept{
	objects.erase(id);
	return;
}

Error Ogl33Scene3d::setObjectPosition(const RenderObject3dId& id, float x, float y, float z)noexcept{
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}

	find->second.pos = {{x,y,z}};

	return noError();
}

Error Ogl33Scene3d::setObjectRotation(const RenderObject3dId& id, float a, float b, float g)noexcept{
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}

	find->second.rot = {{a,b,g}};

	return noError();
}

Error Ogl33Scene3d::setObjectVisibility(const RenderObject3dId& id, bool visible) noexcept{
	auto find = objects.find(id);
	if(find == objects.end()){
		return criticalError("Couldn't find object");
	}

	find->second.visible = visible;

	return noError();
}

void Ogl33Scene3d::updateState(){
	for(auto& iter : objects){
		iter.second.old_pos = iter.second.pos;
		iter.second.old_rot = iter.second.rot;
	}
}

void Ogl33RenderStage::renderOne(Ogl33Program& program, Ogl33RenderProperty& property, Ogl33Scene::RenderObject& object, Ogl33Mesh& mesh, Ogl33Texture& texture, Matrix<float, 3, 3>& vp, float time_interval){
	program.setMesh(mesh);
	program.setTexture(texture);
	program.setLayer(object.layer);
	Matrix<float, 3, 3> mvp;

	std::complex<float> interpol_angle = slerp2D<float>(object.old_angle, object.angle, time_interval);

	mvp(0,0) = std::real(interpol_angle);
	mvp(0,1) = -std::imag(interpol_angle);
	mvp(1,0) = std::imag(interpol_angle);
	mvp(1,1) = std::real(interpol_angle);

	mvp(0,2) = object.pos[0] * ( time_interval ) + object.old_pos[0] * ( 1.f - time_interval );
	mvp(1,2) = object.pos[1] * ( time_interval ) + object.old_pos[1] * ( 1.f - time_interval );
	mvp(2,2) = 1.f;

	mvp = vp * mvp;

	program.setMvp(mvp);

	glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, 0L);
}

void Ogl33RenderStage::render(Ogl33Render& render, float time_interval){
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

	Matrix<float, 3, 3> vp = camera->projection()*camera->view(time_interval);

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
		
		renderOne(*program, *property, *iter, *mesh, *texture, vp, time_interval);		
	}
}

void Ogl33RenderStage3d::renderOne(Ogl33Program3d& program, Ogl33RenderProperty3d& property, Ogl33Scene3d::RenderObject& object, Ogl33Mesh3d& mesh, Ogl33Texture& texture, Matrix<float, 4, 4>& vp){

}

void Ogl33RenderStage3d::render(Ogl33Render& render){
	std::vector<Ogl33Scene3d::RenderObject> draw_queue;
	Ogl33Scene3d* scene = render.getScene3d(scene_id);
	assert(scene);
	if(!scene){
		return;
	}

	Ogl33Camera3d* camera = render.getCamera3d(camera_id);
	assert(camera);
	if(!camera){
		return;
	}

	Ogl33Program3d* program = render.getProgram3d(program_id);
	assert(program);
	if(!program){
		return;
	}

	scene->visit(*camera, draw_queue);

	program->use();

	Matrix<float, 4, 4> vp = camera->projection()*camera->view();

	for(auto& iter: draw_queue){
		// renderOne();
	}
}

Ogl33Render::Ogl33Render(Own<GlContext>&& ctx):
	context{std::move(ctx)},
	old_time_point{std::chrono::steady_clock::now()},
	time_point{old_time_point}
{
}

Ogl33Render::~Ogl33Render(){
	if(loaded_glad && vao > 0){
		glDeleteVertexArrays(1,&vao);
	}
}

namespace {
template<typename K, typename T>
struct Ogl33RenderContainerReturnHelper;

template<typename K, typename T>
struct Ogl33RenderContainerReturnHelper {
	static T* getElement(std::unordered_map<K, T>& ctr, const K& id){
		auto iter = ctr.find(id);
		if(iter != ctr.end()){
			return &iter->second;
		}
		return nullptr;
	}
};

template<typename K, typename T>
struct Ogl33RenderContainerReturnHelper<K, Own<T>> {
	static T* getElement(std::unordered_map<K, Own<T>>& ctr, const K& id){
		auto iter = ctr.find(id);
		if(iter != ctr.end()){
			return iter->second.get();
		}
		return nullptr;
	}
};
}

Ogl33Scene* Ogl33Render::getScene(const RenderSceneId& id) noexcept {
	auto iter = scenes.find(id);
	if(iter != scenes.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Camera* Ogl33Render::getCamera(const RenderCameraId& id) noexcept {
	auto iter = cameras.find(id);
	if(iter != cameras.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Program* Ogl33Render::getProgram(const ProgramId& id) noexcept {
	auto iter = programs.find(id);
	if(iter != programs.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33RenderProperty* Ogl33Render::getProperty(const RenderPropertyId& id) noexcept {
	auto iter = render_properties.find(id);
	if(iter != render_properties.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Mesh* Ogl33Render::getMesh(const MeshId& id) noexcept {
	auto iter = meshes.find(id);
	if(iter != meshes.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Texture* Ogl33Render::getTexture(const TextureId& id) noexcept {
	auto iter = textures.find(id);
	if(iter != textures.end()){
		return &iter->second;
	}
	return nullptr;
}

Ogl33Scene3d* Ogl33Render::getScene3d(const RenderScene3dId& id) noexcept {
	return Ogl33RenderContainerReturnHelper<RenderScene3dId, Ogl33Scene3d>::getElement(scenes_3d, id);
}

Ogl33Camera3d* Ogl33Render::getCamera3d(const RenderCamera3dId& id) noexcept {
	return Ogl33RenderContainerReturnHelper<RenderCamera3dId, Ogl33Camera3d>::getElement(cameras_3d, id);
}

Ogl33Program3d* Ogl33Render::getProgram3d(const Program3dId& id) noexcept {
	return Ogl33RenderContainerReturnHelper<Program3dId, Ogl33Program3d>::getElement(programs_3d, id);
}

Ogl33RenderProperty3d* Ogl33Render::getRenderProperty3d(const RenderProperty3dId& id) noexcept {
	return Ogl33RenderContainerReturnHelper<RenderProperty3dId, Ogl33RenderProperty3d>::getElement(render_properties_3d, id);
}

Ogl33Mesh3d* Ogl33Render::getMesh3d(const Mesh3dId& id) noexcept {
	return Ogl33RenderContainerReturnHelper<Mesh3dId, Ogl33Mesh3d>::getElement(meshes_3d, id);
}

Conveyor<MeshId> Ogl33Render::createMesh(const MeshData& data) noexcept {
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

	try{
		meshes.insert(std::make_pair(m_id,Ogl33Mesh{std::move(ids), data.indices.size()}));
	}catch(const std::bad_alloc&){
		return Conveyor<MeshId>{criticalError("Out of memory")};
	}
	return Conveyor<MeshId>{m_id};
}

Conveyor<void> Ogl33Render::setMeshData(const MeshId& id, const MeshData& data) noexcept {
	auto find = meshes.find(id);
	if(find == meshes.end()){
		return Conveyor<void>{recoverableError("Couldn't find mesh")};
	}

	find->second.setData(data);
	return Conveyor<void>{Void{}};
}

/// @todo check if an error might be necessary
Conveyor<void> Ogl33Render::destroyMesh(const MeshId& id) noexcept {

	meshes.erase(id);

	return Conveyor<void>{Void{}};
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

Conveyor<TextureId> Ogl33Render::createTexture(const Image& image) noexcept {
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

	try{
		textures.insert(std::make_pair(t_id, Ogl33Texture{texture_id}));
	}catch(const std::bad_alloc&){
		return Conveyor<TextureId>{criticalError("Out of memory")};
	}
	return Conveyor<TextureId>{t_id};
}

/// @todo check if an error might be necessary
Conveyor<void> Ogl33Render::destroyTexture(const TextureId& id) noexcept {
	textures.erase(id);

	return Conveyor<void>{Void{}};
}

Conveyor<RenderWindowId> Ogl33Render::createWindow(const RenderVideoMode& mode, const std::string& title) noexcept {
	auto gl_win = context->createWindow(VideoMode{mode.width,mode.height}, title);
	if(!gl_win){
		return criticalError("Couldn't create window");
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

	try{
		auto id = render_targets.insert(Ogl33Window{std::move(gl_win)});
		return Conveyor<RenderWindowId>{id};
	}catch(const std::bad_alloc&){
		return Conveyor<RenderWindowId>{criticalError("Out of memory")};
	} 
}

Conveyor<void> Ogl33Render::setWindowDesiredFPS(const RenderWindowId& id, float fps) noexcept {
	Ogl33Window* window = render_targets.getWindow(id);
	if(!window){
		return Conveyor<void>{criticalError("Couldn't create Window")};
	}

	auto& update = render_target_times[static_cast<RenderTargetId>(id)];

	std::chrono::duration<float, std::ratio<1,1>> fps_chrono{1.0f / fps};
	update.next_update = std::chrono::steady_clock::now();
	update.seconds_per_frame = std::chrono::duration_cast<std::chrono::steady_clock::duration>(fps_chrono);

	return Conveyor<void>{Void{}};
}

Conveyor<void> Ogl33Render::destroyWindow(const RenderWindowId& id) noexcept {
	render_targets.erase(static_cast<RenderTargetId>(id));
	render_target_stages.erase(static_cast<RenderTargetId>(id));
	render_target_times.erase(static_cast<RenderTargetId>(id));

	return Conveyor<void>{Void{}};
}

Conveyor<RenderEvent::Events> Ogl33Render::listenToWindowEvents(const RenderWindowId& id) noexcept {
	Ogl33Window* window = render_targets.getWindow(id);
	if(!window){
		return Conveyor<RenderEvent::Events>{nullptr, nullptr};
	}

	return window->listenToWindowEvents();
}

Conveyor<void> Ogl33Render::setWindowVisibility(const RenderWindowId& id, bool show) noexcept {
	Ogl33Window* window = render_targets.getWindow(id);
	if(!window){
		return Conveyor<void>{criticalError("No window found")};
	}

	if(show){
		window->show();
	}else{
		window->hide();
	}

	return Conveyor<void>{Void{}};
}

namespace {
GLuint createShader(const std::string &source, GLenum type) noexcept {
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

ErrorOr<GLuint> createOgl33Program(const std::string& vertex_src, const std::string& fragment_src) noexcept {

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
		return criticalError("Couldn't create shader");
	}

	GLuint p_id = glCreateProgram();

	if (p_id == 0) {
		glDeleteShader(vertex_shader_id);
		glDeleteShader(fragment_shader_id);
		return criticalError("Couldn't create program");
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
		

		return criticalError("Failed to compile program");
	}

	glDetachShader(p_id, vertex_shader_id);
	glDetachShader(p_id, fragment_shader_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return p_id;
}
}

Conveyor<ProgramId> Ogl33Render::createProgram(const std::string& vertex_src, const std::string& fragment_src) noexcept {
	ErrorOr<GLuint> error_p_id = createOgl33Program(vertex_src, fragment_src);

	if(error_p_id.isError()){
		return Conveyor<ProgramId>{error_p_id.error().copyError()};
	}

	GLuint& p_id = error_p_id.value();

	GLuint mvp_id = glGetUniformLocation(p_id, "mvp");
	GLuint texture_sampler_id = glGetUniformLocation(p_id, "texture_sampler");
	GLuint layer_id = glGetUniformLocation(p_id, "layer");

	ProgramId id = searchForFreeId(programs);

	try{
		programs.insert(std::make_pair(id, Ogl33Program{p_id, texture_sampler_id, mvp_id, layer_id}));
	}catch(const std::bad_alloc&){
		return Conveyor<ProgramId>{criticalError("Out of memory")};
	}

	return Conveyor<ProgramId>{id};
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

const std::string default_vertex_shader_program_3d = R"(#version 330 core

layout (location = 0) in vec3 vertices;
layout (location = 1) in vec2 uvs;
layout (location = 2) in vec3 normals;

out vec2 tex_coord;

uniform mat4 mvp;

void main(){
	tex_coord = uvs;
}

)";

const std::string default_fragment_shader_program_3d = R"(#version 330 core
in vec2 tex_coord;

out vec4 colour;

uniform sampler2D texture_sampler;

void main(){
	vec4 tex_colour = texture(texture_sampler, tex_coord);
	colour = tex_colour;
}
)";
}

Conveyor<ProgramId> Ogl33Render::createProgram() noexcept {
	return createProgram(default_vertex_shader_program, default_fragment_shader_program);
}

Conveyor<void> Ogl33Render::destroyProgram(const ProgramId& id) noexcept {
	programs.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderCameraId> Ogl33Render::createCamera() noexcept {

	RenderCameraId id = searchForFreeId(cameras);

	try{
		cameras.insert(std::make_pair(id, Ogl33Camera{}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderCameraId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderCameraId>{id};
}

Conveyor<void> Ogl33Render::setCameraOrthographic(const RenderCameraId& id, float l, float r, float t, float b) noexcept {
	auto find = cameras.find(id);
	if(find != cameras.end()){
		find->second.setOrtho(l, r, t , b);
		return Conveyor<void>{Void{}};
	}

	return Conveyor<void>{criticalError("No camera found")};
}

Conveyor<void> Ogl33Render::setCameraPosition(const RenderCameraId& id, float x, float y) noexcept {
	auto find = cameras.find(id);
	if(find != cameras.end()){
		find->second.setViewPosition(x,y);
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("No camera found")};
}

Conveyor<void> Ogl33Render::setCameraRotation(const RenderCameraId& id, float angle) noexcept {
	auto find = cameras.find(id);
	if(find != cameras.end()){
		find->second.setViewRotation(angle);
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("No camera found")};
}

Conveyor<void> Ogl33Render::destroyCamera(const RenderCameraId& id) noexcept {
	cameras.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderStageId> Ogl33Render::createStage(const RenderTargetId& target_id, const RenderViewportId& viewport_id, const RenderSceneId& scene, const RenderCameraId& cam, const ProgramId& program_id) noexcept {

	RenderStageId id = searchForFreeId(render_stages);
	try{
		render_stages.insert(std::make_pair(id, Ogl33RenderStage{target_id, viewport_id, scene, cam, program_id}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderStageId>{criticalError("Out of memory")};
	}
	try{
		render_target_stages.insert(std::make_pair(target_id, id));
	}catch(const std::bad_alloc&){
		render_stages.erase(id);
		return Conveyor<RenderStageId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderStageId>{id};
}

Conveyor<void> Ogl33Render::destroyStage(const RenderStageId& id) noexcept {
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

		return Conveyor<void>{Void{}};
	}

	return Conveyor<void>{criticalError("No RenderStage found")};
}

Conveyor<RenderViewportId> Ogl33Render::createViewport() noexcept {
	RenderViewportId id = searchForFreeId(viewports);
	try{
		viewports.insert(std::make_pair(id, Ogl33Viewport{0.f,0.f,0.f,0.f}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderViewportId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderViewportId>{id};
}

Conveyor<void> Ogl33Render::setViewportRect(const RenderViewportId& id, float x, float y, float width, float height) noexcept {
	auto find = viewports.find(id);
	if(find != viewports.end()){
		find->second = Ogl33Viewport{x, y, width, height};
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("No Viewport found")};
}

Conveyor<void> Ogl33Render::destroyViewport(const RenderViewportId& id) noexcept {
	viewports.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderPropertyId> Ogl33Render::createProperty(const MeshId& mesh, const TextureId& texture) noexcept {
	RenderPropertyId id = searchForFreeId(render_properties);
	try{
		render_properties.insert(std::make_pair(id, Ogl33RenderProperty{mesh, texture}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderPropertyId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderPropertyId>{id};
}

Conveyor<void> Ogl33Render::setPropertyMesh(const RenderPropertyId& id, const MeshId& mesh_id) noexcept {
	auto find = render_properties.find(id);
	if(find != render_properties.end()){
		find->second.mesh_id = mesh_id;
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("No Property found")};
}

Conveyor<void> Ogl33Render::setPropertyTexture(const RenderPropertyId& id, const TextureId& texture_id) noexcept {
	auto find = render_properties.find(id);
	if(find != render_properties.end()){
		find->second.texture_id = texture_id;
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("No Property found")};
}

Conveyor<void> Ogl33Render::destroyProperty(const RenderPropertyId& id) noexcept {
	render_properties.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderSceneId> Ogl33Render::createScene() noexcept {
	RenderSceneId id = searchForFreeId(scenes);
	try{
		scenes.insert(std::make_pair(id, Ogl33Scene{}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderSceneId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderSceneId>{id};
}

Conveyor<RenderObjectId> Ogl33Render::createObject(const RenderSceneId& scene, const RenderPropertyId& prop) noexcept {
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		ErrorOr<RenderObjectId> error_id = find->second.createObject(prop);
		if(error_id.isError()){
			return Conveyor<RenderObjectId>{error_id.error().copyError()};
		}else if(error_id.isValue()){
			return error_id.value();
		}else {
			return Conveyor<RenderObjectId>{criticalError("ErrorOr object isn't set properly")};
		}
	}else{
		return Conveyor<RenderObjectId>{criticalError("Couldn't find scene")};
	}
}

Conveyor<void> Ogl33Render::destroyObject(const RenderSceneId& scene, const RenderObjectId& obj) noexcept {
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second.destroyObject(obj);
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("Couldn't find scene")};
}

Conveyor<void> Ogl33Render::setObjectPosition(const RenderSceneId& scene, const RenderObjectId& obj, float x, float y) noexcept {
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second.setObjectPosition(obj, x, y);
		/// @todo
		/// Technically not noError
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("Couldn't find scene")};
}

Conveyor<void> Ogl33Render::setObjectRotation(const RenderSceneId& scene, const RenderObjectId& obj, float angle) noexcept {
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second.setObjectRotation(obj, angle);
		/// @todo
		/// Technically not noError
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("Couldn't find scene")};
}

Conveyor<void> Ogl33Render::setObjectVisibility(const RenderSceneId& scene, const RenderObjectId& obj, bool visible) noexcept {
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second.setObjectVisibility(obj, visible);
		/// @todo
		/// Technically not noError
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("Couldn't find scene")};
}

Conveyor<void> Ogl33Render::setObjectLayer(const RenderSceneId& scene, const RenderObjectId& obj, float layer) noexcept {
	auto find = scenes.find(scene);
	if(find != scenes.end()){
		find->second.setObjectLayer(obj, layer);
		/// @todo
		/// Technically not noError
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("Couldn't find scene")};
}

Conveyor<void> Ogl33Render::destroyScene(const RenderSceneId& id) noexcept {
	scenes.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<Mesh3dId> Ogl33Render::createMesh3d(const Mesh3dData& data) noexcept {
	Mesh3dId id = searchForFreeId(meshes_3d);
	std::array<GLuint,2> ids;

	/// @todo ensure that the current render context is bound

	GLuint vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(2,&ids[0]);
	glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Mesh3dData::Vertex), data.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ids[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh3dData::Vertex), nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh3dData::Vertex), reinterpret_cast<void*>(offsetof(Mesh3dData::Vertex, normals)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh3dData::Vertex), reinterpret_cast<void*>(offsetof(Mesh3dData::Vertex, uvs)));

	#ifndef NDEBUG
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	#endif
	try{
		meshes_3d.insert(std::make_pair(id, Ogl33Mesh3d{vao, std::move(ids), data.indices.size()}));
	}catch(const std::bad_alloc& ){
		return Conveyor<Mesh3dId>{criticalError("Out of memory")};
	}
	return Conveyor<Mesh3dId>{id};
}

Conveyor<void> Ogl33Render::destroyMesh3d(const Mesh3dId& id) noexcept {
	meshes_3d.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderProperty3dId> Ogl33Render::createProperty3d(const Mesh3dId& mesh, const TextureId& texture) noexcept {
	RenderProperty3dId id = searchForFreeId(render_properties_3d);
	try{
		render_properties_3d.insert(std::make_pair(id, Ogl33RenderProperty3d{mesh, texture}));
	}catch(const std::bad_alloc& ){
		return Conveyor<RenderProperty3dId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderProperty3dId>{id};
}

Conveyor<void> Ogl33Render::destroyProperty3d(const RenderProperty3dId& id) noexcept {
	render_properties_3d.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<Program3dId> Ogl33Render::createProgram3d(const std::string& vertex_src, const std::string& fragment_src) noexcept {
	ErrorOr<GLuint> error_p_id = createOgl33Program(vertex_src, fragment_src);

	if(error_p_id.isError()){
		return Conveyor<Program3dId>{error_p_id.error().copyError()};
	}

	GLuint& p_id = error_p_id.value();

	GLuint mvp_id = glGetUniformLocation(p_id, "mvp");
	GLuint texture_sampler_id = glGetUniformLocation(p_id, "texture_sampler");

	Program3dId id = searchForFreeId(programs_3d);
	try{
		programs_3d.insert(std::make_pair(id, Ogl33Program3d{p_id, texture_sampler_id, mvp_id}));
	}catch(const std::bad_alloc&){
		return Conveyor<Program3dId>{criticalError("Out of memory")};
	}
	return Conveyor<Program3dId>{id};
}

Conveyor<Program3dId> Ogl33Render::createProgram3d() noexcept {
	return createProgram3d(default_vertex_shader_program_3d, default_fragment_shader_program_3d);
}

Conveyor<void> Ogl33Render::destroyProgram3d(const Program3dId& id) noexcept {
	programs_3d.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderScene3dId> Ogl33Render::createScene3d() noexcept {
	RenderScene3dId id = searchForFreeId(scenes_3d);

	try{
		scenes_3d.insert(std::make_pair(id, Ogl33Scene3d{}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderScene3dId>{criticalError("Out of memory")};
	}

	return Conveyor<RenderScene3dId>{id};
}

Conveyor<RenderObject3dId> Ogl33Render::createObject3d(const RenderScene3dId& scene_id, const RenderProperty3dId& prop) noexcept {
	auto scene = scenes_3d.find(scene_id);
	if(scene == scenes_3d.end()){
		return Conveyor<RenderObject3dId>{criticalError("Couldn't find scene")};
	}

	ErrorOr<RenderObject3dId> err_id = scene->second.createObject(prop);
	if(err_id.isError()){
		return Conveyor<RenderObject3dId>{err_id.error().copyError()};
	}
	return Conveyor<RenderObject3dId>{err_id.value()};
}

Conveyor<void> Ogl33Render::destroyObject3d(const RenderScene3dId& scene_id, const RenderObject3dId& obj) noexcept {
	auto scene = scenes_3d.find(scene_id);
	if(scene == scenes_3d.end()){
		return Conveyor<void>{criticalError("Couldn't find Scene")};
	}

	scene->second.destroyObject(obj);
	return Conveyor<void>{Void{}};
}

Conveyor<void> Ogl33Render::destroyScene3d(const RenderScene3dId& scene) noexcept {
	scenes_3d.erase(scene);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderCamera3dId> Ogl33Render::createCamera3d() noexcept {
	RenderScene3dId id = searchForFreeId(cameras_3d);
	try{
		cameras_3d.insert(std::make_pair(id, Ogl33Camera3d{}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderCamera3dId>{criticalError("Out of memory")};
	}
	return Conveyor<RenderCamera3dId>{id};
}
Conveyor<void> Ogl33Render::setCamera3dPosition(const RenderCamera3dId& id, float x, float y, float z) noexcept {
	auto find = cameras_3d.find(id);
	if(find != cameras_3d.end()){
		find->second.setViewPosition(x,y,z);
		return Conveyor<void>{Void{}};
	}
	return Conveyor<void>{criticalError("Couldn't find Camera")};
}

Conveyor<void> Ogl33Render::setCamera3dOrthographic(const RenderCamera3dId& id, float, float, float, float, float, float)noexcept {
	return Conveyor<void>{criticalError("Unimplemented")};
}

Conveyor<void> Ogl33Render::destroyCamera3d(const RenderCamera3dId& id) noexcept {
	cameras_3d.erase(id);
	return Conveyor<void>{Void{}};
}

Conveyor<RenderStage3dId> Ogl33Render::createStage3d(const RenderTargetId& target, const RenderViewportId& viewport_id, const RenderScene3dId& scene, const RenderCamera3dId& camera, const Program3dId& program) noexcept {
	RenderStage3dId id = searchForFreeId(render_stages_3d);

	try{
		render_stages_3d.insert(std::make_pair(id, Ogl33RenderStage3d{target, viewport_id, scene, camera, program}));
	}catch(const std::bad_alloc&){
		return Conveyor<RenderStage3dId>{criticalError("Out of memory")};
	}

	try{
		render_target_stages_3d.insert(std::make_pair(target, id));
	}catch(const std::bad_alloc&){
		render_stages_3d.erase(id);
		return Conveyor<RenderStage3dId>{criticalError("Out of memory")};
	}

	return Conveyor<RenderStage3dId>{id};
}

Conveyor<void> Ogl33Render::destroyStage3d(const RenderStage3dId& id) noexcept {
	render_stages_3d.erase(id);
	return Conveyor<void>{Void{}};
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

void Ogl33Render::flush() noexcept {
	assert(context);
	if(context){
		context->flush();
	}
}

void Ogl33Render::step(const std::chrono::steady_clock::time_point& tp) noexcept {
	assert(context);
	if(!context){
		return;
	}

	std::chrono::duration<float> range = time_point - old_time_point;
	std::chrono::duration<float> interval = tp - old_time_point;

	float relative_tp = std::max(0.f, std::min(1.0f, interval.count() / range.count()));

	std::cout<<"Time: "<<relative_tp<<" "<<interval.count() / range.count()<<std::endl;

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
				stage_iter->second.render(*this, relative_tp);
			}
		}

		target->endRender();
	}
}

void Ogl33Render::updateTime(const std::chrono::steady_clock::time_point& new_old_time_point, const std::chrono::steady_clock::time_point& new_time_point) noexcept {

	std::chrono::duration<float> range = time_point - old_time_point;
	std::chrono::duration<float> interval = new_old_time_point - old_time_point;

	float relative_tp = std::max(0.f, std::min(1.0f, interval.count() / range.count()));

	for(auto& iter : scenes){
		iter.second.updateState(relative_tp);
	}

	for(auto& iter : cameras){
		iter.second.updateState(relative_tp);
	}

	old_time_point = new_old_time_point;
	time_point = new_time_point;
}
}

extern "C" gin::LowLevelRender* createRenderer(gin::IoProvider& io_provider){
	gin::Own<gin::GlContext> context = gin::createGlContext(io_provider, gin::GlSettings{});
	if(!context){
		return nullptr;
	}

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
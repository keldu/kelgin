#pragma once

#include <string>

const std::string default_vertex_shader = R"(#version 330 core

layout (location = 0) in vec2 vertices;
layout (location = 1) in vec2 uvs;

out vec2 tex_coord;

uniform mat3 model_view_projection;

void main(){
	// vec3 transformed = model_view_projection * vec3(vertices, 1.0);
	vec3 transformed = vec3(vertices, 0.0);
	gl_Position.xyz = transformed;
	gl_Position.w = 1.0;
	tex_coord = uvs;
}
)";

const std::string default_fragment_shader = R"(#version 330 core

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D texture_sampler;

void main(){
	vec4 tex_colour = texture(texture_sampler, tex_coord);
	colour = tex_colour;
	// color = vec3(1.0, 0.0, 0.0);
}
)";

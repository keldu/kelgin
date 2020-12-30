#pragma once

#include <string>

const std::string default_vertex_shader = R"(#version 330 core

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

#pragma once

#include "render/render.h"

const gin::MeshData default_mesh = {
	{
		0.0f, 0.f, 0.f, .9f, 
		.9f, 0.f, .9f, .9f,
	},
	{
		0.f, 0.f, 0.f, 1.f,
		1.f, 0.f, 1.f, 1.f
	},
	{
		0, 1, 2,
		0, 2, 3
	}
};
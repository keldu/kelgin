#pragma once

#include "render/render.h"

const gin::MeshData default_mesh = {
	{
		-.5f, -.5f,
		.5f, -.5f, 
		-.5f, .5f,
		.5f, .5f,
	},
	{
		0.f, 0.f,
		1.f, 0.f,
		0.f, 1.f,
		1.f, 1.f
	},
	{
		0, 1, 2,
		2, 1, 3
	}
};

const gin::MeshData array_mesh = {
	{
		-.5f, -.5f,
		.5f, -.5f,
		0.f, .5f
	},
	{
		0.f, 0.f,
		0.f, 0.f,
		0.f, 0.f
	},
	{
	}
};
#include "ogl33_render.h"

#include <iostream>

namespace gin {
Own<RenderWorld> Ogl33Render::createWorld(){
	return nullptr;
}
}

extern "C" gin::Render* createRenderer(){
	return new gin::Ogl33Render;
}

extern "C" void destroyRenderer(gin::Render* render){
	if(!render){
		return;
	}

	delete render;
}

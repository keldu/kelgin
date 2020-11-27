#include "ogl33_render.h"

#include <iostream>

namespace gin {
Own<RenderWorld> Ogl33Render::createWorld(){
	return heap<Ogl33RenderWorld>();
}
}

extern "C" gin::Render* createRenderer(){
	std::cout<<"Creating ogl33 plugin"<<std::endl;
	return new gin::Ogl33Render;
}

extern "C" void destroyRenderer(gin::Render* render){
	if(!render){
		return;
	}
	std::cout<<"Destroying ogl33 plugin"<<std::endl;
	delete render;
}

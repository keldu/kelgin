#include "graphics.h"

#include <iostream>

int main(int argc, char** argv){
	using namespace gin;

	AsyncIoContext async = setupAsyncIo();

	GraphicsService service{Graphics{loadAllRenderPluginsIn("bin/plugins/")}};

	async.event_port.onSignal(Signal::Terminate).then([&service](){
		service.stop();
		std::cout<<"Shutdown requested"<<std::endl;
	}).detach([](const Error& error){return error;});

	return service.rcRun(async);
}
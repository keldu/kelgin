#include "graphics.h"

#include <iostream>

int main(int argc, char** argv){
	using namespace gin;

	AsyncIoContext async = setupAsyncIo();
	WaitScope wait_scope{async.event_loop};

	GraphicsService service{Graphics{loadAllRenderPluginsIn("bin/plugins/")}};

	async.event_port.onSignal(Signal::Terminate).then([&service](){
		service.stop();
		std::cout<<"Shutdown requested"<<std::endl;
	}).detach([](const Error& error){return error;});

	return service.rcRun(async, wait_scope);
}
#pragma once

#include <kelgin/io.h>

#include <functional>
#include <thread>

namespace gin {
class Service {
public:
	virtual ~Service() = default;

	virtual void run(AsyncIoContext& aio) = 0;
	virtual void stop() = 0;
};

template<typename T>
class ServiceThread{
private:
	T& srv;
	std::thread thread;
public:
	ServiceThread(T& srv);
	~ServiceThread();

	void run();
};
}

// Template impl

namespace gin {
template<typename T>
ServiceThread<T>::ServiceThread(T& srv):
	srv{srv},
	thread{std::bind(&ServiceThread<T>::run, this)}
{}

template<typename T>
ServiceThread<T>::~ServiceThread(){
	srv.stop();
	if(thread.joinable()){
		thread.join();
	}
}

template<typename T>
void ServiceThread<T>::run(){
	AsyncIoContext aio;
	srv.run(aio);
}
}
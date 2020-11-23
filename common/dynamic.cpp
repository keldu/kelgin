#include "dynamic.h"

#include <functional>
#include <map>

#include <cassert>
#include <dlfcn.h>

namespace gin {
class DynamicLibrary::Impl {
public:
	void* handle = nullptr;
	std::map<std::string, void*> symbol_handles;

	void* symbol(const std::string& sym){
		assert(handle);
		
		auto find_symbol = symbol_handles.find(sym);
		if(find_symbol != symbol_handles.end()){
			return find_symbol->second;
		}
		void* symbol = dlsym(handle, sym.c_str());

		if(!symbol){
			return nullptr;
		}

		symbol_handles.insert(std::make_pair(sym, symbol));
		return symbol;
	}
public:
	Impl(void* handle):handle{handle}{}
	~Impl(){
		assert(handle);
		if(handle){
			dlclose(handle);
		}
	}
};

DynamicLibrary::DynamicLibrary(void* handle):
	impl{gin::heap<Impl>(handle)}
{}

DynamicLibrary::~DynamicLibrary(){}

void* DynamicLibrary::symbol(const std::string& sym){
	assert(impl);
	impl->symbol(sym);
}

ErrorOr<DynamicLibrary> loadDynamicLibrary(const std::string& path){
	void* handle = dlopen(path.c_str(), RTLD_LAZY);
	if(!handle){
		return criticalError("Couldn't open library");
	}

	return DynamicLibrary{handle};
}
}
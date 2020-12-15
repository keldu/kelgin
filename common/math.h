#pragma once

#include <cmath>

namespace gin {
template <typename T, size_t M, size_t N>
class Matrix {
private:
	std::array<T, M*N> data;
public:
	Matrix(){
		for(size_t i = 0; i < M * N; ++i){
			data[i] = 0;
		}
	}

	T& operator()(size_t i, size_t j){
		return data[i * N + j];
	}

	const T& operator()(size_t i, size_t j) const {
		return data[i * N + j];
	}
};
}
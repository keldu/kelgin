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

	Matrix(T mid){
		for(size_t i = 0; i < M * N; ++i){
			data[i] = i % (N+1) == 0 ? mid : 0;
		}
	}

	T& operator()(size_t i, size_t j){
		return data[i * N + j];
	}

	const T& operator()(size_t i, size_t j) const {
		return data[i * N + j];
	}

	template<size_t K>
	Matrix<T, M, K> operator*(const Matrix<T, N, K>& rhs) const {
		Matrix<T, M, K> matrix;

		for(size_t i = 0; i < M; ++i){
			for(size_t j = 0; j < N; ++j){
				for(size_t k = 0; k < K; ++k){
					matrix(i,k) += (*this)(i,j) * rhs(j,k);
				}
			}
		}
		return matrix;
	}
};
}

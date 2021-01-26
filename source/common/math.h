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

template<typename T, size_t M>
class Matrix<T,M,1> {
private:
	std::array<T, M> data;
public:
	Matrix(){
		for(size_t i = 0; i < M; ++i){
			data[i] = 0;
		}
	}

	T& operator()(size_t i, size_t j){
		return data[i + j];
	}

	const T& operator()(size_t i, size_t j) const {
		return data[i + j];
	}

	template<size_t K>
	Matrix<T, M, K> operator*(const Matrix<T, 1, K>& rhs) const {
		Matrix<T, M, K> matrix;

		for(size_t i = 0; i < M; ++i){
			for(size_t j = 0; j < 1; ++j){
				for(size_t k = 0; k < K; ++k){
					matrix(i,k) += (*this)(i,j) * rhs(j,k);
				}
			}
		}
		return matrix;
	}
};

template<typename T, size_t N>
class Matrix<T,1,N> {
private:
	std::array<T, N> data;
public:
	Matrix(){
		for(size_t i = 0; i < N; ++i){
			data[i] = 0;
		}
	}

	T& operator()(size_t i, size_t j){
		return data[i + j];
	}

	const T& operator()(size_t i, size_t j) const {
		return data[i + j];
	}

	template<size_t K>
	Matrix<T, 1, K> operator*(const Matrix<T, N, K>& rhs) const {
		Matrix<T, 1, K> matrix;

		for(size_t i = 0; i < 1; ++i){
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

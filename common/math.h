#pragma once

namespace gin {
template <typename T, size_t M, size_t N>
class Matrix {
private:
	std::array<T, M*N> data;
public:
	T& operator()(size_t i, size_t j);
	const T& operator()(size_t i, size_t j) const;
};
}
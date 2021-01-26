#pragma once

#include "math.h"

#include <array>

namespace gin {

template<typename T, size_t S>
class Point {
private:
	std::array<T, S> data;
public:
	Point();

	T& operator(size_t i);
	const T& operator(size_t i) const;

};

template<typename T>
class Rectangle {
private:
	std::array<T,4> data;
public:
	Rectangle();
	Rectangle(T, T, T, T);

	T& x();
	const T& x() const;
	
	T& y();
	const T& y() const;
	
	T& width();
	const T& width() const;
	
	T& height();
	const T& height() const;
};

template<typename T>
bool isInside(const Rectangle<T>& rect, const Point<T,2>& p);

template<typename T>
Rectangle<T> overlap(const Rectangle<T>&, const Rectangle<T>&);
}

#include "shapes.tmpl.h"
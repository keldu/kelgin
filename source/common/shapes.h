#pragma once

#include "math.h"

#include <array>
#include <cstdint>
#include <optional>

namespace gin {

template<typename T, size_t S>
class Point {
private:
	std::array<T, S> data;
public:
	Point();

	T& operator()(size_t i);
	const T& operator()(size_t i) const;

};

template<typename T>
class Rectangle {
private:
	std::array<Point<T,2>,2> data;
public:
	Rectangle();
	Rectangle(const Point<T,2>& bl, const Point<T,2>& wh);

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
std::optional<Rectangle<T>> overlap(const Rectangle<T>& a, const Rectangle<T>& b);
}

#include "shapes.tmpl.h"
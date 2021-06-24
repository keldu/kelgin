#pragma once

namespace gin {
template<typename T, size_t S>
Point<T,S>::Point(){
	for(size_t i = 0; i < S; ++i){
		data[i] = 0;
	}
}

template<typename T, size_t S>
T& Point<T,S>::operator()(size_t i){
	return data[i];
}

template<typename T, size_t S>
const T& Point<T,S>::operator()(size_t i) const {
	return data[i];
}
template<typename T>
Rectangle<T>::Rectangle(){

}

template<typename T>
Rectangle<T>::Rectangle(const Point<T,2>& bl, const Point<T,2>& wh):
	data{bl, wh}
{}

template<typename T>
T& Rectangle<T>::x(){
	return data[0][0];
}

template<typename T>
const T& Rectangle<T>::x() const{
	return data[0][0];
}
	
template<typename T>
T& Rectangle<T>::y(){
	return data[0][1];
}

template<typename T>
const T& Rectangle<T>::y() const{
	return data[0][1];
}
	
template<typename T>
T& Rectangle<T>::width(){
	return data[1][0];
}

template<typename T>
const T& Rectangle<T>::width() const{
	return data[1][0];
}
	
template<typename T>
T& Rectangle<T>::height(){
	return data[1][1];
}

template<typename T>
const T& Rectangle<T>::height() const{
	return data[1][1];
}

template<typename T>
bool isInside(const Rectangle<T>& rect, const Point<T,2>& p){
	return (rect.x() <= p(0)) && (rect.x()+rect.width() >= p(0)) && (rect.y() <= p(1)) && (rect.y()+rect.height() >= p(1));
}
template<typename T>
std::optional<Rectangle<T>> overlap(const Rectangle<T>& a, const Rectangle<T>& b){
	return std::nullopt;
}
}
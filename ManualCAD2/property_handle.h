#pragma once

namespace ManualCAD
{
	template <class T> using PropertyHandle = T*;
}
//
//template <class T>
//class PropertyHandle {
//	T* ptr;
//
//public:
//	PropertyHandle(T* ptr) : ptr(ptr) {}
//	inline T* get() { return ptr; }
//};
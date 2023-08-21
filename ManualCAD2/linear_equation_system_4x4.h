#pragma once

#include "algebra.h"

namespace ManualCAD
{
	class LinearEquationSystem4x4
	{
	public:
		static Vector4 solve(const Matrix4x4& mat, const Vector4& b);
	};
}
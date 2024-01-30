#pragma once

#include "algebra.h"

namespace ManualCAD
{
	class PolynomialAlgorithms {
	public:
		template <class T>
		inline static T de_casteljau(T b0, T b1, T b2, T b3, const float t) {
			b0 = (1 - t) * b0 + t * b1;
			b1 = (1 - t) * b1 + t * b2;
			b2 = (1 - t) * b2 + t * b3;

			b0 = (1 - t) * b0 + t * b1;
			b1 = (1 - t) * b1 + t * b2;

			return (1 - t) * b0 + t * b1;
		}

		template <class T>
		inline static T de_casteljau(T b0, T b1, T b2, const float t) {
			b0 = (1 - t) * b0 + t * b1;
			b1 = (1 - t) * b1 + t * b2;

			return (1 - t) * b0 + t * b1;
		}
	};
}
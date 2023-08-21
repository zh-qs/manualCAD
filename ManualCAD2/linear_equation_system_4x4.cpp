#include "linear_equation_system_4x4.h"

namespace ManualCAD
{
	template <int COL, int IDX = COL>
	inline constexpr int max_abs_idx(const Matrix4x4& mat)
	{
		float max = abs(mat.elem[IDX][COL]);
		int idx = IDX;
		for (int i = IDX + 1; i < Matrix4x4::DIMENSION; ++i)
		{
			if (abs(mat.elem[i][COL]) > max)
			{
				max = abs(mat.elem[i][COL]);
				idx = i;
			}
		}
		return idx;
	}


	template <int COL>
	inline constexpr void eliminate_column(Matrix4x4& mat, Vector4& b)
	{
		float* d = b.data();
		for (int j = COL + 1; j < Matrix4x4::DIMENSION; ++j)
			mat.elem[COL][j] /= mat.elem[COL][COL];
		d[COL] /= mat.elem[COL][COL];
		mat.elem[COL][COL] = 1.0f;
		for (int i = COL + 1; i < Matrix4x4::DIMENSION; ++i)
		{
			const float elim = mat.elem[i][COL];
			for (int j = COL + 1; j < Matrix4x4::DIMENSION; ++j)
				mat.elem[i][j] -= mat.elem[COL][j] * elim;
			d[i] -= d[COL] * elim;
			mat.elem[i][COL] = 0.0f;
		}
	}

	template <int STEP>
	inline constexpr void gauss_downstep(Matrix4x4& mat, Vector4& b)
	{
		int idx = max_abs_idx<STEP>(mat);
		if (idx != STEP)
		{
			mat.swap_rows(STEP, idx);
			b.swap_elements(STEP, idx);
		}
		eliminate_column<STEP>(mat, b);
	}

	template <int STEP>
	inline constexpr void gauss_upstep(Matrix4x4& mat, Vector4& b)
	{
		float* d = b.data();
		for (int i = 0; i < STEP; ++i)
		{
			d[i] -= mat.elem[i][STEP] * d[STEP];
			mat.elem[i][STEP] = 0.0f;
		}
	}

	Vector4 LinearEquationSystem4x4::solve(const Matrix4x4& mat, const Vector4& b)
	{
		// Gauss Elimination with Partial Pivoting

		Matrix4x4 mat_copy = mat;
		Vector4 b_copy = b;

		// downsweep phase
		gauss_downstep<0>(mat_copy, b_copy);
		gauss_downstep<1>(mat_copy, b_copy);
		gauss_downstep<2>(mat_copy, b_copy);
		b_copy.w /= mat_copy.elem[3][3];
		mat_copy.elem[3][3] = 1.0f;

		// upsweep phase
		gauss_upstep<3>(mat_copy, b_copy);
		gauss_upstep<2>(mat_copy, b_copy);
		gauss_upstep<1>(mat_copy, b_copy);

		return b_copy;
	}
}
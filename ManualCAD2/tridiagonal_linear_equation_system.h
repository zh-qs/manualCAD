#pragma once
#include <vector>

namespace ManualCAD
{
	template <class T>
	class TridiagonalLinearEquationSystem {
		std::vector<float> main_diagonal;
		std::vector<float> lower_diagonal;
		std::vector<float> upper_diagonal;
		std::vector<T> free_terms;

	public:
		TridiagonalLinearEquationSystem(int n) : main_diagonal(n), lower_diagonal(n), upper_diagonal(n), free_terms(n) {
			lower_diagonal[0] = 0.0f;
			upper_diagonal[n - 1] = 0.0f;
		}
		float& main_diagonal_element(int i) { return main_diagonal[i]; }
		float& upper_diagonal_element(int i) { return upper_diagonal[i]; }
		float& lower_diagonal_element(int i) { return lower_diagonal[i]; }
		T& free_term(int i) { return free_terms[i]; }

		std::vector<T> solve();
	};

	template<class T>
	inline std::vector<T> TridiagonalLinearEquationSystem<T>::solve()
	{
		std::vector<T> res(free_terms.size());

		float w;
		const int n = free_terms.size();

		for (int i = 1; i < n; ++i)
		{
			w = lower_diagonal[i] / main_diagonal[i - 1];
			main_diagonal[i] -= w * upper_diagonal[i - 1];
			free_terms[i] -= w * free_terms[i - 1];
		}

		res[n - 1] = free_terms[n - 1] / main_diagonal[n - 1];
		for (int i = n - 2; i >= 0; --i)
		{
			res[i] = (free_terms[i] - upper_diagonal[i] * res[i + 1]) / main_diagonal[i];
		}
		return res;
	}
}

#include "parametric_surface_intersection.h"
#include <ctime>
#include "application_settings.h"
#include "exception.h"
#include "linear_equation_system_4x4.h"
#include <random>

constexpr bool NO_HINT_RANDOM_SAMPLE = true; // when false, we sample values in a grid (not random), more accurate but slower
// TODO refactor methods for NO_HINT_RANDOM_SAMPLE = false;
namespace ManualCAD
{
	std::random_device dev;

	inline void check_timeout(const std::clock_t& start_time)
	{
		if constexpr (ApplicationSettings::BREAK_ON_TIMEOUT)
		{
			auto current_time = std::clock();
			if (current_time - start_time > ApplicationSettings::COMPUTATION_TIMEOUT)
				throw TimeoutException();
		}
	}

	std::vector<Vector3> ParametricSurfaceIntersection::get_points() const
	{
		std::vector<Vector3> result;
		result.reserve(uvs1.size());

		for (const auto& p : uvs1)
			result.push_back(surf1.evaluate(p.x, p.y));

		return result;
	}

	bool ParametricSurfaceIntersection::can_be_started_by(const Vector2& uv1, const Vector2& uv2, const float step) const
	{
		constexpr float EPS = 1e-4f;

		const auto pp1 = surf1.evaluate(uv1.x, uv1.y), pp2 = surf2.evaluate(uv2.x, uv2.y);

		if ((pp1 - pp2).length() >= EPS)
			throw CommonIntersectionPointNotFoundException();

		const auto norm1 = surf1.normal(uv1.x, uv1.y), norm2 = surf2.normal(uv2.x, uv2.y);
		const auto cr = cross(norm1, norm2);

		if (cr.length() == 0.0f)
			return false;

		auto tangent = normalize(cr);

		for (int i = 0; i < uvs1.size(); ++i)
		{
			const auto pi1 = surf1.evaluate(uvs1[i].x, uvs1[i].y),
				pi2 = surf2.evaluate(uvs2[i].x, uvs2[i].y);
			//if ((uv1 - uvs1[i]).length() < step && (uv2 - uvs2[i]).length() < step)
			if ((pp1 - pi1).length() < step && (pp2 - pi2).length() < step)
			{
				/*auto P1 = surf1.evaluate(uvs1[i].x, uvs1[i].y),
					P2 = surf1.evaluate(uvs1[i + 1].x, uvs1[i + 1].y);
				if (dot(tangent, P2 - P1) > 0)
					return true;*/
				return true;
			}
		}
		/*if (looped && (uv1 - uvs1.back()).length() < step && (uv2 - uvs2.back()).length() < step)
		{
			auto P1 = surf1.evaluate(uvs1.back().x, uvs1.back().y),
				P2 = surf1.evaluate(uvs1.front().x, uvs1.front().y);
			if (dot(tangent, P2 - P1) > 0)
				return true;
		}*/
		return false;
	}

	ParametricSurfaceIntersection ParametricSurfaceIntersection::intersect_surfaces(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, const Vector2& uv1start, const Vector2& uv2start, const bool force_loop)
	{
		// TODO make relative errors policy
		static constexpr float EPS = 1e-5f;
		/*const float relative_eps = (surf1.get_u_range().to - surf1.get_u_range().from
			+ surf1.get_v_range().to - surf1.get_v_range().from
			+ surf2.get_u_range().to - surf2.get_u_range().from
			+ surf2.get_v_range().to - surf2.get_v_range().from) * 0.25f * EPS;*/

		//const auto xx1 = surf1.evaluate(uv1start.x, uv1start.y), xx2 = surf2.evaluate(uv2start.x, uv2start.y), xx3 = xx1 - xx2;
		if (!surf1.get_u_range().contains(uv1start.x)
			|| !surf1.get_v_range().contains(uv1start.y)
			|| !surf2.get_u_range().contains(uv2start.x)
			|| !surf2.get_v_range().contains(uv2start.y)
			|| (surf1.evaluate(uv1start.x, uv1start.y) - surf2.evaluate(uv2start.x, uv2start.y)).length() >= 1e-3f)
			throw CommonIntersectionPointNotFoundException();

		std::list<Vector2> uvs1, uvs2;
		//uvs1.reserve(max_steps);
		//uvs2.reserve(max_steps);

		uvs1.push_back(uv1start);
		uvs2.push_back(uv2start);

		bool inverse_direction = false;
		bool reverse_tangent_after_singular_point = false; // used by singular points handling
		bool singular_point_crossed = false;

		bool draw_along_border1 = false; // *****
		bool draw_along_border2 = false; // *****
		bool draw_along_u_border = false; // *****

		int step_count = 1;
		bool looped = false;

		while (step_count++ < max_steps)
		{
			//const auto idx = uvs1.size() - 1;
			const auto uv1 = inverse_direction ? uvs1.front() : uvs1.back(),
				uv2 = inverse_direction ? uvs2.front() : uvs2.back();

			Vector4 sol = { uv1.x, uv1.y, uv2.x, uv2.y }, prev_sol;

			bool should_change_direction = false;

			if (!draw_along_border1 && !draw_along_border2) { // *****
				const auto norm1 = surf1.normal(uv1.x, uv1.y), norm2 = surf2.normal(uv2.x, uv2.y);
				const auto cr = cross(norm1, norm2);

				if (cr.length() == 0.0f)
					throw new CommonIntersectionPointNotFoundException();

				auto tangent = normalize(cr);

				if (inverse_direction)
					tangent = -tangent;

				if (reverse_tangent_after_singular_point)
				{
					//perpendicular_direction = false;
					tangent = -tangent;//normalize(cross(norm1, tangent));
				}

				const auto P = surf1.evaluate(uv1.x, uv1.y);

				// Newton iterations
				Vector4 F;
				Matrix4x4 jacobian;

				const auto start_time = std::clock();

				do
				{
					try
					{
					check_timeout(start_time);
					}
					catch (const TimeoutException&)
					{
						break;
					} // TODO delete try-catch

					const auto P1 = surf1.evaluate(sol.x, sol.y), P2 = surf2.evaluate(sol.z, sol.w);
					F = Vector4::extend(P1 - P2, dot(P1 - P, tangent) - step);
					const auto dP1u = surf1.du(sol.x, sol.y), dP1v = surf1.dv(sol.x, sol.y),
						dP2u = -surf2.du(sol.z, sol.w), dP2v = -surf2.dv(sol.z, sol.w);

					jacobian.elem[0][0] = dP1u.x;
					jacobian.elem[0][1] = dP1v.x;
					jacobian.elem[0][2] = dP2u.x;
					jacobian.elem[0][3] = dP2v.x;

					jacobian.elem[1][0] = dP1u.y;
					jacobian.elem[1][1] = dP1v.y;
					jacobian.elem[1][2] = dP2u.y;
					jacobian.elem[1][3] = dP2v.y;

					jacobian.elem[2][0] = dP1u.z;
					jacobian.elem[2][1] = dP1v.z;
					jacobian.elem[2][2] = dP2u.z;
					jacobian.elem[2][3] = dP2v.z;

					jacobian.elem[3][0] = dot(dP1u, tangent);
					jacobian.elem[3][1] = dot(dP1v, tangent);
					jacobian.elem[3][2] = 0.0f;
					jacobian.elem[3][3] = 0.0f;

					prev_sol = sol;
					auto l = LinearEquationSystem4x4::solve(jacobian, -F);
					auto d = jacobian * l;
					//if ((d + F).length() > EPS)
						//THROW_EXCEPTION;
					if (!isfinite(l.x))
						throw CommonIntersectionPointNotFoundException();
					//return Object::create<IntersectionCurve>(surf1, surf2, uvs1, uvs2, looped);
					sol += l;
					if (!surf1.get_u_range().contains(sol.x) || !surf1.get_v_range().contains(sol.y)
						|| !surf2.get_u_range().contains(sol.z) || !surf2.get_v_range().contains(sol.w))
						break;
				} while (F.length() >= EPS);
				//while ((sol - prev_sol).length() >= EPS);

				if (!surf1.get_u_range().contains(sol.x)) // TODO to s¹ zawiniêcia jak na linii zmiany daty Ziemi, dorobiæ zawiniêcia jak na biegunach
				{
					if (surf1.u_wraps_at_v(sol.y))
						sol.x = surf1.get_u_range().wrap(sol.x);
					else
					{
						sol.x = surf1.get_u_range().clamp(sol.x);
						// *****
						if (force_loop)
						{
							draw_along_border1 = true;
							draw_along_u_border = false;
						}
						else
							should_change_direction = true;
						// *****
					}
				}
				if (!surf1.get_v_range().contains(sol.y))
				{
					if (surf1.v_wraps_at_u(sol.x))
						sol.y = surf1.get_v_range().wrap(sol.y);
					else
					{
						sol.y = surf1.get_v_range().clamp(sol.y);
						// *****
						if (force_loop)
						{
							draw_along_border1 = true;
							draw_along_u_border = true;
						}
						else
							should_change_direction = true;
						// *****
					}
				}
				if (!surf2.get_u_range().contains(sol.z))
				{
					if (surf2.u_wraps_at_v(sol.w))
						sol.z = surf2.get_u_range().wrap(sol.z);
					else
					{
						sol.z = surf2.get_u_range().clamp(sol.z);
						// *****
						if (force_loop)
						{
							draw_along_border2 = true;
							draw_along_u_border = false;
						}
						else
							should_change_direction = true;
						// *****
					}
				}
				if (!surf2.get_v_range().contains(sol.w))
				{
					if (surf2.v_wraps_at_u(sol.z))
						sol.w = surf2.get_v_range().wrap(sol.w);
					else
					{
						sol.w = surf2.get_v_range().clamp(sol.w);
						// *****
						if (force_loop)
						{
							draw_along_border2 = true;
							draw_along_u_border = true;
						}
						else
							should_change_direction = true;
						// *****
					}
				}

				/*	if (!surf1.get_u_range().contains(sol.x) || !surf1.get_v_range().contains(sol.y) || !surf2.get_u_range().contains(sol.z) || !surf2.get_v_range().contains(sol.w))
					{
						if (inverse_direction)
							break;
						inverse_direction = true;
						continue;
					}
				*/

				// check singular points
				if (uvs1.size() >= 2)
				{
					const auto pprev_uv1 = inverse_direction ? *(++uvs1.cbegin()) : *(++uvs1.crbegin());
					const auto pprev_uv2 = inverse_direction ? *(++uvs2.cbegin()) : *(++uvs2.crbegin());
					if ((surf1.evaluate(pprev_uv1.x, pprev_uv1.y) - surf1.evaluate(sol.x, sol.y)).length() < 0.5f * step && (surf2.evaluate(pprev_uv2.x, pprev_uv2.y) - surf2.evaluate(sol.z, sol.w)).length() < 0.5f * step // points are near in world space
						&& (pprev_uv1 - Vector2{ sol.x,sol.y }).length() < (pprev_uv1 - uv1).length() && (pprev_uv2 - Vector2{ sol.z,sol.w }).length() < (pprev_uv2 - uv2).length()) // points are near in parameter space
					{
						// Newton turned back -- passed through singular point
						reverse_tangent_after_singular_point = !reverse_tangent_after_singular_point;
						singular_point_crossed = true;
						continue;
					}
				}
			} // *****

			// ***** LOOP FORCING: when Newton goes to the border of one surface, we go along that border (ignoring Newton), projecting border points on second surface, till the surfaces meet
			// TODO: handle situations: surfaces do not meet again, change coordinate at uv corner, detect loop on whole border of a surface (that loop should be discarded and no loop can be forced)
			else if (draw_along_border1) {
				auto du = surf1.du(uv1.x, uv1.y), dv = surf1.dv(uv1.x, uv1.y);
				auto step_v = surf1.evaluate(uv1.x, uv1.y) + step * normalize(draw_along_u_border ? surf1.du(uv1.x, uv1.y) : surf1.dv(uv1.x, uv1.y));
				auto uv = find_nearest_point(surf1, step_v, uv1, draw_along_u_border ? ConstantParameter::V : ConstantParameter::U);
				auto P1 = surf1.evaluate(uv.x, uv.y);
				auto other_uv = find_nearest_point(surf2, P1, uv2);
				auto P2 = surf2.evaluate(other_uv.x, other_uv.y);
				if ((P1 - P2).length() < 0.5f * step)
				{
					draw_along_border1 = false;
					continue;
				}
				sol = { uv.x,uv.y,other_uv.x,other_uv.y };
			}
			else if (draw_along_border2) {
				auto step_v = surf2.evaluate(uv2.x, uv2.y) + step * normalize(draw_along_u_border ? surf2.du(uv2.x, uv2.y) : surf2.dv(uv2.x, uv2.y));
				auto uv = find_nearest_point(surf2, step_v, uv2, draw_along_u_border ? ConstantParameter::V : ConstantParameter::U);
				auto P2 = surf2.evaluate(uv.x, uv.y);
				auto other_uv = find_nearest_point(surf1, P2, uv1);
				auto P1 = surf1.evaluate(other_uv.x, other_uv.y);
				if ((P1 - P2).length() < 0.5f * step)
				{
					draw_along_border2 = false;
					continue;
				}
				sol = { other_uv.x,other_uv.y,uv.x,uv.y };
			}
			// *****

			if (inverse_direction)
			{
				if ((surf1.evaluate(sol.x, sol.y) - surf1.evaluate(uvs1.back().x, uvs1.back().y)).length() < 0.5f * step)
					//if ((surf1.evaluate(sol.x, sol.y) - surf1.evaluate(uvs1.back().x, uvs1.back().y)).length() < step)
				{
					looped = true;
					break;
				}
				uvs1.push_front({ sol.x, sol.y });
				uvs2.push_front({ sol.z, sol.w });
			}
			else
			{
				if ((surf1.evaluate(sol.x, sol.y) - surf1.evaluate(uvs1.front().x, uvs1.front().y)).length() < 0.5f * step)
					//if ((surf1.evaluate(sol.x, sol.y) - surf1.evaluate(uvs1.front().x, uvs1.front().y)).length() < step)
				{
					looped = true;
					break;
				}
				uvs1.push_back({ sol.x, sol.y });
				uvs2.push_back({ sol.z, sol.w });
			}

			if (should_change_direction)
			{
				if (inverse_direction)
					break;
				inverse_direction = true;
			}
		}

		return { surf1, surf2, uvs1, uvs2, looped, singular_point_crossed, uvs1.size() == max_steps };
	}

	Vector2 ParametricSurfaceIntersection::find_nearest_point(const ParametricSurface& surf, const Vector3& point)
	{
		return find_nearest_point(surf, point, { surf.get_u_range().middle(), surf.get_v_range().middle() });
	}

	Vector2 ParametricSurfaceIntersection::find_nearest_point(const ParametricSurface& surf, const Vector3& point, const Vector2& uvstart, const ConstantParameter is_constant)
	{
		static constexpr float EPS = 1e-6f;

		// gradienty proste (TODO sprzê¿one)
		Vector2 result = uvstart;
		auto surf_point = surf.evaluate(result.x, result.y);
		float fval = dot(surf_point - point, surf_point - point);
		Vector2 grad = {
			is_constant == ConstantParameter::U ? 0.0f : (2.0f * dot(surf_point - point, surf.du(result.x, result.y))),
			is_constant == ConstantParameter::V ? 0.0f : (2.0f * dot(surf_point - point, surf.dv(result.x, result.y)))
		};
		float alpha = 1.0f;
		if (grad.length() < EPS)
			return result;

		const auto start_time = std::clock();

		while (true)
		{
			check_timeout(start_time);

			auto new_result = result - alpha * grad;

			if (!surf.get_u_range().contains(new_result.x))
			{
				if (surf.u_wraps_at_v(new_result.y))
					new_result.x = surf.get_u_range().wrap(new_result.x);
				else
				{
					alpha /= 2;
					continue;
				}
			}
			if (!surf.get_v_range().contains(new_result.y))
			{
				if (surf.v_wraps_at_u(new_result.x))
					new_result.y = surf.get_v_range().wrap(new_result.y);
				else
				{
					alpha /= 2;
					continue;
				}
			}

			/*if (!surf.get_u_range().contains(new_result.x) || !surf.get_v_range().contains(new_result.y))
			{
				alpha /= 2;
				continue;
			}*/ // OLD

			//new_result.x = surf.get_u_range().clamp(new_result.x);
			//new_result.y = surf.get_v_range().clamp(new_result.y);
			const auto old_surf_point = surf_point;
			surf_point = surf.evaluate(new_result.x, new_result.y);
			const float new_fval = dot(surf_point - point, surf_point - point);
			const Vector2 new_grad = {
				is_constant == ConstantParameter::U ? 0.0f : (2.0f * dot(surf_point - point, surf.du(new_result.x, new_result.y))),
				is_constant == ConstantParameter::V ? 0.0f : (2.0f * dot(surf_point - point, surf.dv(new_result.x, new_result.y)))
			};
			//if (grad.length() < EPS)
			//if ((new_result - result).length() < EPS)
			if ((surf_point - old_surf_point).length() < EPS)
			{
				result = new_result;
				break;
			}
			if (new_fval >= fval)
				alpha /= 2;
			else
			{
				fval = new_fval;
				result = new_result;
				grad = new_grad;
			}
		}
		return result;
		//return { surf.get_u_range().clamp(result.x), surf.get_v_range().clamp(result.y) };
	}

	Vector2 ParametricSurfaceIntersection::find_nearest_point_far_from(const ParametricSurface& surf, const Vector2& uv_far, float step)
	{
		static constexpr int MAX_TRIES = 100;

		std::uniform_real_distribution<float> random_u(surf.get_u_range().from, surf.get_u_range().to);
		std::uniform_real_distribution<float> random_v(surf.get_v_range().from, surf.get_v_range().to);

		const auto point_far = surf.evaluate(uv_far.x, uv_far.y);

		for (int i = 0; i < MAX_TRIES; ++i)
		{
			const Vector2 uv = { random_u(dev), random_v(dev) };
			const auto result = find_nearest_point(surf, point_far, uv);

			const auto dist = (result - uv_far).length();
			if (dist > 0.5f * step)
				return result;
		}
		throw CommonIntersectionPointNotFoundException();
	}

	std::pair<Vector2, Vector2> ParametricSurfaceIntersection::find_first_common_point(const ParametricSurface& surf1, const ParametricSurface& surf2, const Vector2& uv1start, const Vector2& uv2start)
	{
		static constexpr float EPS = 1e-6f;
		/*const float relative_eps = (surf1.get_u_range().to - surf1.get_u_range().from
			+ surf1.get_v_range().to - surf1.get_v_range().from
			+ surf2.get_u_range().to - surf2.get_u_range().from
			+ surf2.get_v_range().to - surf2.get_v_range().from) * 0.25f * EPS;*/

			// gradienty proste (TODO sprzê¿one)
		Vector4 result = { uv1start.x, uv1start.y, uv2start.x, uv2start.y };
		auto surf_point1 = surf1.evaluate(result.x, result.y),
			surf_point2 = surf2.evaluate(result.z, result.w);
		float fval = dot(surf_point1 - surf_point2, surf_point1 - surf_point2);
		Vector4 grad = {
			2.0f * dot(surf_point1 - surf_point2, surf1.du(result.x, result.y)),
			2.0f * dot(surf_point1 - surf_point2, surf1.dv(result.x, result.y)),
			-2.0f * dot(surf_point1 - surf_point2, surf2.du(result.z, result.w)),
			-2.0f * dot(surf_point1 - surf_point2, surf2.dv(result.z, result.w)),
		};
		float alpha = 1.0f;
		if (grad.length() < EPS)
			return { {result.x, result.y}, {result.z, result.w} };

		const auto start_time = std::clock();

		while (true)
		{
			check_timeout(start_time);

			auto new_result = result - alpha * grad;
			const auto old_surf_point1 = surf_point1, old_surf_point2 = surf_point2;
			surf_point1 = surf1.evaluate(new_result.x, new_result.y);
			surf_point2 = surf2.evaluate(new_result.z, new_result.w);
			const float new_fval = dot(surf_point1 - surf_point2, surf_point1 - surf_point2);
			const Vector4 new_grad = {
				2.0f * dot(surf_point1 - surf_point2, surf1.du(result.x, result.y)),
				2.0f * dot(surf_point1 - surf_point2, surf1.dv(result.x, result.y)),
				-2.0f * dot(surf_point1 - surf_point2, surf2.du(result.z, result.w)),
				-2.0f * dot(surf_point1 - surf_point2, surf2.dv(result.z, result.w)),
			};
			//if (grad.length() < EPS)
			//auto dif = new_result - result;
			//float l = dif.length();
			//if ((new_result - result).length() < EPS)
			if ((old_surf_point1 - surf_point1).length() + (old_surf_point2 - surf_point2).length() < EPS)
			{
				result = new_result;
				break;
			}
			if (new_fval >= fval)
			{
				alpha /= 2;
				continue;
			}

			// check if result went outside surface ranges *****
			if (!surf1.get_u_range().contains(new_result.x))
			{
				if (surf1.u_wraps_at_v(new_result.y))
					new_result.x = surf1.get_u_range().wrap(new_result.x);
				else
				{
					alpha /= 2;
					continue;
				}
			}
			if (!surf1.get_v_range().contains(new_result.y))
			{
				if (surf1.v_wraps_at_u(new_result.x))
					new_result.y = surf1.get_v_range().wrap(new_result.y);
				else
				{
					alpha /= 2;
					continue;
				}
			}
			if (!surf2.get_u_range().contains(new_result.z))
			{
				if (surf2.u_wraps_at_v(new_result.w))
					new_result.z = surf2.get_u_range().wrap(new_result.z);
				else
				{
					alpha /= 2;
					continue;
				}
			}
			if (!surf2.get_v_range().contains(new_result.w))
			{
				if (surf2.v_wraps_at_u(new_result.z))
					new_result.w = surf2.get_v_range().wrap(new_result.w);
				else
				{
					alpha /= 2;
					continue;
				}
			}
			// *****

			fval = new_fval;
			result = new_result;
			grad = new_grad;
		}

		// wrap or clamp arguments
		// check if result went outside surface ranges *****
		if (!surf1.get_u_range().contains(result.x))
		{
			if (surf1.u_wraps_at_v(result.y))
				result.x = surf1.get_u_range().wrap(result.x);
			else
			{
				result.x = surf1.get_u_range().clamp(result.x);
			}
		}
		if (!surf1.get_v_range().contains(result.y))
		{
			if (surf1.v_wraps_at_u(result.x))
				result.y = surf1.get_v_range().wrap(result.y);
			else
			{
				result.y = surf1.get_v_range().clamp(result.y);
			}
		}
		if (!surf2.get_u_range().contains(result.z))
		{
			if (surf2.u_wraps_at_v(result.w))
				result.z = surf2.get_u_range().wrap(result.z);
			else
			{
				result.z = surf2.get_u_range().clamp(result.z);
			}
		}
		if (!surf2.get_v_range().contains(result.w))
		{
			if (surf2.v_wraps_at_u(result.z))
				result.w = surf2.get_v_range().wrap(result.w);
			else
			{
				result.w = surf2.get_v_range().clamp(result.w);
			}
		}
		// *****
		/*if (surf1.u_wraps_at_v(result.y))
			result.x = surf1.get_u_range().wrap(result.x);
		if (surf1.v_wraps_at_u(result.x))
			result.y = surf1.get_v_range().wrap(result.y);
		if (surf2.u_wraps_at_v(result.w))
			result.z = surf2.get_u_range().wrap(result.z);
		if (surf2.v_wraps_at_u(result.z))
			result.w = surf2.get_v_range().wrap(result.w);*/

		return { {result.x, result.y}, {result.z, result.w} };
	}

	std::pair<Vector2, Vector2> ParametricSurfaceIntersection::find_first_common_point(const ParametricSurface& surf1, const ParametricSurface& surf2, const Vector3& hint)
	{
		auto nearest1 = find_nearest_point(surf1, hint);
		auto nearest2 = find_nearest_point(surf2, hint);

		return find_first_common_point(surf1, surf2, nearest1, nearest2);
	}

	std::pair<Vector2, Vector2> ParametricSurfaceIntersection::find_first_common_point_on_self_intersection(const ParametricSurface& surf, const Vector3& hint, float step)
	{
		const auto uv1 = find_nearest_point(surf, hint);
		const auto uv2 = find_nearest_point_far_from(surf, uv1, step);

		return find_first_common_point(surf, surf, uv1, uv2);
	}

	ParametricSurfaceIntersection ParametricSurfaceIntersection::intersect_surfaces_with_hint(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, const Vector3& hint, const bool force_loop)
	{
		auto start = find_first_common_point(surf1, surf2, hint);
		return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop);
	}

	ParametricSurfaceIntersection ParametricSurfaceIntersection::intersect_surfaces_without_hint(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y, const bool force_loop)
	{
		auto bounds1 = surf1.get_patch_bounds(),
			bounds2 = surf2.get_patch_bounds();

		//auto box1 = surf1.get_bounding_box(),
		//	box2 = surf2.get_bounding_box();
		//size_t max_samples = std::max(sample_count_x, sample_count_y);
		//const float x1 = box1.x_max - box1.x_min,
		//	y1 = box1.y_max - box1.y_min,
		//	x2 = box2.x_max - box2.x_min,
		//	y2 = box2.y_max - box2.y_min;
		//const float min_d = (1.0f / max_samples) * sqrtf(x1 * x1 + y1 * y1 + x2 * x2 + y2 * y2);

		std::list<ParametricSurfaceIntersection> intersections;

		std::list<std::pair<RangedBox<float>, RangedBox<float>>> intersecting_boxes;
		for (const auto& rb1 : bounds1)
		{
			for (const auto& rb2 : bounds2)
			{
				auto prod_box = Box::intersect(rb1.box, rb2.box);
				if (!prod_box.is_empty())
					intersecting_boxes.push_back({ rb1, rb2 });
			}
		}

		/*auto urange1 = surf1.get_u_range(), vrange1 = surf1.get_v_range(),
			urange2 = surf2.get_u_range(), vrange2 = surf2.get_v_range();*/
		for (const auto& p : intersecting_boxes)
		{
			auto urange1 = p.first.us, vrange1 = p.first.vs,
				urange2 = p.second.us, vrange2 = p.second.vs;
			if constexpr (NO_HINT_RANDOM_SAMPLE)
			{
				std::uniform_real_distribution<float> random_u1(urange1.from, urange1.to);
				std::uniform_real_distribution<float> random_v1(vrange1.from, vrange1.to);
				std::uniform_real_distribution<float> random_u2(urange2.from, urange2.to);
				std::uniform_real_distribution<float> random_v2(vrange2.from, vrange2.to);
				const int samples = sample_count_x * sample_count_y / intersecting_boxes.size();

				auto prod_box = Box::intersect(p.first.box, p.second.box);
				const float x1 = prod_box.x_max - prod_box.x_min,
					y1 = prod_box.y_max - prod_box.y_min;
				const float min_d = (1.0f / (samples + 1)) * sqrtf(x1 * x1 + y1 * y1);

				for (int i = 0; i <= samples; ++i)
				{
					const float u1 = random_u1(dev),
						v1 = random_v1(dev),
						u2 = random_u2(dev),
						v2 = random_v2(dev);

					const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
					if (d < min_d)
					{
						try
						{
							const Vector2 uv1 = { u1,v1 }, uv2 = { u2,v2 };
							auto start = find_first_common_point(surf1, surf2, uv1, uv2);
							return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop);
						}
						catch (const std::exception&)
						{
							// nothing
						}
					}
				}
			}
			else
			{
				auto prod_box = Box::intersect(p.first.box, p.second.box);
				const float x1 = prod_box.x_max - prod_box.x_min,
					y1 = prod_box.y_max - prod_box.y_min;
				const float min_d = (1.0f / (sample_count_x * sample_count_y + 1)) * sqrtf(x1 * x1 + y1 * y1);

				for (int i1 = 1; i1 <= sample_count_x; ++i1)
					for (int j1 = 1; j1 <= sample_count_y; ++j1)
						for (int i2 = 1; i2 <= sample_count_x; ++i2)
							for (int j2 = 1; j2 <= sample_count_y; ++j2)
							{
								const float u1 = urange1.from + i1 * (urange1.to - urange1.from) / (sample_count_x + 1),
									v1 = vrange1.from + j1 * (vrange1.to - vrange1.from) / (sample_count_y + 1),
									u2 = urange2.from + i2 * (urange2.to - urange2.from) / (sample_count_x + 1),
									v2 = vrange2.from + j2 * (vrange2.to - vrange2.from) / (sample_count_y + 1); // from 0 and (sample_count - 1) - search includes borders, from 1 and (sample_count + 1) - do not include borders
								const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
								if (d < min_d)
								{
									try
									{
										const Vector2 uv1 = { u1,v1 }, uv2 = { u2,v2 };
										auto start = find_first_common_point(surf1, surf2, uv1, uv2);
										return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop);
									}
									catch (const std::exception&)
									{
										// nothing
									}
								}
							}
			}
		}
		//auto bounds1 = surf1.get_patch_bounds(),
		//	bounds2 = surf2.get_patch_bounds();

		//std::list<std::pair<RangedBox<float>, RangedBox<float>>> intersecting_boxes;
		//for (const auto& rb1 : bounds1)
		//{
		//	for (const auto& rb2 : bounds2)
		//	{
		//		auto prod_box = Box::intersect(rb1.box, rb2.box);
		//		if (!prod_box.is_empty())
		//			intersecting_boxes.push_back({ rb1, rb2 });
		//	}
		//}

		//Vector2 uv1min, uv2min;
		///*auto urange1 = surf1.get_u_range(), vrange1 = surf1.get_v_range(),
		//	urange2 = surf2.get_u_range(), vrange2 = surf2.get_v_range();*/
		//auto box1 = surf1.get_bounding_box(),
		//	box2 = surf2.get_bounding_box();
		//size_t max_samples = std::max(sample_count_x, sample_count_y);
		//const float x1 = box1.x_max - box1.x_min,
		//	y1 = box1.y_max - box1.y_min,
		//	x2 = box2.x_max - box2.x_min,
		//	y2 = box2.y_max - box2.y_min;
		//const float min_d = (1.0f / max_samples) * sqrtf(x1 * x1 + y1 * y1 + x2 * x2 + y2 * y2);

		//for (const auto& p : intersecting_boxes)
		//{
		//	auto urange1 = p.first.us, vrange1 = p.first.vs,
		//		urange2 = p.second.us, vrange2 = p.second.vs;
		//	if constexpr (NO_HINT_RANDOM_SAMPLE)
		//	{
		//		std::uniform_real_distribution<float> random_u1(urange1.from, urange1.to);
		//		std::uniform_real_distribution<float> random_v1(vrange1.from, vrange1.to);
		//		std::uniform_real_distribution<float> random_u2(urange2.from, urange2.to);
		//		std::uniform_real_distribution<float> random_v2(vrange2.from, vrange2.to);
		//		for (int i = 0; i <= sample_count_x * sample_count_y / intersecting_boxes.size(); ++i)
		//		{
		//			const float u1 = random_u1(dev),
		//				v1 = random_v1(dev),
		//				u2 = random_u2(dev),
		//				v2 = random_v2(dev);
		//			const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
		//			try
		//			{
		//				const Vector2 uv1 = { u1,v1 }, uv2 = { u2,v2 };
		//				auto start = find_first_common_point(surf1, surf2, uv1, uv2);
		//				return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop);
		//			}
		//			catch (const CommonIntersectionPointNotFoundException&)
		//			{
		//				// nothing
		//			}
		//		}
		//	}
		//	else
		//	{
		//		float dmin = INFINITY;
		//		for (int i1 = 1; i1 <= sample_count_x; ++i1)
		//			for (int j1 = 1; j1 <= sample_count_y; ++j1)
		//				for (int i2 = 1; i2 <= sample_count_x; ++i2)
		//					for (int j2 = 1; j2 <= sample_count_y; ++j2)
		//					{
		//						const float u1 = urange1.from + i1 * (urange1.to - urange1.from) / (sample_count_x + 1),
		//							v1 = vrange1.from + j1 * (vrange1.to - vrange1.from) / (sample_count_y + 1),
		//							u2 = urange2.from + i2 * (urange2.to - urange2.from) / (sample_count_x + 1),
		//							v2 = vrange2.from + j2 * (vrange2.to - vrange2.from) / (sample_count_y + 1); // from 0 and (sample_count - 1) - search includes borders, from 1 and (sample_count + 1) - do not include borders
		//						const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
		//						if (d < dmin)
		//						{
		//							dmin = d;
		//							uv1min = { u1,v1 };
		//							uv2min = { u2,v2 };
		//						}
		//					}
		//	}
		//}

		throw CommonIntersectionPointNotFoundException();
		//auto start = find_first_common_point(surf1, surf2, uv1min, uv2min);
		//return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop);
	}

	ParametricSurfaceIntersection ParametricSurfaceIntersection::self_intersect_surface_with_hint(const ParametricSurface& surf, float step, size_t max_steps, const Vector3& hint)
	{
		auto start = find_first_common_point_on_self_intersection(surf, hint, step);
		return intersect_surfaces(surf, surf, step, max_steps, start.first, start.second, false); // TODO consider if loop forcing is useful in self-intersections 
	}

	ParametricSurfaceIntersection ParametricSurfaceIntersection::self_intersect_surface_without_hint(const ParametricSurface& surf, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y)
	{
		Vector2 uv1min, uv2min;
		auto urange = surf.get_u_range(), vrange = surf.get_v_range();
		float dmin = INFINITY;
		if constexpr (NO_HINT_RANDOM_SAMPLE)
		{
			std::uniform_real_distribution<float> random_u(urange.from, urange.to);
			std::uniform_real_distribution<float> random_v(vrange.from, vrange.to);
			for (int i = 0; i < sample_count_x * sample_count_y; ++i)
			{
				const float u1 = random_u(dev),
					v1 = random_v(dev),
					u2 = random_u(dev),
					v2 = random_v(dev);
				if ((u1 - u2) * (u1 - u2) + (v1 - v2) * (v1 - v2) < 0.25 * step * step)
					continue;
				const float d = (surf.evaluate(u1, v1) - surf.evaluate(u2, v2)).length();
				if (d < dmin)
				{
					dmin = d;
					uv1min = { u1,v1 };
					uv2min = { u2,v2 };
				}
			}
		}
		else
		{
			for (int i1 = 0; i1 <= sample_count_x; ++i1)
				for (int j1 = 0; j1 <= sample_count_y; ++j1)
					for (int i2 = 0; i2 <= sample_count_x; ++i2)
						for (int j2 = 0; j2 <= sample_count_y; ++j2)
						{
							const float ustep = (urange.to - urange.from) / sample_count_x, vstep = (vrange.to - vrange.from) / sample_count_y;
							const float u1 = (i1 + 0.5f) * ustep, v1 = (j1 + 0.5f) * vstep,
								u2 = (i2 + 0.5f) * ustep, v2 = (j2 + 0.5f) * vstep;
							/*const float u1 = urange.from + i1 * (urange.to - urange.from) / (sample_count_x - 1),
								v1 = vrange.from + j1 * (vrange.to - vrange.from) / (sample_count_y - 1),
								u2 = urange.from + i2 * (urange.to - urange.from) / (sample_count_x - 1),
								v2 = vrange.from + j2 * (vrange.to - vrange.from) / (sample_count_y - 1);*/
							if ((u1 - u2) * (u1 - u2) + (v1 - v2) * (v1 - v2) < 0.25 * step * step)
								continue;
							const float d = (surf.evaluate(u1, v1) - surf.evaluate(u2, v2)).length();
							if (d < dmin)
							{
								dmin = d;
								uv1min = { u1,v1 };
								uv2min = { u2,v2 };
							}
						}
		}

		if (!isfinite(dmin))
			throw CommonIntersectionPointNotFoundException();
		auto start = find_first_common_point(surf, surf, uv1min, uv2min);
		if ((start.first - start.second).length() < 0.5f * step)
			throw CommonIntersectionPointNotFoundException();
		return intersect_surfaces(surf, surf, step, max_steps, start.first, start.second, false); // TODO consider if loop forcing is useful in self-intersections 
	}

	std::list<ParametricSurfaceIntersection> ParametricSurfaceIntersection::find_many_intersections(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y, const bool force_loop)
	{
		auto bounds1 = surf1.get_patch_bounds(),
			bounds2 = surf2.get_patch_bounds();

		//auto box1 = surf1.get_bounding_box(),
		//	box2 = surf2.get_bounding_box();
		//size_t max_samples = std::max(sample_count_x, sample_count_y);
		//const float x1 = box1.x_max - box1.x_min,
		//	y1 = box1.y_max - box1.y_min,
		//	x2 = box2.x_max - box2.x_min,
		//	y2 = box2.y_max - box2.y_min;
		//const float min_d = (1.0f / max_samples) * sqrtf(x1 * x1 + y1 * y1 + x2 * x2 + y2 * y2);

		std::list<ParametricSurfaceIntersection> intersections;

		std::list<std::pair<RangedBox<float>, RangedBox<float>>> intersecting_boxes;
		for (const auto& rb1 : bounds1)
		{
			for (const auto& rb2 : bounds2)
			{
				auto prod_box = Box::intersect(rb1.box, rb2.box);
				if (!prod_box.is_empty())
					intersecting_boxes.push_back({ rb1, rb2 });
			}
		}

		/*auto urange1 = surf1.get_u_range(), vrange1 = surf1.get_v_range(),
			urange2 = surf2.get_u_range(), vrange2 = surf2.get_v_range();*/
		for (const auto& p : intersecting_boxes)
		{
			auto urange1 = p.first.us, vrange1 = p.first.vs,
				urange2 = p.second.us, vrange2 = p.second.vs;
			if constexpr (NO_HINT_RANDOM_SAMPLE)
			{
				std::uniform_real_distribution<float> random_u1(urange1.from, urange1.to);
				std::uniform_real_distribution<float> random_v1(vrange1.from, vrange1.to);
				std::uniform_real_distribution<float> random_u2(urange2.from, urange2.to);
				std::uniform_real_distribution<float> random_v2(vrange2.from, vrange2.to);
				const int samples = sample_count_x * sample_count_y / intersecting_boxes.size();

				auto prod_box = Box::intersect(p.first.box, p.second.box);
				const float x1 = prod_box.x_max - prod_box.x_min,
					y1 = prod_box.y_max - prod_box.y_min;
				const float min_d = (1.0f / (samples + 1)) * sqrtf(x1 * x1 + y1 * y1);

				for (int i = 0; i <= samples; ++i)
				{
					const float u1 = random_u1(dev),
						v1 = random_v1(dev),
						u2 = random_u2(dev),
						v2 = random_v2(dev);

					const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
					if (d < min_d)
					{
						try
						{
							const Vector2 uv1 = { u1,v1 }, uv2 = { u2,v2 };
							auto start = find_first_common_point(surf1, surf2, uv1, uv2);
							bool is_new = true;
							for (const auto& isec : intersections)
							{
								if (isec.can_be_started_by(start.first, start.second, step))
								{
									is_new = false;
									break;
								}
							}
							if (is_new)
								intersections.push_back(intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop));
						}
						catch (const std::exception&)
						{
							// nothing
						}
					}
				}
			}
			else
			{
				auto prod_box = Box::intersect(p.first.box, p.second.box);
				const float x1 = prod_box.x_max - prod_box.x_min,
					y1 = prod_box.y_max - prod_box.y_min;
				const float min_d = (1.0f / (sample_count_x * sample_count_y + 1)) * sqrtf(x1 * x1 + y1 * y1);

				for (int i1 = 1; i1 <= sample_count_x; ++i1)
					for (int j1 = 1; j1 <= sample_count_y; ++j1)
						for (int i2 = 1; i2 <= sample_count_x; ++i2)
							for (int j2 = 1; j2 <= sample_count_y; ++j2)
							{
								const float u1 = urange1.from + i1 * (urange1.to - urange1.from) / (sample_count_x + 1),
									v1 = vrange1.from + j1 * (vrange1.to - vrange1.from) / (sample_count_y + 1),
									u2 = urange2.from + i2 * (urange2.to - urange2.from) / (sample_count_x + 1),
									v2 = vrange2.from + j2 * (vrange2.to - vrange2.from) / (sample_count_y + 1); // from 0 and (sample_count - 1) - search includes borders, from 1 and (sample_count + 1) - do not include borders
								const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
								if (d < min_d)
								{
									try
									{
										const Vector2 uv1 = { u1,v1 }, uv2 = { u2,v2 };
										auto start = find_first_common_point(surf1, surf2, uv1, uv2);
										bool is_new = true;
										for (const auto& isec : intersections)
										{
											if (isec.can_be_started_by(start.first, start.second, step))
											{
												is_new = false;
												break;
											}
										}
										if (is_new)
											intersections.push_back(intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second, force_loop));
									}
									catch (const std::exception&)
									{
										// nothing
									}
								}
							}
			}
		}
		return intersections;
	}
}
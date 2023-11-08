#include "surface_path.h"
#include "offset_surface.h"
#include "parametric_surface_intersection.h"
#include "zig_zag_path.h"

namespace ManualCAD
{
    template <class T, class U>
    int get_idx(const std::vector<T>& container, const U& elem) { return dynamic_cast<const T*>(&elem) - container.data(); }

    std::vector<std::vector<std::vector<Vector2>>> SurfacePath::generate_paths(const PlaneXZ& plane, const float radius, const float epsilon) const
    {
        std::vector<OffsetSurface> offset_surfs;
        offset_surfs.reserve(surfaces.size());

        for (const auto* s : surfaces)
            offset_surfs.push_back(OffsetSurface{ *s, radius });

        std::list<ParametricSurfaceIntersection> intersections;
        for (int i=0;i<offset_surfs.size();++i)
            for (int j = i + 1; j < offset_surfs.size(); ++j)
            {
                auto pair_intersections = ParametricSurfaceIntersection::find_many_intersections(offset_surfs[i], offset_surfs[j], 0.01f, 2000, 20, 20);
                intersections.insert(intersections.end(), pair_intersections.begin(), pair_intersections.end());
            }

        for (int i = 0; i < offset_surfs.size(); ++i)
        {
            auto plane_intersections = ParametricSurfaceIntersection::find_many_intersections(offset_surfs[i], plane, 0.01f, 2000, 20, 20); // TODO offset plane
            intersections.insert(intersections.end(), plane_intersections.begin(), plane_intersections.end());
        }

        std::vector<std::list<std::pair<std::vector<Vector2>, bool>>> intersection_curves_for_surfs(offset_surfs.size());
        for (auto& isec : intersections)
        {
            intersection_curves_for_surfs[get_idx(offset_surfs, isec.get_surf1())].push_back({ isec.get_uvs1(), isec.is_looped() });
            if (&isec.get_surf2() != &plane)
                intersection_curves_for_surfs[get_idx(offset_surfs, isec.get_surf2())].push_back({ isec.get_uvs2(), isec.is_looped() });
        }

        std::vector<std::vector<std::vector<Vector2>>> result(offset_surfs.size());
        for (int i = 0; i < offset_surfs.size(); ++i)
        {
            ZigZagPath path;
            for (const auto& line : intersection_curves_for_surfs[i])
                path.add_line(line.first, line.second);
            auto result2 = path.generate_paths_excluding_segments(offset_surfs[i].get_u_range(), offset_surfs[i].get_v_range(), radius, epsilon, [&offset_surfs, i](const Vector2& start, const Vector2& end) {
                auto middle = 0.5f * (start + end);
                const auto& surf = offset_surfs[i];
                if (dot(surf.normal(middle.x, middle.y), { 0.0f,1.0f,0.0f }) <= 0)
                    return false;

                return true;
            });
            result[i] = result2;
        }

        return result;
    }
}

#include "surface_path.h"
#include "offset_surface.h"
#include "parametric_surface_intersection.h"
#include "zig_zag_path.h"
#include "height_map_renderer.h"

namespace ManualCAD
{
    template <class T, class U>
    int get_idx(const std::vector<T>& container, const U& elem) { return dynamic_cast<const T*>(&elem) - container.data(); }

    std::vector<std::vector<std::vector<Vector2>>> SurfacePath::generate_paths(const PlaneXZ& plane, const float radius, const float epsilon, const float mill_height, const Vector3& real_workpiece_size) const
    {
        std::vector<OffsetSurface> offset_surfs;
        offset_surfs.reserve(surfaces.size());

        for (const auto* s : surfaces)
            offset_surfs.push_back(OffsetSurface{ *s, radius });

        std::list<ParametricSurfaceIntersection> intersections;
        for (int i=0;i<offset_surfs.size();++i)
            for (int j = i + 1; j < offset_surfs.size(); ++j)
            {
                auto pair_intersections = ParametricSurfaceIntersection::find_many_intersections(offset_surfs[i], offset_surfs[j], 0.01f, 2000, 29, 29);
                intersections.insert(intersections.end(), pair_intersections.begin(), pair_intersections.end());
            }

        for (int i = 0; i < offset_surfs.size(); ++i)
        {
            auto plane_intersections = ParametricSurfaceIntersection::find_many_intersections(offset_surfs[i], plane, 0.01f, 2000, 29, 29); // TODO offset plane
            intersections.insert(intersections.end(), plane_intersections.begin(), plane_intersections.end());
        }

        std::vector<std::list<std::pair<std::vector<Vector2>, bool>>> intersection_curves_for_surfs(offset_surfs.size());
        for (auto& isec : intersections)
        {
            intersection_curves_for_surfs[get_idx(offset_surfs, isec.get_surf1())].push_back({ isec.get_uvs1(), isec.is_looped() });
            if (&isec.get_surf2() != &plane)
                intersection_curves_for_surfs[get_idx(offset_surfs, isec.get_surf2())].push_back({ isec.get_uvs2(), isec.is_looped() });
        }

        // ***
        /*std::vector<std::vector<std::vector<Vector2>>> result(offset_surfs.size());
        for (int i = 0; i < offset_surfs.size(); ++i)
        {
            for (const auto& pairs : intersection_curves_for_surfs[i])
            {
                result[i].push_back(pairs.first);
            }
        }
        return result;*/
        // ***

        Box box = plane.get_bounding_box();
        box.y_max += mill_height;
        Vector3 map_size = { box.x_max - box.x_min, box.y_max - box.y_min,box.z_max - box.z_min };
        auto height_map = HeightMapRenderer{ surfaces, box }.render_index_map(map_size);

        auto box_center3 = box.center();
        Vector2 box_center = { box_center3.x, box_center3.z };

        std::vector<std::vector<std::vector<Vector2>>> result(offset_surfs.size());
        std::vector<const ParametricSurfaceObject*> surfs_vector(surfaces.begin(), surfaces.end());
        for (int i = 0; i < offset_surfs.size(); ++i)
        {
            ZigZagPath path;
            for (const auto& line : intersection_curves_for_surfs[i])
                path.add_line(line.first, line.second);

            auto result2 = path.generate_paths_excluding_segments(offset_surfs[i].get_u_range(), offset_surfs[i].get_v_range(), radius, epsilon, [&offset_surfs, i, &height_map, &surfs_vector, box_center](const Vector2& start, const Vector2& end) {
                auto middle = 0.5f * (start + end);
                const auto& surf = offset_surfs[i];
                if (dot(surf.normal(middle.x, middle.y), { 0.0f,1.0f,0.0f }) <= 0)
                    return false;

                auto point3 = surf.evaluate(middle.x, middle.y);
                Vector2 point = { point3.x,point3.z };
                const auto coords = height_map.position_to_pixel(point - box_center);

                int idx = lroundf(height_map.get_pixel(lroundf(coords.y), lroundf(coords.x)));
               /* int idxpx = lroundf(height_map.get_pixel(lroundf(coords.y), lroundf(coords.x) + 1));
                int idxmx = lroundf(height_map.get_pixel(lroundf(coords.y), lroundf(coords.x) - 1));
                int idxpy = lroundf(height_map.get_pixel(lroundf(coords.y) + 1, lroundf(coords.x)));
                int idxmy = lroundf(height_map.get_pixel(lroundf(coords.y) - 1, lroundf(coords.x)));

                if (idx != idxpx || idx != idxmx || idx != idxpy || idx != idxmy)
                {
                    auto other = 0.25f * start + 0.75f * end;
                    point3 = surf.evaluate(other.x, other.y);
                    point = { point3.x,point3.z };
                    const auto coords2 = height_map.position_to_pixel(point - box_center);
                    idx = lroundf(height_map.get_pixel(lroundf(coords2.y), lroundf(coords2.x)));
                }*/

                if ((i + 1) != idx)
                    return false;

                return true;
            });

            result[i] = result2;
        }

        return result;
    }
}

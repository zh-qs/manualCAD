#include "point_set.h"
#include "renderer.h"

namespace ManualCAD
{
    void PointSet::set_data(const std::vector<Vector3>& points)
    {
        vbo.bind();
        vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
        point_count = points.size();
    }

    void PointSet::render(Renderer& renderer, int width, int height, float thickness) const {
        renderer.render_points(*this, color, width, height, thickness);
    }

    //PointSet PointSet::point(const Matrix4x4& model)
    //{
    //    PointSet set(1, model);
    //    set.points[0] = { 0,0,0 };
    //    return set;
    //}
}
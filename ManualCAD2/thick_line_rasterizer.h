#pragma once

/*
 inspired by http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
*/

#include <tuple>
#include <cmath>
#include "height_map.h"
#include "cutter.h"

namespace ManualCAD
{
    class ThickLineRasterizer {

        HeightMap& height_map;
        const Cutter& cutter;
        Vector2 from, to;
        float from_h, to_h;

        float length_from_to;

        const float& max_depth;
        const int& instruction_number;

        struct Vertex {
            int x, y;
        } vt1, vt2, vt3, vt4;

        Vertex to_vertex(const Vector2& v)
        {
            return { lroundf(v.x), lroundf(v.y) };
        }

        void put_pixel(int x, int y)
        {
            auto pos = height_map.pixel_to_position({ static_cast<float>(x), static_cast<float>(y) });
            float distance = abs((to.x - from.x) * (from.y - pos.y) - (from.x - pos.x) * (to.y - from.y)) / length_from_to;
            float t = sqrtf((from - pos).lengthsq() - distance * distance);
            float h = lerp(from_h, to_h, t);

            cutter.check_cutter(height_map, x, y, instruction_number, h, max_depth);
            height_map.set_pixel(x, y, h + cutter.get_height_offset(distance));
        }

        void drawLine(int x1, int x2, int y)
        {
            if (x2 < x1)
                std::swap(x1, x2);
            for (int i = x1; i <= x2; ++i)
                put_pixel(i, y);
        }

        void fillBottomFlatTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
        {
            if (v2.y == v1.y || v3.y == v1.y) return;

            float invslope1 = static_cast<float>(v2.x - v1.x) / static_cast<float>(v2.y - v1.y);
            float invslope2 = static_cast<float>(v3.x - v1.x) / static_cast<float>(v3.y - v1.y);

            float curx1 = v1.x;
            float curx2 = v1.x;

            for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++)
            {
                drawLine(lroundf(curx1), lroundf(curx2), scanlineY);
                curx1 += invslope1;
                curx2 += invslope2;
            }
        }

        void fillTopFlatTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
        {
            if (v3.y == v1.y || v3.y == v2.y) return;

            float invslope1 = static_cast<float>(v3.x - v1.x) / static_cast<float>(v3.y - v1.y);
            float invslope2 = static_cast<float>(v3.x - v2.x) / static_cast<float>(v3.y - v2.y);

            float curx1 = v3.x;
            float curx2 = v3.x;

            for (int scanlineY = v3.y; scanlineY > v1.y; scanlineY--)
            {
                drawLine(lroundf(curx1), lroundf(curx2), scanlineY);
                curx1 -= invslope1;
                curx2 -= invslope2;
            }
        }

        void drawTriangle(Vertex v1, Vertex v2, Vertex v3)
        {
            /* at first sort the three vertices by y-coordinate ascending so v1 is the topmost vertice */
            if (v3.y < v2.y)
            {
                std::swap(v2, v3);
            }
            if (v2.y < v1.y)
            {
                std::swap(v1, v2);
            }
            if (v3.y < v2.y)
            {
                std::swap(v2, v3);
            }

            /* here we know that v1.y <= v2.y <= v3.y */
            /* check for trivial case of bottom-flat triangle */
            if (v2.y == v3.y)
            {
                fillBottomFlatTriangle(v1, v2, v3);
            }
            /* check for trivial case of top-flat triangle */
            else if (v1.y == v2.y)
            {
                fillTopFlatTriangle(v1, v2, v3);
            }
            else
            {
                /* general case - split the triangle in a topflat and bottom-flat one */
                Vertex v4 = {
                    lroundf(v1.x + (static_cast<float>(v2.y - v1.y) / static_cast<float>(v3.y - v1.y)) * (v3.x - v1.x)),
                    v2.y };
                fillBottomFlatTriangle(v1, v2, v4);
                fillTopFlatTriangle(v2, v4, v3);
            }
        }

        void generate_vertices()
        {
            auto line_v = to - from;
            Vector2 normal = Vector2{ -line_v.y, line_v.x }.normalize();
            float radius = cutter.get_diameter() * 0.5f;
            normal *= radius;
            vt1 = to_vertex(height_map.position_to_pixel(from + normal));
            vt2 = to_vertex(height_map.position_to_pixel(from - normal));
            vt3 = to_vertex(height_map.position_to_pixel(to + normal));
            vt4 = to_vertex(height_map.position_to_pixel(to - normal));
        }
    public:
        ThickLineRasterizer(HeightMap& height_map, const Cutter& cutter, const Vector3& from, const Vector3& to, const int& instruction_number, const float& max_depth) : height_map(height_map), cutter(cutter), instruction_number(instruction_number), max_depth(max_depth) {
            this->from = { from.x, from.y };
            this->to = { to.x, to.y };
            from_h = from.z;
            to_h = to.z;
            length_from_to = (this->to - this->from).length();
            generate_vertices();
        }

        void draw()
        {
            drawTriangle(vt1, vt2, vt3);
            drawTriangle(vt2, vt3, vt4);
        }
    };
}

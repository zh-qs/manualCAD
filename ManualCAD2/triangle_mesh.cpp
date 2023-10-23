#include "triangle_mesh.h"
#include "renderer.h"

namespace ManualCAD
{
	void TriangleMesh::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_triangle_mesh(*this, color, width, height, thickness);
	}

	void TriangleMesh::set_data(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const std::vector<IndexTriple>& triangle_indices)
	{
		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		normal_vbo.bind();
		normal_vbo.set_dynamic_data(reinterpret_cast<const float*>(normals.data()), normals.size() * sizeof(Vector3));
		ebo.bind();
		ebo.set_dynamic_data(reinterpret_cast<const unsigned int*>(triangle_indices.data()), triangle_indices.size() * sizeof(IndexTriple));

		point_count = points.size();
		triangle_count = triangle_indices.size();
	}

	void TriangleMesh::generate_cylinder(float radius, float height, unsigned int circle_divisions)
	{
		std::vector<Vector3> points(4 * circle_divisions + 2);
		std::vector<Vector3> normals(points.size());
		std::vector<IndexTriple> triangle_indices(4 * circle_divisions);
		float step = TWO_PI / circle_divisions;

		// points
		points[0] = { 0.0f, 0.0f, 0.0f };
		normals[0] = { 0.0f, -1.0f, 0.0f };
		for (unsigned int i = 1; i <= circle_divisions; ++i)
		{
			float t = i * step;
			float cos = cosf(t), sin = sinf(t);
			points[i] = points[i + 2 * circle_divisions] = { radius * cos, 0.0f, radius * sin };
			points[i + circle_divisions] = points[i + 3 * circle_divisions] = { radius * cos, height, radius * sin };
			normals[i] = { 0.0f, -1.0f, 0.0f };
			normals[i + circle_divisions] = { 0.0f,1.0f,0.0f };
			normals[i + 2 * circle_divisions] = normals[i + 3 * circle_divisions] = { cos, 0.0f, sin };
		}
		points[points.size() - 1] = { 0.0f, height, 0.0f };
		normals[normals.size() - 1] = { 0.0f, 1.0f, 0.0f };

		// lower base faces
		for (unsigned int i = 0; i < circle_divisions - 1; ++i)
		{
			triangle_indices[i] = { 0, i + 1, i + 2 };
		}
		triangle_indices[circle_divisions - 1] = { 0, circle_divisions, 1 };

		// side faces
		for (unsigned int i = 0; i < circle_divisions - 1; ++i)
		{
			triangle_indices[circle_divisions + i] = { i + 2 * circle_divisions + 1, i + 2 * circle_divisions + 2, i + 3 * circle_divisions + 1 };
			triangle_indices[2 * circle_divisions + i] = { i + 3 * circle_divisions + 1, i + 2 * circle_divisions + 2, i + 3 * circle_divisions + 2 };
		}
		triangle_indices[2 * circle_divisions - 1] = { 3 * circle_divisions, 1 + 2 * circle_divisions, 4 * circle_divisions };
		triangle_indices[3 * circle_divisions - 1] = { 4 * circle_divisions, 1 + 2 * circle_divisions, 3 * circle_divisions + 1 };

		// upper base faces
		for (unsigned int i = 0; i < circle_divisions - 1; ++i)
		{
			triangle_indices[3 * circle_divisions + i] = { 4 * circle_divisions + 1, i + circle_divisions + 2, i + circle_divisions + 1 };
		}
		triangle_indices[4 * circle_divisions - 1] = { 4 * circle_divisions + 1, circle_divisions + 1, 2 * circle_divisions };

		set_data(points, normals, triangle_indices);
	}

	void TriangleMesh::generate_bottom_capsule(float radius, float height, unsigned int circle_divisions)
	{
		auto sphere_divisions = circle_divisions / 4 + 1;

		std::vector<Vector3> points((sphere_divisions + 2) * circle_divisions + 2);
		std::vector<Vector3> normals(points.size());
		std::vector<IndexTriple> triangle_indices((2 * sphere_divisions + 3) * circle_divisions);
		float step = TWO_PI / circle_divisions;

		float step_sphere = HALF_PI / sphere_divisions;

		// points
		points[0] = { 0.0f, height, 0.0f };
		normals[0] = { 0.0f,1.0f,0.0f };
		for (unsigned int i = 1; i <= circle_divisions; ++i)
		{
			float t = i * step;
			float cos = cosf(t), sin = sinf(t);

			points[i] = points[i + circle_divisions] = { radius * cos, height, radius * sin };
			normals[i] = { 0.0f,1.0f,0.0f };
			normals[i + circle_divisions] = { cos, 0.0f, sin };

			for (int j = 0; j < sphere_divisions; ++j)
			{
				float s = j * step_sphere;
				float scos = cosf(s), ssin = sinf(s);
				points[circle_divisions * (j + 2) + i] = { radius * cos * scos, radius * (1 - ssin), radius * sin * scos };
				normals[circle_divisions * (j + 2) + i] = { cos * scos, ssin, sin * scos };
			}
		}
		points[points.size() - 1] = { 0.0f, 0.0f, 0.0f };
		normals[normals.size() - 1] = { 0.0f, -1.0f, 0.0f };

		// upper base faces
		for (unsigned int i = 0; i < circle_divisions - 1; ++i)
		{
			triangle_indices[i] = { 0, i + 1, i + 2 };
		}
		triangle_indices[circle_divisions - 1] = { 0, circle_divisions, 1 };

		// side faces
		for (unsigned int j = 1; j < sphere_divisions + 1; ++j)
		{
			for (unsigned int i = 0; i < circle_divisions - 1; ++i)
			{
				triangle_indices[2 * j * circle_divisions + i] = { i + j * circle_divisions + 1, i + j * circle_divisions + 2, i + (j + 1) * circle_divisions + 1 };
				triangle_indices[(2 * j + 1) * circle_divisions + i] = { i + (j + 1) * circle_divisions + 1, i + j * circle_divisions + 2, i + (j + 1) * circle_divisions + 2 };
			}
			triangle_indices[(2 * j + 1) * circle_divisions - 1] = { (j + 1) * circle_divisions, j * circle_divisions + 1, (j + 2) * circle_divisions };
			triangle_indices[(2 * j + 2) * circle_divisions - 1] = { (j + 2) * circle_divisions, j * circle_divisions + 1, (j + 1) * circle_divisions + 1 };
		}

		unsigned int last = points.size() - 1;
		// lower hemisphere
		for (unsigned int i = 0; i < circle_divisions - 1; ++i)
		{
			triangle_indices[i + (2 * sphere_divisions + 2) * circle_divisions] = { last, i + (sphere_divisions + 1) * circle_divisions + 1, i + (sphere_divisions + 1) * circle_divisions + 2 };
		}
		triangle_indices[(2 * sphere_divisions + 3) * circle_divisions - 1] = { last, (sphere_divisions + 2) * circle_divisions, (sphere_divisions + 1) * circle_divisions + 1 };

		set_data(points, normals, triangle_indices);
	}
}
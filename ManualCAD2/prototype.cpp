#include "prototype.h"
#include "object_settings.h"

namespace ManualCAD
{
	int Prototype::counter = 0;

	void Prototype::generate_renderable()
	{
		update_view();
		view.color = color;
	}

	void Prototype::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_prototype_settings(*this, parent);
	}

	void Prototype::update_view()
	{
		if (surfaces.empty())
		{
			view.set_data({});
			return;
		}

		if (!box_valid)
		{
			bounding_box = Box::degenerate();
			for (const auto* s : surfaces)
			{
				auto surf_box = s->get_bounding_box();
				bounding_box.merge_with(surf_box);
			}
			box_valid = true;
		}
		Box box = bounding_box;
		box.offset_by({ offset, 0.0f, offset });

		auto center = box.center();
		// TODO wpasowaæ w size
		float size_scale = size.x / size.z;
		float box_scale = (box.x_max - box.x_min) / (box.z_max - box.z_min);
		if (box_scale > size_scale)
			scale = (box.x_max - box.x_min) / size.x;
		else
			scale = (box.z_max - box.z_min) / size.z;

		float h = box.y_max - mill_height * scale;
		float xm = center.x - 0.5f * size.x * scale, xM = center.x + 0.5f * size.x * scale,
			zm = center.z - 0.5f * size.z * scale, zM = center.z + 0.5f * size.z * scale;
		std::vector<Vector3> points = {
			{xm,h,zm}, {xm, h,zM}, {xM,h,zM}, {xM,h,zm}, {xm,h,zm}
		};
		view.set_data(points);
	}

	std::vector<ObjectHandle> Prototype::clone() const
	{
		return std::vector<ObjectHandle>(); // Prototype is a special object and thus can't be cloned
	}
}
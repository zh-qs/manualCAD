#include "ellipsoid.h"
#include "object_settings.h"

namespace ManualCAD
{
	int Ellipsoid::counter = 0;

	void Ellipsoid::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_ellipsoid_settings(*this, parent);
	}

	std::vector<ObjectHandle> Ellipsoid::clone() const
	{
		std::vector<ObjectHandle> result(1);
		auto handle = Object::create<Ellipsoid>();
		copy_basic_attributes_to(*handle);
		handle->raycastable.form = raycastable.form;
		result[0] = std::move(handle);
		return result;
	}
}

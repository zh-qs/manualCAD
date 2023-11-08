#pragma once

#include "algebra.h"
#include "height_map.h"
#include <string>

namespace ManualCAD
{
	struct CutterMove {
		int instruction_number;
		bool fast;
		Vector3 origin;
		Vector3 destination;

		void to_gcode_positions() {
			std::swap(origin.y, origin.z);
			std::swap(destination.y, destination.z);
		}
	};
}
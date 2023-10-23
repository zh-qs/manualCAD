#pragma once

#include "algebra.h"
#include "height_map.h"

namespace ManualCAD
{
	struct CutterMove {
		int instruction_number;
		bool fast;
		Vector3 origin;
		Vector3 destination;
	};
}
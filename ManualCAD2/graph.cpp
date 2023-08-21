#include "graph.h"
#include "object.h"

namespace ManualCAD
{
	PatchEdge Graph<Point*, PatchEdge>::reverse(const PatchEdge& w)
	{
		PatchEdge result{};
		result.edge[0] = w.edge[3];
		result.edge[1] = w.edge[2];
		result.edge[2] = w.edge[1];
		result.edge[3] = w.edge[0];

		result.second[0] = w.second[3];
		result.second[1] = w.second[2];
		result.second[2] = w.second[1];
		result.second[3] = w.second[0];

		result.patch_ref = w.patch_ref;
		return result;
	}
}
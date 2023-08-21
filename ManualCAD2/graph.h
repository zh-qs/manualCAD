#pragma once

#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <iterator>

namespace ManualCAD {
	template <class TVert, class TEdgeWeight>
	class Graph {
		using vertex_t = int;
		using EdgeInfo = std::pair<vertex_t, TEdgeWeight>;
		struct EdgeInfoLessComparator {
			constexpr bool operator()(const EdgeInfo& i1, const EdgeInfo& i2) const {
				return i1.first < i2.first;
			}
		};
		size_t vertex_count;
		std::map<TVert, vertex_t> vertex_to_idx;
		std::vector<TVert> vertices;
		std::vector<std::multiset<EdgeInfo, EdgeInfoLessComparator>> neighbors;

		// needs to be specialised in graph.cpp before use!
		TEdgeWeight reverse(const TEdgeWeight& w);

	public:
		Graph() : vertices(), neighbors(), vertex_count(0) {}
		Graph(size_t vertex_count) : vertices(vertex_count), neighbors(vertex_count), vertex_count(vertex_count) {}
		void assign_vertex(vertex_t i, const TVert& value) { vertex_to_idx.erase(vertices[i]); vertices[i] = value; vertex_to_idx[value] = i; }
		vertex_t add_vertex(const TVert& value) {
			auto search = vertex_to_idx.find(value);
			if (search == vertex_to_idx.end())
			{
				vertices.push_back(value);
				vertex_to_idx[value] = vertex_count;
				neighbors.push_back({});
				return vertex_count++;
			}
			return search->second;
		}
		void add_edge(vertex_t from, vertex_t to, const TEdgeWeight& weight) { neighbors[from].insert(std::pair(to, weight)); neighbors[to].insert(std::pair(from, reverse(weight))); }
		void add_edge(const TVert& from, const TVert& to, const TEdgeWeight& weight) {
			auto from_num = add_vertex(from);
			auto to_num = add_vertex(to);
			add_edge(from_num, to_num, weight);
		}

		struct Edge
		{
			vertex_t from, to;
			TEdgeWeight weight;
		};
		std::list<Edge> get_edges()
		{
			std::list<Edge> result;

			for (int from = 0; from < vertex_count; ++from)
			{
				for (const auto& info : neighbors[from])
				{
					result.push_back({ from, info.first, info.second });
				}
			}

			return result;
		}
		template <size_t N>
		struct Cycle
		{
			Edge edges[N];
		};
		using Triangle = Cycle<3>;
		std::list<Triangle> find_triangles()
		{
			std::list<Triangle> result;
			auto edges = get_edges();

			for (const auto& e : edges)
			{
				if (e.from >= e.to) continue;
				for (const auto& info : neighbors[e.to])
				{
					if (info.first <= e.to) continue;
					auto it = neighbors[info.first].find({ e.from, {} });
					if (it == neighbors[info.first].end()) continue;
					while (it->first == e.from)
					{
						Triangle t;
						t.edges[0] = e;
						t.edges[1].from = e.to;
						t.edges[1].to = info.first;
						t.edges[1].weight = info.second;
						t.edges[2].from = info.first;
						t.edges[2].to = e.from;
						t.edges[2].weight = it->second;
						result.push_back(t);
						it++;
					}
				}
			}

			return result;
		}
	};
}
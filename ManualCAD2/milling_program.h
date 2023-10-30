#pragma once

#include "algebra.h"
#include "task.h"
#include <list>
#include "cutter_move.h"
#include "cutter.h"
#include "logger.h"
#include <memory>
#include <vector>

namespace ManualCAD
{
	class Workpiece;
	class MillingProgram {
		friend class ObjectSettings;

		float ratio_to_centimeters = 0.1f;
		float cutter_rpm = 10000;
		float cutter_speed = 25;
		std::list<CutterMove> moves;
		std::unique_ptr<Cutter> cutter;
		std::string name;
	public:
		
		MillingProgram(const char* name) : name(name) { }

		Task get_task(Workpiece& workpiece, bool& task_ended) const;
		void execute_on(Workpiece& workpiece) const;

		const std::string& get_name() const { return name; }
		float get_ratio_to_centimeters() { return ratio_to_centimeters; }
		void set_ratio_to_centimeters(float ratio) { ratio_to_centimeters = ratio; }
		void set_cutter_rpm(float rpm) { cutter_rpm = rpm; }
		void set_cutter_speed(float speed) { cutter_speed = speed; }
		void add_move(CutterMove&& move) { moves.push_back(std::move(move)); }
		void set_cutter(std::unique_ptr<Cutter>&& cutter) { this->cutter = std::move(cutter); }
		const Cutter& get_cutter() const { return *cutter; }
		const Vector3& get_start_position() const {
			if (moves.empty())
				return { 0.0f,0.0f,10.0f };
			return moves.front().origin;
		}
		const Vector3& get_end_position() const {
			if (moves.empty()) 
				return { 0.0f,0.0f,10.0f };
			return moves.back().destination;
		}

		std::vector<Vector3> get_cutter_positions() const;

		static MillingProgram read_from_file(const char* filename);
		void save_to_file(const char* filename);
	};
}
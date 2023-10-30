#include "milling_program.h"
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include "workpiece.h"
#include "object_settings.h"
#include "thick_line_rasterizer.h"

namespace ManualCAD
{
	void assert_char(const std::string& line, int i, char c)
	{
		if (line[i] != c)
			throw std::runtime_error("Error while parsing: Line \"" + line + "\" at " + std::to_string(i) + ": Expected '" + c + "', got '" + line[i] + "'");
	}

	void assert_string(const std::string& line, int i, const std::string& str)
	{
		if (line.substr(i, str.size()) != str)
			throw std::runtime_error("Error while parsing: Line \"" + line + "\" at " + std::to_string(i) + ": Expected \"" + str + "\", got \"" + line.substr(i, str.size()) + "\"");
	}

	bool isdigit(char c)
	{
		return c >= '0' && c <= '9';
	}

	float extract_float(const std::string& line, int start, int& end)
	{
		int i = start;
		bool was_dot = false;

		if (line[i] == '-')
		{
			++i;
		}
		while (isdigit(line[i]) || (line[i] == '.' && !was_dot))
		{
			if (line[i] == '.')
				was_dot = true;

			++i;
		}

		end = i;
		return std::stof(line.substr(start, i - start));
	}

	int extract_number(const std::string& line, int start, int& end)
	{
		int result = 0, i = start;
		while (isdigit(line[i]))
		{
			result *= 10;
			result += line[i] - '0';
			++i;
		}
		end = i;
		return result;
	}

	void process_line(const std::string& line, MillingProgram& program)
	{
		int pos;
		assert_char(line, 0, 'N');
		int instruction_number = extract_number(line, 1, pos);
		int opcode = -1;

		if (instruction_number == 1)
		{
			assert_string(line, pos, "G40G90");
			return;
		}
		if (instruction_number == 2)
		{
			assert_char(line, pos, 'S');
			++pos;
			program.set_cutter_rpm(extract_number(line, pos, pos));
			if (pos == line.size())
				return;
			assert_string(line, pos, "M03");
		}

		switch (line[pos])
		{
		case '%':
			++pos;
			assert_char(line, pos, 'G');
			++pos;
			opcode = extract_number(line, pos, pos);
			switch (opcode)
			{
			case 71:
				program.set_ratio_to_centimeters(0.1f);
				break;
			case 70:
				program.set_ratio_to_centimeters(2.54f);
				break;
			default:
				throw std::runtime_error("Error while parsing: Line \"" + line + "\": wrong metric unit code : " + std::to_string(opcode));
			}
			return;
		case 'F':
			++pos;
			program.set_cutter_speed(extract_number(line, pos, pos) / 600); // mm/min -> cm/s
			return;
		case 'M':
			return; // do nothing
		case 'G':
			++pos;
			opcode = extract_number(line, pos, pos);
			if (opcode != 0 && opcode != 1)
				throw std::runtime_error("Error parsing line " + std::to_string(instruction_number) + ": wrong opcode: " + std::to_string(opcode));
			CutterMove move;
			move.instruction_number = instruction_number;
			move.fast = opcode == 0;
			move.origin = move.destination = program.get_end_position();
			while (pos != line.size())
			{
				switch (line[pos])
				{
				case 'X':
					++pos;
					move.destination.x = extract_float(line, pos, pos) * program.get_ratio_to_centimeters();
					break;
				case 'Y':
					++pos;
					move.destination.y = -extract_float(line, pos, pos) * program.get_ratio_to_centimeters();
					break;
				case 'Z':
					++pos;
					move.destination.z = extract_float(line, pos, pos) * program.get_ratio_to_centimeters();
					break;
				default:
					throw std::runtime_error("Error while parsing: Line \"" + line + "\" at " + std::to_string(pos) + ": Expected 'X' or 'Y' or 'Z', got \'" + line[pos] + "\'");
				}
			}
			program.add_move(std::move(move));
			return;
		default:
			throw std::runtime_error("Error parsing line " + std::to_string(instruction_number) + ": wrong character: " + line[pos]);
		}
	}

	std::unique_ptr<Cutter> create_cutter_from_file_extension(const char* filename)
	{
		std::string fstr(filename);
		auto dot_idx = fstr.rfind('.');
		if (dot_idx == std::string::npos || dot_idx == fstr.size() - 1)
			throw std::runtime_error("Empty file extension");

		auto extension = fstr.substr(dot_idx + 1);
		if (extension.size() != 3)
			throw std::runtime_error("Wrong file extension length; should be 3 characters");
		int pos;
		switch (extension[0])
		{
		case 'k':
			return std::make_unique<BallCutter>(extract_number(extension, 1, pos) * 0.1f);
			break;
		case 'f':
			return std::make_unique<FlatCutter>(extract_number(extension, 1, pos) * 0.1f);
			break;
		default:
			throw std::runtime_error("Wrong file extension");
		}
	}

	class MoveCutterTaskStep : public SingleTaskStep
	{
		int instruction_number;
		float percent = 0.0f;
		const float& speed;
		Vector3 from, to;
		float path_length;
		Workpiece& workpiece;
		const Cutter& cutter;

		std::pair<int, int> previous_pixel;
		Vector3 previous_pos;

	public:
		MoveCutterTaskStep(Workpiece& workpiece, int instruction_number, const Cutter& cutter, const Vector3& from, const Vector3& to, const float& speed) : workpiece(workpiece), instruction_number(instruction_number), cutter(cutter), from(from), to(to), speed(speed) {
			auto start = workpiece.height_map.position_to_pixel(from);

			previous_pixel = { lroundf(start.x), lroundf(start.y) };
			previous_pos = from;

			path_length = (to - from).length();
		}

		float interpolate(float a, float b, float l1, float l2)
		{
			if (l1 + l2 == 0)
				return a;
			float percent = l1 / (l1 + l2);
			return a * (1 - percent) + b * percent;
		}

		// very slow, not used actually
		void cut_line_pure_bresenham(const std::pair<int, int>& from_pix, const std::pair<int, int>& to_pix, float from_h, float to_h)
		{
			int x0 = from_pix.first, y0 = from_pix.second,
				x1 = to_pix.first, y1 = to_pix.second;
			auto dx = abs(x1 - x0);
			auto sx = x0 < x1 ? 1 : -1;
			auto dy = -abs(y1 - y0);
			auto sy = y0 < y1 ? 1 : -1;
			auto error = dx + dy;

			while (true) {
				float l1 = sqrtf((from_pix.first - x0) * (from_pix.first - x0) + (from_pix.second - y0) * (from_pix.second - y0));
				float l2 = sqrtf((to_pix.first - x0) * (to_pix.first - x0) + (to_pix.second - y0) * (to_pix.second - y0));

				cutter.cut_pixel(workpiece.height_map, instruction_number, x0, y0, interpolate(from_h, to_h, l1, l2), workpiece.get_max_cutter_depth());
				//workpiece.height_map.set_pixel(x0, y0, interpolate(from_h, to_h, l1, l2));
				//plot(x0, y0);

				if (x0 == x1 && y0 == y1)
					break;
				auto e2 = 2 * error;
				if (e2 >= dy)
				{
					if (x0 == x1)
						break;
					error = error + dy;
					x0 = x0 + sx;
				}
				if (e2 <= dx)
				{
					if (y0 == y1)
						break;
					error = error + dx;
					y0 = y0 + sy;
				}
			}
			workpiece.invalidate();
		}

		// optimized by noting that it suffices to fully cut pixels on the beginning and the end and then draw lines with cutter profile
		void cut_line_optimized(const Vector3& from, const Vector3& to, const std::pair<int, int>& from_pix, const std::pair<int, int>& to_pix)
		{
			int x0 = from_pix.first, y0 = from_pix.second,
				x1 = to_pix.first, y1 = to_pix.second;

			cutter.cut_pixel(workpiece.height_map, instruction_number, x0, y0, from.z, workpiece.get_max_cutter_depth());
			ThickLineRasterizer(workpiece.height_map, cutter, from, to, instruction_number, workpiece.get_max_cutter_depth()).draw();
			cutter.cut_pixel(workpiece.height_map, instruction_number, x1, y1, to.z, workpiece.get_max_cutter_depth());

			workpiece.invalidate();
		}

		bool execute(const TaskParameters& parameters) override
		{
			percent += speed * parameters.delta_time;

			Vector3 current_pos = lerp(from, to, percent / path_length);
			workpiece.set_cutter_mesh_position({ current_pos.x, current_pos.z, current_pos.y });
			auto current = workpiece.height_map.position_to_pixel(current_pos);
			std::pair<int, int> current_pixel = { lroundf(current.x), lroundf(current.y) };

			// check if cutter goes straight down and cuts material with a tip (warning)
			// if (previous_pos.x == current_pos.x && previous_pos.y == current_pos.y && workpiece.height_map.get_pixel(current_pixel.first, current_pixel.second) > current_pos.z) -> float-wise comparison (should usually work, but may rejecting positives)
			if (previous_pixel == current_pixel && workpiece.height_map.get_pixel(current_pixel.first, current_pixel.second) > current_pos.z)
				Logger::log_warning("[WARNING] N%d at (%d,%d): Cutting workpiece with cutter's tip (cutter going straight down)\n", instruction_number, current_pixel.first, current_pixel.second);

			//cut_line_pure_bresenham(previous_pixel, current_pixel, previous_pos.z, current_pos.z);
			cut_line_optimized(previous_pos, current_pos, previous_pixel, current_pixel);

			previous_pixel = current_pixel;
			previous_pos = current_pos;

			return percent < path_length;
		}

		void execute_immediately(const TaskParameters& parameters) override
		{
			percent += path_length;
			execute(parameters);
		}
	};

	Task MillingProgram::get_task(Workpiece& workpiece, bool& task_ended) const
	{
		Task task(task_ended);
		for (const auto& move : moves)
		{
			task.add_step<MoveCutterTaskStep>(workpiece, move.instruction_number, *cutter, move.origin, move.destination, cutter_speed); // TODO fast moves
		}
		return task;
	}

	void MillingProgram::execute_on(Workpiece& workpiece) const
	{
		TaskParameters parameters;
		parameters.delta_time = 1.0f;
		for (const auto& move : moves)
		{
			MoveCutterTaskStep(workpiece, move.instruction_number, *cutter, move.origin, move.destination, INFINITY).execute(parameters);
		}
	}

	std::vector<Vector3> MillingProgram::get_cutter_positions() const
	{
		std::vector<Vector3> points;
		points.reserve(moves.size() + 1);
		points.push_back({ moves.front().origin.x, moves.front().origin.z + 0.01f, moves.front().origin.y });
		for (auto& move : moves)
			points.push_back({ move.destination.x, move.destination.z + 0.01f , move.destination.y });
		return points;
	}

	MillingProgram MillingProgram::read_from_file(const char* filename)
	{
		MillingProgram program(filename);
		program.set_cutter(create_cutter_from_file_extension(filename));

		std::ifstream s(filename);

		if (!s.good())
			throw std::runtime_error("Error opening file " + std::string(filename));

		while (!s.eof())
		{
			std::string line;
			s >> line;
			if (line.empty())
				continue;
			process_line(line, program);
		}

		s.close();

		program.moves.pop_front();

		return program;
	}

	void append_vector(std::ostream& s, const Vector3& vec)
	{
		s << std::setprecision(3)
			<< "X" << vec.x
			<< "Y" << vec.y
			<< "Z" << vec.z
			<< std::endl;
	}

	void MillingProgram::save_to_file(const char* filename)
	{
		int instruction_idx = 3;

		std::ofstream s(filename);

		if (!s.good())
			throw std::runtime_error("Error creating file " + std::string(filename));

		if (moves.empty())
		{
			s.close();
			return;
		}

		auto& first = moves.front();
		s << "N" << instruction_idx << "G" << (first.fast ? "00" : "01");
		append_vector(s, first.origin);
		instruction_idx++;

		for (auto& move : moves)
		{
			s << "N" << instruction_idx << "G" << (move.fast ? "00" : "01");
			append_vector(s, move.destination);
			instruction_idx++;
		}

		s.close();
	}
}

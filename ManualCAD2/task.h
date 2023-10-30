#pragma once

#include <queue>
#include <list>
#include <chrono>
#include <memory>
#include <utility>

namespace ManualCAD
{
	struct TaskParameters {
		float delta_time;
	};

	class SingleTaskStep
	{
	public:
		// Executes a single step (or part of it). Returns true if step should continue in next frame, false if ended.
		virtual bool execute(const TaskParameters& parameters) = 0;
		// Executes whole step immediately.
		virtual void execute_immediately(const TaskParameters& parameters) = 0;
	};	

	class Task
	{
		friend class TaskManager;

		std::list<std::unique_ptr<SingleTaskStep>> steps;
		std::chrono::time_point<std::chrono::high_resolution_clock> previous_time_point;
		bool stopped = false;
		bool paused = false;
		bool& task_ended;

		inline void execute_step() {
			if (paused) 
				return;
			if (ended())
			{
				task_ended = true;
				return;
			}

			auto& step = steps.front();

			auto time_point = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> time_delta = time_point - previous_time_point;
			bool should_continue = step->execute({ time_delta.count() });
			previous_time_point = std::chrono::high_resolution_clock::now();

			if (!should_continue)
			{
				steps.pop_front();
			}
		}
		
	public:
		Task(bool& task_ended) : task_ended(task_ended) { 
			task_ended = false; 
			previous_time_point = std::chrono::high_resolution_clock::now();
		}

		inline void execute_immediately() {
			while (!ended())
			{
				auto time_point = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> time_delta = time_point - previous_time_point;
				steps.front()->execute_immediately({ time_delta.count() });
				previous_time_point = std::chrono::high_resolution_clock::now();
				steps.pop_front();
			}
			task_ended = true;
		}

		template <class T, class... Args>
		inline void add_step(Args&&... args) { steps.push_back(std::make_unique<T>(std::forward<Args>(args)...)); }

		inline bool ended() { return stopped || steps.empty(); }

		inline void pause() {
			paused = true;
		}

		inline void resume() {
			if (!paused) return;
			paused = false;
			previous_time_point = std::chrono::high_resolution_clock::now();
		}

		inline void terminate() { stopped = true; task_ended = true; }
	};

	class TaskManager
	{
		friend class GlApplication;

		std::list<Task> tasks;
		inline void execute_tasks() {
			auto it = tasks.begin();

			while (it != tasks.end())
			{
				it->execute_step();
				if (it->ended())
				{
					it->task_ended = true;
					it = tasks.erase(it);
				}
				else
					++it;
			}
		}
		
	public:
		inline Task& add_task(Task&& task) { 
			tasks.push_back(std::move(task)); 
			return tasks.back();
		}
		
	};
}
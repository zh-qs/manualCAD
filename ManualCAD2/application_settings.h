#pragma once

#include <imgui.h>
#include <ctime>

namespace ManualCAD
{
	class ApplicationSettings {
		ApplicationSettings() = delete;
	public:
		static constexpr ImGuiMouseButton ROTATE_BUTTON = ImGuiMouseButton_Right;
		static constexpr float ROTATE_SPEED = 0.01f;

		static constexpr ImGuiMouseButton MOVE_BUTTON = ImGuiMouseButton_Middle;
		static constexpr float MOVE_SPEED = 5.0f;

		static constexpr ImGuiMouseButton SET_CURSOR_BUTTON = ImGuiMouseButton_Left;
		static constexpr ImGuiKey SET_CURSOR_KEY = ImGuiKey_LeftShift;

		static constexpr ImGuiMouseButton SELECT_OBJECT_BUTTON = ImGuiMouseButton_Left;
		static constexpr ImGuiKey ADD_TO_SELECTION_KEY = ImGuiKey_LeftCtrl;
		static constexpr ImGuiKey BIND_TO_SELECTED_KEY = ImGuiKey_LeftAlt;

		static constexpr ImGuiKey CONTROL_MODIFIER = ImGuiKey_LeftCtrl;
		static constexpr ImGuiKey SELECT_ALL_KEY = ImGuiKey_A;

		static constexpr float ZOOM_FACTOR = 1.3f;
		static constexpr float MAX_ZOOM_SCALE = 4e9;
		static constexpr float MIN_ZOOM_SCALE = 1 / MAX_ZOOM_SCALE;

		static constexpr int INITIAL_DOWNSAMPLING_SCALE = 32;

		static constexpr float RENDER_POINT_SIZE = 5.0f;

		static constexpr float SELECTION_THRESHOLD = 0.005f;

		enum class CameraZoomPolicy {
			MultiplyScale, MultiplyDirection
		};

		static constexpr CameraZoomPolicy ZOOM_POLICY = CameraZoomPolicy::MultiplyScale;

		static constexpr ImGuiMouseCursor SELECTION_HOVER_CURSOR = ImGuiMouseCursor_Hand;

		static constexpr ImGuiKey MOVE_ALONG_X_AXIS = ImGuiKey_X;
		static constexpr ImGuiKey MOVE_ALONG_Y_AXIS = ImGuiKey_Y;
		static constexpr ImGuiKey MOVE_ALONG_Z_AXIS = ImGuiKey_Z;

		static constexpr float SELECTED_THICKNESS = 2.0f;
		static constexpr float NORMAL_THICKNESS = 1.0f;

		static constexpr float INTERSECTION_STEP_DEFAULT_LENGTH = 0.01f;
		static constexpr size_t INTERSECTION_DEFAULT_MAX_STEPS = 1000;

		static constexpr int TRIM_TEXTURE_SIZE_PIXELS = 600;

		static constexpr bool BREAK_ON_TIMEOUT = true;
		static constexpr std::clock_t COMPUTATION_TIMEOUT = 3 * CLOCKS_PER_SEC;

		static constexpr bool DEBUG = true;
	};
}
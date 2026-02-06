#pragma once
#include <glm/glm.hpp>

namespace Cfg {

	// FPS
	inline constexpr double TARGET_FPS = 75.0;

	// Card orientation
	inline constexpr float CARD_FACEUP_X = 0.0f;
	inline constexpr float CARD_FACEDOWN_X = 3.1415926f;

	// Camera
	inline constexpr float MOUSE_SENS = 0.10f;
	inline constexpr float MOVE_SPEED = 2.5f;
	inline constexpr float CAM_FOV = 55.0f;

	// Table / layout
	inline constexpr float TABLE_TOP_Y = 0.06f;
	inline constexpr float CHIP_T = 0.05f;

	// Chip values
	inline constexpr int CHIP_VALUE[4] = { 10, 50, 100, 500 };

	// Hover radii (pixels)
	inline constexpr float HOVER_DECK_RADIUS_PX = 55.0f;
	inline constexpr float HOVER_CHIP_RADIUS_PX = 50.0f;

}
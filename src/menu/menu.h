#pragma once

namespace menu
{
	void Setup() noexcept;
	void Destroy() noexcept;
	void Render() noexcept;

	inline bool isOpen = false;

	// ESP Settings
	namespace esp
	{
		inline bool enabled = true;
		inline bool boxes = true;
		inline bool healthBar = true;
		inline bool names = false;
		inline bool distance = false;
		inline int boxThickness = 1;
		inline float boxColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // White
	}

	// Glow Settings
	namespace glow
	{
		inline bool enabled = true;
		inline float enemyColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red
		inline float teamColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // Blue
	}

	// Aimbot Settings
	namespace aimbot
	{
		inline bool enabled = true;
		inline float fov = 5.0f;
		inline float smoothness = 0.5f;
		inline bool visibleOnly = true;
		inline bool autoShoot = false;
	}

	// Misc Settings
	namespace misc
	{
		inline bool bunnyHop = true;
		inline bool noFlash = false;
		inline bool radarHack = false;
	}
}
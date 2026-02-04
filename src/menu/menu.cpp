#include "menu.h"

#include <d3d9.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "../core/globals.h"

static LPDIRECT3DDEVICE9 device = nullptr;
static bool imguiInitialized = false;

// Custom colors for the theme
namespace colors
{
	inline ImVec4 background = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	inline ImVec4 titleBg = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
	inline ImVec4 accent = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	inline ImVec4 accentHovered = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);
	inline ImVec4 accentActive = ImVec4(0.16f, 0.49f, 0.88f, 1.00f);
	inline ImVec4 text = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	inline ImVec4 textDisabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	inline ImVec4 border = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
}

void SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// Rounding
	style.WindowRounding = 8.0f;
	style.ChildRounding = 6.0f;
	style.FrameRounding = 6.0f;
	style.PopupRounding = 6.0f;
	style.ScrollbarRounding = 6.0f;
	style.GrabRounding = 6.0f;
	style.TabRounding = 6.0f;

	// Spacing
	style.WindowPadding = ImVec2(15, 15);
	style.FramePadding = ImVec2(10, 6);
	style.ItemSpacing = ImVec2(10, 8);
	style.ItemInnerSpacing = ImVec2(8, 6);
	style.IndentSpacing = 25.0f;
	style.ScrollbarSize = 15.0f;
	style.GrabMinSize = 12.0f;

	// Borders
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.TabBorderSize = 0.0f;

	// Colors
	ImVec4* colors_ptr = style.Colors;
	colors_ptr[ImGuiCol_Text] = colors::text;
	colors_ptr[ImGuiCol_TextDisabled] = colors::textDisabled;
	colors_ptr[ImGuiCol_WindowBg] = colors::background;
	colors_ptr[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
	colors_ptr[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 0.95f);
	colors_ptr[ImGuiCol_Border] = colors::border;
	colors_ptr[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors_ptr[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
	colors_ptr[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors_ptr[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
	colors_ptr[ImGuiCol_TitleBg] = colors::titleBg;
	colors_ptr[ImGuiCol_TitleBgActive] = colors::titleBg;
	colors_ptr[ImGuiCol_TitleBgCollapsed] = colors::titleBg;
	colors_ptr[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
	colors_ptr[ImGuiCol_ScrollbarBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
	colors_ptr[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
	colors_ptr[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
	colors_ptr[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
	colors_ptr[ImGuiCol_CheckMark] = colors::accent;
	colors_ptr[ImGuiCol_SliderGrab] = colors::accent;
	colors_ptr[ImGuiCol_SliderGrabActive] = colors::accentActive;
	colors_ptr[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors_ptr[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
	colors_ptr[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
	colors_ptr[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
	colors_ptr[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
	colors_ptr[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
	colors_ptr[ImGuiCol_Separator] = colors::border;
	colors_ptr[ImGuiCol_SeparatorHovered] = colors::accent;
	colors_ptr[ImGuiCol_SeparatorActive] = colors::accentActive;
	colors_ptr[ImGuiCol_ResizeGrip] = colors::accent;
	colors_ptr[ImGuiCol_ResizeGripHovered] = colors::accentHovered;
	colors_ptr[ImGuiCol_ResizeGripActive] = colors::accentActive;
	colors_ptr[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
	colors_ptr[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
	colors_ptr[ImGuiCol_TabActive] = colors::accent;
	colors_ptr[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
	colors_ptr[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
}

void menu::Setup() noexcept
{
	// ImGui will be initialized in the first Present hook call
	// when we have access to the D3D device
}

void menu::Destroy() noexcept
{
	if (imguiInitialized)
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		imguiInitialized = false;
	}
}

void menu::Render() noexcept
{
	if (!isOpen)
		return;

	// Setup custom style if not done yet
	static bool styleSetup = false;
	if (!styleSetup)
	{
		SetupImGuiStyle();
		styleSetup = true;
	}

	ImGui::SetNextWindowSize(ImVec2(750, 500), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(750, 500), ImVec2(900, 700));

	static int selectedTab = 0;

	if (ImGui::Begin("INSIGHT", &isOpen, ImGuiWindowFlags_NoCollapse))
	{
		// Sidebar
		ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.18f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.3f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 0.5f));

			ImGui::Text("AIMBOT");
			ImGui::Spacing();
			if (ImGui::Button("Ragebot", ImVec2(-1, 0))) selectedTab = 0;
			if (ImGui::Button("Anti Aim", ImVec2(-1, 0))) selectedTab = 1;
			if (ImGui::Button("Legitbot", ImVec2(-1, 0))) selectedTab = 2;

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("VISUALS");
			ImGui::Spacing();
			if (ImGui::Button("Players", ImVec2(-1, 0))) selectedTab = 3;
			if (ImGui::Button("Weapon", ImVec2(-1, 0))) selectedTab = 4;
			if (ImGui::Button("Grenades", ImVec2(-1, 0))) selectedTab = 5;
			if (ImGui::Button("World", ImVec2(-1, 0))) selectedTab = 6;

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("MISCELLANEOUS");
			ImGui::Spacing();
			if (ImGui::Button("Main", ImVec2(-1, 0))) selectedTab = 7;
			if (ImGui::Button("Inventory", ImVec2(-1, 0))) selectedTab = 8;
			if (ImGui::Button("Scripts", ImVec2(-1, 0))) selectedTab = 9;
			if (ImGui::Button("Configs", ImVec2(-1, 0))) selectedTab = 10;

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Main content area
		ImGui::BeginChild("Content", ImVec2(0, 0), true);
		{
			switch (selectedTab)
			{
			case 0: // Ragebot
				ImGui::Text("RAGEBOT SETTINGS");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("Enable Aimbot", &aimbot::enabled);
				ImGui::SliderFloat("FOV", &aimbot::fov, 1.0f, 20.0f, "%.1f");
				ImGui::SliderFloat("Smoothness", &aimbot::smoothness, 0.1f, 1.0f, "%.2f");
				ImGui::Checkbox("Visible Only", &aimbot::visibleOnly);
				ImGui::Checkbox("Auto Shoot", &aimbot::autoShoot);
				break;

			case 1: // Anti Aim
				ImGui::Text("ANTI AIM SETTINGS");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 2: // Legitbot
				ImGui::Text("LEGITBOT SETTINGS");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 3: // Players (ESP)
				ImGui::Text("PLAYER ESP");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginChild("Movement", ImVec2(ImGui::GetContentRegionAvail().x * 0.48f, 0), true);
				{
					ImGui::Text("Movement");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Checkbox("Enable ESP", &esp::enabled);
					ImGui::Checkbox("Boxes", &esp::boxes);
					ImGui::Checkbox("Health Bar", &esp::healthBar);
					ImGui::Checkbox("Names", &esp::names);
					ImGui::Checkbox("Distance", &esp::distance);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Text("Glow");
					ImGui::Checkbox("Enable Glow", &glow::enabled);
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild("Other", ImVec2(0, 0), true);
				{
					ImGui::Text("Other");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::SliderInt("Box Thickness", &esp::boxThickness, 1, 5);
					ImGui::ColorEdit4("Box Color", esp::boxColor, ImGuiColorEditFlags_NoInputs);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Text("Glow Colors");
					ImGui::ColorEdit4("Enemy Glow", glow::enemyColor, ImGuiColorEditFlags_NoInputs);
					ImGui::ColorEdit4("Team Glow", glow::teamColor, ImGuiColorEditFlags_NoInputs);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Text("Bind: List, Write");
				}
				ImGui::EndChild();
				break;

			case 4: // Weapon
				ImGui::Text("WEAPON ESP");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 5: // Grenades  
				ImGui::Text("GRENADE ESP");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 6: // World
				ImGui::Text("WORLD ESP");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 7: // Main (Misc)
				ImGui::Text("MISCELLANEOUS");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Checkbox("Bunny Hop", &misc::bunnyHop);
				ImGui::Checkbox("No Flash", &misc::noFlash);
				ImGui::Checkbox("Radar Hack", &misc::radarHack);

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
				break;

			case 8: // Inventory
				ImGui::Text("INVENTORY");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 9: // Scripts
				ImGui::Text("SCRIPTS");
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Coming soon...");
				break;

			case 10: // Configs
				ImGui::Text("CONFIGS");
				ImGui::Separator();
				ImGui::Spacing();

				if (ImGui::Button("Save Config", ImVec2(150, 30)))
				{
					// TODO: Implement config save
				}
				ImGui::SameLine();
				if (ImGui::Button("Load Config", ImVec2(150, 30)))
				{
					// TODO: Implement config load
				}

				ImGui::Spacing();
				ImGui::Text("Press INSERT to toggle menu");
				break;
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
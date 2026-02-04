#include "hooks.h"

// include minhook for epic hookage

#include "../../ext/minhook/minhook.h"
#include "../../ext/x86retspoof/x86RetSpoof.h"

#include <intrin.h>
#include <d3d9.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "../hacks/aimbot.h"
#include "../hacks/misc.h"
#include "../menu/menu.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// DirectX
static LPDIRECT3DDEVICE9 d3dDevice = nullptr;
static HWND window = nullptr;
static WNDPROC originalWndProc = nullptr;
static bool imguiInitialized = false;

// Hook function pointers
using EndSceneFn = long(__stdcall*)(LPDIRECT3DDEVICE9);
static EndSceneFn EndSceneOriginal = nullptr;

using ResetFn = long(__stdcall*)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
static ResetFn ResetOriginal = nullptr;

// WndProc hook
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Toggle menu with INSERT key
	if (msg == WM_KEYDOWN && wParam == VK_INSERT)
	{
		menu::isOpen = !menu::isOpen;
		return true; // Consume the input
	}

	// If menu is open, pass all input to ImGui first
	if (menu::isOpen)
	{
		// Let ImGui handle the input
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		// Block game input when menu is open
		// Allow only menu-related messages through
		switch (msg)
		{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			return true; // Consume input, don't pass to game
		}
	}

	// Call original WndProc
	return CallWindowProcA(originalWndProc, hWnd, msg, wParam, lParam);
}

// DirectX EndScene hook
long __stdcall EndScene(LPDIRECT3DDEVICE9 device) noexcept
{
	if (!imguiInitialized)
	{
		d3dDevice = device;

		// Get window handle
		D3DDEVICE_CREATION_PARAMETERS params;
		device->GetCreationParameters(&params);
		window = params.hFocusWindow;

		// Hook WndProc
		originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

		// Initialize ImGui
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		io.MouseDrawCursor = true; // ImGui draws its own cursor

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX9_Init(device);

		imguiInitialized = true;
	}

	// Start ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Control cursor visibility based on menu state
	ImGuiIO& io = ImGui::GetIO();
	if (menu::isOpen)
	{
		io.MouseDrawCursor = true;  // ImGui draws cursor
		// Hide game cursor when menu is open
		while (ShowCursor(TRUE) < 0);
	}
	else
	{
		io.MouseDrawCursor = false; // Don't draw ImGui cursor
		// Show game cursor when menu is closed
		while (ShowCursor(FALSE) >= 0);
	}

	// Render menu
	menu::Render();

	// End ImGui frame
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return EndSceneOriginal(device);
}

// DirectX Reset hook
long __stdcall Reset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params) noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = ResetOriginal(device, params);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}



void hooks::Setup() noexcept
{
	MH_Initialize();

	// Get D3D9 device for hooking
	LPDIRECT3D9 d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d9)
		return;

	D3DPRESENT_PARAMETERS params = {};
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = GetForegroundWindow();

	LPDIRECT3DDEVICE9 device = nullptr;
	HRESULT result = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, params.hDeviceWindow,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &device);

	if (FAILED(result))
	{
		d3d9->Release();
		return;
	}

	// Get VTable
	void** vTable = *reinterpret_cast<void***>(device);

	// EndScene hook (index 42)
	MH_CreateHook(vTable[42], &EndScene, reinterpret_cast<void**>(&EndSceneOriginal));

	// Reset hook (index 16)
	MH_CreateHook(vTable[16], &Reset, reinterpret_cast<void**>(&ResetOriginal));

	device->Release();
	d3d9->Release();

	// AllocKeyValuesMemory hook
	MH_CreateHook(
		memory::Get(interfaces::keyValuesSystem, 1),
		&AllocKeyValuesMemory,
		reinterpret_cast<void**>(&AllocKeyValuesMemoryOriginal)
	);
	// CreateMove hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 24),
		&CreateMove,
		reinterpret_cast<void**>(&CreateMoveOriginal)
	);

	// PaintTraverse hook
	MH_CreateHook(
		memory::Get(interfaces::panel, 41),
		&PaintTraverse,
		reinterpret_cast<void**>(&PaintTraverseOriginal)
	);

	// DoPostScreenSpaceEffects hook (for glow)
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 44),
		&DoPostScreenSpaceEffects,
		reinterpret_cast<void**>(&DoPostScreenSpaceEffectsOriginal)
	);

	MH_EnableHook(MH_ALL_HOOKS);

	// Initialize menu
	menu::Setup();
}

void hooks::Destroy() noexcept
{
	// Cleanup menu
	menu::Destroy();

	// Restore WndProc
	if (window && originalWndProc)
		SetWindowLongPtrA(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWndProc));

	// Cleanup ImGui
	if (imguiInitialized)
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

void* __stdcall hooks::AllocKeyValuesMemory(const std::int32_t size) noexcept
{
	// if function is returning to speficied addresses, return nullptr to "bypass"
	if (const std::uint32_t address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesEngine) ||
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesClient))
		return nullptr;

	// return original
	return AllocKeyValuesMemoryOriginal(interfaces::keyValuesSystem, size);
}

bool __stdcall hooks::CreateMove(float frameTime, CUserCmd* cmd) noexcept
{
	static const auto sequence = reinterpret_cast<std::uintptr_t>(memory::PatternScan("client.dll", "FF 23"));
	const auto result = x86RetSpoof::invokeStdcall<bool>((uintptr_t)hooks::CreateMoveOriginal, sequence, frameTime, cmd);

	// make sure this function is being called from CInput::CreateMove
	if (!cmd || !cmd->commandNumber)
		return result;

	// Block input when menu is open
	if (menu::isOpen)
	{
		// Clear all inputs - just return false to block movement
		cmd->buttons = 0;
		return false;
	}

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd))
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	// get our local player here
	globals::UpdateLocalPlayer();

	if (globals::localPlayer && globals::localPlayer->IsAlive())
	{
		// example bhop
		hacks::RunBunnyHop(cmd);

		//run aimbot
		hacks::RunAimbot(cmd);
	}
	return false;
}
// esp hook
void __stdcall hooks::PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept
{
	//make sure we have the right panel
	if (vguiPanel == interfaces::engineVGui->GetPanel(PANEL_TOOLS))
	{
		// Only draw ESP if enabled in menu
		if (menu::esp::enabled && globals::localPlayer && interfaces::engine->IsInGame())
		{
			// loop through the player list
			for (int i = 1; i <= interfaces::globals->maxClients; ++i)
			{
				//get the player pointer
				CEntity* player = interfaces::entityList->GetEntityFromIndex(i);
				if (!player)
					continue;

				// make sure player is not dormant && is alive
				if (player->IsDormant() || !player->IsAlive())
					continue;

				// no ESP on teammates
				if (player->GetTeam() == globals::localPlayer->GetTeam())
					continue;

				// don't do ESP on player we are spectating
				if (!globals::localPlayer->IsAlive())
					if (globals::localPlayer->GetObserverTarget() == player)
						continue;

				// player's bone matrix
				CMatrix3x4 bones[128];
				if (!player->SetupBones(bones, 128, 0x7FF00, interfaces::globals->currentTime))
					continue;

				// Get head position (bone 8 is head)
				CVector headPos = bones[8].Origin();
				headPos.z += 11.f; // offset for top of head

				// Get feet position
				CVector feetPos = player->GetAbsOrigin();
				feetPos.z -= 9.f; // offset for bottom of feet

				// Convert world positions to screen
				CVector top, bottom;
				interfaces::debugOverlay->ScreenPosition(headPos, top);
				interfaces::debugOverlay->ScreenPosition(feetPos, bottom);

				// Calculate box dimensions
				const float h = bottom.y - top.y;
				const float w = h * 0.35f;

				// Only validate that dimensions are somewhat reasonable (not behind player)
				if (h < 1.f || h > 1000.f)
					continue;

				const int left = static_cast<int>(top.x - w);
				const int right = static_cast<int>(top.x + w);
				const int topY = static_cast<int>(top.y);
				const int bottomY = static_cast<int>(bottom.y);

				// Draw boxes if enabled
				if (menu::esp::boxes)
				{
					// Get box color from menu settings
					int r = static_cast<int>(menu::esp::boxColor[0] * 255);
					int g = static_cast<int>(menu::esp::boxColor[1] * 255);
					int b = static_cast<int>(menu::esp::boxColor[2] * 255);
					int a = static_cast<int>(menu::esp::boxColor[3] * 255);

					// set drawing color from menu
					interfaces::surface->DrawSetColor(r, g, b, a);

					// draw normal box
					interfaces::surface->DrawOutlinedRect(left, topY, right, bottomY);

					// set the color to black for outlines
					interfaces::surface->DrawSetColor(0, 0, 0, 255);

					// draw outlines
					interfaces::surface->DrawOutlinedRect(left - 1, topY - 1, right + 1, bottomY + 1);
					interfaces::surface->DrawOutlinedRect(left + 1, topY + 1, right - 1, bottomY - 1);
				}

				// Draw health bar if enabled
				if (menu::esp::healthBar)
				{
					// health bar outline (drawing color is already black)
					interfaces::surface->DrawSetColor(0, 0, 0, 255);
					interfaces::surface->DrawOutlinedRect(left - 6, topY - 1, left - 3, bottomY + 1);

					// health is an int from 0 -> 100, get percentage
					const float healthFrac = player->GetHealth() * 0.01f;
					// set the health bar color to a split between red / green
					interfaces::surface->DrawSetColor(static_cast<int>((1.f - healthFrac) * 255), static_cast<int>(255 * healthFrac), 0, 255);

					// draw the health bar
					const int healthBarBottom = static_cast<int>(bottomY - (h * healthFrac));
					interfaces::surface->DrawFilledRect(left - 5, healthBarBottom, left - 4, bottomY);
				}
			}
		}
	}
	//call the original function
	PaintTraverseOriginal(interfaces::panel, vguiPanel, forceRepaint, allowForce);
}

void __stdcall hooks::DoPostScreenSpaceEffects(const void* viewSetup) noexcept
{
	// Only run if glow is enabled in menu
	if (menu::glow::enabled && globals::localPlayer && interfaces::engine->IsInGame())
	{
		// loop through the glow objects
		for (int i = 0; i < interfaces::glow->glowObjects.size; ++i)
		{
			// get the glow object
			IGlowManager::CGlowObject& glowObject = interfaces::glow->glowObjects[i];
			// make sure it is used
			if (glowObject.IsUnused())
				continue;
			// make sure we have a valid entity
			if (!glowObject.entity)
				continue;
			// check the class index of the entity
			switch (glowObject.entity->GetClientClass()->classID)
			{
				// entity is a player
			case CClientClass::CCSPlayer:
				// skip if they are NOT alive
				if (!glowObject.entity->IsAlive())
					continue;

				// enemies
				if (glowObject.entity->GetTeam() != globals::localPlayer->GetTeam())
				{
					// Use enemy color from menu
					glowObject.SetColor(
						menu::glow::enemyColor[0],
						menu::glow::enemyColor[1],
						menu::glow::enemyColor[2]
					);
				}
				// teammates
				else
				{
					// Use team color from menu
					glowObject.SetColor(
						menu::glow::teamColor[0],
						menu::glow::teamColor[1],
						menu::glow::teamColor[2]
					);
				}
				break;

			default:
				break;
			}
		}
	}
	// call the original function
	DoPostScreenSpaceEffectsOriginal(interfaces::clientMode, viewSetup);
}
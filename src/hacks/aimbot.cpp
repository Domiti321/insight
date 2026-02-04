#include "aimbot.h"

#include "../core/globals.h"
#include "../core/interfaces.h"
#include "../menu/menu.h"



void hacks::RunAimbot(CUserCmd* cmd) noexcept
{
	// Check if aimbot is enabled in menu
	if (!menu::aimbot::enabled)
		return;

	//check if we are trying to shoot
	if (!(cmd->buttons & CUserCmd::IN_ATTACK))
		return;

	if (globals::localPlayer->IsDefusing())
		return;

	CEntity* activeWeapon = globals::localPlayer->GetActiveWeapon();
	if (!activeWeapon)
		return;

	const int weaponType = activeWeapon->GetWeaponType();

	switch (weaponType)
	{
	case CEntity::WEAPONTYPE_MACHINEGUN:
	case CEntity::WEAPONTYPE_RIFLE:
	case CEntity::WEAPONTYPE_SHOTGUN:
	case CEntity::WEAPONTYPE_SNIPER:
	case CEntity::WEAPONTYPE_PISTOL:
	case CEntity::WEAPONTYPE_SUBMACHINEGUN:

	{

		if (!activeWeapon->GetClip())

			return;
		if (weaponType == CEntity::WEAPONTYPE_SNIPER)
		{
			if (!globals::localPlayer->IsScoped())
				return;
		}
		break;

	}
	default:
		return;

	}

	CVector bestAngle{ };
	float bestFov{ menu::aimbot::fov }; // Use FOV from menu

	for (int i = 1; i <= interfaces::globals->maxClients; ++i)
	{
		CEntity* player = interfaces::entityList->GetEntityFromIndex(i);

		if (!player)
			continue;

		if (player->IsDormant() || !player->IsAlive())
			continue;

		if (player->GetTeam() == globals::localPlayer->GetTeam())
			continue;

		if (player->HasGunGameImmunity())
			continue;

		CMatrix3x4 bones[128];
		if (!player->SetupBones(bones, 128, 256, interfaces::globals->currentTime))
			continue;

		CVector localEyePosition;
		globals::localPlayer->GetEyePosition(localEyePosition);

		CVector aimPunch{ };

		switch (weaponType)
		{
		case CEntity::WEAPONTYPE_RIFLE:
		case CEntity::WEAPONTYPE_SUBMACHINEGUN:
		case CEntity::WEAPONTYPE_MACHINEGUN:
			globals::localPlayer->GetAimPunch(aimPunch);
		}

		// Visibility check if enabled in menu
		if (menu::aimbot::visibleOnly)
		{
			CTrace trace;
			interfaces::engineTrace->TraceRay(
				CRay{ localEyePosition, bones[8].Origin() },
				MASK_SHOT,
				{ globals::localPlayer },
				trace
			);

			if (!trace.entity || trace.fraction < 0.97f)
				continue;
		}

		CVector enemyAngle
		{
		(bones[8].Origin() - localEyePosition).ToAngle() - (cmd->viewAngles + aimPunch)
		};

		if (const float fov = std::hypot(enemyAngle.x, enemyAngle.y); fov < bestFov)
		{
			bestFov = fov;
			bestAngle = enemyAngle;
		}
	}

	// Apply smoothness from menu settings
	cmd->viewAngles = cmd->viewAngles + bestAngle.Scale(menu::aimbot::smoothness);
}
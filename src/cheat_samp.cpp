/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastH1ckNet/mod_s0b1it_sa/

	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "main.h"
int			g_iSpectateEnabled = 0, g_iSpectateLock = 0, g_iSpectatePlayerID = -1;
int			g_iJoiningServer = 0;
int			iClickWarpEnabled = 0;
int			g_iNumPlayersMuted = 0;
bool		g_bPlayerMuted[SAMP_MAX_PLAYERS];
bool IsPlayerStreamed(uint16_t playerID)
{
	if (g_Players == NULL)
		return false;
	if (g_Players->iIsListed[playerID] != 1)
		return false;
	if (g_Players->pRemotePlayer[playerID] == NULL)
		return false;
	if (g_Players->pRemotePlayer[playerID]->pPlayerData == NULL)
		return false;
	if (g_Players->pRemotePlayer[playerID]->pPlayerData->pSAMP_Actor == NULL)
		return false;
	return true;
}
void CalcScreenCoorsz(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen)
{
	traceLastFunc("CalcScreenCoors()");

	/** C++-ifyed function 0x71DA00, formerly called by CHudSA::CalcScreenCoors **/
	// Get the static view matrix as D3DXMATRIX
	D3DXMATRIX	m((float*)(0xB6FA2C));

	// Get the static virtual screen (x,y)-sizes
	DWORD* dwLenX = (DWORD*)(0xC17044);
	DWORD* dwLenY = (DWORD*)(0xC17048);

	//DWORD *dwLenZ = (DWORD*)(0xC1704C);
	//double aspectRatio = (*dwLenX) / (*dwLenY);
	// Do a transformation
	vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
	vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
	vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

	// Get the correct screen coordinates
	double	fRecip = (double)1.0 / vecScreen->z;	//(vecScreen->z - (*dwLenZ));
	vecScreen->x *= (float)(fRecip * (*dwLenX));
	vecScreen->y *= (float)(fRecip * (*dwLenY));
}
D3DCOLOR RandomColorz()
{
	int one = rand() % 256;
	int two = rand() % 256;
	int three = rand() % 256;

	D3DCOLOR color = D3DCOLOR_ARGB(255, one, two, three);

	return color;
}
float fWeaponRange[39] = {
	100.0, // 0 - Fist
	100.0, // 1 - Brass knuckles
	100.0, // 2 - Golf club
	100.0, // 3 - Nitestick
	100.0, // 4 - Knife
	100.0, // 5 - Bat
	100.0, // 6 - Shovel
	100.0, // 7 - Pool cue
	100.0, // 8 - Katana
	100.0, // 9 - Chainsaw
	100.0, // 10 - Dildo
	100.0, // 11 - Dildo 2
	100.0, // 12 - Vibrator
	100.0, // 13 - Vibrator 2
	100.0, // 14 - Flowers
	100.0, // 15 - Cane
	100.0, // 16 - Grenade
	100.0, // 17 - Teargas
	100.0, // 18 - Molotov
	90.0, // 19 - Vehicle M4 (custom)
	75.0, // 20 - Vehicle minigun (custom)
	100.0, // 21
	35.0, // 22 - Colt 45
	35.0, // 23 - Silenced
	35.0, // 24 - Deagle
	40.0, // 25 - Shotgun
	35.0, // 26 - Sawed-off
	40.0, // 27 - Spas
	35.0, // 28 - UZI
	1000.0, // 29 - MP5
	1000.0, // 30 - AK47
	90.0, // 31 - M4
	35.0, // 32 - Tec9
	100.0, // 33 - Cuntgun
	320.0, // 34 - Sniper
	100.0, // 35 - Rocket launcher
	100.0, // 36 - Heatseeker
	100.0, // 37 - Flamethrower
	75.0  // 38 - Minigun
};
int GetAimingPlayer(bool bFromWhenPress)
{
	int NearestCenterPlayerID = -1;
	float fNearestDistanceScreen = -1;
	float DistPlayerScreen[2];
	if ((!g_Players->pLocalPlayer->onFootData.stSampKeys.keys_primaryFire && bFromWhenPress)
		|| cheat_state->state != CHEAT_STATE_ACTOR
		|| cheat_state->killark.aimedplayer)
	{
		for (int i = 0; i < SAMP_MAX_PLAYERS; i++)
		{
			if (i == g_Players->sLocalPlayerID)
				continue;

			actor_info* player = getGTAPedFromSAMPPlayerID(i);
			if (!player)
				continue;

			CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
			if (!pPed)
				continue;

			CVector pSpinePos;
			D3DXVECTOR3 player_pos, player_pos_on_screen;
			pPed->GetBonePosition(BONE_SPINE1, &pSpinePos);

			player_pos.x = CVecToD3DXVEC(pSpinePos).x;
			player_pos.y = CVecToD3DXVEC(pSpinePos).y;
			player_pos.z = CVecToD3DXVEC(pSpinePos).z;
			CalcScreenCoorsz(&player_pos, &player_pos_on_screen);

			if (player_pos_on_screen.z < 1.f)
				continue;


			DistPlayerScreen[0] = abs((cheat_state->killark.viewfinder[0]) - (player_pos_on_screen.x));
			DistPlayerScreen[1] = abs((cheat_state->killark.viewfinder[1]) - (player_pos_on_screen.y));

			float fCurrentDistanceScreen = sqrtf((DistPlayerScreen[0] * DistPlayerScreen[0]) + (DistPlayerScreen[1] * DistPlayerScreen[1]));

			float fDistanceFromEnemy = vect3_dist(&g_Players->pRemotePlayer[i]->pPlayerData->pSAMP_Actor->pGTA_Ped->base.matrix[12],
				&g_Players->pLocalPlayer->pSAMP_Actor->pGTA_Ped->base.matrix[12]);

			if (fDistanceFromEnemy > fWeaponRange[g_Players->pLocalPlayer->byteCurrentWeapon])
				continue;

			if (fNearestDistanceScreen == -1)
			{
				fNearestDistanceScreen = fCurrentDistanceScreen;
			}


			// get the camera
			CCamera* pCamera = pGame->GetCamera();

			// grab the active cam
			CCamSAInterface* pCam = (CCamSAInterface*)((CCamSA*)pCamera->GetCam(pCamera->GetActiveCam()))->GetInterface();
			CVector src = pCam->Source;

			bool IsPlayerVisible = pGame->GetWorld()->IsLineOfSightClear(&src, &pSpinePos, true, false, false, true, true, false, false);
			if (cheat_state->killark.haha_friends[i])
				continue;
			if (!IsPlayerStreamed(i))
				continue;
			if (g_Players->pRemotePlayer[i]->pPlayerData->iAFKState == 2)
				continue;
			if (fCurrentDistanceScreen <= fNearestDistanceScreen)
			{
				fNearestDistanceScreen = fCurrentDistanceScreen;
				if (cheat_state->killark.silentaim)
				{
					if (!(g_Players->pRemotePlayer[i]->pPlayerData->pSAMP_Actor->pGTA_Ped->hitpoints == 0))
					{
						NearestCenterPlayerID = i;
						cheat_state->killark.aimplayerid[0] = player_pos_on_screen.x;
						cheat_state->killark.aimplayerid[1] = player_pos_on_screen.y;
					}
					else
					{
						continue;
					}
				}
				else
				{
					NearestCenterPlayerID = i;
					cheat_state->killark.aimplayerid[0] = player_pos_on_screen.x;
					cheat_state->killark.aimplayerid[1] = player_pos_on_screen.y;
				}
			}
		
		}
	}
	else
	{
		if (cheat_state->killark.aimedplayer != -1)
		{
			if (IsPlayerStreamed(cheat_state->killark.aimedplayer))
			{
				actor_info* player = getGTAPedFromSAMPPlayerID(cheat_state->killark.aimedplayer);
				if (!player)
					return -1;

				CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
				if (!pPed)
					return -1;

				//CVector pos = *pPed->GetPosition();
				CVector pSpinePos;
				D3DXVECTOR3 player_pos, player_pos_on_screen;
				pPed->GetBonePosition(BONE_SPINE1, &pSpinePos);
				player_pos.x = CVecToD3DXVEC(pSpinePos).x;
				player_pos.y = CVecToD3DXVEC(pSpinePos).y;
				player_pos.z = CVecToD3DXVEC(pSpinePos).z;
				CalcScreenCoorsz(&player_pos, &player_pos_on_screen);

				if (player_pos_on_screen.z < 1.f)
					return -1;
				float fDistanceFromEnemy = vect3_dist(&g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->base.matrix[12],
					&g_Players->pLocalPlayer->pSAMP_Actor->pGTA_Ped->base.matrix[12]);
				/*if (cheat_state->Silah.iSAim_Alive)
				{
					if (ACTOR_IS_DEAD(player) || g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->hitpoints == 0)
						return -1;
				}*/
				if (fDistanceFromEnemy > fWeaponRange[g_Players->pLocalPlayer->byteCurrentWeapon])
					return -1;
				cheat_state->killark.aimplayerid[0] = player_pos_on_screen.x;
				cheat_state->killark.aimplayerid[1] = player_pos_on_screen.y;
			}
		}
	}
	if (cheat_state->killark.silentaim)
	{
		if (cheat_state->killark.aimedplayer != -1)
		{
			if (IsPlayerStreamed(cheat_state->killark.aimedplayer))
			{
				actor_info* player = getGTAPedFromSAMPPlayerID(cheat_state->killark.aimedplayer);
				if (!player)
					return -1;
				CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
				if (!pPed)
					return -1;
				CVector pSpinePos;
				D3DXVECTOR3 player_pos, player_pos_on_screen;
				pPed->GetBonePosition(BONE_SPINE1, &pSpinePos);
				player_pos.x = CVecToD3DXVEC(pSpinePos).x;
				player_pos.y = CVecToD3DXVEC(pSpinePos).y;
				player_pos.z = CVecToD3DXVEC(pSpinePos).z;
				CalcScreenCoorsz(&player_pos, &player_pos_on_screen);
				float fTargetPos[3];
				vect3_copy(&g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->base.matrix[12], fTargetPos);
				if (g_Players->pLocalPlayer->byteCurrentWeapon != 34)
				{
					cheat_state->killark.viewfinder[0] = (float)pPresentParam.BackBufferWidth * 0.5299999714f;
					cheat_state->killark.viewfinder[1] = (float)pPresentParam.BackBufferHeight * 0.4f;
				}
				else
				{
					cheat_state->killark.viewfinder[0] = (float)pPresentParam.BackBufferWidth / 2.f;
					cheat_state->killark.viewfinder[1] = (float)pPresentParam.BackBufferHeight / 2.f;
				}			
			}
		}
	}
	if (cheat_state->killark.box)
	{
		if (cheat_state->killark.aimedplayer != -1)
		{
			actor_info* player = getGTAPedFromSAMPPlayerID(cheat_state->killark.aimedplayer);
			if (!player)
				return -1;
			CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
			if (!pPed)
				return -1;
			float
				max_up_val = 0,
				max_down_val = 0,
				max_left_val = 0,
				max_right_val = 0;
			bool invalid = false;
			for (int bone_id = BONE_PELVIS1; bone_id <= BONE_RIGHTFOOT; ++bone_id)
			{
				if (bone_id >= BONE_PELVIS1 && bone_id <= BONE_HEAD ||
					bone_id >= BONE_RIGHTUPPERTORSO && bone_id <= BONE_RIGHTTHUMB ||
					bone_id >= BONE_LEFTUPPERTORSO && bone_id <= BONE_LEFTTHUMB ||
					bone_id >= BONE_LEFTHIP && bone_id <= BONE_LEFTFOOT ||
					bone_id >= BONE_RIGHTHIP && bone_id <= BONE_RIGHTFOOT)
				{
					CVector bone;
					pPed->GetBonePosition((eBone)bone_id, &bone);

					D3DXVECTOR3 bone_pos;

					bone_pos.x = bone.fX;
					bone_pos.y = bone.fY;
					bone_pos.z = bone.fZ;
					D3DXVECTOR3 bone_screen_pos;
					CalcScreenCoorsz(&bone_pos, &bone_screen_pos);
					// check if the iter is culled or not
					if (bone_screen_pos.z < 1.f)
					{
						invalid = true;
						break;
					}


					if (!max_up_val && !max_down_val &&
						!max_left_val && !max_right_val)
					{
						max_up_val = bone_screen_pos.y;
						max_down_val = bone_screen_pos.y;
						max_right_val = bone_screen_pos.x;
						max_left_val = bone_screen_pos.x;
					}


					if (bone_screen_pos.y > max_up_val)
						max_up_val = bone_screen_pos.y;

					if (bone_screen_pos.y < max_down_val)
						max_down_val = bone_screen_pos.y;

					if (bone_screen_pos.x > max_right_val)
						max_right_val = bone_screen_pos.x;

					if (bone_screen_pos.x < max_left_val)
						max_left_val = bone_screen_pos.x;

				}
			}
			float height_box = max_up_val - max_down_val;
			float widht_box = max_right_val - max_left_val + 12.0f;
			D3DCOLOR redcolor = D3DCOLOR_XRGB(255, 0, 0);
			CVector mySpinePos, TargetSpinePos;
			D3DCOLOR color;
			color = D3DCOLOR_XRGB(255, 0, 0);
			if (cheat_state->killark.trace)
			{
				pPedSelf->GetBonePosition(BONE_PELVIS1, &mySpinePos);
				pPed->GetBonePosition(BONE_PELVIS1, &TargetSpinePos);
				render->DrawLine(CVecToD3DXVEC(mySpinePos), CVecToD3DXVEC(TargetSpinePos), color);
			}

			render->D3DBoxBorder(max_left_val,
				max_down_val,
				widht_box,
				height_box, redcolor, 255);
		}

	}

	return NearestCenterPlayerID;
}
void sampMainCheat(void)
{
	traceLastFunc("sampMainCheat()");

	// g_Vehicles & g_Players pointers need to be refreshed or nulled
	if (isBadPtr_writeAny(g_SAMP->pPools, sizeof(stSAMPPools)))
	{
		g_Vehicles = NULL;
		g_Players = NULL;
	}
	else if (g_Vehicles != g_SAMP->pPools->pVehicle || g_Players != g_SAMP->pPools->pPlayer)
	{
		if (isBadPtr_writeAny(g_SAMP->pPools->pVehicle, sizeof(stVehiclePool)))
			g_Vehicles = NULL;
		else
			g_Vehicles = g_SAMP->pPools->pVehicle;
		if (isBadPtr_writeAny(g_SAMP->pPools->pPlayer, sizeof(stPlayerPool)))
			g_Players = NULL;
		else
			g_Players = g_SAMP->pPools->pPlayer;
	}

	// update GTA to SAMP translation structures
	update_translateGTASAMP_vehiclePool();
	update_translateGTASAMP_pedPool();

	spectateHandle();
	sampAntiHijack();

	// start chatbox logging
	if (set.chatbox_logging)
	{
		static int	chatbox_init;
		if (!chatbox_init)
		{
			SYSTEMTIME	time;
			GetLocalTime(&time);
			LogChatbox(false, "Session started at %02d/%02d/%02d", time.wDay, time.wMonth, time.wYear);
			chatbox_init = 1;
		}
	}

	if (KEYCOMBO_PRESSED(set.key_player_info_list))
		cheat_state->player_info_list ^= 1;

	if (KEYCOMBO_PRESSED(set.key_rejoin))
	{
		restartGame();
		disconnect(500);
		cheat_state_text("Rejoining in %d seconds...", set.rejoin_delay / 1000);
		cheat_state->_generic.rejoinTick = GetTickCount();
	}

	if (KEYCOMBO_PRESSED(set.key_respawn))
		playerSpawn();

	if (KEYCOMBO_DOWN(set.chat_secondary_key))
	{
		int			i, key, spam;
		const char* msg;
		for (i = 0; i < INI_CHATMSGS_MAX; i++)
		{
			struct chat_msg* msg_item = &set.chat[i];
			if (msg_item->key == NULL)
				continue;
			if (msg_item->msg == NULL)
				continue;
			if (msg_item->key != key_being_pressed)
				continue;
			key = msg_item->key;
			msg = msg_item->msg;
			spam = msg_item->spam;
			if (spam)
			{
				if (msg)
					if (KEY_DOWN(key))
						say("%s", msg);
			}
			else
			{
				if (msg)
					if (KEY_PRESSED(key))
						say("%s", msg);
			}
		}
	}
	if (set.clickwarp_enabled && iIsSAMPSupported)
	{
		if (KEYCOMBO_PRESSED(set.key_clickwarp_enable))
		{
			g_iCursorEnabled ^= 1;
			toggleSAMPCursor(g_iCursorEnabled);
		}
		if (g_iCursorEnabled && KEYCOMBO_PRESSED(set.key_clickwarp_click))
		{
			iClickWarpEnabled = 1;
		}
	}
	if (cheat_state->killark.silentaim)
	{
		cheat_state->killark.aimedplayer = GetAimingPlayer(true);
	}
	static int	iSAMPHooksInstalled;
	if (!iSAMPHooksInstalled)
	{
		installSAMPHooks();
		iSAMPHooksInstalled = 1;
	}

	if (cheat_state->_generic.rejoinTick && cheat_state->_generic.rejoinTick < (GetTickCount() - set.rejoin_delay))
	{
		g_SAMP->iGameState = GAMESTATE_WAIT_CONNECT;
		cheat_state->_generic.rejoinTick = 0;
	}

	if (g_iJoiningServer == 1)
	{
		restartGame();
		disconnect(500);
		cheat_state_text("Joining server in %d seconds...", set.rejoin_delay / 1000);
		cheat_state->_generic.join_serverTick = GetTickCount();
		g_iJoiningServer = 2;
	}

	if (g_iJoiningServer == 2
		&& cheat_state->_generic.join_serverTick
		&& cheat_state->_generic.join_serverTick < (GetTickCount() - set.rejoin_delay))
	{
		g_SAMP->iGameState = GAMESTATE_WAIT_CONNECT;
		g_iJoiningServer = 0;
		cheat_state->_generic.join_serverTick = 0;
	}
}

void spectateHandle(void)
{
	if (g_iSpectateEnabled)
	{
		if (g_iSpectateLock) return;

		if (g_iSpectatePlayerID != -1)
		{
			if (g_Players->iIsListed[g_iSpectatePlayerID] != 0)
			{
				if (g_Players->pRemotePlayer[g_iSpectatePlayerID] != NULL)
				{
					int iState = getPlayerState(g_iSpectatePlayerID);

					if (iState == PLAYER_STATE_ONFOOT)
					{
						struct actor_info* pPlayer = getGTAPedFromSAMPPlayerID(g_iSpectatePlayerID);
						if (pPlayer == NULL) return;
						GTAfunc_CameraOnActor(pPlayer);
						g_iSpectateLock = 1;
					}
					else if (iState == PLAYER_STATE_DRIVER)
					{
						struct vehicle_info* pPlayerVehicleID = g_Players->pRemotePlayer[g_iSpectatePlayerID]->pPlayerData->pSAMP_Vehicle->pGTA_Vehicle;
						if (pPlayerVehicleID == NULL) return;
						GTAfunc_CameraOnVehicle(pPlayerVehicleID);
						g_iSpectateLock = 1;
					}
					else if (iState == PLAYER_STATE_PASSENGER)
					{
						struct vehicle_info* pPlayerVehicleID = g_Players->pRemotePlayer[g_iSpectatePlayerID]->pPlayerData->pSAMP_Vehicle->pGTA_Vehicle;
						if (pPlayerVehicleID == NULL) return;
						GTAfunc_CameraOnVehicle(pPlayerVehicleID);
						g_iSpectateLock = 1;
					}
				}
				else
				{
					cheat_state_text("Player is not streamed in");
					g_iSpectateEnabled = 0;
				}
			}
		}
	}
}

void spectatePlayer(int iPlayerID)
{
	if (iPlayerID == -1)
	{
		GTAfunc_TogglePlayerControllable(0);
		GTAfunc_LockActor(0);
		pGameInterface->GetCamera()->RestoreWithJumpCut();

		g_iSpectateEnabled = 0;
		g_iSpectateLock = 0;
		g_iSpectatePlayerID = -1;
		return;
	}

	g_iSpectatePlayerID = iPlayerID;
	g_iSpectateLock = 0;
	g_iSpectateEnabled = 1;
}

void sampAntiHijack(void)
{
	if (g_SAMP == NULL) return;
	traceLastFunc("sampAntiHijack()");

	vehicle_info* veh = vehicle_info_get(VEHICLE_SELF, VEHICLE_ALIVE);
	if (set.anti_carjacking && veh == NULL)
	{
		if (cheat_state->_generic.got_vehicle_id)
			cheat_state->_generic.got_vehicle_id = false;
		if (cheat_state->_generic.anti_carjackTick
			&& cheat_state->_generic.anti_carjackTick < (GetTickCount() - 500)
			&& cheat_state->_generic.car_jacked)
		{
			if (cheat_state->_generic.car_jacked_last_vehicle_id == 0)
			{
				GTAfunc_showStyledText("~r~Unable To Unjack~w~!", 1000, 5);
				cheat_state->_generic.anti_carjackTick = 0;
				cheat_state->_generic.car_jacked = false;
				return;
			}

			cheat_state->_generic.anti_carjackTick = 0;
			cheat_state->_generic.car_jacked = false;
			GTAfunc_PutActorInCar(GetVehicleByGtaId(cheat_state->_generic.car_jacked_last_vehicle_id));

			struct vehicle_info* veh = GetVehicleByGtaId(cheat_state->_generic.car_jacked_last_vehicle_id);
			//if ( veh != NULL )
			//	vect3_copy( cheat_state->_generic.car_jacked_lastPos, &veh->base.matrix[4 * 3] );
			GTAfunc_showStyledText("~r~Car Unjacked~w~!", 1000, 5);
			return;
		}
	}
	else if (set.anti_carjacking)
	{
		if (veh->passengers[0] == actor_info_get(ACTOR_SELF, 0))
		{
			if (!cheat_state->_generic.got_vehicle_id)
			{
				cheat_state->_generic.car_jacked_last_vehicle_id = getPlayerVehicleGTAScriptingID(ACTOR_SELF);
				if (cheat_state->_generic.car_jacked_last_vehicle_id > 0)
					cheat_state->_generic.got_vehicle_id = true;
			}
		}
	}
}

void HandleRPCPacketFunc(unsigned char id, RPCParameters* rpcParams, void(*callback) (RPCParameters*))
{
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[id][INCOMING_RPC] != nullptr && set.netPatchAssoc[id][INCOMING_RPC]->enabled)
			return;
	}
	if (rpcParams != nullptr && rpcParams->numberOfBitsOfData >= 8)
	{
		switch (id)
		{
		case RPC_SetPlayerHealth:
		{
			if (isCheatPanicEnabled() || !set.enable_extra_godmade || !cheat_state->_generic.hp_cheat) break;

			actor_info* self = actor_info_get(ACTOR_SELF, NULL);
			if (self)
			{
				BitStream bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
				float fHealth;
				bsData.Read(fHealth);
				if (fHealth < self->hitpoints)
				{
					cheat_state_text("Warning: Server tried change your health to %0.1f", fHealth);
					return; // exit from the function without processing RPC
				}
			}
			break;
		}
		case RPC_SetVehicleHealth:
		{
			if (isCheatPanicEnabled() || !set.enable_extra_godmade || !cheat_state->_generic.hp_cheat) break;

			vehicle_info* vself = vehicle_info_get(VEHICLE_SELF, NULL);
			if (vself)
			{
				BitStream bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
				short sId;
				float fHealth;
				bsData.Read(sId);
				bsData.Read(fHealth);
				if (sId == g_Players->pLocalPlayer->sCurrentVehicleID && fHealth < vself->hitpoints)
				{
					cheat_state_text("Warning: Server tried change your vehicle health to %0.1f", fHealth);
					return; // exit from the function without processing RPC
				}
			}
			break;
		}
		case RPC_ClientMessage:
		{
			if (isCheatPanicEnabled() || !set.anti_spam && !set.chatbox_logging) break;

			BitStream		bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
			uint32_t		dwStrLen, dwColor;
			char			szMsg[256];
			static char		last_servermsg[256];
			static DWORD	allow_show_again = 0;

			if (cheat_state->_generic.cheat_panic_enabled)
				break;

			bsData.Read(dwColor);
			bsData.Read(dwStrLen);
			if (dwStrLen >= sizeof(szMsg)) dwStrLen = sizeof(szMsg) - 1;
			bsData.Read(szMsg, dwStrLen);
			szMsg[dwStrLen] = '\0';

			if (set.anti_spam && (strcmp(last_servermsg, szMsg) == 0 && GetTickCount() < allow_show_again))
				return; // exit without processing rpc

			// might be a personal message by muted player - look for name in server message
			// ignore message, if name was found
			if (set.anti_spam && g_iNumPlayersMuted > 0)
			{
				int i, j;
				char* playerName = NULL;
				for (i = 0, j = 0; i < SAMP_MAX_PLAYERS && j < g_iNumPlayersMuted; i++)
				{
					if (g_bPlayerMuted[i])
					{
						playerName = (char*)getPlayerName(i);

						if (playerName == NULL)
						{
							// Player not connected anymore - remove player from muted list
							g_bPlayerMuted[i] = false;
							g_iNumPlayersMuted--;
							continue;
						}
						else if (strstr(szMsg, playerName) != NULL)
						{
							return;
						}
						j++;
					}
				}
			}
			if (set.chatbox_logging)
				LogChatbox(false, "%s", szMsg);
			strncpy_s(last_servermsg, szMsg, sizeof(last_servermsg) - 1);
			allow_show_again = GetTickCount() + 5000;
			break;
		}
		case RPC_Chat:
		{
			if (isCheatPanicEnabled() || !set.anti_spam && !set.chatbox_logging) break;

			BitStream		bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
			static char		last_clientmsg[SAMP_MAX_PLAYERS][256];
			static DWORD	allow_show_again = 0;
			uint16_t		playerId = uint16_t(-1);
			uint8_t			byteTextLen;
			char			szText[256];

			if (cheat_state->_generic.cheat_panic_enabled)
				break;

			bsData.Read(playerId);
			if (isBadSAMPPlayerID(playerId))
				break;

			bsData.Read(byteTextLen);
			bsData.Read(szText, byteTextLen);
			szText[byteTextLen] = '\0';

			if (set.anti_spam && ((strcmp(last_clientmsg[playerId], szText) == 0 && GetTickCount() < allow_show_again) || (g_iNumPlayersMuted > 0 && g_bPlayerMuted[playerId])))
				return; // exit without processing rpc

			// nothing to copy anymore, after chatbox_logging, so copy this before
			strncpy_s(last_clientmsg[playerId], szText, sizeof(last_clientmsg[playerId]) - 1);

			if (set.chatbox_logging)
				LogChatbox(false, "%s: %s", getPlayerName(playerId), szText);
			allow_show_again = GetTickCount() + 5000;
			break;
		}
		case RPC_SetPlayerAttachedObject:
		{
			BitStream bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);

			unsigned short usPlayerID;
			int iIndex, iModelID;
			bool bUnk;

			bsData.Read(usPlayerID);
			bsData.Read(iIndex);
			bsData.Read(bUnk);
			bsData.Read(iModelID);
			g_iLastAttachedObjectIndex = iIndex;
		}
		} // switch
	}
	callback(rpcParams);
}

bool OnSendRPC(int uniqueID, BitStream* parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[uniqueID][OUTCOMING_RPC] != nullptr && set.netPatchAssoc[uniqueID][OUTCOMING_RPC]->enabled)
			return false;
	}
	if (uniqueID == RPC_Chat && g_Players != nullptr)
	{
		uint8_t byteTextLen;
		char szText[256];

		parameters->Read(byteTextLen);
		parameters->Read(szText, byteTextLen);
		szText[byteTextLen] = '\0';

		if (set.chatbox_logging)
			LogChatbox(false, "%s: %s", getPlayerName(g_Players->sLocalPlayerID), szText);
	}

	// prevent invulnerability detection
	if (uniqueID == RPC_ClientCheck && cheat_state && cheat_state->_generic.hp_cheat)
	{
		uint8_t type = 0;
		parameters->Read(type);
		if (type == 2)
		{
			uint32_t arg = 0;
			uint8_t response = 0;
			parameters->Read(arg);
			parameters->Read(response);

			// remove invulnerability flags from our real flags
			uint32_t fakeFlags = arg & (0xFF00FFFF | ((~ACTOR_FLAGS_INVULNERABLE) << 16));

			// reform packet data
			parameters->SetWriteOffset(0);
			parameters->Write(type);
			parameters->Write(fakeFlags);
			parameters->Write(response);
		}
	}
	return true;
}

bool OnSendPacket(BitStream* parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	if (!isCheatPanicEnabled())
	{
		uint8_t packetId;
		parameters->Read(packetId);
		if (set.netPatchAssoc[packetId][OUTCOMING_PACKET] != nullptr && set.netPatchAssoc[packetId][OUTCOMING_PACKET]->enabled)
			return false;
	}
	return true;
}

bool OnReceivePacket(Packet* p)
{
	if (p->data == nullptr || p->length == 0)
		return true;
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[p->data[0]][INCOMING_PACKET] != nullptr && set.netPatchAssoc[p->data[0]][INCOMING_PACKET]->enabled)
			return false;
	}
	if (p->data[0] == ID_MARKERS_SYNC) // packetId
	{
		BitStream	bs(p->data, p->length, false);
		int			iNumberOfPlayers = 0;
		uint16_t	playerID = uint16_t(-1);
		short		sPos[3] = { 0, 0, 0 };
		bool		bIsPlayerActive = false;

		bs.IgnoreBits(8);
		bs.Read(iNumberOfPlayers);
		if (iNumberOfPlayers < 0 || iNumberOfPlayers > SAMP_MAX_PLAYERS)
			return true;

		for (int i = 0; i < iNumberOfPlayers; i++)
		{
			bs.Read(playerID);
			bs.ReadCompressed(bIsPlayerActive);
			if (bIsPlayerActive == 0 || isBadSAMPPlayerID(playerID))
				continue;
			bs.Read(sPos);
			g_stStreamedOutInfo.iPlayerID[playerID] = playerID;
			g_stStreamedOutInfo.fPlayerPos[playerID][0] = sPos[0];
			g_stStreamedOutInfo.fPlayerPos[playerID][1] = sPos[1];
			g_stStreamedOutInfo.fPlayerPos[playerID][2] = sPos[2];
		}
	}
	return true;
}

// commands below

void cmd_change_server(char* param)	// 127.0.0.1 7777 Username Password
{
	traceLastFunc("cmd_change_server()");

	bool	success = false;

	char	IP[128], Nick[SAMP_MAX_PLAYER_NAME], Password[128] = "", Port[128];
	int		iPort;

	int ipc = sscanf(param, "%s%s%s%s", IP, Port, Nick, Password);
	if (ipc < 2)
	{
		addMessageToChatWindow("USAGE: /mod_change_server <ip> <port> <Username> <Server Password>");
		addMessageToChatWindow("Variables that are set to \"NULL\" (capitalized) will be ignored.");
		addMessageToChatWindow("If you set the Password to \"NULL\" it is set to <no server password>.");
		addMessageToChatWindow("Username and password can also be left out completely.");
		return;
	}
	if (stricmp(IP, "NULL") == NULL)
		strcpy(IP, g_SAMP->szIP);

	if (stricmp(Port, "NULL") == NULL)
		iPort = g_SAMP->ulPort;
	else
		iPort = atoi(Port);

	if (ipc > 2)
	{
		if (stricmp(Nick, "NULL") != NULL)
		{
			if (strlen(Nick) > SAMP_ALLOWED_PLAYER_NAME_LENGTH)
				Nick[SAMP_ALLOWED_PLAYER_NAME_LENGTH] = '\0';
			setLocalPlayerName(Nick);
		}
	}
	if (ipc > 3)
	{
		if (stricmp(Password, "NULL") == NULL)
			strcpy(Password, "");
	}

	changeServer(IP, iPort, Password);
}

void cmd_change_server_fav(char* param)
{
	traceLastFunc("cmd_change_server_fav()");

	if (strlen(param) == 0)
	{
		addMessageToChatWindow("/mod_fav_server <server name/part of server name>");
		addMessageToChatWindow("In order to see the favorite server list type: /mod_fav_server list");
		return;
	}

	if (strncmp(param, "list", 4) == 0)
	{
		int count = 0;
		for (int i = 0; i < INI_SERVERS_MAX; i++)
		{
			if (set.server[i].server_name == NULL)
				continue;

			count++;
			addMessageToChatWindow("%s", set.server[i].server_name);
		}
		if (count == 0)
			addMessageToChatWindow("No servers in favorite server list. Edit the ini file to add some.");
		return;
	}

	for (int i = 0; i < INI_SERVERS_MAX; i++)
	{
		if (set.server[i].server_name == NULL || set.server[i].ip == NULL
			|| strlen(set.server[i].ip) < 7 || set.server[i].port == 0)
			continue;

		if (!findstrinstr((char*)set.server[i].server_name, param))
			continue;

		if (!set.use_current_namq)
			setLocalPlayerName(set.server[i].nickname);

		changeServer(set.server[i].ip, set.server[i].port, set.server[i].password);

		return;
	}

	addMessageToChatWindow("/mod_fav_server <server name/part of server name>");
	return;
}

void cmd_current_server(char* param)
{
	addMessageToChatWindow("Server Name: %s", g_SAMP->szHostname);
	addMessageToChatWindow("Server Address: %s:%i", g_SAMP->szIP, g_SAMP->ulPort);
	addMessageToChatWindow("Username: %s", getPlayerName(g_Players->sLocalPlayerID));
}

void cmd_tele_loc(char* param)
{
	if (strlen(param) == 0)
	{
		addMessageToChatWindow("USAGE: /mod_tele_loc <location name>");
		addMessageToChatWindow("Use /mod_tele_locations to show the location names.");
		addMessageToChatWindow("The more specific you are on location name the better the result.");
		return;
	}

	for (int i = 0; i < STATIC_TELEPORT_MAX; i++)
	{
		if (strlen(set.static_teleport_name[i]) == 0 || vect3_near_zero(set.static_teleport[i].pos))
			continue;

		if (!findstrinstr(set.static_teleport_name[i], param))
			continue;

		cheat_state_text("Teleported to: %s.", set.static_teleport_name[i]);
		cheat_teleport(set.static_teleport[i].pos, set.static_teleport[i].interior_id);
		return;
	}

	addMessageToChatWindow("USAGE: /mod_tele_loc <location name>");
	addMessageToChatWindow("Use /mod_tele_locations to show the location names.");
	addMessageToChatWindow("The more specific you are on location name the better the result.");
}

void cmd_tele_locations(char*)
{
	for (int i = 0; i < STATIC_TELEPORT_MAX; i++)
	{
		if (strlen(set.static_teleport_name[i]) == 0 || vect3_near_zero(set.static_teleport[i].pos))
			continue;
		addMessageToChatWindow("%s", set.static_teleport_name[i]);
	}

	addMessageToChatWindow("To teleport use the menu or: /mod_tele_loc <location name>");
}

void cmd_pickup(char* params)
{
	if (!strlen(params))
	{
		addMessageToChatWindow("USAGE: /mod_pickup <pickup id>");
		return;
	}

	g_RakClient->SendPickUp(atoi(params));
}

void cmd_setclass(char* params)
{
	if (!strlen(params))
	{
		addMessageToChatWindow("USAGE: /mod_setclass <class id>");
		return;
	}

	g_RakClient->RequestClass(atoi(params));
	g_RakClient->SendSpawn();
}

void cmd_fakekill(char* params)
{
	int killer, reason, amount;
	if (!strlen(params) || sscanf(params, "%d%d%d", &killer, &reason, &amount) < 3)
		return addMessageToChatWindow("USAGE: /mod_fakekill <killer id> <reason> <amount>");

	if (amount < 1 || killer < 0 || killer > SAMP_MAX_PLAYERS)
		return;

	for (int i = 0; i < amount; i++)
		g_RakClient->SendDeath(killer, reason);
}

void cmd_warp(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.goto <player id>");

	int playerid = atoi(params);
	if (isBadSAMPPlayerID(playerid) || g_Players->iIsListed[playerid] != 1)
		return addMessageToChatWindow("Player does not exist.");

	float pos[3];
	actor_info* actor = nullptr;
	if (g_Players->pRemotePlayer[playerid]->pPlayerData == nullptr
		|| g_Players->pRemotePlayer[playerid]->pPlayerData->pSAMP_Actor == nullptr)
	{
		if (vect3_near_zero(g_stStreamedOutInfo.fPlayerPos[playerid]))
			return addMessageToChatWindow("Player is not streamed in.");

		vect3_copy(g_stStreamedOutInfo.fPlayerPos[playerid], pos);
		pos[1] += 1.0f;
		cheat_teleport(pos, 0);
		return;
	}

	if (!getPlayerPos(playerid, pos) ||
		g_Players->pRemotePlayer[playerid]->pPlayerData == nullptr ||
		g_Players->pRemotePlayer[playerid]->pPlayerData->pSAMP_Actor == nullptr ||
		(actor = g_Players->pRemotePlayer[playerid]->pPlayerData->pSAMP_Actor->pGTA_Ped) == nullptr)
	{
		return addMessageToChatWindow("Bad player info.");
	}

	pos[1] += 1.0f;
	cheat_teleport(pos, actor->base.interior_id);
}

void cmd_showCMDS(char*)
{
	for (int i = 0; i < g_m0dCmdNum; i++)
	{
		addMessageToChatWindow("%s", g_m0dCmdlist[i]);
	}
}
void CalcScreenCoorss(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen)
{
	traceLastFunc("CalcScreenCoors()");

	/** C++-ifyed function 0x71DA00, formerly called by CHudSA::CalcScreenCoors **/
	// Get the static view matrix as D3DXMATRIX
	D3DXMATRIX	m((float*)(0xB6FA2C));

	// Get the static virtual screen (x,y)-sizes
	DWORD* dwLenX = (DWORD*)(0xC17044);
	DWORD* dwLenY = (DWORD*)(0xC17048);

	//DWORD *dwLenZ = (DWORD*)(0xC1704C);
	//double aspectRatio = (*dwLenX) / (*dwLenY);
	// Do a transformation
	vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
	vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
	vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

	// Get the correct screen coordinates
	double	fRecip = (double)1.0 / vecScreen->z;	//(vecScreen->z - (*dwLenZ));
	vecScreen->x *= (float)(fRecip * (*dwLenX));
	vecScreen->y *= (float)(fRecip * (*dwLenY));
}
float vect2_dist(float* point1, float* point2)
{
	return sqrt(pow(point1[0] - point2[0], 2) + pow(point1[1] - point2[1], 2));
}
actor_info* GetAimingTarget(int flugs)
{
	actor_info* min_actor = 0;
	double minDist = 0.0f;
	const double radius_max = pPresentParam.BackBufferHeight / 2; // pixels


	actor_info* my_actor = actor_info_get(ACTOR_SELF, NULL);

	for (int id = NULL; id < SAMP_MAX_PLAYERS; id++)
	{
		actor_info* actor = getGTAPedFromSAMPPlayerID(id);
		if (!actor)
			continue;

		if ((flugs & ACTOR_ALIVE) && actor->hitpoints <= 0.0f)
			continue;

		D3DXVECTOR3 vect_pos_on_screen;
		D3DXVECTOR3 vect_actor_pos(actor->base.matrix + 12);
		float center_of_aim[2];

		if (my_actor->weapon_model != 358)
		{
			center_of_aim[0] = (float)pPresentParam.BackBufferWidth * 0.5299999714f,
				center_of_aim[1] = (float)pPresentParam.BackBufferHeight * 0.4f;
		}
		else
		{
			center_of_aim[0] = (float)pPresentParam.BackBufferWidth * 0.5f;
			center_of_aim[1] = (float)pPresentParam.BackBufferHeight * 0.5f;
		}


		CalcScreenCoorss(&vect_actor_pos, &vect_pos_on_screen); // get player screen position


		if (vect_pos_on_screen.z < -1) // not on screen
			continue;

		double dist_from_center = vect2_dist(vect_pos_on_screen, center_of_aim); // distance from player pos and center of aim


		if (dist_from_center >= radius_max) // so much distance
			continue;

		if (!min_actor || (min_actor && (minDist > dist_from_center)))
		{
			min_actor = actor;
			minDist = dist_from_center;
		}

	}

	return min_actor;
}
int BulletDataHook(stBulletData* data)
{

	if (cheat_state->killark.silentaim)
	{
		actor_info* founded_actor = GetAimingTarget(ACTOR_ALIVE);

		if (founded_actor)
		{
			vect3_copy(founded_actor->base.matrix + 12, data->fTarget);

			data->sTargetID = getSAMPPlayerIDFromGTAPed(founded_actor);

			data->byteType = 1;

			data->fCenter[0] = 0.01;
			data->fCenter[1] = 0.02;
			data->fCenter[2] = -0.04f;
		}
	}
	return 1;
}
void cmd_sa(char* params)
{
	if (!cheat_state->killark.silentaim)
	{
		addMessageToChatWindow("Slient Aim: On");
		cheat_state->killark.silentaim = true;
	}
	else
	{
		addMessageToChatWindow("Slient Aim: Off");
		cheat_state->killark.silentaim = false;
	}
}

void cmd_wallz(char* params)
{
	if (!cheat_state->killark.wallz)
	{
		addMessageToChatWindow("Wall Shot: On");
		cheat_state->killark.wallz = true;
	}
	else
	{
		addMessageToChatWindow("Wall Shot: Off");
		cheat_state->killark.wallz = false;
	}
}
void cmd_box(char* params)
{
	if (!cheat_state->killark.box)
	{
		addMessageToChatWindow("Box: On");
		cheat_state->killark.box = true;
	}
	else
	{
		addMessageToChatWindow("Box: Off");
		cheat_state->killark.box = false;
	}
}
void cmd_nr(char* params)
{
	if (!cheat_state->killark.noreload)
	{
		addMessageToChatWindow("No Reload: On");
		cheat_state->killark.noreload = true;
	}
	else
	{
		addMessageToChatWindow("No Reload: Off");
		cheat_state->killark.noreload = false;
	}
}
void cmd_fk(char* params)
{
	if (!cheat_state->killark.fullkafa)
	{
		addMessageToChatWindow("Only Head: On");
		cheat_state->killark.fullkafa = true;
	}
	else
	{
		addMessageToChatWindow("Only Head: Off");
		cheat_state->killark.fullkafa = false;
	}
}
void cmd_inv(char* params)
{
	if (!cheat_state->killark.invisible)
	{
		addMessageToChatWindow("Invisible: On");
		cheat_state->killark.invisible = true;
	}
	else
	{
		addMessageToChatWindow("Invisible: Off");
		cheat_state->killark.invisible = false;
	}
}
void cmd_fakebullet(char* params)
{
	if (!cheat_state->killark.fakebullet)
	{
		addMessageToChatWindow("Fakebullet: On");
		cheat_state->killark.fakebullet = true;
	}
	else
	{
		addMessageToChatWindow("Fakebullet: Off");
		cheat_state->killark.fakebullet = false;
	}
}
void cmd_inv2(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.inv2 <ID>");
	int id = atoi(params);
	if (!cheat_state->killark.invisible2)
	{
		addMessageToChatWindow("Invisible: On");
		cheat_state->killark.invisible2 = true;
		cheat_state->killark.invisibletarget = id;
	}
	else
	{
		addMessageToChatWindow("Invisible: Off");
		cheat_state->killark.invisible2 = false;
	}
}
void cmd_anim(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.anim <value>");

	int value = atoi(params);
	if (cheat_state->killark.animation)
	{
		cheat_state->killark.animation = false;
		cheat_state->killark.anim = 0;
		addMessageToChatWindow("Animation: Off");
	}
	else
	{
		cheat_state->killark.animation = true;
		cheat_state->killark.anim = value;
		addMessageToChatWindow("Animation: On");
	}
}
void cmd_silahbypass(char* params)
{
	int iSilahID, iMermi;
	if (g_Players->pLocalPlayer == NULL)
		return;
	loadAllWeaponModels();
	if (!strlen(params) || sscanf(params, "%d%d", &iSilahID, &iMermi) < 2)
		return addMessageToChatWindow("Örnek Kullanım: /.silah <Silah ID> <Mermi Değeri>");
	if (iSilahID < 0 || iSilahID > 46) {

		return addMessageToChatWindow("Hatalı silah ID'si.");
	}
	else if (iMermi <= 0) {
		return addMessageToChatWindow("Silah mermisi 0'dan yüksek olmak zorunda.");
	}
	else {
		addMessageToChatWindow("Silah verildi.");
	}
	cheat_state->killark.silahbypass = 1;
	pPedSelf->GiveWeapon((eWeaponType)iSilahID, iMermi);
	g_RakClient->GivePlayerWeapon(g_Players->sLocalPlayerID, iSilahID, iMermi);
}
void cmd_gs(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.gs <value>");

	int value = atoi(params);
	cheat_state->game_speed += (float)value * 0.05f;
}
void cmd_trace(char* params)
{
	if (!cheat_state->killark.trace)
	{
		addMessageToChatWindow("Trace: On");
		cheat_state->killark.trace = true;
	}
	else
	{
		addMessageToChatWindow("Trace: Off");
		cheat_state->killark.trace = false;
	}
}
void cmd_friend(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.friend <ID>");
	int id = atoi(params);
	cheat_state->killark.haha_friends[id] = true;
}

void cmd_dm(char* params)
{
	if (cheat_state->killark.ragedm) {
		addMessageToChatWindow("{FF0000}Rage DM Mode: OFF");
		cheat_state->killark.ragedm = false;
		cheat_state->killark.silentaim = false;
		cheat_state->killark.box = false;
		cheat_state->killark.trace = true;
		cheat_state->killark.noreload = false;
		cheat_state->game_speed = 1.0;
	}
	else {
		addMessageToChatWindow("Rage DM Mode: ON");
		cheat_state->killark.ragedm = true;
		cheat_state->killark.silentaim = true;
		cheat_state->killark.box = true;
		cheat_state->killark.trace = true;
		cheat_state->killark.noreload = true;
		cheat_state->game_speed = 1.6;
	}
}
void cmd_pdm(char* params)
{
	if (cheat_state->killark.ldm) {
		addMessageToChatWindow("PD DM Mode: OFF");
		cheat_state->killark.ldm = false;

		cheat_state->killark.silentaim = false;
		cheat_state->game_speed = 1.0;
	}
	else {
		addMessageToChatWindow("PD DM Mode: ON");
		cheat_state->killark.ldm = true;
		cheat_state->killark.silentaim = true;
		cheat_state->game_speed = 1.1;
	}
}
void cmd_delfriend(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.dfriend <ID>");
	int id = atoi(params);
	cheat_state->killark.haha_friends[id] = false;
}
void cmd_dcar(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.dcar <ID>");
	int id = atoi(params);
	if (cheat_state->killark.dcar)
	{
		cheat_state->killark.dcar = false;
		cheat_state->killark.dcartarget = id;
	}
	else
	{
		cheat_state->killark.dcar = true;
		cheat_state->killark.dcartarget = id;
	}
}
void cmd_kick(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.kick <ID>");
	int id = atoi(params);
	if (cheat_state->killark.kicker)
	{
		cheat_state->killark.kicker = false;
		addMessageToChatWindow("Kicker: Off");
	}
	else
	{
		cheat_state->killark.kicker = true;
		cheat_state->killark.kickid = id;
		addMessageToChatWindow("Kicker: On");
	}
}
void cmd_afk(char* params)
{

	if (!cheat_state->killark.afk)
	{
		addMessageToChatWindow("{00FF00}AFK: ON");
		cheat_state->killark.afk = true;

	}
	else
	{
		addMessageToChatWindow("{FF0000}AFK: OFF");
		cheat_state->killark.afk = false;
	}
}
void cmd_rkprotect(char* params)
{
	if (!cheat_state->killark.rkprotect)
	{
		addMessageToChatWindow("RK Protect: On");
		cheat_state->killark.rkprotect = true;
	}
	else
	{
		addMessageToChatWindow("RK Protect: Off");
		cheat_state->killark.rkprotect = false;
		//cheat_state->killark.rktracer = false;
	}
}
void cmd_acs(char* params)
{
	if (!cheat_state->killark.antics)
	{
		addMessageToChatWindow("Anti Car Surfing: On");
		cheat_state->killark.antics = true;
	}
	else
	{
		addMessageToChatWindow("Anti Car Surfing: Off");
		cheat_state->killark.antics = false;
	}
}
void cmd_slap(char* params)
{
	if (!cheat_state->killark.slapper)
	{
		addMessageToChatWindow("Slapper: On");
		cheat_state->killark.slapper = true;
		//cheat_state->killark.slaptarget = id;
	}
	else
	{
		addMessageToChatWindow("Slapper: Off");
		cheat_state->killark.slapper = false;
	}
}
void cmd_lag(char* params)
{
	if (!cheat_state->killark.lag)
	{
		addMessageToChatWindow("Lagger: On");
		cheat_state->killark.lag = true;
		//cheat_state->killark.slaptarget = id;
	}
	else
	{
		addMessageToChatWindow("Lagger: Off");
		cheat_state->killark.lag = false;
	}
}

void cmd_aracsilme(char* params)
{

	char szVehicleID[128];

	int ipc = sscanf(params, "%s", szVehicleID);
	if (ipc < 1)
	{
		addMessageToChatWindow("/.dcar <aracid>");
		return;
	}
	float fPos[3];
	fPos[0] = 2.0;
	fPos[1] = 2.0;
	fPos[2] = 2.0;
	int iVehicleID = atoi(szVehicleID);
	struct vehicle_info* vehicle = getGTAVehicleFromSAMPVehicleID(iVehicleID);
	BitStream bs;
	bs.Write(iVehicleID);
	bs.Write(1);
	g_RakClient->RPC(RPC_EnterVehicle, &bs);
	Sleep(750);
	g_RakClient->SendFakeDriverSyncData(getSAMPVehicleIDFromGTAVehicle(vehicle), fPos, 0.0, vehicle->speed, NULL);
}
void sendDialogResponse(WORD wDialogID, BYTE bButtonID, WORD wListBoxItem, char* szInputResp)
{
	BYTE respLen = (BYTE)strlen(szInputResp);
	BitStream bsSend;
	bsSend.Write(wDialogID);
	bsSend.Write(bButtonID);
	bsSend.Write(wListBoxItem);
	bsSend.Write(respLen);
	bsSend.Write(szInputResp, respLen);
	g_RakClient->RPC(RPC_DialogResponse, &bsSend);
}

void cmd_dialogresponse(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /.dialogresponse <ID>");
	int id = atoi(params);

	sendDialogResponse(id, 1, 1, "");

	addMessageToChatWindow("Dialog response sent.");
}
void cmd_crash(char* params)
{
	attachedObjectCrasher();
}
void initChatCmds(void)
{
	if (g_m0dCommands == true)
		return;

	cheat_state_text("initiated modcommands");
	g_m0dCommands = true;

	addClientCommand(".cmds", cmd_showCMDS);
	addClientCommand("mod_change_server", cmd_change_server);
	addClientCommand("mod_fav_server", cmd_change_server_fav);
	addClientCommand("mod_current_server", cmd_current_server);
	addClientCommand("mod_tele_loc", cmd_tele_loc);
	addClientCommand("mod_teleport_location", cmd_tele_loc);
	addClientCommand("mod_tele_locations", cmd_tele_locations);
	addClientCommand("mod_teleport_locations", cmd_tele_locations);
	addClientCommand("mod_pickup", cmd_pickup);
	addClientCommand("mod_setclass", cmd_setclass);
	addClientCommand("mod_fakekill", cmd_fakekill);
	addClientCommand(".goto", cmd_warp);
	addClientCommand(".sa", cmd_sa);
	addClientCommand(".box", cmd_box);
	addClientCommand(".nr", cmd_nr);
	addClientCommand(".fk", cmd_fk);
	addClientCommand(".inv", cmd_inv);
	addClientCommand(".gs", cmd_gs);
	addClientCommand(".trace", cmd_trace);
	addClientCommand(".d", cmd_friend);
	addClientCommand(".wshot", cmd_wallz);
	addClientCommand(".df", cmd_delfriend);
	addClientCommand(".fakebullet", cmd_fakebullet);
	addClientCommand(".rkprotect", cmd_rkprotect);
	addClientCommand(".dialogresponse", cmd_dialogresponse);
	addClientCommand(".wep", cmd_silahbypass);
	addClientCommand(".acs", cmd_acs);
	addClientCommand(".dm", cmd_dm);
	addClientCommand(".pdm", cmd_pdm);
	addClientCommand(".afk", cmd_afk);
	addClientCommand(".slap", cmd_slap);
	addClientCommand(".dcar", cmd_aracsilme);
	addClientCommand(".lag", cmd_lag);
	addClientCommand(".inv2", cmd_inv2);
	addClientCommand(".anim", cmd_anim);
	addClientCommand(".kick", cmd_kick);
	addClientCommand(".crasher", cmd_crash);
}

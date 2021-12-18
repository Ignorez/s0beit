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

RakClient *g_RakClient = NULL;

RakClient::RakClient( void *pRakClientInterface )
{
	pRakClient = (RakClientInterface *)pRakClientInterface;
}

bool RakClient::RPC( int rpcId, BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp )
{
	if ( !pRakClient )
		return false;

	return pRakClient->RPC( &rpcId, bitStream, priority, reliability, orderingChannel, shiftTimestamp );
}

bool RakClient::Send( BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	if ( !pRakClient )
		return false;

	return pRakClient->Send( bitStream, priority, reliability, orderingChannel );
}

// misc functions
void RakClient::SendDeath( uint16_t killerId, uint8_t reason )
{
	BitStream bsDeath;

	bsDeath.Write( reason );
	bsDeath.Write( killerId );
	g_RakClient->RPC( RPC_Death, &bsDeath );
}

void RakClient::SendPickUp( int pickupId )
{
	BitStream bsPickup;

	bsPickup.Write( pickupId );
	
	RPC( RPC_PickedUpPickup, &bsPickup );
}

void RakClient::RequestClass( int classId )
{
	BitStream bsClass;

	bsClass.Write( classId );
	g_RakClient->RPC( RPC_RequestClass, &bsClass );
}
void RakClient::SendGiveDamage(uint16_t uiPlayerID, float fDamageAmountToSend, int iWeaponID, int iBodyPart)
{
	BitStream bsSendDamage; // Bunlar heppp rapidin baþýnýn altýndan çýkýyo heeppp - rapidin notlarý xd | 06.1.2021 | 02:10
	bsSendDamage.Write(false); // I will "send" the damage... not "take"... I will create a different function to "take" damage...
	bsSendDamage.Write(uiPlayerID); // Send the damage, but send the damage to who? -> Player!
	bsSendDamage.Write(fDamageAmountToSend); // Send the damage to player, but how much?!?! -> fDamageAmount!
	bsSendDamage.Write(iWeaponID);
	bsSendDamage.Write(iBodyPart);
	g_RakClient->RPC(RPC_GiveTakeDamage, &bsSendDamage, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, FALSE);
}

void RakClient::SendSCMEvent( int vehicleID, int eventId, int param1, int param2 )
{
	BitStream bsScmEvent;

	bsScmEvent.Write ( vehicleID );
	bsScmEvent.Write ( param1 );
	bsScmEvent.Write ( param2 );
	bsScmEvent.Write ( eventId );

	RPC( RPC_SCMEvent, &bsScmEvent );
}
void RakClient::SendFakeDriverSyncData(WORD vehicleid, float fPos[3], float HealthCar, float Speed[3], WORD key, WORD trailer)
{
	struct actor_info* self = actor_info_get(ACTOR_SELF, ACTOR_ALIVE);
	stInCarData InCarData;
	ZeroMemory(&InCarData, sizeof(stInCarData));
	BitStream bsVehicleSync;
	InCarData.sVehicleID = vehicleid;

	InCarData.fPosition[0] = fPos[0];
	InCarData.fPosition[1] = fPos[1];
	InCarData.fPosition[2] = fPos[2];

	InCarData.fVehicleHealth = HealthCar;

	InCarData.fMoveSpeed[0] = Speed[0];
	InCarData.fMoveSpeed[1] = Speed[1];
	InCarData.fMoveSpeed[2] = Speed[2];

	InCarData.sKeys = key;
	InCarData.bytePlayerHealth = self->hitpoints;
	InCarData.byteArmor = self->armor;

	InCarData.sTrailerID = trailer;

	bsVehicleSync.Write((BYTE)ID_VEHICLE_SYNC);
	bsVehicleSync.Write((PCHAR)&InCarData, sizeof(stInCarData));
	g_RakClient->Send(&bsVehicleSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
}
void RakClient::GivePlayerWeapon(uint16_t playerid, BYTE weaponid, WORD ammo)
{
	BitStream bsWep;
	bsWep.Write(playerid);
	bsWep.Write(weaponid);
	bsWep.Write(ammo);
	g_RakClient->RPC(RPC_GivePlayerWeapon, &bsWep, HIGH_PRIORITY, RELIABLE, 0, 0);
}
void RakClient::SendSpawn( void )
{
	BitStream bsSpawn;

	g_RakClient->RPC( RPC_RequestSpawn, &bsSpawn );
	g_RakClient->RPC( RPC_Spawn, &bsSpawn );
}

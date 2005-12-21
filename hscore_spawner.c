#include <string.h>

#include "asss.h"
#include "clientset.h"
#include "hscore.h"
#include "hscore_storeman.h"
#include "hscore_database.h"
#include "hscore_shipnames.h"

typedef struct PlayerDataStruct
{
	short underOurControl;
	short dirty;
	short spawned;
	ticks_t lastDeath;
} PlayerDataStruct;

typedef struct ShipOverrideKeys
{
	override_key_t ShrapnelMax;
	override_key_t ShrapnelRate;
	override_key_t CloakStatus;
	override_key_t StealthStatus;
	override_key_t XRadarStatus;
	override_key_t AntiWarpStatus;
	override_key_t InitialGuns;
	override_key_t MaxGuns;
	override_key_t InitialBombs;
	override_key_t MaxBombs;
	override_key_t SeeMines;
	override_key_t SeeBombLevel;
	override_key_t Gravity;
	override_key_t GravityTopSpeed;
	override_key_t BulletFireEnergy;
	override_key_t MultiFireEnergy;
	override_key_t BombFireEnergy;
	override_key_t BombFireEnergyUpgrade;
	override_key_t LandmineFireEnergy;
	override_key_t LandmineFireEnergyUpgrade;
	override_key_t BulletSpeed;
	override_key_t BombSpeed;
	override_key_t CloakEnergy;
	override_key_t StealthEnergy;
	override_key_t AntiWarpEnergy;
	override_key_t XRadarEnergy;
	override_key_t MaximumRotation;
	override_key_t MaximumThrust;
	override_key_t MaximumSpeed;
	override_key_t MaximumRecharge;
	override_key_t MaximumEnergy;
	override_key_t InitialRotation;
	override_key_t InitialThrust;
	override_key_t InitialSpeed;
	override_key_t InitialRecharge;
	override_key_t InitialEnergy;
	override_key_t UpgradeRotation;
	override_key_t UpgradeThrust;
	override_key_t UpgradeSpeed;
	override_key_t UpgradeRecharge;
	override_key_t UpgradeEnergy;
	override_key_t AfterburnerEnergy;
	override_key_t BombThrust;
	override_key_t TurretThrustPenalty;
	override_key_t TurretSpeedPenalty;
	override_key_t BulletFireDelay;
	override_key_t MultiFireDelay;
	override_key_t BombFireDelay;
	override_key_t LandmineFireDelay;
	override_key_t RocketTime;
	override_key_t InitialBounty;
	override_key_t DamageFactor;
	override_key_t AttachBounty;
	override_key_t SoccerThrowTime;
	override_key_t SoccerBallProximity;
	override_key_t MaxMines;
	override_key_t RepelMax;
	override_key_t BurstMax;
	override_key_t DecoyMax;
	override_key_t ThorMax;
	override_key_t BrickMax;
	override_key_t RocketMax;
	override_key_t PortalMax;
	override_key_t InitialRepel;
	override_key_t InitialBurst;
	override_key_t InitialBrick;
	override_key_t InitialRocket;
	override_key_t InitialThor;
	override_key_t InitialDecoy;
	override_key_t InitialPortal;
} ShipOverrideKeys;

//modules
local Iplayerdata *pd;
local Inet *net;
local Igame *game;
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Ihscoreitems *items;
local Iclientset *clientset;
local Ihscoredatabase *database;

local int playerDataKey;

local ShipOverrideKeys shipOverrideKeys[8];

local void spawnPlayer(Player *p)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	data->spawned = 1;


	//do needed prizing
	Target *t;
	t->type = T_PLAYER;
	t->u.p = p;

	int bounce = items->getPropertySum(p, p->pkt.ship, "bounce");
	if (bounce) game->GivePrize(t, 10, bounce);
	int prox = items->getPropertySum(p, p->pkt.ship, "prox");
	if (prox) game->GivePrize(t, 16, prox);
	int multifire = items->getPropertySum(p, p->pkt.ship, "multifire");
	if (multifire) game->GivePrize(t, 15, multifire);

}

local void loadOverrides()
{
	for (int i = 0; i < 8; i++)
	{
		shipOverrideKeys[i].ShrapnelMax = clientset->GetOverrideKey(shipNames[i], "ShrapnelMax");
		shipOverrideKeys[i].ShrapnelRate = clientset->GetOverrideKey(shipNames[i], "ShrapnelRate");
		shipOverrideKeys[i].CloakStatus = clientset->GetOverrideKey(shipNames[i], "CloakStatus");
		shipOverrideKeys[i].StealthStatus = clientset->GetOverrideKey(shipNames[i], "StealthStatus");
		shipOverrideKeys[i].XRadarStatus = clientset->GetOverrideKey(shipNames[i], "XRadarStatus");
		shipOverrideKeys[i].AntiWarpStatus = clientset->GetOverrideKey(shipNames[i], "AntiWarpStatus");
		shipOverrideKeys[i].InitialGuns = clientset->GetOverrideKey(shipNames[i], "InitialGuns");
		shipOverrideKeys[i].MaxGuns = clientset->GetOverrideKey(shipNames[i], "MaxGuns");
		shipOverrideKeys[i].InitialBombs = clientset->GetOverrideKey(shipNames[i], "InitialBombs");
		shipOverrideKeys[i].MaxBombs = clientset->GetOverrideKey(shipNames[i], "MaxBombs");
		shipOverrideKeys[i].SeeMines = clientset->GetOverrideKey(shipNames[i], "SeeMines");
		shipOverrideKeys[i].SeeBombLevel = clientset->GetOverrideKey(shipNames[i], "SeeBombLevel");
		shipOverrideKeys[i].Gravity = clientset->GetOverrideKey(shipNames[i], "Gravity");
		shipOverrideKeys[i].GravityTopSpeed = clientset->GetOverrideKey(shipNames[i], "GravityTopSpeed");
		shipOverrideKeys[i].BulletFireEnergy = clientset->GetOverrideKey(shipNames[i], "BulletFireEnergy");
		shipOverrideKeys[i].MultiFireEnergy = clientset->GetOverrideKey(shipNames[i], "MultiFireEnergy");
		shipOverrideKeys[i].BombFireEnergy = clientset->GetOverrideKey(shipNames[i], "BombFireEnergy");
		shipOverrideKeys[i].BombFireEnergyUpgrade = clientset->GetOverrideKey(shipNames[i], "BombFireEnergyUpgrade");
		shipOverrideKeys[i].LandmineFireEnergy = clientset->GetOverrideKey(shipNames[i], "LandmineFireEnergy");
		shipOverrideKeys[i].LandmineFireEnergyUpgrade = clientset->GetOverrideKey(shipNames[i], "LandmineFireEnergyUpgrade");
		shipOverrideKeys[i].CloakEnergy = clientset->GetOverrideKey(shipNames[i], "CloakEnergy");
		shipOverrideKeys[i].StealthEnergy = clientset->GetOverrideKey(shipNames[i], "StealthEnergy");
		shipOverrideKeys[i].AntiWarpEnergy = clientset->GetOverrideKey(shipNames[i], "AntiWarpEnergy");
		shipOverrideKeys[i].XRadarEnergy = clientset->GetOverrideKey(shipNames[i], "XRadarEnergy");
		shipOverrideKeys[i].MaximumRotation = clientset->GetOverrideKey(shipNames[i], "MaximumRotation");
		shipOverrideKeys[i].MaximumThrust = clientset->GetOverrideKey(shipNames[i], "MaximumThrust");
		shipOverrideKeys[i].MaximumSpeed = clientset->GetOverrideKey(shipNames[i], "MaximumSpeed");
		shipOverrideKeys[i].MaximumRecharge = clientset->GetOverrideKey(shipNames[i], "MaximumRecharge");
		shipOverrideKeys[i].MaximumEnergy = clientset->GetOverrideKey(shipNames[i], "MaximumEnergy");
		shipOverrideKeys[i].InitialRotation = clientset->GetOverrideKey(shipNames[i], "InitialRotation");
		shipOverrideKeys[i].InitialThrust = clientset->GetOverrideKey(shipNames[i], "InitialThrust");
		shipOverrideKeys[i].InitialSpeed = clientset->GetOverrideKey(shipNames[i], "InitialSpeed");
		shipOverrideKeys[i].InitialRecharge = clientset->GetOverrideKey(shipNames[i], "InitialRecharge");
		shipOverrideKeys[i].InitialEnergy = clientset->GetOverrideKey(shipNames[i], "InitialEnergy");
		shipOverrideKeys[i].UpgradeRotation = clientset->GetOverrideKey(shipNames[i], "UpgradeRotation");
		shipOverrideKeys[i].UpgradeThrust = clientset->GetOverrideKey(shipNames[i], "UpgradeThrust");
		shipOverrideKeys[i].UpgradeSpeed = clientset->GetOverrideKey(shipNames[i], "UpgradeSpeed");
		shipOverrideKeys[i].UpgradeRecharge = clientset->GetOverrideKey(shipNames[i], "UpgradeRecharge");
		shipOverrideKeys[i].UpgradeEnergy = clientset->GetOverrideKey(shipNames[i], "UpgradeEnergy");
		shipOverrideKeys[i].AfterburnerEnergy = clientset->GetOverrideKey(shipNames[i], "AfterburnerEnergy");
		shipOverrideKeys[i].BombThrust = clientset->GetOverrideKey(shipNames[i], "BombThrust");
		shipOverrideKeys[i].TurretThrustPenalty = clientset->GetOverrideKey(shipNames[i], "TurretThrustPenalty");
		shipOverrideKeys[i].TurretSpeedPenalty = clientset->GetOverrideKey(shipNames[i], "TurretSpeedPenalty");
		shipOverrideKeys[i].BulletFireDelay = clientset->GetOverrideKey(shipNames[i], "BulletFireDelay");
		shipOverrideKeys[i].MultiFireDelay = clientset->GetOverrideKey(shipNames[i], "MultiFireDelay");
		shipOverrideKeys[i].BombFireDelay = clientset->GetOverrideKey(shipNames[i], "BombFireDelay");
		shipOverrideKeys[i].LandmineFireDelay = clientset->GetOverrideKey(shipNames[i], "LandmineFireDelay");
		shipOverrideKeys[i].RocketTime = clientset->GetOverrideKey(shipNames[i], "RocketTime");
		shipOverrideKeys[i].InitialBounty = clientset->GetOverrideKey(shipNames[i], "InitialBounty");
		shipOverrideKeys[i].DamageFactor = clientset->GetOverrideKey(shipNames[i], "DamageFactor");
		shipOverrideKeys[i].AttachBounty = clientset->GetOverrideKey(shipNames[i], "AttachBounty");
		shipOverrideKeys[i].SoccerThrowTime = clientset->GetOverrideKey(shipNames[i], "SoccerThrowTime");
		shipOverrideKeys[i].SoccerBallProximity = clientset->GetOverrideKey(shipNames[i], "SoccerBallProximity");
		shipOverrideKeys[i].MaxMines = clientset->GetOverrideKey(shipNames[i], "MaxMines");
		shipOverrideKeys[i].RepelMax = clientset->GetOverrideKey(shipNames[i], "RepelMax");
		shipOverrideKeys[i].BurstMax = clientset->GetOverrideKey(shipNames[i], "BurstMax");
		shipOverrideKeys[i].DecoyMax = clientset->GetOverrideKey(shipNames[i], "DecoyMax");
		shipOverrideKeys[i].ThorMax = clientset->GetOverrideKey(shipNames[i], "ThorMax");
		shipOverrideKeys[i].BrickMax = clientset->GetOverrideKey(shipNames[i], "BrickMax");
		shipOverrideKeys[i].RocketMax = clientset->GetOverrideKey(shipNames[i], "RocketMax");
		shipOverrideKeys[i].PortalMax = clientset->GetOverrideKey(shipNames[i], "PortalMax");
		shipOverrideKeys[i].InitialRepel = clientset->GetOverrideKey(shipNames[i], "InitialRepel");
		shipOverrideKeys[i].InitialBurst = clientset->GetOverrideKey(shipNames[i], "InitialBurst");
		shipOverrideKeys[i].InitialBrick = clientset->GetOverrideKey(shipNames[i], "InitialBrick");
		shipOverrideKeys[i].InitialRocket = clientset->GetOverrideKey(shipNames[i], "InitialRocket");
		shipOverrideKeys[i].InitialThor = clientset->GetOverrideKey(shipNames[i], "InitialThor");
		shipOverrideKeys[i].InitialDecoy = clientset->GetOverrideKey(shipNames[i], "InitialDecoy");
		shipOverrideKeys[i].InitialPortal = clientset->GetOverrideKey(shipNames[i], "InitialPortal");
	}
}

local void addOverrides(Player *p)
{
	for (int i = 0; i < 8; i++)
	{
		int cloak = 2 * items->getPropertySum(p, p->pkt.ship, "cloak");
		clientset->PlayerOverride(p, shipOverrideKeys[i].CloakStatus, cloak);

		int stealth = 2 * items->getPropertySum(p, p->pkt.ship, "stealth");
		clientset->PlayerOverride(p, shipOverrideKeys[i].StealthStatus, stealth);

		int xradar = 2 * items->getPropertySum(p, p->pkt.ship, "xradar");
		clientset->PlayerOverride(p, shipOverrideKeys[i].XRadarStatus, xradar);

		int antiwarp = 2 * items->getPropertySum(p, p->pkt.ship, "antiwarp");
		clientset->PlayerOverride(p, shipOverrideKeys[i].AntiWarpStatus, antiwarp);

		int gunlevel = items->getPropertySum(p, p->pkt.ship, "gunlevel");
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialGuns, gunlevel);
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaxGuns, gunlevel);

		int bomblevel = items->getPropertySum(p, p->pkt.ship, "bomblevel");
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBombs, bomblevel);
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaxBombs, bomblevel);


		/*
		int thrust = items->getPropertySum(p, p->pkt.ship, "thrust");
		int speed = items->getPropertySum(p, p->pkt.ship, "speed");
		int energy = items->getPropertySum(p, p->pkt.ship, "energy");
		int recharge = items->getPropertySum(p, p->pkt.ship, "recharge");
		int rotation = items->getPropertySum(p, p->pkt.ship, "rotation");
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumRotation);
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumThrust);
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumSpeed);
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumRecharge);
		clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumEnergy);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRotation);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialThrust);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialSpeed);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRecharge);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialEnergy);
		clientset->PlayerOverride(p, shipOverrideKeys[i].UpgradeRotation);
		clientset->PlayerOverride(p, shipOverrideKeys[i].UpgradeThrust);
		clientset->PlayerOverride(p, shipOverrideKeys[i].UpgradeSpeed);
		clientset->PlayerOverride(p, shipOverrideKeys[i].UpgradeRecharge);
		clientset->PlayerOverride(p, shipOverrideKeys[i].UpgradeEnergy);*/

		int shrapnel = items->getPropertySum(p, p->pkt.ship, "shrapnel");
		clientset->PlayerOverride(p, shipOverrideKeys[i].ShrapnelMax, shrapnel);

		int burst = items->getPropertySum(p, p->pkt.ship, "burst");
		clientset->PlayerOverride(p, shipOverrideKeys[i].BurstMax, burst);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBurst, burst);

		int repel = items->getPropertySum(p, p->pkt.ship, "repel");
		clientset->PlayerOverride(p, shipOverrideKeys[i].RepelMax, repel);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRepel, repel);

		int decoy = items->getPropertySum(p, p->pkt.ship, "decoy");
		clientset->PlayerOverride(p, shipOverrideKeys[i].DecoyMax, decoy);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialDecoy, decoy);

		int thor = items->getPropertySum(p, p->pkt.ship, "thor");
		clientset->PlayerOverride(p, shipOverrideKeys[i].ThorMax, thor);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialThor, thor);

		int brick = items->getPropertySum(p, p->pkt.ship, "brick");
		clientset->PlayerOverride(p, shipOverrideKeys[i].BrickMax, brick);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBrick, brick);

		int rocket = items->getPropertySum(p, p->pkt.ship, "rocket");
		clientset->PlayerOverride(p, shipOverrideKeys[i].RocketMax, rocket);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRocket, rocket);

		int portal = items->getPropertySum(p, p->pkt.ship, "portal");
		clientset->PlayerOverride(p, shipOverrideKeys[i].PortalMax, portal);
		clientset->PlayerOverride(p, shipOverrideKeys[i].InitialPortal, portal);
	}
}

local void removeOverrides(Player *p)
{
	for (int i = 0; i < 8; i++)
	{
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].ShrapnelMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].ShrapnelRate);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].CloakStatus);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].StealthStatus);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].XRadarStatus);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].AntiWarpStatus);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialGuns);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaxGuns);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBombs);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaxBombs);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].SeeMines);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].SeeBombLevel);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].Gravity);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].GravityTopSpeed);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BulletFireEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MultiFireEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombFireEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombFireEnergyUpgrade);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].LandmineFireEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].LandmineFireEnergyUpgrade);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].CloakEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].StealthEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].AntiWarpEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].XRadarEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumRotation);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumThrust);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumSpeed);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumRecharge);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialRotation);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialThrust);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialSpeed);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialRecharge);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].UpgradeRotation);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].UpgradeThrust);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].UpgradeSpeed);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].UpgradeRecharge);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].UpgradeEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].AfterburnerEnergy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombThrust);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].TurretThrustPenalty);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].TurretSpeedPenalty);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BulletFireDelay);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MultiFireDelay);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombFireDelay);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].LandmineFireDelay);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].RocketTime);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBounty);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].DamageFactor);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].AttachBounty);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].SoccerThrowTime);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].SoccerBallProximity);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaxMines);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].RepelMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BurstMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].DecoyMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].ThorMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].BrickMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].RocketMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].PortalMax);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialRepel);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBurst);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBrick);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialRocket);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialThor);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialDecoy);
		clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialPortal);
	}
}

local void Pppk(Player *p, byte *p2, int len)
{
	Arena *arena = p->arena;
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	/* handle common errors */
	if (!arena) return;

	/* speccers don't get their position sent to anyone */
	if (p->p_ship == SHIP_SPEC)
		return;

	if (data->spawned == 0 && current_ticks() + 100 > data->lastDeath) //player hasn't been spawned
	{
		if (data->underOurControl == 1) //attached to the arena
		{
			spawnPlayer(p);
		}
	}
}

local void playerActionCallback(Player *p, int action, Arena *arena)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	if (action == PA_ENTERARENA)
	{
		data->underOurControl = 1;
		data->spawned = 0;
		//the player is entering the arena.

		addOverrides(p);
		//send the packet the first time
		clientset->SendClientSettings(p);
	}
	else if (action == PA_LEAVEARENA)
	{
		data->underOurControl = 0;
		removeOverrides(p);
	}
}

local void itemCountChangedCallback(Player *p, Item *item, InventoryEntry *entry, int newCount, int oldCount)
{
	//check if it changed anything in clientset, and if it did, recompute and flag dirty

	//if we didn't recompute, then check if the player has anything changeable that uses this item as ammo.
	//if they do, recompute and flag dirty
}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{
	PlayerDataStruct *data = PPDATA(killed, playerDataKey);
	data->spawned = 0;
	data->lastDeath = current_ticks();

	//if the dirty bit is set, then send new settings while they're dead
	if (data->dirty == 1)
	{
		clientset->SendClientSettings(killed);
	}

}

EXPORT int MM_hscore_spawner(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		net = mm->GetInterface(I_NET, ALLARENAS);
		game = mm->GetInterface(I_GAME, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		items = mm->GetInterface(I_HSCORE_ITEMS, ALLARENAS);
		clientset = mm->GetInterface(I_CLIENTSET, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !pd || !net || !game || !chat || !cfg || !items || !clientset || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(pd);
			mm->ReleaseInterface(net);
			mm->ReleaseInterface(game);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(items);
			mm->ReleaseInterface(clientset);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		playerDataKey = pd->AllocatePlayerData(sizeof(PlayerDataStruct));
		if (playerDataKey == -1)
		{
			return MM_FAIL;
		}

		net->AddPacket(C2S_POSITION, Pppk);

		loadOverrides();

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		net->RemovePacket(C2S_POSITION, Pppk);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(clientset);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegCallback(CB_PLAYERACTION, playerActionCallback, arena);
		mm->RegCallback(CB_KILL, killCallback, arena);
		mm->RegCallback(CB_ITEM_COUNT_CHANGED, itemCountChangedCallback, arena);
		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		mm->UnregCallback(CB_PLAYERACTION, playerActionCallback, arena);
		mm->UnregCallback(CB_KILL, killCallback, arena);
		mm->UnregCallback(CB_ITEM_COUNT_CHANGED, itemCountChangedCallback, arena);
		return MM_OK;
	}
	return MM_FAIL;
}



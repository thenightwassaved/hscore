
#include <stdlib.h>
#include <string.h>

#include "asss.h"
#include "clientset.h"
#include "hscore.h"
#include "hscore_storeman.h"
#include "hscore_database.h"
#include "hscore_spawner.h"
#include "hscore_shipnames.h"
#include "fg_wz.h"
#include "selfpos.h"

typedef struct PrizeData
{
	Player *player;
	int prizeNumber;
	int count;
} PrizeData;

typedef struct CallbackData
{
	Player *player;
	int ship;
	Item *item;
	int mult;
	int force;
} CallbackData;

typedef struct PlayerDataStruct
{
	short underOurControl;
	short dirty;
	short spawned;
	short usingPerShip[8];
	int currentShip;
	ticks_t lastDeath;

	ticks_t lastSet;
	int oldBounty;

	LinkedList ignoredPrizes;
} PlayerDataStruct;

typedef struct IgnorePrizeStruct
{
	int prize;
	ticks_t timeout;
} IgnorePrizeStruct;

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
	override_key_t DisableFastShooting;
} ShipOverrideKeys;

typedef struct GlobalOverrideKeys
{
	override_key_t BulletDamageLevel;
	override_key_t BulletDamageUpgrade;
	override_key_t BombDamageLevel;
	override_key_t EBombShutdownTime;
	override_key_t EBombDamagePercent;
	override_key_t BBombDamagePercent;
	override_key_t BurstDamageLevel;
	override_key_t DecoyAliveTime;
	override_key_t WarpPointDelay;
	override_key_t RocketThrust;
	override_key_t RocketSpeed;
	override_key_t InactiveShrapDamage;
	override_key_t ShrapnelDamagePercent;
	override_key_t MapZoomFactor;
	override_key_t FlaggerGunUpgrade;
	override_key_t FlaggerBombUpgrade;
	override_key_t AllowBombs;
	override_key_t AllowGuns;
	override_key_t UseFlagger;
	override_key_t BallLocation;
	override_key_t DoorMode;
	override_key_t JitterTime;
	override_key_t BombExplodePixels;
} GlobalOverrideKeys;

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
local Imainloop *ml;
local Iselfpos *selfpos;

local int playerDataKey;

local ShipOverrideKeys shipOverrideKeys[8];
local GlobalOverrideKeys globalOverrideKeys;

//interface function
local void respawn(Player *p);

local inline int min(int a, int b)
{
	if (a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

local inline int max(int a, int b)
{
	if (a > b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

local void spawnPlayer(Player *p)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	Target t;
	int bounce, prox, multifire, shrapnel, energyViewing;
	data->spawned = 1;


	//do needed prizing
	t.type = T_PLAYER;
	t.u.p = p;

	bounce = items->getPropertySum(p, p->pkt.ship, "bounce", 0);
	if (bounce) game->GivePrize(&t, 10, bounce);
	prox = items->getPropertySum(p, p->pkt.ship, "prox", 0);
	if (prox) game->GivePrize(&t, 16, prox);
	multifire = items->getPropertySum(p, p->pkt.ship, "multifire", 0);
	if (multifire) game->GivePrize(&t, 15, multifire);

	shrapnel = items->getPropertySum(p, p->pkt.ship, "shrapnel", 0);
	if (shrapnel) game->GivePrize(&t, 19, shrapnel);

	//set energy viewing
	energyViewing = items->getPropertySum(p, p->pkt.ship, "energyviewing", 0);
	if (energyViewing) game->SetPlayerEnergyViewing(p, ENERGY_SEE_ALL);
	else game->ResetPlayerEnergyViewing(p);
}

local void loadOverrides()
{
	int i;
	for (i = 0; i < 8; i++)
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
		shipOverrideKeys[i].DisableFastShooting = clientset->GetOverrideKey(shipNames[i], "DisableFastShooting");
	}

	globalOverrideKeys.BulletDamageLevel = clientset->GetOverrideKey("Bullet", "BulletDamageLevel");
	globalOverrideKeys.BulletDamageUpgrade = clientset->GetOverrideKey("Bullet", "BulletDamageUpgrade");

	globalOverrideKeys.BombDamageLevel = clientset->GetOverrideKey("Bomb", "BombDamageLevel");
	globalOverrideKeys.EBombShutdownTime = clientset->GetOverrideKey("Bomb", "EBombShutdownTime");
	globalOverrideKeys.EBombDamagePercent = clientset->GetOverrideKey("Bomb", "EBombDamagePercent");
	globalOverrideKeys.BBombDamagePercent = clientset->GetOverrideKey("Bomb", "BBombDamagePercent");
	globalOverrideKeys.JitterTime = clientset->GetOverrideKey("Bomb", "JitterTime");

	globalOverrideKeys.BurstDamageLevel = clientset->GetOverrideKey("Burst", "BurstDamageLevel");

	globalOverrideKeys.DecoyAliveTime = clientset->GetOverrideKey("Misc", "DecoyAliveTime");
	globalOverrideKeys.WarpPointDelay = clientset->GetOverrideKey("Misc", "WarpPointDelay");

	globalOverrideKeys.RocketThrust = clientset->GetOverrideKey("Rocket", "RocketThrust");
	globalOverrideKeys.RocketSpeed = clientset->GetOverrideKey("Rocket", "RocketSpeed");

	globalOverrideKeys.InactiveShrapDamage = clientset->GetOverrideKey("Shrapnel", "InactiveShrapDamage");
	globalOverrideKeys.ShrapnelDamagePercent = clientset->GetOverrideKey("Shrapnel", "ShrapnelDamagePercent");

	globalOverrideKeys.MapZoomFactor = clientset->GetOverrideKey("Radar", "MapZoomFactor");

	globalOverrideKeys.FlaggerGunUpgrade = clientset->GetOverrideKey("Flag", "FlaggerGunUpgrade");
	globalOverrideKeys.FlaggerBombUpgrade = clientset->GetOverrideKey("Flag", "FlaggerBombUpgrade");

	globalOverrideKeys.AllowBombs = clientset->GetOverrideKey("Soccer", "AllowBombs");
	globalOverrideKeys.AllowGuns = clientset->GetOverrideKey("Soccer", "AllowGuns");
	globalOverrideKeys.UseFlagger = clientset->GetOverrideKey("Soccer", "UseFlagger");
	globalOverrideKeys.BallLocation = clientset->GetOverrideKey("Soccer", "BallLocation");

	globalOverrideKeys.DoorMode = clientset->GetOverrideKey("Door", "DoorMode");

	globalOverrideKeys.BombExplodePixels = clientset->GetOverrideKey("Bomb", "BombExplodePixels");
}

local int checkUsingPerShip(Player *p, int ship)
{
	if (items->getPropertySumNoLock(p, ship, "bulletdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "bulletdamageup", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "bombdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "ebombtime", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "ebombdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "bbombdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "burstdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "decoyalive", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "warppointdelay", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "rocketthrust", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "rocketspeed", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "inactshrapdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "shrapdamage", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "mapzoom", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "flaggunup", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "flagbombup", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "soccerallowbombs", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "soccerallowguns", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "socceruseflag", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "soccerseeball", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "jittertime", 0)) return 1;
	if (items->getPropertySumNoLock(p, ship, "explodepixels", 0)) return 1;

	return 0;
}

local void addOverrides(Player *p)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	if (p->arena == NULL) return;

	ConfigHandle conf = p->arena->cfg;
	int i;
	for (i = 0; i < 8; i++)
	{
		if (playerData->hull[i] != NULL)
		{
			char *shipname = shipNames[i];

			int cloak = items->getPropertySumNoLock(p, i, "cloak", 0);
			if (cloak) clientset->PlayerOverride(p, shipOverrideKeys[i].CloakStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].CloakStatus);

			int stealth = items->getPropertySumNoLock(p, i, "stealth", 0);
			if (stealth) clientset->PlayerOverride(p, shipOverrideKeys[i].StealthStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].StealthStatus);

			int xradar = items->getPropertySumNoLock(p, i, "xradar", 0);
			if (xradar) clientset->PlayerOverride(p, shipOverrideKeys[i].XRadarStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].XRadarStatus);

			int antiwarp = items->getPropertySumNoLock(p, i, "antiwarp", 0);
			if (antiwarp) clientset->PlayerOverride(p, shipOverrideKeys[i].AntiWarpStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].AntiWarpStatus);




			int gunlevel = min(3, items->getPropertySumNoLock(p, i, "gunlevel", 0));
			if (gunlevel > 0)
			{
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialGuns, gunlevel);
				clientset->PlayerOverride(p, shipOverrideKeys[i].MaxGuns, gunlevel);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialGuns);
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaxGuns);
			}

			int bomblevel = min(3, items->getPropertySumNoLock(p, i, "bomblevel", 0));
			if (bomblevel > 0)
			{
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBombs, bomblevel);
				clientset->PlayerOverride(p, shipOverrideKeys[i].MaxBombs, bomblevel);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBombs);
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaxBombs);
			}




			int thrust = items->getPropertySumNoLock(p, i, "thrust", 0);
			int initThrust = cfg->GetInt(conf, shipname, "InitialThrust", 0);
			int upThrust = cfg->GetInt(conf, shipname, "UpgradeThrust", 0);
			int newThrust = max(0, initThrust + (upThrust * thrust));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialThrust, newThrust);

			int speed = items->getPropertySumNoLock(p, i, "speed", 0);
			int initSpeed = cfg->GetInt(conf, shipname, "InitialSpeed", 0);
			int upSpeed = cfg->GetInt(conf, shipname, "UpgradeSpeed", 0);
			int newSpeed = max(0, initSpeed + (upSpeed * speed));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialSpeed, newSpeed);

			int energy = items->getPropertySumNoLock(p, i, "energy", 0);
			int initEnergy = cfg->GetInt(conf, shipname, "InitialEnergy", 0);
			int upEnergy = cfg->GetInt(conf, shipname, "UpgradeEnergy", 0);
			int newEnergy = max(0, initEnergy + (upEnergy * energy));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialEnergy, newEnergy);

			int recharge = items->getPropertySumNoLock(p, i, "recharge", 0);
			int initRecharge = cfg->GetInt(conf, shipname, "InitialRecharge", 0);
			int upRecharge = cfg->GetInt(conf, shipname, "UpgradeRecharge", 0);
			int newRecharge = max(0, initRecharge + (upRecharge * recharge));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRecharge, newRecharge);

			int rotation = items->getPropertySumNoLock(p, i, "rotation", 0);
			int initRotation = cfg->GetInt(conf, shipname, "InitialRotation", 0);
			int upRotation = cfg->GetInt(conf, shipname, "UpgradeRotation", 0);
			int newRotation = max(0, initRotation + (upRotation * rotation));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRotation, newRotation);

			int maxthrust = items->getPropertySumNoLock(p, i, "maxthrust", 0);
			int initMaxThrust = cfg->GetInt(conf, shipname, "MaximumThrust", 0);
			int newMaxThrust = max(0, initMaxThrust + (upThrust * maxthrust));
			clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumThrust, newMaxThrust);

			int maxspeed = items->getPropertySumNoLock(p, i, "maxspeed", 0);
			int initMaxSpeed = cfg->GetInt(conf, shipname, "MaximumSpeed", 0);
			int newMaxSpeed = max(0, initMaxSpeed + (upSpeed * maxspeed));
			clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumSpeed, newMaxSpeed);

			int afterburner = items->getPropertySumNoLock(p, i, "afterburner", 0);
			int initAfterburner =  cfg->GetInt(conf, shipname, "AfterburnerEnergy", 0);
			int newAfterburner = initAfterburner + (upRecharge * afterburner);
			clientset->PlayerOverride(p, shipOverrideKeys[i].AfterburnerEnergy, newAfterburner);




			int initBurstMax = cfg->GetInt(conf, shipname, "BurstMax", 0);
			int burstMax = max(0, items->getPropertySumNoLock(p, i, "burstmax", initBurstMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].BurstMax, burstMax);

			int initRepelMax = cfg->GetInt(conf, shipname, "RepelMax", 0);
			int repelMax = max(0, items->getPropertySumNoLock(p, i, "repelmax", initRepelMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].RepelMax, repelMax);

			int initDecoyMax = cfg->GetInt(conf, shipname, "DecoyMax", 0);
			int decoyMax = max(0, items->getPropertySumNoLock(p, i, "decoymax", initDecoyMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].DecoyMax, decoyMax);

			int initThorMax = cfg->GetInt(conf, shipname, "ThorMax", 0);
			int thorMax = max(0, items->getPropertySumNoLock(p, i, "thormax", initThorMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].ThorMax, thorMax);

			int initBrickMax = cfg->GetInt(conf, shipname, "BrickMax", 0);
			int brickMax = max(0, items->getPropertySumNoLock(p, i, "brickmax", initBrickMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].BrickMax, brickMax);

			int initRocketMax = cfg->GetInt(conf, shipname, "RocketMax", 0);
			int rocketMax = max(0, items->getPropertySumNoLock(p, i, "rocketmax", initRocketMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].RocketMax, rocketMax);

			int initPortalMax = cfg->GetInt(conf, shipname, "PortalMax", 0);
			int portalMax = max(0, items->getPropertySumNoLock(p, i, "portalmax", initPortalMax));
			clientset->PlayerOverride(p, shipOverrideKeys[i].PortalMax, portalMax);




			int initBurst = cfg->GetInt(conf, shipname, "InitialBurst", 0);
			int burst = max(0, items->getPropertySumNoLock(p, i, "burst", initBurst));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBurst, burst);

			int initRepel = cfg->GetInt(conf, shipname, "InitialRepel", 0);
			int repel = max(0, items->getPropertySumNoLock(p, i, "repel", initRepel));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRepel, repel);

			int initDecoy = cfg->GetInt(conf, shipname, "InitialDecoy", 0);
			int decoy = max(0, items->getPropertySumNoLock(p, i, "decoy", initDecoy));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialDecoy, decoy);

			int initThor = cfg->GetInt(conf, shipname, "InitialThor", 0);
			int thor = max(0, items->getPropertySumNoLock(p, i, "thor", initThor));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialThor, thor);

			int initBrick = cfg->GetInt(conf, shipname, "InitialBrick", 0);
			int brick = max(0, items->getPropertySumNoLock(p, i, "brick", initBrick));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBrick, brick);

			int initRocket = cfg->GetInt(conf, shipname, "InitialRocket", 0);
			int rocket = max(0, items->getPropertySumNoLock(p, i, "rocket", initRocket));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRocket, rocket);

			int initPortal = cfg->GetInt(conf, shipname, "InitialPortal", 0);
			int portal = max(0, items->getPropertySumNoLock(p, i, "portal", initPortal));
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialPortal, portal);




			int initShrapRate = cfg->GetInt(conf, shipname, "ShrapnelRate", 0);
			int shraprate = items->getPropertySumNoLock(p, i, "shraprate", initShrapRate);
			clientset->PlayerOverride(p, shipOverrideKeys[i].ShrapnelRate, shraprate);

			int initMaxMines = cfg->GetInt(conf, shipname, "MaxMines", 0);
			int maxmines = items->getPropertySumNoLock(p, i, "maxmines", initMaxMines);
			clientset->PlayerOverride(p, shipOverrideKeys[i].MaxMines, maxmines);

			int initAttachBounty = cfg->GetInt(conf, shipname, "AttachBounty", 0);
			int attachbounty = items->getPropertySumNoLock(p, i, "attachbounty", initAttachBounty);
			clientset->PlayerOverride(p, shipOverrideKeys[i].AttachBounty, attachbounty);

			int initInitialBounty = cfg->GetInt(conf, shipname, "InitialBounty", 0);
			int initialbounty = items->getPropertySumNoLock(p, i, "initialbounty", initInitialBounty);
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBounty, initialbounty);

			int initSeeMines = cfg->GetInt(conf, shipname, "SeeMines", 0);
			int seemines = items->getPropertySumNoLock(p, i, "seemines", initSeeMines);
			if (seemines) seemines = 1;
			clientset->PlayerOverride(p, shipOverrideKeys[i].SeeMines, seemines);

			int initSeeBombLevel = cfg->GetInt(conf, shipname, "SeeBombLevel", 0);
			int seebomblevel = items->getPropertySumNoLock(p, i, "seebomblevel", initSeeBombLevel);
			clientset->PlayerOverride(p, shipOverrideKeys[i].SeeBombLevel, seebomblevel);

			int initBulletEnergy = cfg->GetInt(conf, shipname, "BulletFireEnergy", 0);
			int bulletenergy = items->getPropertySumNoLock(p, i, "bulletenergy", initBulletEnergy);
			clientset->PlayerOverride(p, shipOverrideKeys[i].BulletFireEnergy, bulletenergy);

			int initMultiEnergy = cfg->GetInt(conf, shipname, "MultiFireEnergy", 0);
			int multienergy = items->getPropertySumNoLock(p, i, "multienergy", initMultiEnergy);
			clientset->PlayerOverride(p, shipOverrideKeys[i].MultiFireEnergy, multienergy);

			int initBombEnergy = cfg->GetInt(conf, shipname, "BombFireEnergy", 0);
			int bombenergy = items->getPropertySumNoLock(p, i, "bombenergy", initBombEnergy);
			clientset->PlayerOverride(p, shipOverrideKeys[i].BombFireEnergy, bombenergy);

			int initBombEnergyUp = cfg->GetInt(conf, shipname, "BombFireEnergyUpgrade", 0);
			int bombenergyup = items->getPropertySumNoLock(p, i, "bombenergyup", initBombEnergyUp);
			clientset->PlayerOverride(p, shipOverrideKeys[i].BombFireEnergyUpgrade, bombenergyup);

			int initMineEnergy = cfg->GetInt(conf, shipname, "LandmineFireEnergy", 0);
			int mineenergy = items->getPropertySumNoLock(p, i, "mineenergy", initMineEnergy);
			clientset->PlayerOverride(p, shipOverrideKeys[i].LandmineFireEnergy, mineenergy);

			int initMineEnergyUp = cfg->GetInt(conf, shipname, "LandmineFireEnergyUpgrade", 0);
			int mineenergyup = items->getPropertySumNoLock(p, i, "mineenergyup", initMineEnergyUp);
			clientset->PlayerOverride(p, shipOverrideKeys[i].LandmineFireEnergyUpgrade, mineenergyup);

			int initBulletDelay = cfg->GetInt(conf, shipname, "BulletFireDelay", 0);
			int bulletdelay = items->getPropertySumNoLock(p, i, "bulletdelay", initBulletDelay);
			clientset->PlayerOverride(p, shipOverrideKeys[i].BulletFireDelay, bulletdelay);

			int initMultiDelay = cfg->GetInt(conf, shipname, "MultiFireDelay", 0);
			int multidelay = items->getPropertySumNoLock(p, i, "multidelay", initMultiDelay);
			clientset->PlayerOverride(p, shipOverrideKeys[i].MultiFireDelay, multidelay);

			int initBombDelay = cfg->GetInt(conf, shipname, "BombFireDelay", 0);
			int bombdelay = items->getPropertySumNoLock(p, i, "bombdelay", initBombDelay);
			clientset->PlayerOverride(p, shipOverrideKeys[i].BombFireDelay, bombdelay);

			int initMineDelay = cfg->GetInt(conf, shipname, "LandmineFireDelay", 0);
			int minedelay = items->getPropertySumNoLock(p, i, "minedelay", initMineDelay);
			clientset->PlayerOverride(p, shipOverrideKeys[i].LandmineFireDelay, minedelay);

			int initCloakEnergy = cfg->GetInt(conf, shipname, "CloakEnergy", 0);
			int cloakenergy = max(0, items->getPropertySumNoLock(p, i, "cloakenergy", initCloakEnergy));
			clientset->PlayerOverride(p, shipOverrideKeys[i].CloakEnergy, cloakenergy);

			int initStealthEnergy = cfg->GetInt(conf, shipname, "StealthEnergy", 0);
			int stealthenergy = max(0, items->getPropertySumNoLock(p, i, "stealthenergy", initStealthEnergy));
			clientset->PlayerOverride(p, shipOverrideKeys[i].StealthEnergy, stealthenergy);

			int initAntiEnergy = cfg->GetInt(conf, shipname, "AntiWarpEnergy", 0);
			int antienergy = max(0, items->getPropertySumNoLock(p, i, "antienergy", initAntiEnergy));
			clientset->PlayerOverride(p, shipOverrideKeys[i].AntiWarpEnergy, antienergy);

			int initXRadarEnergy = cfg->GetInt(conf, shipname, "XRadarEnergy", 0);
			int xradarenergy = max(0, items->getPropertySumNoLock(p, i, "xradarenergy", initXRadarEnergy));
			clientset->PlayerOverride(p, shipOverrideKeys[i].XRadarEnergy, xradarenergy);

			int initBombThrust = cfg->GetInt(conf, shipname, "BombThrust", 0);
			int bombthrust = items->getPropertySumNoLock(p, i, "bombthrust", initBombThrust);
			clientset->PlayerOverride(p, shipOverrideKeys[i].BombThrust, bombthrust);




			int initRocketTime = cfg->GetInt(conf, shipname, "RocketTime", 0);
			int rockettime = items->getPropertySumNoLock(p, i, "rockettime", initRocketTime);
			clientset->PlayerOverride(p, shipOverrideKeys[i].RocketTime, rockettime);

			int initDamageFactor = cfg->GetInt(conf, shipname, "DamageFactor", 0);
			int damagefactor = items->getPropertySumNoLock(p, i, "damagefactor", initDamageFactor);
			clientset->PlayerOverride(p, shipOverrideKeys[i].DamageFactor, damagefactor);

			int initSoccerTime = cfg->GetInt(conf, shipname, "SoccerThrowTime", 0);
			int soccertime = items->getPropertySumNoLock(p, i, "soccertime", initSoccerTime);
			clientset->PlayerOverride(p, shipOverrideKeys[i].SoccerThrowTime, soccertime);

			int initSoccerProx = cfg->GetInt(conf, shipname, "SoccerBallProximity", 0);
			int soccerprox = items->getPropertySumNoLock(p, i, "soccerprox", initSoccerProx);
			clientset->PlayerOverride(p, shipOverrideKeys[i].SoccerBallProximity, soccerprox);

			/*int initGravSpeed = cfg->GetInt(conf, shipname, "GravityTopSpeed", 0);
			int gravspeed = items->getPropertySumNoLock(p, i, "gravspeed", initGravSpeed);
			clientset->PlayerOverride(p, shipOverrideKeys[i].GravityTopSpeed, gravspeed);*/

			int initGravity = cfg->GetInt(conf, shipname, "Gravity", 0);
			int grav = items->getPropertySumNoLock(p, i, "grav", initGravity);
			clientset->PlayerOverride(p, shipOverrideKeys[i].Gravity, grav);

			int initTurretThrust = cfg->GetInt(conf, shipname, "TurretThrustPenalty", 0);
			int turretThrust = items->getPropertySumNoLock(p, i, "turretthrust", initTurretThrust);
			clientset->PlayerOverride(p, shipOverrideKeys[i].TurretThrustPenalty, turretThrust);

			int initTurretSpeed = cfg->GetInt(conf, shipname, "TurretSpeedPenalty", 0);
			int turretSpeed = items->getPropertySumNoLock(p, i, "turretspeed", initTurretSpeed);
			clientset->PlayerOverride(p, shipOverrideKeys[i].TurretSpeedPenalty, turretSpeed);

			int initNoFastShoot = cfg->GetInt(conf, shipname, "DisableFastShooting", 0);
			int fastShoot = items->getPropertySumNoLock(p, i, "nofastshoot", initNoFastShoot);
			clientset->PlayerOverride(p, shipOverrideKeys[i].DisableFastShooting, fastShoot);

			data->usingPerShip[i] = checkUsingPerShip(p, i);
		}
	}

	//add globals if the ship uses them
	if (p->p_ship != SHIP_SPEC)
	{
		int initBulletDamage = cfg->GetInt(conf, "Bullet", "BulletDamageLevel", 0);
		int bulletdamage = items->getPropertySumNoLock(p, p->p_ship, "bulletdamage", initBulletDamage) * 1000;
		clientset->PlayerOverride(p, globalOverrideKeys.BulletDamageLevel, bulletdamage);

		int initBulletDamageUp = cfg->GetInt(conf, "Bullet", "BulletDamageUpgrade", 0);
		int bulletdamageup = items->getPropertySumNoLock(p, p->p_ship, "bulletdamageup", initBulletDamageUp) * 1000;
		clientset->PlayerOverride(p, globalOverrideKeys.BulletDamageUpgrade, bulletdamageup);

		int initBombDamage = cfg->GetInt(conf, "Bomb", "BombDamageLevel", 0);
		int bombdamage = items->getPropertySumNoLock(p, p->p_ship, "bombdamage", initBombDamage) * 1000;
		clientset->PlayerOverride(p, globalOverrideKeys.BombDamageLevel, bombdamage);

		int initEBombTime = cfg->GetInt(conf, "Bomb", "EBombShutdownTime", 0);
		int ebombtime = items->getPropertySumNoLock(p, p->p_ship, "ebombtime", initEBombTime);
		clientset->PlayerOverride(p, globalOverrideKeys.EBombShutdownTime, ebombtime);

		int initEBombDamage = cfg->GetInt(conf, "Bomb", "EBombDamagePercent", 0);
		int ebombdamage = items->getPropertySumNoLock(p, p->p_ship, "ebombdamage", initEBombDamage);
		clientset->PlayerOverride(p, globalOverrideKeys.EBombDamagePercent, ebombdamage);

		int initBBombDamage = cfg->GetInt(conf, "Bomb", "BBombDamagePercent", 0);
		int bbombdamage = items->getPropertySumNoLock(p, p->p_ship, "bbombdamage", initBBombDamage);
		clientset->PlayerOverride(p, globalOverrideKeys.BBombDamagePercent, bbombdamage);

		int initBurstDamage = cfg->GetInt(conf, "Burst", "BurstDamageLevel", 0);
		int burstdamage = items->getPropertySumNoLock(p, p->p_ship, "burstdamage", initBurstDamage) * 1000;
		clientset->PlayerOverride(p, globalOverrideKeys.BurstDamageLevel, burstdamage);

		int initJitterTime = cfg->GetInt(conf, "Bomb", "JitterTime", 0);
		int jittertime = items->getPropertySumNoLock(p, p->p_ship, "jittertime", initJitterTime);
		clientset->PlayerOverride(p, globalOverrideKeys.JitterTime, jittertime);

		int initDecoyAlive = cfg->GetInt(conf, "Misc", "DecoyAliveTime", 0);
		int decoyalive = items->getPropertySumNoLock(p, p->p_ship, "decoyalive", initDecoyAlive);
		clientset->PlayerOverride(p, globalOverrideKeys.DecoyAliveTime, decoyalive);

		int initWarpPointDelay = cfg->GetInt(conf, "Misc", "WarpPointDelay", 0);
		int warppointdelay = items->getPropertySumNoLock(p, p->p_ship, "warppointdelay", initWarpPointDelay);
		clientset->PlayerOverride(p, globalOverrideKeys.WarpPointDelay, warppointdelay);

		int initRocketThrust = cfg->GetInt(conf, "Rocket", "RocketThrust", 0);
		int rocketthrust = items->getPropertySumNoLock(p, p->p_ship, "rocketthrust", initRocketThrust);
		clientset->PlayerOverride(p, globalOverrideKeys.RocketThrust, rocketthrust);

		int initRocketSpeed = cfg->GetInt(conf, "Rocket", "RocketSpeed", 0);
		int rocketspeed = items->getPropertySumNoLock(p, p->p_ship, "rocketspeed", initRocketSpeed);
		clientset->PlayerOverride(p, globalOverrideKeys.RocketSpeed, rocketspeed);

		int initInactShrapDamage = cfg->GetInt(conf, "Shrapnel", "InactiveShrapDamage", 0);
		int inactshrapdamage = items->getPropertySumNoLock(p, p->p_ship, "inactshrapdamage", initInactShrapDamage) * 1000;
		clientset->PlayerOverride(p, globalOverrideKeys.InactiveShrapDamage, inactshrapdamage);

		int initShrapDamage = cfg->GetInt(conf, "Shrapnel", "ShrapnelDamagePercent", 0);
		int shrapdamage = items->getPropertySumNoLock(p, p->p_ship, "shrapdamage", initShrapDamage);
		clientset->PlayerOverride(p, globalOverrideKeys.ShrapnelDamagePercent, shrapdamage);

		int initMapZoom = cfg->GetInt(conf, "Radar", "MapZoomFactor", 0);
		int mapzoom = items->getPropertySumNoLock(p, p->p_ship, "mapzoom", initMapZoom);
		clientset->PlayerOverride(p, globalOverrideKeys.MapZoomFactor, mapzoom);

		int initFlagGunUp = cfg->GetInt(conf, "Flag", "FlaggerGunUpgrade", 0);
		int flaggunup = items->getPropertySumNoLock(p, p->p_ship, "flaggunup", initFlagGunUp);
		clientset->PlayerOverride(p, globalOverrideKeys.FlaggerGunUpgrade, flaggunup);

		int initFlagBombUp = cfg->GetInt(conf, "Flag", "FlaggerBombUpgrade", 0);
		int flagbombup = items->getPropertySumNoLock(p, p->p_ship, "flagbombup", initFlagBombUp);
		clientset->PlayerOverride(p, globalOverrideKeys.FlaggerBombUpgrade, flagbombup);

		int initSoccerAllowBombs = cfg->GetInt(conf, "Soccer", "AllowBombs", 0);
		int soccerallowbombs = items->getPropertySumNoLock(p, p->p_ship, "soccerallowbombs", initSoccerAllowBombs);
		clientset->PlayerOverride(p, globalOverrideKeys.AllowBombs, soccerallowbombs);

		int initSoccerAllowGuns = cfg->GetInt(conf, "Soccer", "AllowGuns", 0);
		int soccerallowguns = items->getPropertySumNoLock(p, p->p_ship, "soccerallowguns", initSoccerAllowGuns);
		clientset->PlayerOverride(p, globalOverrideKeys.AllowGuns, soccerallowguns);

		int initSoccerUseFlag = cfg->GetInt(conf, "Soccer", "UseFlagger", 0);
		int socceruseflag = items->getPropertySumNoLock(p, p->p_ship, "socceruseflag", initSoccerUseFlag);
		clientset->PlayerOverride(p, globalOverrideKeys.UseFlagger, socceruseflag);

		int initSoccerSeeBall = cfg->GetInt(conf, "Soccer", "BallLocation", 0);
		int soccerseeball = items->getPropertySumNoLock(p, p->p_ship, "soccerseeball", initSoccerSeeBall);
		clientset->PlayerOverride(p, globalOverrideKeys.BallLocation, soccerseeball);

		int initDoorMode = cfg->GetInt(conf, "Door", "DoorMode", 0);
		int doormode = items->getPropertySumNoLock(p, p->p_ship, "doormode", initDoorMode);
		clientset->PlayerOverride(p, globalOverrideKeys.DoorMode, doormode);

		int initExplodePixels = cfg->GetInt(conf, "Bomb", "BombExplodePixels", 0);
		int explodePixels = items->getPropertySumNoLock(p, p->p_ship, "explodepixels", initExplodePixels);
		clientset->PlayerOverride(p, globalOverrideKeys.BombExplodePixels, explodePixels);
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

	clientset->PlayerUnoverride(p, globalOverrideKeys.BulletDamageLevel);
	clientset->PlayerUnoverride(p, globalOverrideKeys.BulletDamageUpgrade);
	clientset->PlayerUnoverride(p, globalOverrideKeys.BombDamageLevel);
	clientset->PlayerUnoverride(p, globalOverrideKeys.EBombShutdownTime);
	clientset->PlayerUnoverride(p, globalOverrideKeys.EBombDamagePercent);
	clientset->PlayerUnoverride(p, globalOverrideKeys.BBombDamagePercent);
	clientset->PlayerUnoverride(p, globalOverrideKeys.JitterTime);
	clientset->PlayerUnoverride(p, globalOverrideKeys.BurstDamageLevel);
	clientset->PlayerUnoverride(p, globalOverrideKeys.DecoyAliveTime);
	clientset->PlayerUnoverride(p, globalOverrideKeys.WarpPointDelay);
	clientset->PlayerUnoverride(p, globalOverrideKeys.RocketThrust);
	clientset->PlayerUnoverride(p, globalOverrideKeys.RocketSpeed);
	clientset->PlayerUnoverride(p, globalOverrideKeys.InactiveShrapDamage);
	clientset->PlayerUnoverride(p, globalOverrideKeys.ShrapnelDamagePercent);
	clientset->PlayerUnoverride(p, globalOverrideKeys.MapZoomFactor);
	clientset->PlayerUnoverride(p, globalOverrideKeys.FlaggerGunUpgrade);
	clientset->PlayerUnoverride(p, globalOverrideKeys.FlaggerBombUpgrade);
	clientset->PlayerUnoverride(p, globalOverrideKeys.AllowBombs);
	clientset->PlayerUnoverride(p, globalOverrideKeys.AllowGuns);
	clientset->PlayerUnoverride(p, globalOverrideKeys.UseFlagger);
	clientset->PlayerUnoverride(p, globalOverrideKeys.BallLocation);
	clientset->PlayerUnoverride(p, globalOverrideKeys.DoorMode);
	clientset->PlayerUnoverride(p, globalOverrideKeys.BombExplodePixels);
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

	if (data->spawned == 0) //player hasn't been spawned
	{
		int enterDelay = cfg->GetInt(p->arena->cfg, "Kill", "EnterDelay", 100);
		if (current_ticks() > (data->lastDeath + enterDelay + 150)) //not still dead
		{
			if (data->underOurControl == 1) //attached to the arena
			{
				spawnPlayer(p);
			}
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
		LLInit(&data->ignoredPrizes);
		//the player is entering the arena.
	}
	else if (action == PA_LEAVEARENA)
	{
		data->underOurControl = 0;
		removeOverrides(p);

		LLEnum(&data->ignoredPrizes, afree);
		LLEmpty(&data->ignoredPrizes);
	}
}

local void shipsLoadedCallback(Player *p)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	database->lock();
	addOverrides(p);
	database->unlock();
	//send the packet the first time
	clientset->SendClientSettings(p);

	data->dirty = 0;
	data->currentShip = p->p_ship;
}

local void itemCountChangedCallback(Player *p, Item *item, InventoryEntry *entry, int newCount, int oldCount) //called with lock held
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	if (item->resendSets)
	{
		data->dirty = 0;
		addOverrides(p);
		clientset->SendClientSettings(p);
	}
	else //check if it changed anything in clientset, and if it did, recompute and flag dirty
	{
		if (item->affectsSets)
		{
			data->dirty = 1;
			return;
		}
	}
}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{
	PlayerDataStruct *data = PPDATA(killed, playerDataKey);
	data->spawned = 0;
	data->lastDeath = current_ticks();

	//if the dirty bit is set, then send new settings while they're dead
	if (data->dirty == 1)
	{
		data->dirty = 0;
		database->lock();
		addOverrides(killed);
		database->unlock();
		clientset->SendClientSettings(killed);
	}
}

local void freqChangeCallback(Player *p, int newfreq)
{
	//check if they were in a safe zone. if not, then need a respawn
	if ((p->position.status & STATUS_SAFEZONE) == 0) //not in safe
	{
		PlayerDataStruct *data = PPDATA(p, playerDataKey);
		data->spawned = 0;

		if (data->dirty == 1)
		{
			data->dirty = 0;
			clientset->SendClientSettings(p);
		}
	}
}

local void shipChangeCallback(Player *p, int newship, int newfreq)
{
	//they need a respawn whenever they change ships
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	data->spawned = 0;

	int oldship = data->currentShip;
	if (oldship == SHIP_SPEC) oldship = 0;

	int perShipInUse = (data->usingPerShip[oldship] || (newship != SHIP_SPEC && data->usingPerShip[newship]));
	int changingIntoNewShip = (newship != SHIP_SPEC && newship != data->currentShip);


	//resend the data if it's dirty or if the player changed ships, isn't going into spec, and at least one of the ships uses per ship settings.
	if (data->dirty == 1 || (changingIntoNewShip && perShipInUse))
	{
		data->dirty = 0;
		database->lock();
		addOverrides(p);
		database->unlock();
		clientset->SendClientSettings(p);
	}

	if (newship != SHIP_SPEC)
		data->currentShip = newship;
}

local void flagWinCallback(Arena *arena, int freq, int *points)
{
	//players on the winning freq need a respawn because of a continuum quirk
	Link *link;
	Player *p;
    pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		if(p->arena == arena && p->p_freq == freq && p->p_ship != SHIP_SPEC)
		{
			PlayerDataStruct *data = PPDATA(p, playerDataKey);
			data->spawned = 0;

			if (data->dirty == 1)
			{
				data->dirty = 0;
				clientset->SendClientSettings(p);
			}
		}
	}
	pd->Unlock();
}

local int resetBountyTimerCallback(void *clos)
{
	Player *p = (Player*)clos;

	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	if (data->oldBounty < p->position.bounty)
	{
		selfpos->SetBounty(p, data->oldBounty);
	}

	return FALSE;
}

local int handleItemCallback(void *clos)
{
	CallbackData *data = clos;
	Player *p = data->player;
	PlayerDataStruct *pdata = PPDATA(p, playerDataKey);
	int ship = data->ship;
	Item *item = data->item;
	int mult = data->mult;
	int force = data->force;
	Link *propLink;
	int oldBounty = p->position.bounty;
	int prized = 0;

	afree(clos);

	if (ship != p->p_ship)
	{
		//it's not on their current ship!
		return FALSE;
	}

	if (item == NULL)
	{
		lm->LogP(L_ERROR, "hscore_spawner", p, "NULL item in callback.");
		return FALSE;
	}

	if (item->ammo && item->needsAmmo)
	{
		// needs ammo
		int count = items->getItemCountNoLock(p, item->ammo, ship);
		if (!force && count < item->minAmmo)
		{
			// no ammo present
			return FALSE;
		}
	}

	for (propLink = LLGetHead(&item->propertyList); propLink; propLink = propLink->next)
	{
		Property *prop = propLink->data;
		const char *propName = prop->name;
		int prizeNumber = -1;

		if (prop->value == 0) continue;

		int count = abs(prop->value);
		int mult2 = count/prop->value;

		if (strcasecmp(propName, "bomblevel") == 0)
			prizeNumber = 9;
		else if (strcasecmp(propName, "gunlevel") == 0)
		{
			prizeNumber = 8;
			count--;
			if (count == 0)
				prizeNumber = -1;
		}
		else if (strcasecmp(propName, "repel") == 0)
			prizeNumber = 21;
		else if (strcasecmp(propName, "burst") == 0)
			prizeNumber = 22;
		else if (strcasecmp(propName, "thor") == 0)
			prizeNumber = 24;
		else if (strcasecmp(propName, "portal") == 0)
			prizeNumber = 28;
		else if (strcasecmp(propName, "decoy") == 0)
			prizeNumber = 23;
		else if (strcasecmp(propName, "brick") == 0)
			prizeNumber = 26;
		else if (strcasecmp(propName, "rocket") == 0)
			prizeNumber = 27;
		else if (strcasecmp(propName, "xradar") == 0)
			prizeNumber = 6;
		else if (strcasecmp(propName, "cloak") == 0)
			prizeNumber = 5;
		else if (strcasecmp(propName, "stealth") == 0)
			prizeNumber = 4;
		else if (strcasecmp(propName, "antiwarp") == 0)
			prizeNumber = 20;
		else if (strcasecmp(propName, "thrust") == 0)
		{
			if (mult*mult2 > 0)
				prizeNumber = -1;
			else
				prizeNumber = 11;
		}
		else if (strcasecmp(propName, "speed") == 0)
		{
			if (mult*mult2 > 0)
				prizeNumber = -1;
			else
				prizeNumber = 12;
		}
		else if (strcasecmp(propName, "energy") == 0)
		{
			if (mult*mult2 > 0)
				prizeNumber = -1;
			else
				prizeNumber = 2;
		}
		else if (strcasecmp(propName, "recharge") == 0)
		{
			if (mult*mult2 > 0)
				prizeNumber = -1;
			else
				prizeNumber = 1;
		}
		else if (strcasecmp(propName, "rotation") == 0)
		{
			if (mult*mult2 > 0)
				prizeNumber = -1;
			else
				prizeNumber = 3;
		}
		else if (strcasecmp(propName, "bounce") == 0)
			prizeNumber = 10;
		else if (strcasecmp(propName, "prox") == 0)
			prizeNumber = 16;
		else if (strcasecmp(propName, "shrapnel") == 0)
			prizeNumber = 19;
		else if (strcasecmp(propName, "multifire") == 0)
			prizeNumber = 15;

		if (prizeNumber != -1)
		{
			Link *link;
			int shouldPrize = 1;

			link = LLGetHead(&pdata->ignoredPrizes);
			while (link)
			{
				IgnorePrizeStruct *entry = link->data;
				link = link->next;

				if (entry->timeout < current_ticks())
				{
					// remove it
					afree(entry);
					LLRemove(&pdata->ignoredPrizes, entry);
				}
				else if (prizeNumber == entry->prize)
				{
					shouldPrize = 0;
				}
			}

			if (shouldPrize)
			{
				Target t;
				t.type = T_PLAYER;
				t.u.p = p;
				game->GivePrize(&t, mult*mult2*prizeNumber, count);
				prized = 1;
			}
		}
	}

	if (prized)
	{
		if (pdata->lastSet + 100 < current_ticks())
		{
			pdata->oldBounty = oldBounty;
		}

		pdata->lastSet = current_ticks();
		ml->ClearTimer(resetBountyTimerCallback, p);
		ml->SetTimer(resetBountyTimerCallback, 50, 0, p, p);
	}

	return FALSE;
}

local void ammoAddedCallback(Player *p, int ship, Item *ammoUser) //warnings: cache is out of sync, and lock is held
{
	PlayerDataStruct *pdata = PPDATA(p, playerDataKey);

	CallbackData *data = amalloc(sizeof(*data));
	data->player = p;
	data->ship = ship;
	data->item = ammoUser;
	data->mult = 1;
	data->force = 1;

	if (ammoUser->resendSets)
	{
		pdata->dirty = 0;
		addOverrides(p);
		clientset->SendClientSettings(p);
	}
	else //check if it changed anything in clientset, and if it did, recompute and flag dirty
	{
		if (ammoUser->affectsSets)
		{
			pdata->dirty = 1;
			return;
		}
	}

	handleItemCallback(data);
}

local void ammoRemovedCallback(Player *p, int ship, Item *ammoUser) //warnings: cache is out of sync, and lock is held
{
	PlayerDataStruct *pdata = PPDATA(p, playerDataKey);

	CallbackData *data = amalloc(sizeof(*data));
	data->player = p;
	data->ship = ship;
	data->item = ammoUser;
	data->mult = -1;
	data->force = 1;

	if (ammoUser->resendSets)
	{
		pdata->dirty = 0;
		addOverrides(p);
		clientset->SendClientSettings(p);
	}
	else //check if it changed anything in clientset, and if it did, recompute and flag dirty
	{
		if (ammoUser->affectsSets)
		{
			pdata->dirty = 1;
			return;
		}
	}

	handleItemCallback(data);
}

local void triggerEventCallback(Player *p, Item *item, int ship, const char *eventName) //called with lock held
{
	if (strcasecmp(eventName, "add") == 0)
	{
		CallbackData *data = amalloc(sizeof(*data));
		data->player = p;
		data->ship = ship;
		data->item = item;
		data->mult = 1;
		data->force = 0;
		handleItemCallback(data);
	}
	else if (strcasecmp(eventName, "del") == 0)
	{
		CallbackData *data = amalloc(sizeof(*data));
		data->player = p;
		data->ship = ship;
		data->item = item;
		data->mult = -1;
		data->force = 0;
		handleItemCallback(data);
	}
	else
	{
		return; //nothing to do
	}
}

local void respawn(Player *p)
{
	//a simple redirect won't work because of locking issues.
	//instead rig them for respawn on next position packet
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	data->spawned = 0;
	data->lastDeath = 0;
}

local int getFullEnergy(Player *p)
{
	if (0 <= p->p_ship && p->p_ship <= 8)
	{
		ConfigHandle conf = p->arena->cfg;
		const char *shipname = shipNames[p->p_ship];
		int energy = items->getPropertySum(p, p->p_ship, "energy", 0);
		int initEnergy = cfg->GetInt(conf, shipname, "InitialEnergy", 0);
		int upEnergy = cfg->GetInt(conf, shipname, "UpgradeEnergy", 0);

		return initEnergy + (upEnergy * energy);
	}
	else
	{
		return 0;
	}
}

local void ignorePrize(Player *p, int prize)
{
	IgnorePrizeStruct *prizeData = amalloc(sizeof(*prizeData));
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	prizeData->prize = prize;
	prizeData->timeout = current_ticks() + 50;

	LLAdd(&data->ignoredPrizes, prizeData);
}

local void HSItemReloadCallback(void)
{
	Player *p;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		database->lock();
		addOverrides(p);
		database->unlock();
		clientset->SendClientSettings(p);
	}
	pd->Unlock();
}

local Ihscorespawner interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_SPAWNER, "hscore_spawner")
	respawn, getFullEnergy, ignorePrize
};

EXPORT const char info_hscore_spawner[] = "v1.1 Dr Brain <drbrain@gmail.com>";

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
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		selfpos = mm->GetInterface(I_SELFPOS, ALLARENAS);

		if (!lm || !pd || !net || !game || !chat || !cfg || !items || !clientset || !database || !ml || !selfpos)
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
			mm->ReleaseInterface(ml);
			mm->ReleaseInterface(selfpos);

			return MM_FAIL;
		}

		playerDataKey = pd->AllocatePlayerData(sizeof(PlayerDataStruct));
		if (playerDataKey == -1)
		{
			return MM_FAIL;
		}

		net->AddPacket(C2S_POSITION, Pppk);

		loadOverrides();

		mm->RegCallback(CB_HS_ITEMRELOAD, HSItemReloadCallback, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->UnregCallback(CB_HS_ITEMRELOAD, HSItemReloadCallback, ALLARENAS);

		net->RemovePacket(C2S_POSITION, Pppk);

		ml->ClearTimer(resetBountyTimerCallback, NULL);

		pd->FreePlayerData(playerDataKey);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(clientset);
		mm->ReleaseInterface(database);
		mm->ReleaseInterface(ml);
		mm->ReleaseInterface(selfpos);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegInterface(&interface, arena);

		mm->RegCallback(CB_WARZONEWIN, flagWinCallback, arena);
		mm->RegCallback(CB_SHIPS_LOADED, shipsLoadedCallback, arena);
		mm->RegCallback(CB_SHIPCHANGE, shipChangeCallback, arena);
		mm->RegCallback(CB_PLAYERACTION, playerActionCallback, arena);
		mm->RegCallback(CB_KILL, killCallback, arena);
		mm->RegCallback(CB_ITEM_COUNT_CHANGED, itemCountChangedCallback, arena);
		mm->RegCallback(CB_FREQCHANGE, freqChangeCallback, arena);

		mm->RegCallback(CB_TRIGGER_EVENT, triggerEventCallback, arena);
		mm->RegCallback(CB_AMMO_ADDED, ammoAddedCallback, arena);
		mm->RegCallback(CB_AMMO_REMOVED, ammoRemovedCallback, arena);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		if (mm->UnregInterface(&interface, arena))
		{
			return MM_FAIL;
		}

		mm->UnregCallback(CB_FREQCHANGE, freqChangeCallback, arena);
		mm->UnregCallback(CB_ITEM_COUNT_CHANGED, itemCountChangedCallback, arena);
		mm->UnregCallback(CB_KILL, killCallback, arena);
		mm->UnregCallback(CB_PLAYERACTION, playerActionCallback, arena);
		mm->UnregCallback(CB_SHIPCHANGE, shipChangeCallback, arena);
		mm->UnregCallback(CB_SHIPS_LOADED, shipsLoadedCallback, arena);
		mm->UnregCallback(CB_WARZONEWIN, flagWinCallback, arena);

		mm->UnregCallback(CB_TRIGGER_EVENT, triggerEventCallback, arena);
		mm->UnregCallback(CB_AMMO_ADDED, ammoAddedCallback, arena);
		mm->UnregCallback(CB_AMMO_REMOVED, ammoRemovedCallback, arena);

		return MM_OK;
	}
	return MM_FAIL;
}



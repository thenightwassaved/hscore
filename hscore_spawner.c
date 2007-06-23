
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

typedef struct PrizeData
{
	Player *player;
	int prizeNumber;
	int count;
} PrizeData;

typedef struct PlayerDataStruct
{
	short underOurControl;
	short dirty;
	short spawned;
	short usingPerShip[8];
	int currentShip;
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

local int playerDataKey;

local ShipOverrideKeys shipOverrideKeys[8];
local GlobalOverrideKeys globalOverrideKeys;

//interface function
local void respawn(Player *p);

local void spawnPlayer(Player *p)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	Target t;
	int bounce, prox, multifire, shrapnel, energyViewing;
	data->spawned = 1;


	//do needed prizing
	t.type = T_PLAYER;
	t.u.p = p;

	bounce = items->getPropertySum(p, p->pkt.ship, "bounce");
	if (bounce) game->GivePrize(&t, 10, bounce);
	prox = items->getPropertySum(p, p->pkt.ship, "prox");
	if (prox) game->GivePrize(&t, 16, prox);
	multifire = items->getPropertySum(p, p->pkt.ship, "multifire");
	if (multifire) game->GivePrize(&t, 15, multifire);

	shrapnel = items->getPropertySum(p, p->pkt.ship, "shrapnel");
	if (shrapnel) game->GivePrize(&t, 19, shrapnel);
	
	//set energy viewing
	energyViewing = items->getPropertySum(p, p->pkt.ship, "energyviewing");
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
}

local int checkUsingPerShip(Player *p, int ship)
{
	if (items->getPropertySum(p, ship, "bulletdamage")) return 1;		
	if (items->getPropertySum(p, ship, "bulletdamageup")) return 1;	
	if (items->getPropertySum(p, ship, "bombdamage")) return 1;		
	if (items->getPropertySum(p, ship, "ebombtime")) return 1;		
	if (items->getPropertySum(p, ship, "ebombdamage")) return 1;		
	if (items->getPropertySum(p, ship, "bbombdamage")) return 1;	
	if (items->getPropertySum(p, ship, "burstdamage")) return 1;
	if (items->getPropertySum(p, ship, "decoyalive")) return 1;
	if (items->getPropertySum(p, ship, "warppointdelay")) return 1;		
	if (items->getPropertySum(p, ship, "rocketthrust")) return 1;		
	if (items->getPropertySum(p, ship, "rocketspeed")) return 1;		
	if (items->getPropertySum(p, ship, "inactshrapdamage")) return 1;		
	if (items->getPropertySum(p, ship, "shrapdamage")) return 1;		
	if (items->getPropertySum(p, ship, "mapzoom")) return 1;
	if (items->getPropertySum(p, ship, "flaggunup")) return 1;
	if (items->getPropertySum(p, ship, "flagbombup")) return 1;	
	if (items->getPropertySum(p, ship, "soccerallowbombs")) return 1;	
	if (items->getPropertySum(p, ship, "soccerallowguns")) return 1;
	if (items->getPropertySum(p, ship, "socceruseflag")) return 1;
	if (items->getPropertySum(p, ship, "soccerseeball")) return 1;
	if (items->getPropertySum(p, ship, "jittertime")) return 1;

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

			int cloak = items->getPropertySum(p, i, "cloak");
			if (cloak) clientset->PlayerOverride(p, shipOverrideKeys[i].CloakStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].CloakStatus);

			int stealth = items->getPropertySum(p, i, "stealth");
			if (stealth) clientset->PlayerOverride(p, shipOverrideKeys[i].StealthStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].StealthStatus);

			int xradar = items->getPropertySum(p, i, "xradar");
			if (xradar) clientset->PlayerOverride(p, shipOverrideKeys[i].XRadarStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].XRadarStatus);

			int antiwarp = items->getPropertySum(p, i, "antiwarp");
			if (antiwarp) clientset->PlayerOverride(p, shipOverrideKeys[i].AntiWarpStatus, 2);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].AntiWarpStatus);



			int gunlevel = items->getPropertySum(p, i, "gunlevel");
			if (gunlevel > 0) clientset->PlayerOverride(p, shipOverrideKeys[i].InitialGuns, gunlevel);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialGuns);

			int bomblevel = items->getPropertySum(p, i, "bomblevel");
			if (bomblevel > 0) clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBombs, bomblevel);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBombs);




			int thrust = items->getPropertySum(p, i, "thrust");
			if (thrust)
			{
				int initThrust = cfg->GetInt(conf, shipname, "InitialThrust", 0);
				int upThrust = cfg->GetInt(conf, shipname, "UpgradeThrust", 0);
				int newThrust = initThrust + (upThrust * thrust);
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialThrust, newThrust);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialThrust);
			}

			int speed = items->getPropertySum(p, i, "speed");
			if (speed)
			{
				int initSpeed = cfg->GetInt(conf, shipname, "InitialSpeed", 0);
				int upSpeed = cfg->GetInt(conf, shipname, "UpgradeSpeed", 0);
				int newSpeed = initSpeed + (upSpeed * speed);
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialSpeed, newSpeed);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialSpeed);
			}

			int energy = items->getPropertySum(p, i, "energy");
			if (energy)
			{
				int initEnergy = cfg->GetInt(conf, shipname, "InitialEnergy", 0);
				int upEnergy = cfg->GetInt(conf, shipname, "UpgradeEnergy", 0);
				int newEnergy = initEnergy + (upEnergy * energy);
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialEnergy, newEnergy);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialEnergy);
			}

			int recharge = items->getPropertySum(p, i, "recharge");
			if (recharge)
			{
				int initRecharge = cfg->GetInt(conf, shipname, "InitialRecharge", 0);
				int upRecharge = cfg->GetInt(conf, shipname, "UpgradeRecharge", 0);
				int newRecharge = initRecharge + (upRecharge * recharge);
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRecharge, newRecharge);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialRecharge);
			}

			int rotation = items->getPropertySum(p, i, "rotation");
			if (rotation)
			{
				int initRotation = cfg->GetInt(conf, shipname, "InitialRotation", 0);
				int upRotation = cfg->GetInt(conf, shipname, "UpgradeRotation", 0);
				int newRotation = initRotation + (upRotation * rotation);
				clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRotation, newRotation);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialRotation);
			}
			
			int maxthrust = items->getPropertySum(p, i, "maxthrust");
			if (maxthrust) 
			{
				int initThrust = cfg->GetInt(conf, shipname, "MaximumThrust", 0);
				int upThrust = cfg->GetInt(conf, shipname, "UpgradeThrust", 0);
				int newThrust = initThrust + (upThrust * maxthrust);
				clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumThrust, newThrust);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumThrust);
			}

			int maxspeed = items->getPropertySum(p, i, "maxspeed");
			if (maxspeed) 
			{
				int initSpeed = cfg->GetInt(conf, shipname, "MaximumSpeed", 0);
				int upSpeed = cfg->GetInt(conf, shipname, "UpgradeSpeed", 0);
				int newSpeed = initSpeed + (upSpeed * maxspeed);		
				clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumSpeed, newSpeed);
			}
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumSpeed);
			}
			
			int afterburner = items->getPropertySum(p, i, "afterburner");
			if (afterburner) 
			{
				int upRecharge = cfg->GetInt(conf, shipname, "UpgradeRecharge", 0);
				clientset->PlayerOverride(p, shipOverrideKeys[i].AfterburnerEnergy, upRecharge * afterburner);
			else
			{
				clientset->PlayerUnoverride(p, shipOverrideKeys[i].AfterburnerEnergy);
			}
			

			int initBurst = cfg->GetInt(conf, shipname, "InitialBurst", 0);
			int burst = items->getPropertySum(p, i, "burst") + initBurst;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBurst, burst);

			int initRepel = cfg->GetInt(conf, shipname, "InitialRepel", 0);
			int repel = items->getPropertySum(p, i, "repel") + initRepel;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRepel, repel);

			int initDecoy = cfg->GetInt(conf, shipname, "InitialDecoy", 0);
			int decoy = items->getPropertySum(p, i, "decoy") + initDecoy;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialDecoy, decoy);

			int initThor = cfg->GetInt(conf, shipname, "InitialThor", 0);
			int thor = items->getPropertySum(p, i, "thor") + initThor;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialThor, thor);

			int initBrick = cfg->GetInt(conf, shipname, "InitialBrick", 0);
			int brick = items->getPropertySum(p, i, "brick") + initBrick;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBrick, brick);

			int initRocket = cfg->GetInt(conf, shipname, "InitialRocket", 0);
			int rocket = items->getPropertySum(p, i, "rocket") + initRocket;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialRocket, rocket);

			int initPortal = cfg->GetInt(conf, shipname, "InitialPortal", 0);
			int portal = items->getPropertySum(p, i, "portal") + initPortal;
			clientset->PlayerOverride(p, shipOverrideKeys[i].InitialPortal, portal);




			int seemines = items->getPropertySum(p, i, "seemines");
			if (seemines) clientset->PlayerOverride(p, shipOverrideKeys[i].SeeMines, seemines);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].SeeMines);

			int seebomblevel = items->getPropertySum(p, i, "seebomblevel");
			if (seebomblevel) clientset->PlayerOverride(p, shipOverrideKeys[i].SeeBombLevel, seebomblevel);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].SeeBombLevel);

			int bulletenergy = items->getPropertySum(p, i, "bulletenergy");
			if (bulletenergy) clientset->PlayerOverride(p, shipOverrideKeys[i].BulletFireEnergy, bulletenergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].BulletFireEnergy);

			int multienergy = items->getPropertySum(p, i, "multienergy");
			if (multienergy) clientset->PlayerOverride(p, shipOverrideKeys[i].MultiFireEnergy, multienergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].MultiFireEnergy);

			int bombenergy = items->getPropertySum(p, i, "bombenergy");
			if (bombenergy) clientset->PlayerOverride(p, shipOverrideKeys[i].BombFireEnergy, bombenergy);

			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombFireEnergy);

			int bombenergyup = items->getPropertySum(p, i, "bombenergyup");
			if (bombenergyup) clientset->PlayerOverride(p, shipOverrideKeys[i].BombFireEnergyUpgrade, bombenergyup);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombFireEnergyUpgrade);

			int mineenergy = items->getPropertySum(p, i, "mineenergy");
			if (mineenergy) clientset->PlayerOverride(p, shipOverrideKeys[i].LandmineFireEnergy, mineenergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].LandmineFireEnergy);

			int mineenergyup = items->getPropertySum(p, i, "mineenergyup");
			if (mineenergyup) clientset->PlayerOverride(p, shipOverrideKeys[i].LandmineFireEnergyUpgrade, mineenergyup);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].LandmineFireEnergyUpgrade);

			int cloakenergy = items->getPropertySum(p, i, "cloakenergy");
			if (cloakenergy) clientset->PlayerOverride(p, shipOverrideKeys[i].CloakEnergy, cloakenergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].CloakEnergy);

			int stealthenergy = items->getPropertySum(p, i, "stealthenergy");
			if (stealthenergy) clientset->PlayerOverride(p, shipOverrideKeys[i].StealthEnergy, stealthenergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].StealthEnergy);

			int antienergy = items->getPropertySum(p, i, "antienergy");
			if (antienergy) clientset->PlayerOverride(p, shipOverrideKeys[i].AntiWarpEnergy, antienergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].AntiWarpEnergy);

			int xradarenergy = items->getPropertySum(p, i, "xradarenergy");
			if (xradarenergy) clientset->PlayerOverride(p, shipOverrideKeys[i].XRadarEnergy, xradarenergy);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].XRadarEnergy);

			int bombthrust = items->getPropertySum(p, i, "bombthrust");
			if (bombthrust) clientset->PlayerOverride(p, shipOverrideKeys[i].BombThrust, bombthrust);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombThrust);

			int bulletdelay = items->getPropertySum(p, i, "bulletdelay");
			if (bulletdelay) clientset->PlayerOverride(p, shipOverrideKeys[i].BulletFireDelay, bulletdelay);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].BulletFireDelay);

			int multidelay = items->getPropertySum(p, i, "multidelay");
			if (multidelay) clientset->PlayerOverride(p, shipOverrideKeys[i].MultiFireDelay, multidelay);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].MultiFireDelay);

			int bombdelay = items->getPropertySum(p, i, "bombdelay");
			if (bombdelay) clientset->PlayerOverride(p, shipOverrideKeys[i].BombFireDelay, bombdelay);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].BombFireDelay);

			int minedelay = items->getPropertySum(p, i, "minedelay");
			if (minedelay) clientset->PlayerOverride(p, shipOverrideKeys[i].LandmineFireDelay, minedelay);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].LandmineFireDelay);

			int rockettime = items->getPropertySum(p, i, "rockettime");
			if (rockettime) clientset->PlayerOverride(p, shipOverrideKeys[i].RocketTime, rockettime);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].RocketTime);

			int initInitialBounty = cfg->GetInt(conf, shipname, "InitialBounty", 0);
			int initialbounty = items->getPropertySum(p, i, "initialbounty") + initInitialBounty;
			if (initialbounty) clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBounty, initialbounty);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialBounty);

			int damagefactor = items->getPropertySum(p, i, "damagefactor");
			if (damagefactor) clientset->PlayerOverride(p, shipOverrideKeys[i].DamageFactor, damagefactor);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].DamageFactor);

			int initAttachBounty = cfg->GetInt(conf, shipname, "AttachBounty", 0);
			int attachbounty = items->getPropertySum(p, i, "attachbounty") + initAttachBounty;
			if (attachbounty) clientset->PlayerOverride(p, shipOverrideKeys[i].AttachBounty, attachbounty);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].AttachBounty);

			int soccertime = items->getPropertySum(p, i, "soccertime");
			if (soccertime) clientset->PlayerOverride(p, shipOverrideKeys[i].SoccerThrowTime, soccertime);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].SoccerThrowTime);

			int soccerprox = items->getPropertySum(p, i, "soccerprox");
			if (soccerprox) clientset->PlayerOverride(p, shipOverrideKeys[i].SoccerBallProximity, soccerprox);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].SoccerBallProximity);

			//int initMaxMines = cfg->GetInt(conf, shipname, "MaxMines", 0);
			//int maxmines = items->getPropertySum(p, i, "maxmines") + initMaxMines;
			int maxmines = items->getPropertySum(p, i, "maxmines");
			if (maxmines) clientset->PlayerOverride(p, shipOverrideKeys[i].MaxMines, maxmines);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaxMines);

			int gravspeed = items->getPropertySum(p, i, "gravspeed");
			if (gravspeed) clientset->PlayerOverride(p, shipOverrideKeys[i].GravityTopSpeed, gravspeed);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].GravityTopSpeed);

			int grav = items->getPropertySum(p, i, "grav");
			if (grav) clientset->PlayerOverride(p, shipOverrideKeys[i].Gravity, grav);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].Gravity);

			int fastShoot = items->getPropertySum(p, i, "nofastshoot");
			if (fastShoot) clientset->PlayerOverride(p, shipOverrideKeys[i].DisableFastShooting, fastShoot);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].DisableFastShooting);
			
			int turretThrust = items->getPropertySum(p, i, "turretthrust");
			if (turretThrust) clientset->PlayerOverride(p, shipOverrideKeys[i].TurretThrustPenalty, turretThrust);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].TurretThrustPenalty);
			
			int turretSpeed = items->getPropertySum(p, i, "turretspeed");
			if (turretSpeed) clientset->PlayerOverride(p, shipOverrideKeys[i].TurretSpeedPenalty, turretSpeed);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].TurretSpeedPenalty);
			
			data->usingPerShip[i] = checkUsingPerShip(p, i);
		}
	}
	
	//add globals if the ship uses them
	if (p->p_ship != SHIP_SPEC)
	{
		int bulletdamage = items->getPropertySum(p, p->p_ship, "bulletdamage") * 1000;
		if (bulletdamage) clientset->PlayerOverride(p, globalOverrideKeys.BulletDamageLevel, bulletdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.BulletDamageLevel);
		
		int bulletdamageup = items->getPropertySum(p, p->p_ship, "bulletdamageup") * 1000;
		if (bulletdamageup) clientset->PlayerOverride(p, globalOverrideKeys.BulletDamageUpgrade, bulletdamageup);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.BulletDamageUpgrade);
		
		int bombdamage = items->getPropertySum(p, p->p_ship, "bombdamage") * 1000;
		if (bombdamage) clientset->PlayerOverride(p, globalOverrideKeys.BombDamageLevel, bombdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.BombDamageLevel);
		
		int ebombtime = items->getPropertySum(p, p->p_ship, "ebombtime");
		if (ebombtime) clientset->PlayerOverride(p, globalOverrideKeys.EBombShutdownTime, ebombtime);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.EBombShutdownTime);
		
		int ebombdamage = items->getPropertySum(p, p->p_ship, "ebombdamage");
		if (ebombdamage) clientset->PlayerOverride(p, globalOverrideKeys.EBombDamagePercent, ebombdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.EBombDamagePercent);
		
		int bbombdamage = items->getPropertySum(p, p->p_ship, "bbombdamage");
		if (bbombdamage) clientset->PlayerOverride(p, globalOverrideKeys.BBombDamagePercent, bbombdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.BBombDamagePercent);
		
		int burstdamage = items->getPropertySum(p, p->p_ship, "burstdamage") * 1000;
		if (burstdamage) clientset->PlayerOverride(p, globalOverrideKeys.BurstDamageLevel, burstdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.BurstDamageLevel);
		
		int jittertime = items->getPropertySum(p, p->p_ship, "jittertime");
		if (jittertime) clientset->PlayerOverride(p, globalOverrideKeys.JitterTime, jittertime);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.JitterTime);
		
		globalOverrideKeys.JitterTime = clientset->GetOverrideKey("Bomb", "JitterTime");
		
		int decoyalive = items->getPropertySum(p, p->p_ship, "decoyalive");
		if (decoyalive) clientset->PlayerOverride(p, globalOverrideKeys.DecoyAliveTime, decoyalive);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.DecoyAliveTime);
		
		int warppointdelay = items->getPropertySum(p, p->p_ship, "warppointdelay");
		if (warppointdelay) clientset->PlayerOverride(p, globalOverrideKeys.WarpPointDelay, warppointdelay);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.WarpPointDelay);
		
		int rocketthrust = items->getPropertySum(p, p->p_ship, "rocketthrust");
		if (rocketthrust) clientset->PlayerOverride(p, globalOverrideKeys.RocketThrust, rocketthrust);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.RocketThrust);
		
		int rocketspeed = items->getPropertySum(p, p->p_ship, "rocketspeed");
		if (rocketspeed) clientset->PlayerOverride(p, globalOverrideKeys.RocketSpeed, rocketspeed);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.RocketSpeed);
		
		int inactshrapdamage = items->getPropertySum(p, p->p_ship, "inactshrapdamage") * 1000;
		if (inactshrapdamage) clientset->PlayerOverride(p, globalOverrideKeys.InactiveShrapDamage, inactshrapdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.InactiveShrapDamage);
		
		int shrapdamage = items->getPropertySum(p, p->p_ship, "shrapdamage");
		if (shrapdamage) clientset->PlayerOverride(p, globalOverrideKeys.ShrapnelDamagePercent, shrapdamage);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.ShrapnelDamagePercent);
		
		int mapzoom = items->getPropertySum(p, p->p_ship, "mapzoom");
		if (mapzoom) clientset->PlayerOverride(p, globalOverrideKeys.MapZoomFactor, mapzoom);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.MapZoomFactor);
		
		int flaggunup = items->getPropertySum(p, p->p_ship, "flaggunup");
		if (flaggunup) clientset->PlayerOverride(p, globalOverrideKeys.FlaggerGunUpgrade, flaggunup);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.FlaggerGunUpgrade);
		
		int flagbombup = items->getPropertySum(p, p->p_ship, "flagbombup");
		if (flagbombup) clientset->PlayerOverride(p, globalOverrideKeys.FlaggerBombUpgrade, flagbombup);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.FlaggerBombUpgrade);
		
		int soccerallowbombs = items->getPropertySum(p, p->p_ship, "soccerallowbombs");
		if (soccerallowbombs) clientset->PlayerOverride(p, globalOverrideKeys.AllowBombs, soccerallowbombs);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.AllowBombs);
		
		int soccerallowguns = items->getPropertySum(p, p->p_ship, "soccerallowguns");
		if (soccerallowguns) clientset->PlayerOverride(p, globalOverrideKeys.AllowGuns, soccerallowguns);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.AllowGuns);
		
		int socceruseflag = items->getPropertySum(p, p->p_ship, "socceruseflag");
		if (socceruseflag) clientset->PlayerOverride(p, globalOverrideKeys.UseFlagger, socceruseflag);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.UseFlagger);
		
		int soccerseeball = items->getPropertySum(p, p->p_ship, "soccerseeball");
		if (soccerseeball) clientset->PlayerOverride(p, globalOverrideKeys.BallLocation, soccerseeball);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.BallLocation);
		
		int doormode = items->getPropertySum(p, p->p_ship, "doormode");
		if (doormode) clientset->PlayerOverride(p, globalOverrideKeys.DoorMode, doormode);
		else clientset->PlayerUnoverride(p, globalOverrideKeys.DoorMode);
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
		//the player is entering the arena.
	}
	else if (action == PA_LEAVEARENA)
	{
		data->underOurControl = 0;
		removeOverrides(p);
	}
}

local void shipsLoadedCallback(Player *p)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	addOverrides(p);
	//send the packet the first time
	clientset->SendClientSettings(p);

	data->dirty = 0;
	data->currentShip = p->p_ship;
}

local int itemCountChangedTimer(void *clos)
{
	Player *p = clos;
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	data->dirty = 0;
	addOverrides(p);
	clientset->SendClientSettings(p);
	
	return FALSE;
}

local void itemCountChangedCallback(Player *p, Item *item, InventoryEntry *entry, int newCount, int oldCount) //called with lock held
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	if (item->resendSets)
	{
		//resend the settings in a callback to avoid a deadlock
		ml->SetTimer(itemCountChangedTimer, 0, 0, p, p);
	}
	else //check if it changed anything in clientset, and if it did, recompute and flag dirty
	{
		if (item->affectsSets)
		{
			data->dirty = 1;
			return;
		}

		//if we didn't recompute, then check if the player has anything changeable that uses this item as ammo.
		//if they do, recompute and flag dirty
		//NOTE: only need to do it if count was or is 0
		if (oldCount == 0 || newCount == 0)
		{
			Link *link;
			for (link = LLGetHead(database->getItemList()); link; link = link->next)
			{
				Item *linkItem = link->data;

				if (linkItem->ammo == item)
				{
					if (linkItem->affectsSets)
					{
						data->dirty = 1;
						return;
					}
				}
			}
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
		addOverrides(killed);
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
		addOverrides(p);
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

local int prizeTimerCallback(void *clos)
{
	PrizeData *prizeData = clos;
	Target t;
	t.type = T_PLAYER;
	t.u.p = prizeData->player;
	
	game->GivePrize(&t, prizeData->prizeNumber, prizeData->count);
	
	afree(clos);
	
	return FALSE;
}

local void handleItemCallback(Player *p, int ship, Item *item, int mult) //if add, mult = 1, if del, mult = -1
{
	Link *propLink;
	
	if (ship != p->p_ship)
	{
		//it's not on their current ship!
		return;
	}
	
	if (item == NULL)
	{
		lm->LogP(L_ERROR, "hscore_spawner", p, "NULL item in callback.");
		return;
	}
	
	for (propLink = LLGetHead(&item->propertyList); propLink; propLink = propLink->next)
	{
		Property *prop = propLink->data;
		const char *propName = prop->name;
		int prizeNumber = -1;
		
		if (prop->value == 0) return;
		
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
			prizeNumber = 11;
		else if (strcasecmp(propName, "speed") == 0)
			prizeNumber = 12;
		else if (strcasecmp(propName, "energy") == 0)
			prizeNumber = 2;
		else if (strcasecmp(propName, "recharge") == 0)
			prizeNumber = 1;
		else if (strcasecmp(propName, "rotation") == 0)
			prizeNumber = 3;
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
			PrizeData *prizeData = amalloc(sizeof(*prizeData));
			prizeData->player = p;
			prizeData->prizeNumber = mult*mult2*prizeNumber;
			prizeData->count = count;
			ml->SetTimer(prizeTimerCallback, 1, 1, prizeData, prizeData);
		}
	}	
}

local void ammoAddedCallback(Player *p, int ship, Item *ammoUser) //warnings: cache is out of sync, and lock is held
{
	lm->LogP(L_DRIVEL, "hscore_spawner", p, "Ammo added callback on %s", ammoUser->name);
	handleItemCallback(p, ship, ammoUser, 1);
}

local void ammoRemovedCallback(Player *p, int ship, Item *ammoUser) //warnings: cache is out of sync, and lock is held
{
	lm->LogP(L_DRIVEL, "hscore_spawner", p, "Ammo removed callback on %s", ammoUser->name);
	handleItemCallback(p, ship, ammoUser, -1);
}

local void triggerEventCallback(Player *p, Item *item, int ship, const char *eventName) //called with lock held
{
	if (strcasecmp(eventName, "add") == 0)
	{
		lm->LogP(L_DRIVEL, "hscore_spawner", p, "Item added callback on %s", item->name);
		handleItemCallback(p, ship, item, 1);
	}
	else if (strcasecmp(eventName, "del") == 0)
	{
		lm->LogP(L_DRIVEL, "hscore_spawner", p, "Item del callback on %s", item->name);
		handleItemCallback(p, ship, item, -1);
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

local void HSItemReloadCallback(void)
{
	Player *p;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		addOverrides(p);
		clientset->SendClientSettings(p);
	}
	pd->Unlock();
}

local Ihscorespawner interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_SPAWNER, "hscore_spawner")
	respawn,
};

EXPORT const char info_hscore_spawner[] = "v1.0 Dr Brain <drbrain@gmail.com>";

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

		if (!lm || !pd || !net || !game || !chat || !cfg || !items || !clientset || !database || !ml)
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

		pd->FreePlayerData(playerDataKey);

		ml->ClearTimer(itemCountChangedTimer, NULL);	
		ml->ClearTimer(prizeTimerCallback, NULL);		
		
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

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegInterface(&interface, ALLARENAS);

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
		if (mm->UnregInterface(&interface, ALLARENAS))
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



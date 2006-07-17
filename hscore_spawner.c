

#include <string.h>

#include "asss.h"
#include "clientset.h"
#include "hscore.h"
#include "hscore_storeman.h"
#include "hscore_database.h"
#include "hscore_spawner.h"
#include "hscore_shipnames.h"
#include "fg_wz.h"

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

//interface function
local void respawn(Player *p);

local void spawnPlayer(Player *p)
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);
	Target t;
	int bounce, prox, multifire, shrapnel;
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
	}
}

local void addOverrides(Player *p)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
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
			if (gunlevel) clientset->PlayerOverride(p, shipOverrideKeys[i].InitialGuns, gunlevel);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].InitialGuns);

			int bomblevel = items->getPropertySum(p, i, "bomblevel");
			if (bomblevel) clientset->PlayerOverride(p, shipOverrideKeys[i].InitialBombs, bomblevel);
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

			int maxthrust = items->getPropertySum(p, i, "maxthrust");
			if (maxthrust) clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumThrust, maxthrust);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumThrust);

			int maxspeed = items->getPropertySum(p, i, "maxspeed");
			if (maxspeed) clientset->PlayerOverride(p, shipOverrideKeys[i].MaximumSpeed, maxspeed);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].MaximumSpeed);

			int afterburner = items->getPropertySum(p, i, "afterburner");
			if (afterburner) clientset->PlayerOverride(p, shipOverrideKeys[i].AfterburnerEnergy, afterburner);
			else clientset->PlayerUnoverride(p, shipOverrideKeys[i].AfterburnerEnergy);

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
		}
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

}

local void itemCountChangedCallback(Player *p, Item *item, InventoryEntry *entry, int newCount, int oldCount) //called with lock held
{
	PlayerDataStruct *data = PPDATA(p, playerDataKey);

	if (item->resendSets)
	{
		//resend the settings now
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

	if (data->dirty == 1)
	{
		data->dirty = 0;
		clientset->SendClientSettings(p);
	}
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

		mm->RegCallback(CB_HS_ITEMRELOAD, HSItemReloadCallback, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->UnregCallback(CB_HS_ITEMRELOAD, HSItemReloadCallback, ALLARENAS);

		net->RemovePacket(C2S_POSITION, Pppk);

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

		return MM_OK;
	}
	return MM_FAIL;
}



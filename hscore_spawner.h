#ifndef HSCORE_SPAWNER_H
#define HSCORE_SPAWNER_H

#define I_HSCORE_SPAWNER "hscore_spawner-3"

typedef struct Ihscorespawner
{
	INTERFACE_HEAD_DECL

	void (*respawn)(Player *p); //called when the player has just been shipreset
	int (*getFullEnergy)(Player *p);
	void (*ignorePrize)(Player *p, int prize);
} Ihscorespawner;

#endif //HSCORE_SPAWNER_H

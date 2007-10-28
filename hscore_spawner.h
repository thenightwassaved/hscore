#ifndef HSCORE_SPAWNER_H
#define HSCORE_SPAWNER_H

#define I_HSCORE_SPAWNER "hscore_spawner-2"

typedef struct Ihscorespawner
{
	INTERFACE_HEAD_DECL

	void (*respawn)(Player *p); //called when the player has just been shipreset
	int (*getFullEnergy)(Player *p);
} Ihscorespawner;

#endif //HSCORE_SPAWNER_H

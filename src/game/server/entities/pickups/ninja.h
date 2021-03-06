/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PICKUPS_NINJA_H
#define GAME_SERVER_ENTITIES_PICKUPS_NINJA_H

#include <game/server/entities/pickup.h>

class CPickupNinja : public CPickup
{
protected:
	/* Pickup functions */
	virtual bool OnPickup(class CCharacter *pChar);

public:
	/* Constructor */
	CPickupNinja(CGameWorld *pGameWorld, vec2 Pos);
};

#endif

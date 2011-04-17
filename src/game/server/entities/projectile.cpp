/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "projectile.h"

CProjectile::CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Type = Type;
	m_Pos = Pos;
	m_Direction = Dir;
	m_LifeSpan = Span;
	m_Owner = Owner;
	m_Force = Force;
	m_Damage = Damage;
	m_SoundImpact = SoundImpact;
	m_Weapon = Weapon;
	m_StartTick = Server()->Tick();
	m_Explosive = Explosive;
	m_Curvature = GameServer()->Tuning()->m_FlashGrenadeCurvature;
	m_Speed = GameServer()->Tuning()->m_FlashGrenadeSpeed;

	GameWorld()->InsertEntity(this);
}

void CProjectile::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CProjectile::GetPos(float Time)
{
	float Curvature = 0;
	float Speed = 0;
	
	switch(m_Type)
	{
		case WEAPON_GRENADE:
			Curvature = GameServer()->Tuning()->m_GrenadeCurvature;
			Speed = GameServer()->Tuning()->m_GrenadeSpeed;
			break;
			
		case WEAPON_SHOTGUN:
			Curvature = GameServer()->Tuning()->m_ShotgunCurvature;
			Speed = GameServer()->Tuning()->m_ShotgunSpeed;
			break;
			
		case WEAPON_GUN:
			Curvature = GameServer()->Tuning()->m_GunCurvature;
			Speed = GameServer()->Tuning()->m_GunSpeed;
			break;
			
		case WEAPON_FLASHGRENADE:
			Curvature = m_Curvature;
			Speed = m_Speed;
			break;
	}
	
	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


void CProjectile::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0);
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, m_Weapon == WEAPON_FLASHGRENADE ? 8.0f : 6.0f, CurPos, OwnerChar);

	m_LifeSpan--;
	//dbg_msg("", "collide %d, speed %d, pos %f %f, difftick %d", Collide, m_Speed, CurPos.x, CurPos.y, Server()->Tick()-m_StartTick);
	
	if(TargetChr || Collide || m_LifeSpan < 0 || GameLayerClipped(CurPos))
	{
		if(Collide && m_Weapon == WEAPON_FLASHGRENADE) // bounce
		{
			vec2 Vel = GetPos(Ct)-PrevPos;
			if(GameServer()->Collision()->GetCollisionAt(PrevPos.x, CurPos.y)&CCollision::COLFLAG_SOLID)
				Vel.y *= -1.0f;
			if(GameServer()->Collision()->GetCollisionAt(CurPos.x, PrevPos.y)&CCollision::COLFLAG_SOLID)
				Vel.x *= -1.0f;
			Vel.x *= 0.75f;
			m_Pos = PrevPos;
			m_Direction = normalize(Vel);
			m_StartTick = Server()->Tick();
			m_Curvature *= 1.8f;
			m_Speed *= 0.7f;
			if(m_Speed > 100.0f)
				return;
		}
		
		if(m_LifeSpan >= 0 || m_Weapon == WEAPON_GRENADE || m_Weapon == WEAPON_FLASHGRENADE)
			GameServer()->CreateSound(CurPos, m_SoundImpact);

		if(m_Explosive && m_Weapon == WEAPON_GRENADE)
			GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, false);
		else if(m_Explosive && m_Weapon == WEAPON_FLASHGRENADE)
			GameServer()->CreateFlash(CurPos);
			
		else if(TargetChr)
			TargetChr->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, m_Weapon);

		GameServer()->m_World.DestroyEntity(this);
	}
}

void CProjectile::FillInfo(CNetObj_Projectile *pProj)
{
	pProj->m_X = (int)m_Pos.x;
	pProj->m_Y = (int)m_Pos.y;
	pProj->m_VelX = (int)(m_Direction.x*100.0f);
	pProj->m_VelY = (int)(m_Direction.y*100.0f);
	pProj->m_Curvature = (int)(m_Curvature*100.0f);
	pProj->m_Speed = (int)(m_Speed*100.0f);
	pProj->m_StartTick = m_StartTick;
	pProj->m_Type = m_Type;
}

void CProjectile::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	
	if(NetworkClipped(SnappingClient, GetPos(Ct)))
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(pProj)
		FillInfo(pProj);
}

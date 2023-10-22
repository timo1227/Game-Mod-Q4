#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

const int SHOTGUN_MOD_AMMO = BIT(0);

class rvWeaponMP155 : public rvWeapon {
public:

	CLASS_PROTOTYPE( rvWeaponMP155 );

	rvWeaponMP155 ( void );

	virtual void			Spawn				( void );
	void					Save				( idSaveGame *savefile ) const;
	void					Restore				( idRestoreGame *savefile );
	void					PreSave				( void );
	void					PostSave			( void );

protected:
	int						hitscans;

	bool				UpdateSuppressor(void);
	void				Suppressor(bool on);

	bool				UpdateCompensator(void);
	void				Compensator(bool on);

	bool				UpdateBarrel(void);
	void				Barrel(bool on);

private:

	stateResult_t		State_Idle		( const stateParms_t& parms );
	stateResult_t		State_Fire		( const stateParms_t& parms );
	stateResult_t		State_Reload	( const stateParms_t& parms );
	stateResult_t		State_Suppressor(const stateParms_t& parms);
	stateResult_t		State_Compensator(const stateParms_t& parms);
	stateResult_t		State_Barrel(const stateParms_t& parms);
	
	CLASS_STATES_PROTOTYPE( rvWeaponMP155 );
};

CLASS_DECLARATION( rvWeapon, rvWeaponMP155 )
END_CLASS

/*
================
rvWeaponMP155::rvWeaponMP155
================
*/
rvWeaponMP155::rvWeaponMP155( void ) {
}

/*
================
rvWeaponMP155::Spawn
================
*/
void rvWeaponMP155::Spawn( void ) {
	hitscans   = spawnArgs.GetFloat( "hitscans" );
	
	SetState( "Raise", 0 );	

	Suppressor(owner->IsSuppressorOn());
	Compensator(owner->IsCompensatorOn());
	Barrel(owner->IsBarrelOn());
}

/*
================
rvWeaponMP155::Save
================
*/
void rvWeaponMP155::Save( idSaveGame *savefile ) const {
}

/*
================
rvWeaponMP155::Restore
================
*/
void rvWeaponMP155::Restore( idRestoreGame *savefile ) {
	hitscans   = spawnArgs.GetFloat( "hitscans" );
}

/*
================
rvWeaponMP155::PreSave
================
*/
void rvWeaponMP155::PreSave ( void ) {
}

/*
================
rvWeaponMP155::PostSave
================
*/
void rvWeaponMP155::PostSave ( void ) {
}


/*
================
rvWeaponMP155::UpdateSuppressor
================
*/
bool rvWeaponMP155::UpdateSuppressor(void) {
	if (!wsfl.suppressor) {
		return false;
	}

	SetState("Suppressor", 0);
	return true;
}

/*
================
rvWeaponMP155::Suppressor
================
*/
void rvWeaponMP155::Suppressor(bool on) {
	owner->Suppressor(on);
}

/*
================
rvWeaponMP155::UpdateCompensator
================
*/
bool rvWeaponMP155::UpdateCompensator(void) {
	if (!wsfl.compensator) {
		return false;
	}

	SetState("Compensator", 0);
	return true;
}

/*
================
rvWeaponMP155::Compensator
================
*/
void rvWeaponMP155::Compensator(bool on) {
	owner->Compensator(on);
}

/*
================
rvWeaponMP155::UpdateBarrel
================
*/
bool rvWeaponMP155::UpdateBarrel(void) {
	if (!wsfl.barrel) {
		return false;
	}

	SetState("Barrel", 0);
	return true;
}

/*
================
rvWeaponMP155::Barrel
================
*/
void rvWeaponMP155::Barrel(bool on) {
	owner->Barrel(on);
}


/*
===============================================================================

	States 

===============================================================================
*/

CLASS_STATES_DECLARATION( rvWeaponMP155 )
	STATE( "Idle",				rvWeaponMP155::State_Idle)
	STATE( "Fire",				rvWeaponMP155::State_Fire )
	STATE( "Reload",			rvWeaponMP155::State_Reload )
	STATE("Suppressor",			rvWeaponMP155::State_Suppressor)
	STATE("Compensator",		rvWeaponMP155::State_Compensator)
	STATE("Barrel",				rvWeaponMP155::State_Barrel)
END_CLASS_STATES

/*
================
rvWeaponMP155::State_Idle
================
*/
stateResult_t rvWeaponMP155::State_Idle( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( !AmmoAvailable( ) ) {
				SetStatus( WP_OUTOFAMMO );
			} else {
				SetStatus( WP_READY );
			}
		
			PlayCycle( ANIMCHANNEL_ALL, "idle", parms.blendFrames );
			return SRESULT_STAGE ( STAGE_WAIT );
		
		case STAGE_WAIT:			
			if ( wsfl.lowerWeapon ) {
				SetState( "Lower", 4 );
				return SRESULT_DONE;
			}			
			if ( !clipSize ) {
				if ( gameLocal.time > nextAttackTime && wsfl.attack && AmmoAvailable ( ) ) {
					SetState( "Fire", 0 );
					return SRESULT_DONE;
				}  
			} else {				
				if ( gameLocal.time > nextAttackTime && wsfl.attack && AmmoInClip ( ) ) {
					SetState( "Fire", 0 );
					return SRESULT_DONE;
				}  
				if ( wsfl.attack && AutoReload() && !AmmoInClip ( ) && AmmoAvailable () ) {
					SetState( "Reload", 4 );
					return SRESULT_DONE;			
				}
				if ( wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable()>AmmoInClip()) ) {
					SetState( "Reload", 4 );
					return SRESULT_DONE;			
				}				
			}
			if (UpdateSuppressor()) {
				return SRESULT_DONE;
			}
			if (UpdateCompensator()) {
				return SRESULT_DONE;
			}
			if (UpdateBarrel()) {
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponMP155::State_Fire
================
*/
stateResult_t rvWeaponMP155::State_Fire( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier ( PMOD_FIRERATE ));
			Attack( false, hitscans, spread, 0, 1.0f );

			if (!owner->IsSuppressorOn()) {
				PlayAnim(ANIMCHANNEL_ALL, "fire", 0);
			}
			
			return SRESULT_STAGE( STAGE_WAIT );
	
		case STAGE_WAIT:
			if ( (!gameLocal.isMultiplayer && (wsfl.lowerWeapon || AnimDone( ANIMCHANNEL_ALL, 0 )) ) || AnimDone( ANIMCHANNEL_ALL, 0 ) ) {
				SetState( "Idle", 0 );
				return SRESULT_DONE;
			}									
			if ( wsfl.attack && gameLocal.time >= nextAttackTime && AmmoInClip() ) {
				SetState( "Fire", 0 );
				return SRESULT_DONE;
			}
			if ( clipSize ) {
				if ( (wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable()>AmmoInClip())) ) {
					SetState( "Reload", 4 );
					return SRESULT_DONE;			
				}				
			}
			if (UpdateSuppressor()) {
				return SRESULT_DONE;
			}
			if (UpdateCompensator()) {
				return SRESULT_DONE;
			}
			if (UpdateBarrel()) {
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponMP155::State_Reload
================
*/
stateResult_t rvWeaponMP155::State_Reload ( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
		STAGE_RELOADSTARTWAIT,
		STAGE_RELOADLOOP,
		STAGE_RELOADLOOPWAIT,
		STAGE_RELOADDONE,
		STAGE_RELOADDONEWAIT
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( wsfl.netReload ) {
				wsfl.netReload = false;
			} else {
				NetReload ( );
			}
			
			SetStatus ( WP_RELOAD );
			
			if ( mods & SHOTGUN_MOD_AMMO ) {				
				PlayAnim ( ANIMCHANNEL_ALL, "reload_clip", parms.blendFrames );
			} else {
				PlayAnim ( ANIMCHANNEL_ALL, "reload_start", parms.blendFrames );
				return SRESULT_STAGE ( STAGE_RELOADSTARTWAIT );
			}
			return SRESULT_STAGE ( STAGE_WAIT );
			
		case STAGE_WAIT:
			if ( AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				AddToClip ( ClipSize() );
				SetState ( "Idle", 4 );
				return SRESULT_DONE;
			}
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
			
		case STAGE_RELOADSTARTWAIT:
			if ( AnimDone ( ANIMCHANNEL_ALL, 0 ) ) {
				return SRESULT_STAGE ( STAGE_RELOADLOOP );
			}
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
			
		case STAGE_RELOADLOOP:		
			if ( (wsfl.attack && AmmoInClip() ) || AmmoAvailable ( ) <= AmmoInClip ( ) || AmmoInClip() == ClipSize() ) {
				return SRESULT_STAGE ( STAGE_RELOADDONE );
			}
			PlayAnim ( ANIMCHANNEL_ALL, "reload_loop", 0 );
			return SRESULT_STAGE ( STAGE_RELOADLOOPWAIT );
			
		case STAGE_RELOADLOOPWAIT:
			if ( (wsfl.attack && AmmoInClip() ) || wsfl.netEndReload ) {
				return SRESULT_STAGE ( STAGE_RELOADDONE );
			}
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			if ( AnimDone ( ANIMCHANNEL_ALL, 0 ) ) {
				AddToClip( 1 );
				return SRESULT_STAGE ( STAGE_RELOADLOOP );
			}
			return SRESULT_WAIT;
		
		case STAGE_RELOADDONE:
			NetEndReload ( );
			PlayAnim ( ANIMCHANNEL_ALL, "reload_end", 0 );
			return SRESULT_STAGE ( STAGE_RELOADDONEWAIT );

		case STAGE_RELOADDONEWAIT:
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			if ( wsfl.attack && AmmoInClip ( ) && gameLocal.time > nextAttackTime ) {
				SetState ( "Fire", 0 );
				return SRESULT_DONE;
			}
			if ( AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				SetState ( "Idle", 4 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;	
}
			
/*
================
rvWeaponMP155::State_Suppressor
================
*/
stateResult_t rvWeaponMP155::State_Suppressor(const stateParms_t& parms) {
	enum {
		SUPPRESSOR_INIT,
		SUPPRESSOR_WAIT,
	};
	switch (parms.stage) {
	case SUPPRESSOR_INIT:
		SetStatus(WP_SUPPRESSOR);
		// Wait for the suppressor anim to play		
		PlayAnim(ANIMCHANNEL_ALL, "suppresor", 0);
		return SRESULT_STAGE(SUPPRESSOR_WAIT);

	case SUPPRESSOR_WAIT:
		if (!AnimDone(ANIMCHANNEL_ALL, 4)) {
			return SRESULT_WAIT;
		}

		if (owner->IsSuppressorOn()) {
			Suppressor(false);
		}
		else {
			Suppressor(true);
		}

		SetState("Idle", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponMP155::State_Compensator
================
*/
stateResult_t rvWeaponMP155::State_Compensator(const stateParms_t& parms) {
	enum {
		COMPENSATOR_INIT,
		COMPENSATOR_WAIT,
	};

	switch (parms.stage) {
	case COMPENSATOR_INIT:
		SetStatus(WP_COMPENSATOR);
		// Wait for the compensator anim to play		
		PlayAnim(ANIMCHANNEL_ALL, "compensator", 0);
		return SRESULT_STAGE(COMPENSATOR_WAIT);

	case COMPENSATOR_WAIT:
		if (!AnimDone(ANIMCHANNEL_ALL, 4)) {
			return SRESULT_WAIT;
		}

		if (owner->IsCompensatorOn()) {
			Compensator(false);
		}
		else {
			Compensator(true);
		}

		SetState("Idle", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponMP155::State_Barrel
================
*/
stateResult_t rvWeaponMP155::State_Barrel(const stateParms_t& parms) {
	enum {
		BARREL_INIT,
		BARREL_WAIT,
	};

	switch (parms.stage) {
	case BARREL_INIT:
		SetStatus(WP_BARREL);
		// Wait for the barrel anim to play		
		PlayAnim(ANIMCHANNEL_ALL, "barrel", 0);
		return SRESULT_STAGE(BARREL_WAIT);

	case BARREL_WAIT:
		if (!AnimDone(ANIMCHANNEL_ALL, 4)) {
			return SRESULT_WAIT;
		}

		if (owner->IsBarrelOn()) {
			Barrel(false);
		}
		else {
			Barrel(true);
		}

		SetState("Idle", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}
#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

class rvWeaponM4A1 : public rvWeapon {
public:

	CLASS_PROTOTYPE( rvWeaponM4A1 );

	rvWeaponM4A1 ( void );

	virtual void		Spawn				( void );
	virtual void		Think				( void );
	void				Save				( idSaveGame *savefile ) const;
	void				Restore				( idRestoreGame *savefile );
	void					PreSave				( void );
	void					PostSave			( void );

protected:

	float				spreadZoom;
	bool				fireHeld;

	bool				UpdateFlashlight	( void );
	void				Flashlight			( bool on );

	bool				UpdateSuppressor	( void );
	void				Suppressor			( bool on );

	bool				UpdateCompensator	( void );
	void				Compensator			( bool on );

private:

	stateResult_t		State_Idle			( const stateParms_t& parms );
	stateResult_t		State_Fire			( const stateParms_t& parms );
	stateResult_t		State_Reload		( const stateParms_t& parms );
	stateResult_t		State_Flashlight	( const stateParms_t& parms );
	stateResult_t		State_Suppressor	( const stateParms_t& parms );
	stateResult_t		State_Compensator	( const stateParms_t& parms );

	CLASS_STATES_PROTOTYPE ( rvWeaponM4A1 );
};

CLASS_DECLARATION( rvWeapon, rvWeaponM4A1 )
END_CLASS

/*
================
rvWeaponM4A1::rvWeaponM4A1
================
*/
rvWeaponM4A1::rvWeaponM4A1 ( void ) {
}

/*
================
rvWeaponM4A1::Spawn
================
*/
void rvWeaponM4A1::Spawn ( void ) {
	spreadZoom = spawnArgs.GetFloat ( "spreadZoom" );
	fireHeld   = false;
		
	SetState ( "Raise", 0 );	
	
	Flashlight ( owner->IsFlashlightOn() );
	Suppressor ( owner->IsSuppressorOn() );
	Compensator( owner->IsCompensatorOn() );
}

/*
================
rvWeaponM4A1::Save
================
*/
void rvWeaponM4A1::Save ( idSaveGame *savefile ) const {
	savefile->WriteFloat ( spreadZoom );
	savefile->WriteBool ( fireHeld );
}

/*
================
rvWeaponM4A1::Restore
================
*/
void rvWeaponM4A1::Restore ( idRestoreGame *savefile ) {
	savefile->ReadFloat ( spreadZoom );
	savefile->ReadBool ( fireHeld );
}

/*
================
rvWeaponM4A1::PreSave
================
*/
void rvWeaponM4A1::PreSave ( void ) {
}

/*
================
rvWeaponM4A1::PostSave
================
*/
void rvWeaponM4A1::PostSave ( void ) {
}


/*
================
rvWeaponM4A1::Think
================
*/
void rvWeaponM4A1::Think()
{
	rvWeapon::Think();
	if ( zoomGui && owner == gameLocal.GetLocalPlayer( ) ) {
		zoomGui->SetStateFloat( "playerYaw", playerViewAxis.ToAngles().yaw );
	}
}

/*
================
rvWeaponM4A1::UpdateFlashlight
================
*/
bool rvWeaponM4A1::UpdateFlashlight ( void ) {
	if ( !wsfl.flashlight ) {
		return false;
	}
	
	SetState ( "Flashlight", 0 );
	return true;		
}

/*
================
rvWeaponM4A1::Flashlight
================
*/
void rvWeaponM4A1::Flashlight ( bool on ) {
	owner->Flashlight ( on );
	
	if ( on ) {
		viewModel->ShowSurface ( "models/weapons/blaster/flare" );
		worldModel->ShowSurface ( "models/weapons/blaster/flare" );
	} else {
		viewModel->HideSurface ( "models/weapons/blaster/flare" );
		worldModel->HideSurface ( "models/weapons/blaster/flare" );
	}
}

/*
================
rvWeaponM4A1::UpdateSuppressor
================
*/
bool rvWeaponM4A1::UpdateSuppressor(void) {
	if (!wsfl.suppressor) {
		return false;
	}

	SetState ( "Suppressor", 0 );
	return true;
}

/*
================
rvWeaponM4A1::Suppressor
================
*/
void rvWeaponM4A1::Suppressor(bool on) {
	owner->Suppressor ( on );
}

/*
================
rvWeaponM4A1::UpdateCompensator
================
*/
bool rvWeaponM4A1::UpdateCompensator( void ) {
	if (!wsfl.compensator) {
		return false;
	}

	SetState ( "Compensator", 0 );
	return true;
}

/*
================
rvWeaponM4A1::Compensator
================
*/
void rvWeaponM4A1::Compensator( bool on ) {
	owner->Compensator ( on );
}

/*
===============================================================================

	States 

===============================================================================
*/

CLASS_STATES_DECLARATION ( rvWeaponM4A1 )
	STATE ( "Idle",				rvWeaponM4A1::State_Idle)
	STATE ( "Fire",				rvWeaponM4A1::State_Fire )
	STATE ( "Reload",			rvWeaponM4A1::State_Reload )
	STATE ( "Flashlight",		rvWeaponM4A1::State_Flashlight )
	STATE ( "Suppressor",		rvWeaponM4A1::State_Suppressor )
	STATE ( "Compensator",		rvWeaponM4A1::State_Compensator )
END_CLASS_STATES

/*
================
rvWeaponM4A1::State_Idle
================
*/
stateResult_t rvWeaponM4A1::State_Idle( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( !AmmoAvailable ( ) ) {
				SetStatus ( WP_OUTOFAMMO );
			} else {
				SetStatus ( WP_READY );
			}
		
			PlayCycle( ANIMCHANNEL_ALL, "idle", parms.blendFrames );
			return SRESULT_STAGE ( STAGE_WAIT );
		
		case STAGE_WAIT:			
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}		
			if ( UpdateFlashlight ( ) ) {
				return SRESULT_DONE;
			}
			if ( UpdateSuppressor ( ) ) {
				return SRESULT_DONE;
			}
			if ( UpdateCompensator ( ) ) {
				return SRESULT_DONE;
			}

			if ( fireHeld && !wsfl.attack ) {
				fireHeld = false;
			}
			if ( !clipSize ) {
				if ( !fireHeld && gameLocal.time > nextAttackTime && wsfl.attack && AmmoAvailable ( ) ) {
					SetState ( "Fire", 0 );
					return SRESULT_DONE;
				}
			} else {
				if ( !fireHeld && gameLocal.time > nextAttackTime && wsfl.attack && AmmoInClip ( ) ) {
					SetState ( "Fire", 0 );
					return SRESULT_DONE;
				}  
				if ( wsfl.attack && AutoReload() && !AmmoInClip ( ) && AmmoAvailable () ) {
					SetState ( "Reload", 4 );
					return SRESULT_DONE;			
				}
				if ( wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable()>AmmoInClip()) ) {
					SetState ( "Reload", 4 );
					return SRESULT_DONE;			
				}				
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponM4A1::State_Fire
================
*/
stateResult_t rvWeaponM4A1::State_Fire ( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	

	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( wsfl.zoom ) {
				nextAttackTime = gameLocal.time + (altFireRate * owner->PowerUpModifier ( PMOD_FIRERATE ));
				Attack ( true, 1, spreadZoom, 0, 1.0f );
				fireHeld = true;
			} else {
				nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier ( PMOD_FIRERATE ));
				Attack ( false, 1, spread, 0, 1.0f );
			}
			if (!owner->IsSuppressorOn()) {
				PlayAnim(ANIMCHANNEL_ALL, "fire", 0);
			}
			return SRESULT_STAGE ( STAGE_WAIT );
	
		case STAGE_WAIT:		
			if ( !fireHeld && wsfl.attack && gameLocal.time >= nextAttackTime && AmmoInClip() && !wsfl.lowerWeapon ) {
				SetState ( "Fire", 0 );
				return SRESULT_DONE;
			}
			if ( AnimDone ( ANIMCHANNEL_ALL, 0 ) ) {
				SetState ( "Idle", 0 );
				return SRESULT_DONE;
			}		
			if ( UpdateFlashlight ( ) ) {
				return SRESULT_DONE;
			}
			if ( UpdateSuppressor ( ) ) {
				return SRESULT_DONE;
			}
			if ( UpdateCompensator ( ) ) {
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponM4A1::State_Reload
================
*/
stateResult_t rvWeaponM4A1::State_Reload ( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( wsfl.netReload ) {
				wsfl.netReload = false;
			} else {
				NetReload ( );
			}
			
			SetStatus ( WP_RELOAD );
			PlayAnim ( ANIMCHANNEL_ALL, "reload", parms.blendFrames );
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
	}
	return SRESULT_ERROR;
}
			

/*
================
rvWeaponM4A1::State_Flashlight
================
*/
stateResult_t rvWeaponM4A1::State_Flashlight ( const stateParms_t& parms ) {
	enum {
		FLASHLIGHT_INIT,
		FLASHLIGHT_WAIT,
	};	
	switch ( parms.stage ) {
		case FLASHLIGHT_INIT:			
			SetStatus ( WP_FLASHLIGHT );
			// Wait for the flashlight anim to play		
			PlayAnim( ANIMCHANNEL_ALL, "flashlight", 0 );
			return SRESULT_STAGE ( FLASHLIGHT_WAIT );
			
		case FLASHLIGHT_WAIT:
			if ( !AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				return SRESULT_WAIT;
			}
			
			if ( owner->IsFlashlightOn() ) {
				Flashlight ( false );
			} else {
				Flashlight ( true );
			}
			
			SetState ( "Idle", 4 );
			return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponM4A1::State_Suppressor
================
*/
stateResult_t rvWeaponM4A1::State_Suppressor(const stateParms_t& parms) {
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
rvWeaponM4A1::State_Compensator
================
*/
stateResult_t rvWeaponM4A1::State_Compensator(const stateParms_t& parms) {
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
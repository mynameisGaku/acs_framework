// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimation3DProfile.h"

#include <cmath>

namespace
{
	/** クリップ名に1文字以上あるか返す。 */
	bool HasText( const FString& Text ) noexcept
	{
		return !Text.IsEmpty();
	}

	/** この規則が扱う4状態か返す。 */
	bool IsLocomotionState( EAnimationGraphState State ) noexcept
	{
		switch ( State )
		{
		case EAnimationGraphState::Idle:
		case EAnimationGraphState::Walk:
		case EAnimationGraphState::Run:
		case EAnimationGraphState::Jump:
			return true;
		default:
			return false;
		}
	}
}


bool FCharacterAnimation3DProfile::TrySelectState( const FCharacterAnimation3DInput& Input,
	EAnimationGraphState CurrentState, EAnimationGraphState& OutState ) const noexcept
{
	if ( !Input.IsValid() || !IsValid() || !IsLocomotionState( CurrentState ) ) return false;

	EAnimationGraphState Candidate = EAnimationGraphState::Idle;
	if ( !Input.bGrounded ) Candidate = EAnimationGraphState::Jump;
	else if ( CurrentState == EAnimationGraphState::Run )
	{
		if ( Input.HorizontalSpeed >= RunExitSpeed ) Candidate = EAnimationGraphState::Run;
		else if ( Input.HorizontalSpeed > WalkExitSpeed ) Candidate = EAnimationGraphState::Walk;
	}
	else if ( CurrentState == EAnimationGraphState::Walk )
	{
		if ( Input.HorizontalSpeed >= RunEnterSpeed ) Candidate = EAnimationGraphState::Run;
		else if ( Input.HorizontalSpeed > WalkExitSpeed ) Candidate = EAnimationGraphState::Walk;
	}
	else
	{
		if ( Input.HorizontalSpeed >= RunEnterSpeed ) Candidate = EAnimationGraphState::Run;
		else if ( Input.HorizontalSpeed >= WalkEnterSpeed ) Candidate = EAnimationGraphState::Walk;
	}

	OutState = Candidate;
	return true;
}


FStringView FCharacterAnimation3DProfile::ClipFor( EAnimationGraphState State ) const noexcept
{
	switch ( State )
	{
	case EAnimationGraphState::Idle: return IdleClip.View();
	case EAnimationGraphState::Walk: return WalkClip.View();
	case EAnimationGraphState::Run:  return RunClip.View();
	case EAnimationGraphState::Jump: return JumpClip.View();
	default:                         return FStringView();
	}
}


bool FCharacterAnimation3DProfile::LoopsFor( EAnimationGraphState State ) const noexcept
{
	switch ( State )
	{
	case EAnimationGraphState::Idle:
	case EAnimationGraphState::Walk:
	case EAnimationGraphState::Run:
		return true;
	case EAnimationGraphState::Jump:
		return bLoopJump;
	default:
		return false;
	}
}


bool FCharacterAnimation3DProfile::IsValid() const noexcept
{
	if ( !HasText( IdleClip ) || !HasText( WalkClip ) || !HasText( RunClip ) || !HasText( JumpClip ) ) return false;
	if ( !std::isfinite( WalkEnterSpeed ) || !std::isfinite( WalkExitSpeed ) ) return false;
	if ( !std::isfinite( RunEnterSpeed ) || !std::isfinite( RunExitSpeed ) ) return false;
	if ( !std::isfinite( BlendSeconds ) || BlendSeconds < 0.0f ) return false;
	if ( WalkExitSpeed < 0.0f || WalkExitSpeed > WalkEnterSpeed ) return false;
	if ( WalkEnterSpeed > RunExitSpeed || RunExitSpeed > RunEnterSpeed ) return false;
	return true;
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DActionSet.h"

#include <cmath>

namespace
{
	/** 操作軸を第三者視点入力の許容範囲へ制限する。 */
	f32 ClampInputAxis( f32 Value ) noexcept
	{
		if ( Value < -1.0f ) return -1.0f;
		if ( Value > 1.0f ) return 1.0f;
		return Value;
	}

	/** 4個の軸番号が全て範囲内かつ互いに異なるか返す。 */
	bool AreAxesValid( u32 First, u32 Second, u32 Third, u32 Fourth ) noexcept
	{
		return First < kActionAxisCount && Second < kActionAxisCount && Third < kActionAxisCount && Fourth < kActionAxisCount && First != Second && First != Third && First != Fourth && Second != Third && Second != Fourth && Third != Fourth;
	}

	/** 3個のアクション番号が全て範囲内かつ互いに異なるか返す。 */
	bool AreActionsValid( u32 First, u32 Second, u32 Third ) noexcept
	{
		return First < kActionButtonCount && Second < kActionButtonCount && Third < kActionButtonCount && First != Second && First != Third && Second != Third;
	}
}


bool FThirdPersonCharacter3DActionSet::IsValid() const noexcept
{
	return AreAxesValid( MoveRightAxis, MoveForwardAxis, LookYawAxis, LookPitchAxis ) && AreActionsValid( JumpAction, ZoomInAction, ZoomOutAction );
}


bool FThirdPersonCharacter3DActionSet::TryEvaluate( const FActionInput& CurrentInput, const FActionInput& PreviousInput, FThirdPersonCharacter3DInput& OutInput ) const noexcept
{
	if ( !IsValid() ) return false;

	const f32 MoveRight = CurrentInput.GetAxis( MoveRightAxis );
	const f32 MoveForward = CurrentInput.GetAxis( MoveForwardAxis );
	const f32 LookYaw = CurrentInput.GetAxis( LookYawAxis );
	const f32 LookPitch = CurrentInput.GetAxis( LookPitchAxis );
	if ( !std::isfinite( MoveRight ) || !std::isfinite( MoveForward ) || !std::isfinite( LookYaw ) || !std::isfinite( LookPitch ) ) return false;

	const bool bZoomIn = CurrentInput.IsDown( ZoomInAction );
	const bool bZoomOut = CurrentInput.IsDown( ZoomOutAction );
	FThirdPersonCharacter3DInput Evaluated;
	Evaluated.MoveAxes = FVec2{ ClampInputAxis( MoveRight ), ClampInputAxis( MoveForward ) };
	Evaluated.LookAxes = FVec2{ ClampInputAxis( LookYaw ), ClampInputAxis( LookPitch ) };
	Evaluated.ZoomAxis = bZoomIn == bZoomOut ? 0.0f : ( bZoomIn ? 1.0f : -1.0f );
	Evaluated.bJumpRequested = CurrentInput.IsDown( JumpAction ) && !PreviousInput.IsDown( JumpAction );
	OutInput = Evaluated;
	return true;
}

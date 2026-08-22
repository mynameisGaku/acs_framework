// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3D.h"

#include <cmath>


CThirdPersonCharacter3D::~CThirdPersonCharacter3D() noexcept
{
	Unbind();
}


bool CThirdPersonCharacter3D::Bind( CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene, ANode& Character, const FThirdPersonCharacter3DParams& Params ) noexcept
{
	if ( m_Mover.IsBound() || m_Camera.IsBound() || !IsValidParams_Internal( Params ) ) return false;
	if ( !m_Mover.Bind( Collision, Character, Params.LocalCollisionCenter, Params.Movement ) ) return false;
	m_Mover.SetCollisionFilter( Params.SelfShape, Params.CollisionMask );
	if ( !m_Camera.Bind( Scene, Character, Params.Camera ) )
	{
		m_Mover.Unbind();
		return false;
	}

	m_Scene = &Scene;
	m_Params = Params;
	return true;
}


void CThirdPersonCharacter3D::Unbind() noexcept
{
	m_Animator.Unbind();
	m_Camera.Unbind();
	m_Mover.Unbind();
	m_Scene = nullptr;
	m_Params = FThirdPersonCharacter3DParams{};
}


bool CThirdPersonCharacter3D::TryBindAnimation( const FCharacterAnimation3DProfile& Profile ) noexcept
{
	ANode* const Target = Character();
	return IsBound() && Target != nullptr && m_Animator.Bind( *Target, Profile );
}


bool CThirdPersonCharacter3D::TryBindAnimation( ASkinnedMeshComponent3D& Component, const FCharacterAnimation3DProfile& Profile ) noexcept
{
	return IsBound() && m_Animator.Bind( Component, Profile );
}


FThirdPersonCharacter3DUpdateResult CThirdPersonCharacter3D::Update( const FThirdPersonCharacter3DInput& Input, f32 DeltaSeconds ) noexcept
{
	FThirdPersonCharacter3DUpdateResult Result;
	if ( !IsBound() || !Input.IsValid() || !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return Result;

	Result.bCameraInputApplied = m_Camera.Update( Input.LookAxes, Input.ZoomAxis, DeltaSeconds );
	if ( !Result.bCameraInputApplied ) return Result;

	Result.bMovementApplied = m_Mover.MoveFromCamera( m_Scene->Camera(), Input.MoveAxes, m_Params.MaximumMoveSpeed, Input.bJumpRequested, DeltaSeconds );
	if ( !Result.bMovementApplied ) return Result;

	Result.bFacingApplied = m_Mover.TurnTowardMovement( m_Params.MaximumTurnDegreesPerSecond, DeltaSeconds );
	Result.bCameraFollowApplied = m_Camera.RefreshTarget();
	Result.bAnimationWasBound = m_Animator.IsBound();
	if ( Result.bAnimationWasBound )
	{
		const FVec3 Velocity = m_Mover.Velocity();
		const FCharacterAnimation3DInput AnimationInput{ Length( FVec2{ Velocity.x, Velocity.z } ), m_Mover.IsGrounded() };
		Result.bAnimationApplied = m_Animator.Update( AnimationInput );
	}
	return Result;
}


FThirdPersonCharacter3DUpdateResult CThirdPersonCharacter3D::Update( const FActionInput& CurrentInput, const FActionInput& PreviousInput, f32 DeltaSeconds, const FThirdPersonCharacter3DActionSet& Actions ) noexcept
{
	FThirdPersonCharacter3DInput Input;
	if ( !Actions.TryEvaluate( CurrentInput, PreviousInput, Input ) ) return {};
	return Update( Input, DeltaSeconds );
}


bool CThirdPersonCharacter3D::IsValidParams_Internal( const FThirdPersonCharacter3DParams& Params ) noexcept
{
	return IsFinite_Internal( Params.LocalCollisionCenter ) && std::isfinite( Params.MaximumMoveSpeed ) && Params.MaximumMoveSpeed >= 0.0f && std::isfinite( Params.MaximumTurnDegreesPerSecond ) && Params.MaximumTurnDegreesPerSecond >= 0.0f;
}


bool CThirdPersonCharacter3D::IsFinite_Internal( FVec3 Value ) noexcept
{
	return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
}

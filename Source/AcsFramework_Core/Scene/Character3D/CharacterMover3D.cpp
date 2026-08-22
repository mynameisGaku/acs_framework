// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/CharacterMover3D.h"

#include <cmath>


bool CCharacterMover3D::Bind( CSceneCollision3D& Collision, ANode& Node, FVec3 LocalCenter, const FKinematicCharacterMovementParams3D& Params ) noexcept
{
	FVec3 WorldCenter;
	if ( Node.IsPendingDestroy() || !IsValidParams_Internal( Params ) || !TryWorldCenter_Internal( Node, LocalCenter, WorldCenter ) ) return false;

	m_Collision = &Collision;
	m_Node = &Node;
	m_LocalCenter = LocalCenter;
	m_Params = Params;
	m_State = FKinematicCharacterState3D{};
	m_State.Position = WorldCenter;
	m_LastResult = FKinematicCharacterMovementResult3D{};
	m_SelfShape = FCollisionShapeId3D{};
	m_CollisionMask = CSceneCollision3D::kAllLayers;
	return true;
}


void CCharacterMover3D::Unbind() noexcept
{
	m_Collision = nullptr;
	m_Node = nullptr;
	m_LocalCenter = FVec3{};
	m_Params = FKinematicCharacterMovementParams3D{};
	m_State = FKinematicCharacterState3D{};
	m_LastResult = FKinematicCharacterMovementResult3D{};
	m_SelfShape = FCollisionShapeId3D{};
	m_CollisionMask = CSceneCollision3D::kAllLayers;
}


bool CCharacterMover3D::Move( FVec2 DesiredWorldXZVelocity, bool bJumpRequested, f32 DeltaSeconds ) noexcept
{
	if ( m_Collision == nullptr || m_Node == nullptr || m_Node->IsPendingDestroy() ) return false;

	FVec3 WorldCenter;
	if ( !TryWorldCenter_Internal( *m_Node, m_LocalCenter, WorldCenter ) ) return false;

	FKinematicCharacterState3D State = m_State;
	State.Position = WorldCenter;
	FKinematicCharacterMovementInput3D Input;
	Input.DesiredHorizontalVelocity = DesiredWorldXZVelocity;
	Input.SelfShape = m_SelfShape;
	Input.CollisionMask = m_CollisionMask;
	Input.JumpRequested = bJumpRequested;

	FKinematicCharacterMovementResult3D Result;
	if ( !m_Collision->TryMoveCharacter( Input, State, DeltaSeconds, m_Params, Result ) ) return false;

	FVec3 LocalPosition;
	if ( !TryLocalPositionAfterWorldTranslation_Internal( *m_Node, Result.Translation, LocalPosition ) ) return false;

	m_Node->SetPosition( LocalPosition );
	m_State = Result.NextState;
	m_LastResult = Result;
	return true;
}


bool CCharacterMover3D::ResetMotion() noexcept
{
	if ( m_Node == nullptr || m_Node->IsPendingDestroy() ) return false;

	FVec3 WorldCenter;
	if ( !TryWorldCenter_Internal( *m_Node, m_LocalCenter, WorldCenter ) ) return false;

	FKinematicCharacterState3D State;
	State.Position = WorldCenter;
	m_State = State;
	m_LastResult = FKinematicCharacterMovementResult3D{};
	return true;
}


bool CCharacterMover3D::SetMovementParams( const FKinematicCharacterMovementParams3D& Params ) noexcept
{
	if ( !IsValidParams_Internal( Params ) ) return false;
	m_Params = Params;
	return true;
}


void CCharacterMover3D::SetCollisionFilter( FCollisionShapeId3D SelfShape, u32 CollisionMask ) noexcept
{
	m_SelfShape = SelfShape;
	m_CollisionMask = CollisionMask;
}


bool CCharacterMover3D::TryWorldCenter_Internal( const ANode& Node, FVec3 LocalCenter, FVec3& OutWorldCenter ) noexcept
{
	if ( !IsFinite_Internal( LocalCenter ) ) return false;
	const FVec3 WorldCenter = TransformPoint( LocalCenter, Node.World().ToMat4() );
	if ( !IsFinite_Internal( WorldCenter ) ) return false;
	OutWorldCenter = WorldCenter;
	return true;
}


bool CCharacterMover3D::TryLocalPositionAfterWorldTranslation_Internal( const ANode& Node, FVec3 WorldTranslation, FVec3& OutLocalPosition ) noexcept
{
	const FVec3 CurrentWorldPosition = Node.World().position;
	if ( !IsFinite_Internal( CurrentWorldPosition ) || !IsFinite_Internal( WorldTranslation ) ) return false;
	const FVec3 TargetWorldPosition = CurrentWorldPosition + WorldTranslation;
	if ( !IsFinite_Internal( TargetWorldPosition ) ) return false;

	FVec3 LocalPosition = TargetWorldPosition;
	if ( const ANode* const Parent = Node.Parent() )
	{
		LocalPosition = TransformPoint( TargetWorldPosition, Inverse( Parent->World().ToMat4() ) );
	}
	if ( !IsFinite_Internal( LocalPosition ) ) return false;

	OutLocalPosition = LocalPosition;
	return true;
}


bool CCharacterMover3D::IsValidParams_Internal( const FKinematicCharacterMovementParams3D& Params ) noexcept
{
	CCollisionWorld3D EmptyWorld;
	FKinematicCharacterMovementResult3D Result;
	return TryMoveKinematicCharacter3D( EmptyWorld, {}, {}, 0.0f, Params, Result );
}


bool CCharacterMover3D::IsFinite_Internal( FVec3 Value ) noexcept
{
	return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
}

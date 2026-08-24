// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawnResult.h"

#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3D.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"

#include <cmath>


FCheckpoint3DSpawnResult FCheckpoint3DSpawnResult::TrySpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CCheckpoint3D& Checkpoint, FCollisionShapeId3D TargetShape,
	FVec3 Position, const FCheckpoint3DParams& Params, ANode* Parent ) noexcept
{
	if ( Checkpoint.IsBound() || !Collision.IsBoundTo( Graph )
		|| !TargetShape.IsValid() || !IsFinitePosition_Internal( Position )
		|| !Params.IsValid() ) return {};

	const FScene3DSpawnResult Spawned = Graph.TrySpawn(
		FStringView( "Checkpoint3D" ), Parent );
	if ( !Spawned ) return {};
	Spawned.Node->SetPosition( Position );

	if ( !Checkpoint.Bind(
		Graph, Collision, *Spawned.Node, TargetShape, Params ) )
	{
		Rollback_Internal( Graph, Spawned );
		return {};
	}
	ANode* const Target = Checkpoint.Target();
	if ( Target == nullptr )
	{
		Checkpoint.Unbind();
		Rollback_Internal( Graph, Spawned );
		return {};
	}
	const FNodeId TargetId = Graph.IdOf( Target );
	if ( !TargetId.IsValid() || Graph.Get( TargetId ) != Target )
	{
		Checkpoint.Unbind();
		Rollback_Internal( Graph, Spawned );
		return {};
	}

	FCheckpoint3DSpawnResult Result;
	Result.m_OriginId = Spawned.Id;
	Result.m_TargetId = TargetId;
	Result.m_OwnerGraph = &Graph;
	Result.m_OwnerCollision = &Collision;
	Result.m_OwnerCheckpoint = &Checkpoint;
	Result.m_RootIdentity = &Graph.Root();
	Result.m_BindingRevision = Checkpoint.BindingRevision();
	return Result;
}


ANode* FCheckpoint3DSpawnResult::Origin() const noexcept
{
	if ( !Succeeded() || !m_OwnerGraph->HasRoot()
		|| &m_OwnerGraph->Root() != m_RootIdentity ) return nullptr;
	ANode* const CurrentOrigin = m_OwnerGraph->Get( m_OriginId );
	return CurrentOrigin != nullptr && IsNodeAlive_Internal( *CurrentOrigin )
		? CurrentOrigin : nullptr;
}


bool FCheckpoint3DSpawnResult::IsOwnedBy(
	const CSceneNodeGraph& Graph, const CSceneCollision3D& Collision,
	const CCheckpoint3D& Checkpoint ) const noexcept
{
	return m_OwnerGraph == &Graph && m_OwnerCollision == &Collision
		&& m_OwnerCheckpoint == &Checkpoint;
}


bool FCheckpoint3DSpawnResult::IsFinitePosition_Internal( FVec3 Position ) noexcept
{
	return std::isfinite( Position.x ) && std::isfinite( Position.y )
		&& std::isfinite( Position.z );
}


bool FCheckpoint3DSpawnResult::IsNodeAlive_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


void FCheckpoint3DSpawnResult::Rollback_Internal(
	CSceneNodeGraph& Graph, FScene3DSpawnResult Spawned ) noexcept
{
	if ( Spawned ) (void)Graph.Destroy( Spawned.Id );
}

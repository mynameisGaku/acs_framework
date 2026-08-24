// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"


FCheckpoint3DSpawnResult CCheckpoint3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CCheckpoint3D& Checkpoint, FCollisionShapeId3D TargetShape,
	FVec3 Position, const FCheckpoint3DParams& Params, ANode* Parent ) noexcept
{
	return FCheckpoint3DSpawnResult::TrySpawnInto(
		Graph, Collision, Checkpoint, TargetShape, Position, Params, Parent );
}


bool CCheckpoint3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, CCheckpoint3D& Checkpoint,
	FCheckpoint3DSpawnResult& Spawned ) noexcept
{
	if ( !Spawned || !Graph.HasRoot() || !Collision.IsBoundTo( Graph )
		|| !Spawned.IsOwnedBy( Graph, Collision, Checkpoint )
		|| !Spawned.IsFromRoot( Graph.Root() ) ) return false;

	ANode* const CurrentOrigin = Graph.Get( Spawned.OriginId() );
	const bool bOwnsCurrentBinding = Checkpoint.HasBindingIdentity(
		Graph, Collision, Spawned.OriginId(), Spawned.BindingRevision() );
	if ( Checkpoint.HasBindingOrigin( Graph, Spawned.OriginId() )
		&& !bOwnsCurrentBinding ) return false;
	ANode* const BoundOrigin = Checkpoint.Origin();
	if ( CurrentOrigin != nullptr && BoundOrigin != nullptr
		&& CurrentOrigin != BoundOrigin
		&& IsAncestorOf_Internal( *CurrentOrigin, *BoundOrigin ) ) return false;
	ANode* const CurrentTarget = Graph.Get( Spawned.TargetId() );
	if ( CurrentOrigin != nullptr && CurrentTarget != nullptr
		&& CurrentOrigin != CurrentTarget
		&& IsAncestorOf_Internal( *CurrentOrigin, *CurrentTarget ) ) return false;
	ANode* const BoundTarget = Checkpoint.IsBoundToGraph( Graph )
		? Graph.Get( Checkpoint.TargetNodeId() ) : nullptr;
	if ( CurrentOrigin != nullptr && BoundTarget != nullptr
		&& ( CurrentOrigin == BoundTarget
			|| IsAncestorOf_Internal( *CurrentOrigin, *BoundTarget ) ) ) return false;
	if ( CurrentOrigin != nullptr && !CurrentOrigin->IsPendingDestroy()
		&& !Graph.Destroy( Spawned.OriginId() ) ) return false;
	if ( bOwnsCurrentBinding ) Checkpoint.Unbind();
	Spawned.Reset();
	return true;
}


bool CCheckpoint3DSpawner::IsAncestorOf_Internal(
	const ANode& Ancestor, const ANode& Node ) noexcept
{
	const ANode* Current = Node.Parent();
	while ( Current != nullptr )
	{
		if ( Current == &Ancestor ) return true;
		Current = Current->Parent();
	}
	return false;
}

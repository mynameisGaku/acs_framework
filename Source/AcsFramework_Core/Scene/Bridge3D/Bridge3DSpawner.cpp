// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawner.h"

#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawner.h"
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"


FBridge3DSpawnResult CBridge3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FBridge3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	FGround3DSpawnParams DeckParams;
	FFence3DSpawnParams NegativeRailingParams;
	FFence3DSpawnParams PositiveRailingParams;
	if ( !Params.TryBuildParts(
		DeckParams, NegativeRailingParams, PositiveRailingParams ) ) return {};

	FBridge3DSpawnResult Bridge;
	Bridge.Deck = CGround3DSpawner::SpawnInto(
		Graph, Collision, DeckParams, Parent );
	if ( !Bridge.Deck ) return {};

	Bridge.NegativeSideRailing = CFence3DSpawner::SpawnInto(
		Graph, Collision, NegativeRailingParams, Parent );
	if ( !Bridge.NegativeSideRailing )
	{
		Rollback_Internal( Graph, Collision, Bridge );
		return {};
	}

	Bridge.PositiveSideRailing = CFence3DSpawner::SpawnInto(
		Graph, Collision, PositiveRailingParams, Parent );
	if ( !Bridge.PositiveSideRailing )
	{
		Rollback_Internal( Graph, Collision, Bridge );
		return {};
	}
	return Bridge;
}


bool CBridge3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FBridge3DSpawnResult& Bridge ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, Bridge ) ) return false;

	const usize PartCount = static_cast<usize>( Bridge.PartCount() );
	for ( usize Remaining = PartCount; Remaining > 0u; --Remaining )
	{
		const FCollidableModel3DSpawnResult* const Part = PartAt_Internal(
			Bridge, Remaining - 1u );
		if ( Part == nullptr ) return false;
		FCollidableModel3DSpawnResult MutablePart = *Part;
		if ( !CModel3DSpawner::DestroyCollidable(
			Graph, Collision, MutablePart ) ) return false;
	}
	Bridge = FBridge3DSpawnResult{};
	return true;
}


bool CBridge3DSpawner::IsOwnedPart_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Part ) noexcept
{
	if ( !Part || Part.Node == &Graph.Root() ) return false;
	const FNodeId NodeId = Graph.IdOf( Part.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Part.Node
		&& Collision.IsRegisteredTo( Part.Shape, *Part.Node );
}


const FCollidableModel3DSpawnResult* CBridge3DSpawner::RailingPartAt_Internal(
	const FFence3DSpawnResult& Railing, usize Index ) noexcept
{
	if ( Index < Railing.Posts.Num() ) return &Railing.Posts[Index];
	Index -= Railing.Posts.Num();
	return Index < Railing.Rails.Num() ? &Railing.Rails[Index] : nullptr;
}


const FCollidableModel3DSpawnResult* CBridge3DSpawner::PartAt_Internal(
	const FBridge3DSpawnResult& Bridge, usize Index ) noexcept
{
	if ( Index == 0u ) return &Bridge.Deck;
	--Index;
	const usize NegativeCount = static_cast<usize>(
		Bridge.NegativeSideRailing.PartCount() );
	if ( Index < NegativeCount ) return RailingPartAt_Internal(
		Bridge.NegativeSideRailing, Index );
	return RailingPartAt_Internal(
		Bridge.PositiveSideRailing, Index - NegativeCount );
}


bool CBridge3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FBridge3DSpawnResult& Bridge ) noexcept
{
	if ( !Bridge ) return false;
	const usize PartCount = static_cast<usize>( Bridge.PartCount() );
	for ( usize Index = 0u; Index < PartCount; ++Index )
	{
		const FCollidableModel3DSpawnResult* const Part = PartAt_Internal(
			Bridge, Index );
		if ( Part == nullptr || !IsOwnedPart_Internal(
			Graph, Collision, *Part ) ) return false;
		for ( usize OtherIndex = Index + 1u; OtherIndex < PartCount; ++OtherIndex )
		{
			const FCollidableModel3DSpawnResult* const Other = PartAt_Internal(
				Bridge, OtherIndex );
			if ( Other == nullptr || Part->Node == Other->Node
				|| Part->Shape == Other->Shape ) return false;
		}
	}
	return true;
}


void CBridge3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FBridge3DSpawnResult& Bridge ) noexcept
{
	if ( Bridge.PositiveSideRailing ) (void)CFence3DSpawner::Destroy(
		Graph, Collision, Bridge.PositiveSideRailing );
	if ( Bridge.NegativeSideRailing ) (void)CFence3DSpawner::Destroy(
		Graph, Collision, Bridge.NegativeSideRailing );
	if ( Bridge.Deck ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Bridge.Deck );
}

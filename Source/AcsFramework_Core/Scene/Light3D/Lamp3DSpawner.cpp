// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawner.h"


FLamp3DSpawnResult CLamp3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, const FLamp3DParams& Params,
	ANode* Parent ) noexcept
{
	return FLamp3DSpawnResult::TrySpawnInto( Graph, Params, Parent );
}


bool CLamp3DSpawner::Destroy( CSceneNodeGraph& Graph,
	FLamp3DSpawnResult& Spawned ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Spawned ) ) return false;

	const FNodeId NodeIds[]
	{
		Spawned.LightId(),
		Spawned.BulbId(),
	};
	for ( const FNodeId NodeId : NodeIds )
	{
		ANode* const Node = Graph.Get( NodeId );
		if ( Node != nullptr && !Node->IsPendingDestroy()
			&& !Graph.Destroy( NodeId ) ) return false;
	}

	Spawned.Reset();
	return true;
}


bool CLamp3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	const FLamp3DSpawnResult& Spawned ) noexcept
{
	if ( !Spawned || !Graph.HasRoot() || !Spawned.IsOwnedBy( Graph )
		|| !Spawned.IsFromRoot( Graph.Root() ) ) return false;

	const FNodeId BulbId = Spawned.BulbId();
	const FNodeId LightId = Spawned.LightId();
	if ( BulbId == LightId ) return false;

	ANode* const Bulb = Graph.Get( BulbId );
	ANode* const Light = Graph.Get( LightId );
	if ( Bulb == &Graph.Root() || Light == &Graph.Root() ) return false;
	if ( Bulb != nullptr && Bulb->GetComponent<AMeshComponent3D>() == nullptr ) return false;
	if ( Light != nullptr && Light->GetComponent<ALightComponent3D>() == nullptr ) return false;
	return true;
}

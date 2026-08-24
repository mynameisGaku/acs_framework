// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawner.h"


FStudioLightRig3DSpawnResult CStudioLightRig3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, const FStudioLightRig3DParams& Params,
	ANode* Parent ) noexcept
{
	return FStudioLightRig3DSpawnResult::TrySpawnInto(
		Graph, Params, Parent );
}


bool CStudioLightRig3DSpawner::Destroy( CSceneNodeGraph& Graph,
	FStudioLightRig3DSpawnResult& Spawned ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Spawned ) ) return false;

	const FNodeId LightIds[]
	{
		Spawned.RimLightId(),
		Spawned.FillLightId(),
		Spawned.KeyLightId(),
	};
	for ( const FNodeId LightId : LightIds )
	{
		ANode* const Light = Graph.Get( LightId );
		if ( Light != nullptr && !Light->IsPendingDestroy()
			&& !Graph.Destroy( LightId ) ) return false;
	}

	Spawned.Reset();
	return true;
}


bool CStudioLightRig3DSpawner::CanDestroy_Internal(
	CSceneNodeGraph& Graph,
	const FStudioLightRig3DSpawnResult& Spawned ) noexcept
{
	if ( !Spawned || !Graph.HasRoot() || !Spawned.IsOwnedBy( Graph )
		|| !Spawned.IsFromRoot( Graph.Root() ) ) return false;

	const FNodeId KeyId = Spawned.KeyLightId();
	const FNodeId FillId = Spawned.FillLightId();
	const FNodeId RimId = Spawned.RimLightId();
	if ( KeyId == FillId || KeyId == RimId || FillId == RimId ) return false;

	const FNodeId LightIds[] { KeyId, FillId, RimId };
	for ( const FNodeId LightId : LightIds )
	{
		ANode* const Light = Graph.Get( LightId );
		if ( Light == &Graph.Root() ) return false;
	}
	return true;
}

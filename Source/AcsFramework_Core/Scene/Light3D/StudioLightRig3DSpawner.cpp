// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawner.h"

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"


FStudioLightRig3DSpawnResult CStudioLightRig3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, const FStudioLightRig3DParams& Params,
	ANode* Parent ) noexcept
{
	return FStudioLightRig3DSpawnResult::TrySpawnInto(
		Graph, Params, Parent );
}


bool CStudioLightRig3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	const FStudioLightRig3DSpawnResult& Spawned,
	const FStudioLightRig3DParams& Params ) noexcept
{
	FLight3DSpawnParams KeyParams;
	FLight3DSpawnParams FillParams;
	FLight3DSpawnParams RimParams;
	if ( !Params.TryBuildLights( KeyParams, FillParams, RimParams )
		|| !CanApply_Internal( Graph, Spawned ) ) return false;

	ANode* const Lights[]
	{
		Spawned.KeyLight(),
		Spawned.FillLight(),
		Spawned.RimLight(),
	};
	const FLight3DSpawnParams* const LightParams[]
	{
		&KeyParams,
		&FillParams,
		&RimParams,
	};
	// 事前確認後は、各灯の既存部品へ値を代入するだけで失敗しない。
	for ( usize Index = 0u; Index < 3u; ++Index )
	{
		if ( !CLight3DSpawner::TryApplyTo(
			*Lights[Index], *LightParams[Index] ) ) return false;
	}
	return true;
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


bool CStudioLightRig3DSpawner::CanApply_Internal(
	CSceneNodeGraph& Graph,
	const FStudioLightRig3DSpawnResult& Spawned ) noexcept
{
	if ( !Spawned || !Graph.HasRoot() || !Spawned.IsOwnedBy( Graph )
		|| !Spawned.IsFromRoot( Graph.Root() ) ) return false;

	const FNodeId KeyId = Spawned.KeyLightId();
	const FNodeId FillId = Spawned.FillLightId();
	const FNodeId RimId = Spawned.RimLightId();
	if ( KeyId == FillId || KeyId == RimId || FillId == RimId ) return false;

	ANode* const Key = Spawned.KeyLight();
	ANode* const Fill = Spawned.FillLight();
	ANode* const Rim = Spawned.RimLight();
	return Key != nullptr && Fill != nullptr && Rim != nullptr
		&& Key != &Graph.Root() && Fill != &Graph.Root() && Rim != &Graph.Root()
		&& Graph.Get( KeyId ) == Key && Graph.Get( FillId ) == Fill
		&& Graph.Get( RimId ) == Rim && Key->Parent() != nullptr
		&& Fill->Parent() == Key->Parent() && Rim->Parent() == Key->Parent()
		&& Key->GetComponent<ALightComponent3D>() != nullptr
		&& Fill->GetComponent<ALightComponent3D>() != nullptr
		&& Rim->GetComponent<ALightComponent3D>() != nullptr;
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

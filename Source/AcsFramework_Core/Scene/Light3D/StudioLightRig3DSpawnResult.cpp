// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawnResult.h"

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DParams.h"


FStudioLightRig3DSpawnResult FStudioLightRig3DSpawnResult::TrySpawnInto(
	CSceneNodeGraph& Graph, const FStudioLightRig3DParams& Params,
	ANode* Parent ) noexcept
{
	FLight3DSpawnParams KeyParams;
	FLight3DSpawnParams FillParams;
	FLight3DSpawnParams RimParams;
	if ( !Params.TryBuildLights( KeyParams, FillParams, RimParams ) ) return {};

	const FLight3DSpawnParams* const LightParams[]
	{
		&KeyParams,
		&FillParams,
		&RimParams,
	};
	FNodeId LightIds[3]{};
	for ( usize Index = 0u; Index < 3u; ++Index )
	{
		ANode* const Light = CLight3DSpawner::SpawnInto(
			Graph, *LightParams[Index], Parent );
		if ( Light == nullptr )
		{
			Rollback_Internal( Graph, LightIds, Index );
			return {};
		}
		LightIds[Index] = Graph.IdOf( Light );
		if ( !LightIds[Index].IsValid() )
		{
			Rollback_Internal( Graph, LightIds, Index + 1u );
			return {};
		}
	}

	FStudioLightRig3DSpawnResult Result;
	Result.m_KeyLightId = LightIds[0];
	Result.m_FillLightId = LightIds[1];
	Result.m_RimLightId = LightIds[2];
	Result.m_OwnerGraph = &Graph;
	Result.m_RootIdentity = &Graph.Root();
	return Result;
}


ANode* FStudioLightRig3DSpawnResult::KeyLight() const noexcept
{
	return ResolveLight_Internal( m_KeyLightId );
}


ANode* FStudioLightRig3DSpawnResult::FillLight() const noexcept
{
	return ResolveLight_Internal( m_FillLightId );
}


ANode* FStudioLightRig3DSpawnResult::RimLight() const noexcept
{
	return ResolveLight_Internal( m_RimLightId );
}


ANode* FStudioLightRig3DSpawnResult::ResolveLight_Internal(
	FNodeId LightId ) const noexcept
{
	if ( !Succeeded() || !m_OwnerGraph->HasRoot()
		|| &m_OwnerGraph->Root() != m_RootIdentity ) return nullptr;
	ANode* const Light = m_OwnerGraph->Get( LightId );
	return Light != nullptr && IsNodeAlive_Internal( *Light ) ? Light : nullptr;
}


bool FStudioLightRig3DSpawnResult::IsNodeAlive_Internal(
	const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


void FStudioLightRig3DSpawnResult::Rollback_Internal(
	CSceneNodeGraph& Graph, const FNodeId* LightIds,
	usize LightCount ) noexcept
{
	while ( LightCount > 0u )
	{
		--LightCount;
		if ( LightIds[LightCount].IsValid() )
			(void)Graph.Destroy( LightIds[LightCount] );
	}
}

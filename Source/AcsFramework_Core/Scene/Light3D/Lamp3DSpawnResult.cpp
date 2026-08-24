// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawnResult.h"

#include "AcsFramework_Core/Scene/Light3D/Lamp3DParams.h"
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"


FLamp3DSpawnResult FLamp3DSpawnResult::TrySpawnInto(
	CSceneNodeGraph& Graph, const FLamp3DParams& Params,
	ANode* Parent ) noexcept
{
	FModel3DSpawnParams BulbParams;
	FLight3DSpawnParams LightParams;
	if ( !Params.TryBuildParts( BulbParams, LightParams ) ) return {};

	ANode* const Bulb = CModel3DSpawner::SpawnInto(
		Graph, BulbParams, Parent );
	if ( Bulb == nullptr ) return {};
	const FNodeId BulbId = Graph.IdOf( Bulb );
	if ( !BulbId.IsValid() ) return {};

	ANode* const Light = CLight3DSpawner::SpawnInto(
		Graph, LightParams, Parent );
	if ( Light == nullptr )
	{
		Rollback_Internal( Graph, BulbId, {} );
		return {};
	}
	const FNodeId LightId = Graph.IdOf( Light );
	if ( !LightId.IsValid() )
	{
		Rollback_Internal( Graph, BulbId, LightId );
		return {};
	}

	FLamp3DSpawnResult Result;
	Result.m_BulbId = BulbId;
	Result.m_LightId = LightId;
	Result.m_OwnerGraph = &Graph;
	Result.m_RootIdentity = &Graph.Root();
	return Result;
}


ANode* FLamp3DSpawnResult::Bulb() const noexcept
{
	return ResolveNode_Internal( m_BulbId );
}


ANode* FLamp3DSpawnResult::Light() const noexcept
{
	return ResolveNode_Internal( m_LightId );
}


ANode* FLamp3DSpawnResult::ResolveNode_Internal( FNodeId NodeId ) const noexcept
{
	if ( !Succeeded() || !m_OwnerGraph->HasRoot()
		|| &m_OwnerGraph->Root() != m_RootIdentity ) return nullptr;
	ANode* const Node = m_OwnerGraph->Get( NodeId );
	return Node != nullptr && IsNodeAlive_Internal( *Node ) ? Node : nullptr;
}


bool FLamp3DSpawnResult::IsNodeAlive_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


void FLamp3DSpawnResult::Rollback_Internal( CSceneNodeGraph& Graph,
	FNodeId BulbId, FNodeId LightId ) noexcept
{
	if ( LightId.IsValid() ) (void)Graph.Destroy( LightId );
	if ( BulbId.IsValid() ) (void)Graph.Destroy( BulbId );
}

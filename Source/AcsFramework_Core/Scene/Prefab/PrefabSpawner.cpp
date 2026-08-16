// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Prefab/PrefabSpawner.h"


ANode* CPrefabSpawner::SpawnAttached( CPrefabSystem& Prefabs, FPrefabId Id, ANode& Parent, const FPrefabSpawnParams& Params ) noexcept
{
	TObjectPtr<ANode> Spawned = Prefabs.Spawn( Id );
	if ( Spawned.Get() == nullptr ) return nullptr;

	// 親へ渡す前に施す。渡した後だと、親の側で並び替えなどが走った後の状態へ触ることになる。
	ApplyParams( *Spawned, Params );

	return &Parent.AddChild( Move( Spawned ) );
}


TObjectPtr<ANode> CPrefabSpawner::SpawnDetached( CPrefabSystem& Prefabs, FPrefabId Id, const FPrefabSpawnParams& Params ) noexcept
{
	TObjectPtr<ANode> Spawned = Prefabs.Spawn( Id );
	if ( Spawned.Get() == nullptr ) return Spawned;

	ApplyParams( *Spawned, Params );

	return Spawned;
}


void CPrefabSpawner::ApplyParams( ANode& Node, const FPrefabSpawnParams& Params ) noexcept
{
	if ( !Params.Name.IsEmpty() ) Node.SetName( Params.Name.View() );

	if ( Params.bApplyTransform ) Node.Local() = Params.LocalTransform;

	if ( Params.bApplyEnabled ) Node.SetEnabled( Params.bEnabled );
}

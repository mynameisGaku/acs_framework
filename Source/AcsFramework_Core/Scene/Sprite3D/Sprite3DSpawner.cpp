// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawner.h"

namespace
{
	/** 度をラジアンへ直す係数。 */
	constexpr f32 kDegreesToRadians = 3.14159265358979323846f / 180.0f;
}


ANode* CSprite3DSpawner::SpawnInto( CSceneNodeGraph& Graph, const FSprite3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsReady() ) return nullptr;

	const FScene3DSpawnResult Spawned = Graph.TrySpawn( Params.Name, Parent );
	if ( !Spawned ) return nullptr;

	ApplyTransform_Internal( *Spawned.Node, Params );
	ApplySprite_Internal( *Spawned.Node, Params );
	return Spawned.Node;
}


ANode* CSprite3DSpawner::SpawnInto( CSceneNodeGraph& Graph, const FSprite3DSpawnParams& Params,
	CImageLibrary& Library, ANode* Parent ) noexcept
{
	FSprite3DSpawnParams Prepared;
	if ( !Prepare_Internal( Params, Library, Prepared ) ) return nullptr;
	return SpawnInto( Graph, Prepared, Parent );
}


ANode* CSprite3DSpawner::SpawnInto( ANode& Parent, const FSprite3DSpawnParams& Params ) noexcept
{
	if ( !Params.IsReady() ) return nullptr;

	TObjectPtr<ANode> Node = NewObject<ANode>();
	if ( !Node ) return nullptr;

	if ( Params.Name.Data() != nullptr && Params.Name.Size() != 0u ) Node->SetName( Params.Name );
	ApplyTransform_Internal( *Node, Params );
	ApplySprite_Internal( *Node, Params );

	ANode* const Placed = Node.Get();
	Parent.AddChild( Move( Node ) );
	return Placed;
}


ANode* CSprite3DSpawner::SpawnInto( ANode& Parent, const FSprite3DSpawnParams& Params,
	CImageLibrary& Library ) noexcept
{
	FSprite3DSpawnParams Prepared;
	if ( !Prepare_Internal( Params, Library, Prepared ) ) return nullptr;
	return SpawnInto( Parent, Prepared );
}


bool CSprite3DSpawner::Prepare_Internal( const FSprite3DSpawnParams& Params,
	CImageLibrary& Library, FSprite3DSpawnParams& OutPrepared ) noexcept
{
	if ( !Params.IsValid() ) return false;
	OutPrepared = Params;
	if ( !OutPrepared.ImageAsset ) OutPrepared.ImageAsset = Library.Load( Params.TexturePath );
	return OutPrepared.IsReady();
}


void CSprite3DSpawner::ApplyTransform_Internal( ANode& Node, const FSprite3DSpawnParams& Params ) noexcept
{
	FTransform3D& Local = Node.Local();
	Local.position = Params.Position;
	Local.rotation = FQuat::Euler( Params.RotationDeg.x * kDegreesToRadians,
		Params.RotationDeg.y * kDegreesToRadians, Params.RotationDeg.z * kDegreesToRadians );
	Local.scale = FVec3{ Params.Size.x, Params.Size.y, 1.0f };
}


void CSprite3DSpawner::ApplySprite_Internal( ANode& Node, const FSprite3DSpawnParams& Params ) noexcept
{
	ASprite3DComponent& Sprite = Node.AddComponent<ASprite3DComponent>();
	Sprite.SetTexturePath( Params.TexturePath );
	Sprite.SetImageAsset( Params.ImageAsset );
}

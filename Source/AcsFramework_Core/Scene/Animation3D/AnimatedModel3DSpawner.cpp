// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"

namespace
{
	/** 度をラジアンへ直す係数。 */
	constexpr f32 kDegreesToRadians = 3.14159265358979323846f / 180.0f;

	/** 文字列に1文字以上あるか返す。 */
	bool HasText( FStringView Text ) noexcept
	{
		return Text.Data() != nullptr && Text.Size() > 0u;
	}
}


ANode* CAnimatedModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	const FAnimatedModel3DSpawnParams& Params, ANode* Parent ) noexcept
{
	u32 AnimationIndex = 0u;
	bool bShouldPlay = false;
	if ( !Params.IsValid() || !Params.MeshAsset ) return nullptr;
	if ( !TryResolveAnimation_Internal( Params, AnimationIndex, bShouldPlay ) ) return nullptr;

	const FScene3DSpawnResult Spawned = Graph.TrySpawn( Params.Name, Parent );
	if ( !Spawned ) return nullptr;

	ApplyTransform_Internal( *Spawned.Node, Params );
	ApplySkin_Internal( *Spawned.Node, Params, AnimationIndex, bShouldPlay );
	return Spawned.Node;
}


ANode* CAnimatedModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	const FAnimatedModel3DSpawnParams& Params, CModelLibrary& Library, ANode* Parent ) noexcept
{
	if ( Params.MeshAsset ) return SpawnInto( Graph, Params, Parent );
	if ( !Params.IsValid() || !HasText( Params.MeshPath ) ) return nullptr;

	FAnimatedModel3DSpawnParams Loaded = Params;
	Loaded.MeshAsset = Library.LoadSkinned( Params.MeshPath );
	if ( !Loaded.MeshAsset ) return nullptr;
	return SpawnInto( Graph, Loaded, Parent );
}


FCollidableModel3DSpawnResult CAnimatedModel3DSpawner::SpawnCollidableInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	const FAnimatedModel3DSpawnParams& Params, const FCollisionShape3DParams& CollisionParams,
	ANode* Parent ) noexcept
{
	ANode* const Node = SpawnInto( Graph, Params, Parent );
	return RegisterCollisionOrRollback_Internal( Graph, Collision, Node, CollisionParams );
}


FCollidableModel3DSpawnResult CAnimatedModel3DSpawner::SpawnCollidableInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	const FAnimatedModel3DSpawnParams& Params, CModelLibrary& Library,
	const FCollisionShape3DParams& CollisionParams, ANode* Parent ) noexcept
{
	ANode* const Node = SpawnInto( Graph, Params, Library, Parent );
	return RegisterCollisionOrRollback_Internal( Graph, Collision, Node, CollisionParams );
}


ANode* CAnimatedModel3DSpawner::SpawnInto( ANode& Parent,
	const FAnimatedModel3DSpawnParams& Params ) noexcept
{
	u32 AnimationIndex = 0u;
	bool bShouldPlay = false;
	if ( !Params.IsValid() || !Params.MeshAsset ) return nullptr;
	if ( !TryResolveAnimation_Internal( Params, AnimationIndex, bShouldPlay ) ) return nullptr;

	TObjectPtr<ANode> Node = NewObject<ANode>();
	if ( !Node ) return nullptr;
	if ( HasText( Params.Name ) ) Node->SetName( Params.Name );

	ApplyTransform_Internal( *Node, Params );
	ApplySkin_Internal( *Node, Params, AnimationIndex, bShouldPlay );

	ANode* const Placed = Node.Get();
	Parent.AddChild( Move( Node ) );
	return Placed;
}


ANode* CAnimatedModel3DSpawner::SpawnInto( ANode& Parent,
	const FAnimatedModel3DSpawnParams& Params, CModelLibrary& Library ) noexcept
{
	if ( Params.MeshAsset ) return SpawnInto( Parent, Params );
	if ( !Params.IsValid() || !HasText( Params.MeshPath ) ) return nullptr;

	FAnimatedModel3DSpawnParams Loaded = Params;
	Loaded.MeshAsset = Library.LoadSkinned( Params.MeshPath );
	if ( !Loaded.MeshAsset ) return nullptr;
	return SpawnInto( Parent, Loaded );
}


FCollidableModel3DSpawnResult CAnimatedModel3DSpawner::RegisterCollisionOrRollback_Internal(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ANode* Node,
	const FCollisionShape3DParams& CollisionParams ) noexcept
{
	if ( Node == nullptr ) return {};

	const FCollisionShapeId3D Shape = Collision.TryAdd( *Node, CollisionParams );
	if ( Shape.IsValid() ) return FCollidableModel3DSpawnResult{ Node, Shape };

	const FNodeId NodeId = Graph.IdOf( Node );
	if ( NodeId.IsValid() ) (void)Graph.Destroy( NodeId );
	return {};
}


bool CAnimatedModel3DSpawner::IsRenderable_Internal( const ASkinnedMeshAsset& Mesh ) noexcept
{
	return !Mesh.Vertices().IsEmpty() && !Mesh.Indices().IsEmpty() && !Mesh.Bones().IsEmpty();
}


bool CAnimatedModel3DSpawner::TryResolveAnimation_Internal(
	const FAnimatedModel3DSpawnParams& Params, u32& OutAnimationIndex, bool& OutShouldPlay ) noexcept
{
	const ASkinnedMeshAsset* const Mesh = Params.MeshAsset.Get();
	if ( Mesh == nullptr || !IsRenderable_Internal( *Mesh ) ) return false;

	OutAnimationIndex = 0u;
	OutShouldPlay = false;
	if ( !Params.bAutoPlay ) return true;

	const TArray<FAnimation>& Animations = Mesh->Animations();
	if ( Animations.IsEmpty() ) return !HasText( Params.InitialAnimation );

	if ( !HasText( Params.InitialAnimation ) )
	{
		if ( Params.InitialAnimationIndex >= Animations.Num() ) return false;
		OutAnimationIndex = Params.InitialAnimationIndex;
		OutShouldPlay = true;
		return true;
	}

	for ( usize Index = 0u; Index < Animations.Num(); ++Index )
	{
		if ( Animations[Index].name.View() != Params.InitialAnimation ) continue;
		OutAnimationIndex = static_cast<u32>( Index );
		OutShouldPlay = true;
		return true;
	}

	return false;
}


void CAnimatedModel3DSpawner::ApplyTransform_Internal(
	ANode& Node, const FAnimatedModel3DSpawnParams& Params ) noexcept
{
	FTransform3D& Local = Node.Local();
	Local.position = Params.Position;
	Local.scale = Params.Scale;
	Local.rotation = FQuat::Euler( Params.RotationDeg.x * kDegreesToRadians,
		Params.RotationDeg.y * kDegreesToRadians,
		Params.RotationDeg.z * kDegreesToRadians );
}


void CAnimatedModel3DSpawner::ApplySkin_Internal( ANode& Node,
	const FAnimatedModel3DSpawnParams& Params, u32 AnimationIndex, bool bShouldPlay ) noexcept
{
	ASkinnedMeshComponent3D& Skin = Node.AddComponent<ASkinnedMeshComponent3D>();
	Skin.SetMeshAsset( Params.MeshAsset );
	Skin.SetColor( Params.Color );
	if ( bShouldPlay ) Skin.Play( AnimationIndex, Params.bLoop );
}

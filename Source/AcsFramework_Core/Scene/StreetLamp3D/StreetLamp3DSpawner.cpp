// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"


FStreetLamp3DSpawnResult CStreetLamp3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	const FStreetLamp3DSpawnParams& Params, ANode* Parent ) noexcept
{
	FBlock3DSpawnParams PostParams;
	FLamp3DParams LampParams;
	if ( !Params.TryBuildParts( PostParams, LampParams ) ) return {};

	FStreetLamp3DSpawnResult StreetLamp;
	StreetLamp.Post = CBlock3DSpawner::SpawnInto(
		Graph, Collision, PostParams, Parent );
	if ( !StreetLamp.Post ) return {};

	StreetLamp.Lamp = CLamp3DSpawner::SpawnInto(
		Graph, LampParams, Parent );
	if ( !StreetLamp.Lamp )
	{
		Rollback_Internal( Graph, Collision, StreetLamp );
		return {};
	}
	return StreetLamp;
}


bool CStreetLamp3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FStreetLamp3DSpawnResult& StreetLamp,
	const FStreetLamp3DSpawnParams& Params ) noexcept
{
	FBlock3DSpawnParams PostParams;
	FLamp3DParams LampParams;
	if ( !Params.TryBuildParts( PostParams, LampParams )
		|| !CanApply_Internal( Graph, Collision, StreetLamp ) ) return false;

	// ポスト側のレイヤー更新だけが失敗し得る。ランプ側は事前確認後の値代入だけになる。
	if ( !CBlock3DSpawner::TryApplyTo(
		Graph, Collision, StreetLamp.Post, PostParams ) ) return false;
	return CLamp3DSpawner::TryApplyTo( Graph, StreetLamp.Lamp, LampParams );
}


bool CStreetLamp3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	FStreetLamp3DSpawnResult& StreetLamp ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, StreetLamp ) ) return false;
	if ( !CLamp3DSpawner::Destroy( Graph, StreetLamp.Lamp ) ) return false;
	if ( !CModel3DSpawner::DestroyCollidable(
		Graph, Collision, StreetLamp.Post ) ) return false;

	StreetLamp = FStreetLamp3DSpawnResult{};
	return true;
}


bool CStreetLamp3DSpawner::CanApply_Internal(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	const FStreetLamp3DSpawnResult& StreetLamp ) noexcept
{
	if ( !StreetLamp || !Graph.HasRoot()
		|| StreetLamp.Post.Node == &Graph.Root() ) return false;

	const FNodeId PostId = Graph.IdOf( StreetLamp.Post.Node );
	if ( !PostId.IsValid() || Graph.Get( PostId ) != StreetLamp.Post.Node
		|| !IsNodeAlive_Internal( *StreetLamp.Post.Node )
		|| !Collision.IsRegisteredTo(
			StreetLamp.Post.Shape, *StreetLamp.Post.Node )
		|| StreetLamp.Post.Node->GetComponent<AMeshComponent3D>() == nullptr )
		return false;

	const FLamp3DSpawnResult& Lamp = StreetLamp.Lamp;
	ANode* const Bulb = Lamp.Bulb();
	ANode* const Light = Lamp.Light();
	return Lamp.IsOwnedBy( Graph ) && Lamp.IsFromRoot( Graph.Root() )
		&& Lamp.BulbId() != Lamp.LightId()
		&& PostId != Lamp.BulbId() && PostId != Lamp.LightId()
		&& Bulb != nullptr && Light != nullptr
		&& Bulb != &Graph.Root() && Light != &Graph.Root()
		&& Graph.Get( Lamp.BulbId() ) == Bulb
		&& Graph.Get( Lamp.LightId() ) == Light
		&& Bulb->Parent() == StreetLamp.Post.Node->Parent()
		&& Light->Parent() == StreetLamp.Post.Node->Parent()
		&& Bulb->GetComponent<AMeshComponent3D>() != nullptr
		&& Light->GetComponent<ALightComponent3D>() != nullptr;
}


bool CStreetLamp3DSpawner::IsNodeAlive_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


bool CStreetLamp3DSpawner::CanDestroy_Internal(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	const FStreetLamp3DSpawnResult& StreetLamp ) noexcept
{
	if ( !StreetLamp || !Graph.HasRoot()
		|| StreetLamp.Post.Node == &Graph.Root() ) return false;
	const FNodeId PostId = Graph.IdOf( StreetLamp.Post.Node );
	if ( !PostId.IsValid() || Graph.Get( PostId ) != StreetLamp.Post.Node
		|| !Collision.IsRegisteredTo(
			StreetLamp.Post.Shape, *StreetLamp.Post.Node ) ) return false;

	const FLamp3DSpawnResult& Lamp = StreetLamp.Lamp;
	if ( !Lamp.IsOwnedBy( Graph ) || !Lamp.IsFromRoot( Graph.Root() )
		|| Lamp.BulbId() == Lamp.LightId()
		|| PostId == Lamp.BulbId() || PostId == Lamp.LightId() ) return false;

	ANode* const Bulb = Graph.Get( Lamp.BulbId() );
	ANode* const Light = Graph.Get( Lamp.LightId() );
	if ( Bulb == &Graph.Root() || Light == &Graph.Root() ) return false;
	if ( Bulb != nullptr && Bulb->GetComponent<AMeshComponent3D>() == nullptr )
		return false;
	if ( Light != nullptr && Light->GetComponent<ALightComponent3D>() == nullptr )
		return false;
	return true;
}


void CStreetLamp3DSpawner::Rollback_Internal(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	FStreetLamp3DSpawnResult& StreetLamp ) noexcept
{
	if ( StreetLamp.Lamp )
		(void)CLamp3DSpawner::Destroy( Graph, StreetLamp.Lamp );
	if ( StreetLamp.Post )
		(void)CModel3DSpawner::DestroyCollidable(
			Graph, Collision, StreetLamp.Post );
}

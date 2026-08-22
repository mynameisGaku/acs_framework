// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawner.h"

#include "AcsFramework_Core/Assets/Model3D/ModelLibrary.h"
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3D.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"


FThirdPersonCharacter3DSpawnResult CThirdPersonCharacter3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
	CThirdPersonCharacter3D& Controller, const FModel3DSpawnParams& ModelParams,
	const FThirdPersonCharacter3DSpawnParams& SpawnParams, ANode* Parent ) noexcept
{
	if ( Controller.IsBound() ) return {};
	const FCollidableModel3DSpawnResult Spawned = CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, ModelParams, SpawnParams.Collision, Parent );
	return BindOrRollback_Internal( Graph, Collision, Scene, Controller, Spawned, SpawnParams, false );
}


FThirdPersonCharacter3DSpawnResult CThirdPersonCharacter3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
	CThirdPersonCharacter3D& Controller, const FModel3DSpawnParams& ModelParams,
	CModelLibrary& Library, const FThirdPersonCharacter3DSpawnParams& SpawnParams,
	ANode* Parent ) noexcept
{
	if ( Controller.IsBound() ) return {};
	const FCollidableModel3DSpawnResult Spawned = CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, ModelParams, Library, SpawnParams.Collision, Parent );
	return BindOrRollback_Internal( Graph, Collision, Scene, Controller, Spawned, SpawnParams, false );
}


FThirdPersonCharacter3DSpawnResult CThirdPersonCharacter3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
	CThirdPersonCharacter3D& Controller, const FAnimatedModel3DSpawnParams& ModelParams,
	const FThirdPersonCharacter3DSpawnParams& SpawnParams, ANode* Parent ) noexcept
{
	if ( Controller.IsBound() ) return {};
	const FCollidableModel3DSpawnResult Spawned = CAnimatedModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, ModelParams, SpawnParams.Collision, Parent );
	return BindOrRollback_Internal( Graph, Collision, Scene, Controller, Spawned, SpawnParams, true );
}


FThirdPersonCharacter3DSpawnResult CThirdPersonCharacter3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
	CThirdPersonCharacter3D& Controller, const FAnimatedModel3DSpawnParams& ModelParams,
	CModelLibrary& Library, const FThirdPersonCharacter3DSpawnParams& SpawnParams,
	ANode* Parent ) noexcept
{
	if ( Controller.IsBound() ) return {};
	const FCollidableModel3DSpawnResult Spawned = CAnimatedModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, ModelParams, Library, SpawnParams.Collision, Parent );
	return BindOrRollback_Internal( Graph, Collision, Scene, Controller, Spawned, SpawnParams, true );
}


FThirdPersonCharacter3DSpawnResult CThirdPersonCharacter3DSpawner::BindOrRollback_Internal(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	ALegacyScene3DAdapter& Scene, CThirdPersonCharacter3D& Controller,
	const FCollidableModel3DSpawnResult& Spawned,
	const FThirdPersonCharacter3DSpawnParams& SpawnParams,
	bool bCanBindAnimation ) noexcept
{
	if ( !Spawned ) return {};

	FThirdPersonCharacter3DParams Control = SpawnParams.Control;
	Control.SelfShape = Spawned.Shape;
	if ( !Controller.Bind( Collision, Scene, *Spawned.Node, Control ) )
	{
		Rollback_Internal( Graph, Collision, Spawned );
		return {};
	}

	FThirdPersonCharacter3DSpawnResult Result;
	Result.Node = Spawned.Node;
	Result.Shape = Spawned.Shape;
	Result.bAnimationBound = bCanBindAnimation && SpawnParams.bBindAnimation
		&& Controller.TryBindAnimation( SpawnParams.Animation );
	return Result;
}


void CThirdPersonCharacter3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Spawned ) noexcept
{
	(void)Collision.Remove( Spawned.Shape );
	if ( Spawned.Node == nullptr ) return;

	const FNodeId NodeId = Graph.IdOf( Spawned.Node );
	if ( NodeId.IsValid() ) (void)Graph.Destroy( NodeId );
}

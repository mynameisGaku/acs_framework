// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractableModel3DSpawner.h"

#include "AcsFramework_Core/Assets/Model3D/ModelLibrary.h"
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"


ANode* CInteractableModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CInteractionFocus3D& Focus, const FModel3DSpawnParams& ModelParams,
	FStringView Prompt, FVec3 WorldOffset, ANode* Parent ) noexcept
{
	ANode* const Node = CModel3DSpawner::SpawnInto( Graph, ModelParams, Parent );
	return RegisterOrRollback_Internal( Graph, Focus, Node, Prompt, WorldOffset );
}


ANode* CInteractableModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CInteractionFocus3D& Focus, const FModel3DSpawnParams& ModelParams,
	CModelLibrary& Library, FStringView Prompt, FVec3 WorldOffset,
	ANode* Parent ) noexcept
{
	ANode* const Node = CModel3DSpawner::SpawnInto(
		Graph, ModelParams, Library, Parent );
	return RegisterOrRollback_Internal( Graph, Focus, Node, Prompt, WorldOffset );
}


FCollidableModel3DSpawnResult CInteractableModel3DSpawner::SpawnCollidableInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CInteractionFocus3D& Focus, const FModel3DSpawnParams& ModelParams,
	FStringView Prompt, const FCollisionShape3DParams& CollisionParams,
	FVec3 WorldOffset, ANode* Parent ) noexcept
{
	const FCollidableModel3DSpawnResult Spawned = CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, ModelParams, CollisionParams, Parent );
	return RegisterCollidableOrRollback_Internal(
		Graph, Collision, Focus, Spawned, Prompt, WorldOffset );
}


FCollidableModel3DSpawnResult CInteractableModel3DSpawner::SpawnCollidableInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CInteractionFocus3D& Focus, const FModel3DSpawnParams& ModelParams,
	CModelLibrary& Library, FStringView Prompt,
	const FCollisionShape3DParams& CollisionParams, FVec3 WorldOffset,
	ANode* Parent ) noexcept
{
	const FCollidableModel3DSpawnResult Spawned = CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, ModelParams, Library, CollisionParams, Parent );
	return RegisterCollidableOrRollback_Internal(
		Graph, Collision, Focus, Spawned, Prompt, WorldOffset );
}


FCollidableModel3DSpawnResult CInteractableModel3DSpawner::SpawnCollidableInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CInteractionFocus3D& Focus,
	const FAnimatedModel3DSpawnParams& ModelParams, FStringView Prompt,
	const FCollisionShape3DParams& CollisionParams, FVec3 WorldOffset,
	ANode* Parent ) noexcept
{
	const FCollidableModel3DSpawnResult Spawned =
		CAnimatedModel3DSpawner::SpawnCollidableInto(
			Graph, Collision, ModelParams, CollisionParams, Parent );
	return RegisterCollidableOrRollback_Internal(
		Graph, Collision, Focus, Spawned, Prompt, WorldOffset );
}


FCollidableModel3DSpawnResult CInteractableModel3DSpawner::SpawnCollidableInto(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CInteractionFocus3D& Focus,
	const FAnimatedModel3DSpawnParams& ModelParams, CModelLibrary& Library,
	FStringView Prompt, const FCollisionShape3DParams& CollisionParams,
	FVec3 WorldOffset, ANode* Parent ) noexcept
{
	const FCollidableModel3DSpawnResult Spawned =
		CAnimatedModel3DSpawner::SpawnCollidableInto(
			Graph, Collision, ModelParams, Library, CollisionParams, Parent );
	return RegisterCollidableOrRollback_Internal(
		Graph, Collision, Focus, Spawned, Prompt, WorldOffset );
}


bool CInteractableModel3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CInteractionFocus3D& Focus, ANode*& Model ) noexcept
{
	if ( Model == nullptr ) return false;
	const FNodeId NodeId = Graph.IdOf( Model );
	if ( !NodeId.IsValid() || Graph.Get( NodeId ) != Model ) return false;
	if ( !Model->IsPendingDestroy() && !Graph.Destroy( NodeId ) ) return false;

	(void)Focus.UnregisterTarget( *Model );
	Model = nullptr;
	return true;
}


bool CInteractableModel3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, CInteractionFocus3D& Focus,
	FCollidableModel3DSpawnResult& Model ) noexcept
{
	if ( !Model ) return false;
	const FNodeId NodeId = Graph.IdOf( Model.Node );
	if ( !NodeId.IsValid() || Graph.Get( NodeId ) != Model.Node ) return false;
	if ( !Model.Node->IsPendingDestroy() && !Graph.Destroy( NodeId ) ) return false;

	(void)Focus.UnregisterTarget( *Model.Node );
	(void)Collision.Remove( Model.Shape );
	Model = FCollidableModel3DSpawnResult{};
	return true;
}


ANode* CInteractableModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CInteractionFocus3D& Focus, const FAnimatedModel3DSpawnParams& ModelParams,
	FStringView Prompt, FVec3 WorldOffset, ANode* Parent ) noexcept
{
	ANode* const Node = CAnimatedModel3DSpawner::SpawnInto(
		Graph, ModelParams, Parent );
	return RegisterOrRollback_Internal( Graph, Focus, Node, Prompt, WorldOffset );
}


ANode* CInteractableModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CInteractionFocus3D& Focus, const FAnimatedModel3DSpawnParams& ModelParams,
	CModelLibrary& Library, FStringView Prompt, FVec3 WorldOffset,
	ANode* Parent ) noexcept
{
	ANode* const Node = CAnimatedModel3DSpawner::SpawnInto(
		Graph, ModelParams, Library, Parent );
	return RegisterOrRollback_Internal( Graph, Focus, Node, Prompt, WorldOffset );
}


ANode* CInteractableModel3DSpawner::RegisterOrRollback_Internal(
	CSceneNodeGraph& Graph, CInteractionFocus3D& Focus, ANode* Node,
	FStringView Prompt, FVec3 WorldOffset ) noexcept
{
	if ( Node == nullptr ) return nullptr;
	if ( Focus.RegisterTarget( *Node, Prompt, WorldOffset ) ) return Node;

	const FNodeId NodeId = Graph.IdOf( Node );
	if ( NodeId.IsValid() ) (void)Graph.Destroy( NodeId );
	return nullptr;
}


FCollidableModel3DSpawnResult
CInteractableModel3DSpawner::RegisterCollidableOrRollback_Internal(
	CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	CInteractionFocus3D& Focus, FCollidableModel3DSpawnResult Spawned,
	FStringView Prompt, FVec3 WorldOffset ) noexcept
{
	if ( !Spawned ) return {};
	if ( Focus.RegisterTarget( *Spawned.Node, Prompt, WorldOffset ) ) return Spawned;

	(void)Collision.Remove( Spawned.Shape );
	const FNodeId NodeId = Graph.IdOf( Spawned.Node );
	if ( NodeId.IsValid() ) (void)Graph.Destroy( NodeId );
	return {};
}

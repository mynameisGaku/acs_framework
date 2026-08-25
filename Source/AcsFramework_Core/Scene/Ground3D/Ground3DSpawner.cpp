// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FCollidableModel3DSpawnResult CGround3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FGround3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	FModel3DSpawnParams Plane = FModel3DSpawnParams::FromPrimitive(
		EMeshPrimitive3D::Plane, Params.Position );
	Plane.Scale = FVec3{ Params.Size.x, Params.Thickness, Params.Size.y };
	Plane.Color = Params.Color;
	Plane.Metallic = Params.Metallic;
	Plane.Roughness = Params.Roughness;
	Plane.bCastsShadow = Params.bCastsShadow;
	Plane.Name = Params.Name;

	const FCollisionShape3DParams GroundBox = FCollisionShape3DParams::FromBox(
		FVec3{ 0.0f, -0.5f, 0.0f }, FVec3{ 0.5f, 0.5f, 0.5f },
		Params.CollisionLayer );
	return CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, Plane, GroundBox, Parent );
}


bool CGround3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Ground,
	const FGround3DSpawnParams& Params ) noexcept
{
	if ( !Params.IsValid() || !CanApply_Internal( Graph, Collision, Ground ) ) return false;

	ANode& Node = *Ground.Node;
	AMeshComponent3D& Mesh = *Node.GetComponent<AMeshComponent3D>();
	// 失敗し得る更新を先に終え、以降は検証済み値の代入だけにする。
	if ( !Collision.TrySetLayer( Ground.Shape, Params.CollisionLayer ) ) return false;

	ApplyToNode_Internal( Node, Mesh, Params );
	if ( Params.Name.Data() != nullptr && Params.Name.Size() > 0u )
		Node.SetName( Params.Name );
	return true;
}


bool CGround3DSpawner::CanApply_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Ground ) noexcept
{
	if ( !Ground || !Graph.HasRoot() || Ground.Node == &Graph.Root() ) return false;

	const FNodeId NodeId = Graph.IdOf( Ground.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Ground.Node
		&& !Ground.Node->IsPendingDestroy()
		&& Collision.IsRegisteredTo( Ground.Shape, *Ground.Node )
		&& Ground.Node->GetComponent<AMeshComponent3D>() != nullptr;
}


void CGround3DSpawner::ApplyToNode_Internal( ANode& Node,
	AMeshComponent3D& Mesh,
	const FGround3DSpawnParams& Params ) noexcept
{
	Node.Local().position = Params.Position;
	Node.Local().scale = FVec3{ Params.Size.x, Params.Thickness, Params.Size.y };

	Mesh.SetMeshAsset( TSharedPtr<AAsset>{} );
	Mesh.SetMeshPath( FStringView{} );
	Mesh.SetPrimitive( EMeshPrimitive3D::Plane );
	Mesh.SetColor( Params.Color );
	Mesh.SetCastsShadow( Params.bCastsShadow );

	FMaterial2D Material{};
	Material.pbr.metallic = Params.Metallic;
	Material.pbr.roughness = Params.Roughness;
	Mesh.SetMaterial( Material );
}

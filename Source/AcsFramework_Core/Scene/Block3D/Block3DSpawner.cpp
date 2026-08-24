// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

namespace
{
	/** 度をラジアンへ直す係数。 */
	constexpr f32 kDegreesToRadians = 3.14159265358979323846f / 180.0f;
}

FCollidableModel3DSpawnResult CBlock3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FBlock3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	FModel3DSpawnParams Block = FModel3DSpawnParams::FromPrimitive(
		EMeshPrimitive3D::Cube, Params.Position );
	Block.RotationDeg = Params.RotationDeg;
	Block.Scale = Params.Size;
	Block.Color = Params.Color;
	Block.Metallic = Params.Metallic;
	Block.Roughness = Params.Roughness;
	Block.bCastsShadow = Params.bCastsShadow;
	Block.Name = Params.Name;

	const FCollisionShape3DParams BlockBox = FCollisionShape3DParams::FromBox(
		FVec3{}, FVec3{ 0.5f, 0.5f, 0.5f }, Params.CollisionLayer );
	return CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, Block, BlockBox, Parent );
}


bool CBlock3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Block,
	const FBlock3DSpawnParams& Params ) noexcept
{
	if ( !Params.IsValid() || !CanApply_Internal( Graph, Collision, Block ) ) return false;

	ANode& Node = *Block.Node;
	AMeshComponent3D& Mesh = *Node.GetComponent<AMeshComponent3D>();
	// 失敗し得る更新を先に終え、以降は検証済み値の代入だけにする。
	if ( !Collision.TrySetLayer( Block.Shape, Params.CollisionLayer ) ) return false;

	ApplyToNode_Internal( Node, Mesh, Params );
	if ( Params.Name.Data() != nullptr && Params.Name.Size() > 0u )
		Node.SetName( Params.Name );
	return true;
}


bool CBlock3DSpawner::CanApply_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Block ) noexcept
{
	if ( !Block || !Graph.HasRoot() || Block.Node == &Graph.Root()
		|| Block.Node->IsPendingDestroy() ) return false;

	const FNodeId NodeId = Graph.IdOf( Block.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Block.Node
		&& Collision.IsRegisteredTo( Block.Shape, *Block.Node )
		&& Block.Node->GetComponent<AMeshComponent3D>() != nullptr;
}


void CBlock3DSpawner::ApplyToNode_Internal( ANode& Node,
	AMeshComponent3D& Mesh,
	const FBlock3DSpawnParams& Params ) noexcept
{
	Node.Local().position = Params.Position;
	Node.Local().rotation = FQuat::Euler(
		Params.RotationDeg.x * kDegreesToRadians,
		Params.RotationDeg.y * kDegreesToRadians,
		Params.RotationDeg.z * kDegreesToRadians );
	Node.Local().scale = Params.Size;

	Mesh.SetMeshAsset( TSharedPtr<AAsset>{} );
	Mesh.SetMeshPath( FStringView{} );
	Mesh.SetPrimitive( EMeshPrimitive3D::Cube );
	Mesh.SetColor( Params.Color );
	Mesh.SetCastsShadow( Params.bCastsShadow );

	FMaterial2D Material{};
	Material.pbr.metallic = Params.Metallic;
	Material.pbr.roughness = Params.Roughness;
	Mesh.SetMaterial( Material );
}

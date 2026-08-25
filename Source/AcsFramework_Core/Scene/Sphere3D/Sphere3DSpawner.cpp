// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Sphere3D/Sphere3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FCollidableModel3DSpawnResult CSphere3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FSphere3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	// ACSのローカル直径1の球を、指定半径と一致させる均一尺度。
	const f32 Diameter = Params.Radius * 2.0f;
	// 表示と衝突が同じノード変形を共有する球モデル設定。
	FModel3DSpawnParams Sphere = FModel3DSpawnParams::FromPrimitive(
		EMeshPrimitive3D::Sphere, Params.Position );
	Sphere.Scale = FVec3{ Diameter, Diameter, Diameter };
	Sphere.Color = Params.Color;
	Sphere.Metallic = Params.Metallic;
	Sphere.Roughness = Params.Roughness;
	Sphere.bCastsShadow = Params.bCastsShadow;
	Sphere.Name = Params.Name;

	// ローカル直径1の表示へ一致し、均一尺度後に指定半径となる球衝突。
	const FCollisionShape3DParams SphereCollision = FCollisionShape3DParams::FromSphere(
		FVec3{}, 0.5f, Params.CollisionLayer );
	return CModel3DSpawner::SpawnCollidableInto(
		Graph, Collision, Sphere, SphereCollision, Parent );
}


bool CSphere3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Sphere,
	const FSphere3DSpawnParams& Params ) noexcept
{
	if ( !Params.IsValid() || !CanApply_Internal( Graph, Collision, Sphere ) ) return false;

	ANode& Node = *Sphere.Node;
	AMeshComponent3D& Mesh = *Node.GetComponent<AMeshComponent3D>();
	// 失敗し得る更新を先に終え、以降は検証済み値の代入だけにする。
	if ( !Collision.TrySetLayer( Sphere.Shape, Params.CollisionLayer ) ) return false;

	ApplyToNode_Internal( Node, Mesh, Params );
	if ( Params.Name.Data() != nullptr && Params.Name.Size() > 0u )
		Node.SetName( Params.Name );
	return true;
}


bool CSphere3DSpawner::CanApply_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision,
	const FCollidableModel3DSpawnResult& Sphere ) noexcept
{
	if ( !Sphere || !Graph.HasRoot() || Sphere.Node == &Graph.Root() ) return false;

	const FNodeId NodeId = Graph.IdOf( Sphere.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Sphere.Node
		&& !Sphere.Node->IsPendingDestroy()
		&& Collision.IsRegisteredTo( Sphere.Shape, *Sphere.Node )
		&& Sphere.Node->GetComponent<AMeshComponent3D>() != nullptr;
}


void CSphere3DSpawner::ApplyToNode_Internal( ANode& Node,
	AMeshComponent3D& Mesh,
	const FSphere3DSpawnParams& Params ) noexcept
{
	const f32 Diameter = Params.Radius * 2.0f;
	Node.Local().position = Params.Position;
	Node.Local().scale = FVec3{ Diameter, Diameter, Diameter };

	Mesh.SetMeshAsset( TSharedPtr<AAsset>{} );
	Mesh.SetMeshPath( FStringView{} );
	Mesh.SetPrimitive( EMeshPrimitive3D::Sphere );
	Mesh.SetColor( Params.Color );
	Mesh.SetCastsShadow( Params.bCastsShadow );

	FMaterial2D Material{};
	Material.pbr.metallic = Params.Metallic;
	Material.pbr.roughness = Params.Roughness;
	Mesh.SetMaterial( Material );
}

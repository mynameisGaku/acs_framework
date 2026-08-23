// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"

namespace
{
	/** 度をラジアンへ直す係数。 */
	constexpr f32 kDegreesToRadians = 3.14159265358979323846f / 180.0f;
}


ANode* CModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph, const FModel3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	// シーンへ半端なノードを残さないよう、pool登録より前に入力を確定する。
	if ( !Params.IsValid() ) return nullptr;

	const FScene3DSpawnResult Spawned = Graph.TrySpawn( Params.Name, Parent );
	if ( !Spawned ) return nullptr;

	ApplyTransform( *Spawned.Node, Params );
	ApplyMesh( *Spawned.Node, Params );
	return Spawned.Node;
}


ANode* CModel3DSpawner::SpawnInto( CSceneNodeGraph& Graph, const FModel3DSpawnParams& Params,
	CModelLibrary& Library, ANode* Parent ) noexcept
{
	// 読込済みなら置き場へ触れず、その共有所有権をそのまま使う。
	if ( Params.MeshAsset ) return SpawnInto( Graph, Params, Parent );

	// プリミティブなら読むものが無い。そのままシーンへ置く。
	if ( Params.MeshPath.Data() == nullptr || Params.MeshPath.Size() == 0u )
		return SpawnInto( Graph, Params, Parent );

	FModel3DSpawnParams Loaded = Params;
	Loaded.MeshAsset = Library.Load( Params.MeshPath );
	if ( !Loaded.MeshAsset ) return nullptr;

	return SpawnInto( Graph, Loaded, Parent );
}


FCollidableModel3DSpawnResult CModel3DSpawner::SpawnCollidableInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FModel3DSpawnParams& Params,
	const FCollisionShape3DParams& CollisionParams, ANode* Parent ) noexcept
{
	ANode* const Node = SpawnInto( Graph, Params, Parent );
	return RegisterCollisionOrRollback_Internal( Graph, Collision, Node, CollisionParams );
}


FCollidableModel3DSpawnResult CModel3DSpawner::SpawnCollidableInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FModel3DSpawnParams& Params, CModelLibrary& Library,
	const FCollisionShape3DParams& CollisionParams, ANode* Parent ) noexcept
{
	ANode* const Node = SpawnInto( Graph, Params, Library, Parent );
	return RegisterCollisionOrRollback_Internal( Graph, Collision, Node, CollisionParams );
}


bool CModel3DSpawner::DestroyCollidable( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FCollidableModel3DSpawnResult& Model ) noexcept
{
	if ( !Model ) return false;

	const FNodeId NodeId = Graph.IdOf( Model.Node );
	if ( !NodeId.IsValid() || Graph.Get( NodeId ) != Model.Node ) return false;
	if ( !Collision.IsRegisteredTo( Model.Shape, *Model.Node ) ) return false;
	if ( !Model.Node->IsPendingDestroy() && !Graph.Destroy( NodeId ) ) return false;
	if ( !Collision.Remove( Model.Shape ) ) return false;

	Model = FCollidableModel3DSpawnResult{};
	return true;
}


ANode* CModel3DSpawner::SpawnInto( ANode& Parent, const FModel3DSpawnParams& Params ) noexcept
{
	// 置いてから «見えない» と気付くのが一番たちが悪いので、作る前に弾く。
	if ( !Params.IsValid() ) return nullptr;

	TObjectPtr<ANode> Node = NewObject<ANode>();
	if ( Node.Get() == nullptr ) return nullptr;

	if ( Params.Name.Data() != nullptr && Params.Name.Size() != 0u ) Node->SetName( Params.Name );

	ApplyTransform( *Node, Params );
	ApplyMesh( *Node, Params );

	ANode* const Placed = Node.Get();
	Parent.AddChild( Move( Node ) );

	return Placed;
}


ANode* CModel3DSpawner::SpawnInto( ANode& Parent, const FModel3DSpawnParams& Params,
	CModelLibrary& Library ) noexcept
{
	// 読込済みなら置き場へ触れず、その共有所有権をそのまま使う。
	if ( Params.MeshAsset ) return SpawnInto( Parent, Params );

	// プリミティブなら読むものが無い。そのまま置く。
	if ( Params.MeshPath.Data() == nullptr || Params.MeshPath.Size() == 0u )
		return SpawnInto( Parent, Params );

	FModel3DSpawnParams Loaded = Params;
	Loaded.MeshAsset = Library.Load( Params.MeshPath );

	// 読めなかったら置かない。**置いてから «出ない» と悩むより、置かない方が早く気付ける。**
	// 理由は Library が 1 行残している。
	if ( !Loaded.MeshAsset ) return nullptr;

	return SpawnInto( Parent, Loaded );
}


FCollidableModel3DSpawnResult CModel3DSpawner::RegisterCollisionOrRollback_Internal(
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


void CModel3DSpawner::ApplyTransform( ANode& Node, const FModel3DSpawnParams& Params ) noexcept
{
	FTransform3D& Local = Node.Local();

	Local.position = Params.Position;
	Local.scale = Params.Scale;
	Local.rotation = FQuat::Euler( Params.RotationDeg.x * kDegreesToRadians,
		Params.RotationDeg.y * kDegreesToRadians,
		Params.RotationDeg.z * kDegreesToRadians );
}


void CModel3DSpawner::ApplyMesh( ANode& Node, const FModel3DSpawnParams& Params ) noexcept
{
	AMeshComponent3D& Mesh = Node.AddComponent<AMeshComponent3D>();

	// モデルの場所を入れると、形は自動で Mesh になる (エンジン側の決まり)。
	if ( Params.MeshPath.Data() != nullptr && Params.MeshPath.Size() != 0u ) Mesh.SetMeshPath( Params.MeshPath );
	else Mesh.SetPrimitive( Params.Primitive );

	// **パスを入れただけでは映らない。** 部品はパスを覚えるだけで、読むのは別の仕事。
	// 読み込み済みのものが渡っていれば、ここで結び付ける。
	if ( Params.MeshAsset ) Mesh.SetMeshAsset( Params.MeshAsset );

	Mesh.SetColor( Params.Color );
	Mesh.SetCastsShadow( Params.bCastsShadow );
	ApplyMaterial( Mesh, Params );
}


void CModel3DSpawner::ApplyMaterial( AMeshComponent3D& Mesh, const FModel3DSpawnParams& Params ) noexcept
{
	// 材質を «置いていない» ままだと、エンジンは metallic 0 / roughness 0.5 の
	// 決め打ちで描く。同じ既定を持たせた材質を必ず置いて、**触れるようにしておく**。
	//
	// baseColor は 1 のまま。エンジン側が Color と掛け算するので、ここで色を入れると
	// 二重に掛かる。
	FMaterial2D Material{};
	Material.pbr.metallic = Clamp( Params.Metallic, 0.0f, 1.0f );
	Material.pbr.roughness = Clamp( Params.Roughness, 0.0f, 1.0f );
	Material.pbr.shadingMode = Params.bToonShading ? 1 : 0;
	Material.pbr.clearcoat = Params.Clearcoat;
	Material.pbr.clearcoatRoughness = Params.ClearcoatRoughness;
	Material.pbr.subsurface = Params.SubsurfaceStrength;
	Material.pbr.subsurfaceColor = Params.SubsurfaceColor;
	Material.pbr.emissive = Params.EmissiveColor;
	Material.pbr.emissiveStrength = Params.EmissiveStrength;

	Mesh.SetMaterial( Material );
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

namespace
{
	/** 度をラジアンへ直す係数。 */
	constexpr f32 kDegreesToRadians = 3.14159265358979323846f / 180.0f;
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

	Mesh.SetMaterial( Material );
}

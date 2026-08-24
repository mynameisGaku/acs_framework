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

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

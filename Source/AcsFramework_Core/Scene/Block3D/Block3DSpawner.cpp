// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"

#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

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

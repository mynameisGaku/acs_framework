// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FCorridor3DSpawnResult CCorridor3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCorridor3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	// 左右の壁外面まで含む床の全幅。
	const f32 OuterWidth = Params.InnerWidth + Params.WallThickness * 2.0f;
	// 入口から通路中心までの距離。
	const f32 HalfLength = Params.Length * 0.5f;
	// 内幅を保つために各側壁中心を外へずらす距離。
	const f32 WallOffset = Params.InnerWidth * 0.5f + Params.WallThickness * 0.5f;
	// 壁を床上面から上へ置くためのY中心。
	const f32 WallCenterY = Params.EntranceCenter.y + Params.WallHeight * 0.5f;

	// 既定のZ方向形状。X方向では長さと幅を入れ替える。
	FVec2 FloorSize{ OuterWidth, Params.Length };
	// 入口境界から出口側へ半分進めた床上面中心。
	FVec3 FloorTopPosition = Params.EntranceCenter;
	// 幅軸の負方向にある側壁の中心。
	FVec3 NegativeWallPosition = Params.EntranceCenter;
	// 幅軸の正方向にある側壁の中心。
	FVec3 PositiveWallPosition = Params.EntranceCenter;
	// 既定のZ方向通路に使う側壁寸法。
	FVec3 WallSize{ Params.WallThickness, Params.WallHeight, Params.Length };

	switch ( Params.Direction )
	{
	case ECorridor3DDirection::PositiveX:
		FloorSize = FVec2{ Params.Length, OuterWidth };
		FloorTopPosition.x += HalfLength;
		NegativeWallPosition = FVec3{ FloorTopPosition.x, WallCenterY,
			Params.EntranceCenter.z - WallOffset };
		PositiveWallPosition = FVec3{ FloorTopPosition.x, WallCenterY,
			Params.EntranceCenter.z + WallOffset };
		WallSize = FVec3{ Params.Length, Params.WallHeight, Params.WallThickness };
		break;
	case ECorridor3DDirection::NegativeX:
		FloorSize = FVec2{ Params.Length, OuterWidth };
		FloorTopPosition.x -= HalfLength;
		NegativeWallPosition = FVec3{ FloorTopPosition.x, WallCenterY,
			Params.EntranceCenter.z - WallOffset };
		PositiveWallPosition = FVec3{ FloorTopPosition.x, WallCenterY,
			Params.EntranceCenter.z + WallOffset };
		WallSize = FVec3{ Params.Length, Params.WallHeight, Params.WallThickness };
		break;
	case ECorridor3DDirection::PositiveZ:
		FloorTopPosition.z += HalfLength;
		NegativeWallPosition = FVec3{ Params.EntranceCenter.x - WallOffset,
			WallCenterY, FloorTopPosition.z };
		PositiveWallPosition = FVec3{ Params.EntranceCenter.x + WallOffset,
			WallCenterY, FloorTopPosition.z };
		break;
	case ECorridor3DDirection::NegativeZ:
		FloorTopPosition.z -= HalfLength;
		NegativeWallPosition = FVec3{ Params.EntranceCenter.x - WallOffset,
			WallCenterY, FloorTopPosition.z };
		PositiveWallPosition = FVec3{ Params.EntranceCenter.x + WallOffset,
			WallCenterY, FloorTopPosition.z };
		break;
	default:
		return {};
	}

	// 壁外面まで覆い、床上面から下へ伸びる厚みを持つ床設定。
	FGround3DSpawnParams FloorParams = FGround3DSpawnParams::FromSize(
		FloorSize, FloorTopPosition );
	FloorParams.Thickness = Params.FloorThickness;
	FloorParams.Color = Params.FloorColor;
	FloorParams.Metallic = Params.FloorMetallic;
	FloorParams.Roughness = Params.FloorRoughness;
	FloorParams.bCastsShadow = Params.bFloorCastsShadow;
	FloorParams.CollisionLayer = Params.CollisionLayer;
	FloorParams.Name = Params.FloorName;

	// 見た目と衝突設定を共有し、位置だけを左右で差し替える側壁設定。
	FBlock3DSpawnParams WallParams = FBlock3DSpawnParams::FromSize(
		WallSize, NegativeWallPosition );
	WallParams.Color = Params.WallColor;
	WallParams.Metallic = Params.WallMetallic;
	WallParams.Roughness = Params.WallRoughness;
	WallParams.bCastsShadow = Params.bWallsCastShadow;
	WallParams.CollisionLayer = Params.CollisionLayer;
	WallParams.Name = Params.NegativeWallName;

	// 途中失敗時に生成済み部分を逆順で戻すため、順番に保持する結果。
	FCorridor3DSpawnResult Corridor;
	Corridor.Floor = CGround3DSpawner::SpawnInto( Graph, Collision, FloorParams, Parent );
	if ( !Corridor.Floor ) return {};

	Corridor.NegativeSideWall = CBlock3DSpawner::SpawnInto(
		Graph, Collision, WallParams, Parent );
	if ( !Corridor.NegativeSideWall )
	{
		Rollback_Internal( Graph, Collision, Corridor );
		return {};
	}

	WallParams.Position = PositiveWallPosition;
	WallParams.Name = Params.PositiveWallName;
	Corridor.PositiveSideWall = CBlock3DSpawner::SpawnInto(
		Graph, Collision, WallParams, Parent );
	if ( !Corridor.PositiveSideWall )
	{
		Rollback_Internal( Graph, Collision, Corridor );
		return {};
	}

	return Corridor;
}


bool CCorridor3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FCorridor3DSpawnResult& Corridor ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, Corridor ) ) return false;

	// 生成と逆の順で側壁から床へ片付ける。
	FCollidableModel3DSpawnResult* const Parts[]
	{
		&Corridor.PositiveSideWall,
		&Corridor.NegativeSideWall,
		&Corridor.Floor,
	};
	for ( usize Index = 0u; Index < kPartCount; ++Index )
	{
		if ( !CModel3DSpawner::DestroyCollidable( Graph, Collision, *Parts[Index] ) ) return false;
	}
	return true;
}


bool CCorridor3DSpawner::IsOwnedPart_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept
{
	if ( !Part || Part.Node == &Graph.Root() ) return false;
	// ポインターだけでなく、この場面が発行した生存中の識別子でも所有を確かめる。
	const FNodeId NodeId = Graph.IdOf( Part.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Part.Node
		&& Collision.IsRegisteredTo( Part.Shape, *Part.Node );
}


bool CCorridor3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCorridor3DSpawnResult& Corridor ) noexcept
{
	if ( !Corridor ) return false;
	// 生成結果の全要素を同じ規則で所有確認するための一覧。
	const FCollidableModel3DSpawnResult* const Parts[]
	{
		&Corridor.Floor,
		&Corridor.NegativeSideWall,
		&Corridor.PositiveSideWall,
	};
	for ( usize Index = 0u; Index < kPartCount; ++Index )
	{
		if ( !IsOwnedPart_Internal( Graph, Collision, *Parts[Index] ) ) return false;
		for ( usize OtherIndex = Index + 1u; OtherIndex < kPartCount; ++OtherIndex )
		{
			if ( Parts[Index]->Node == Parts[OtherIndex]->Node
				|| Parts[Index]->Shape == Parts[OtherIndex]->Shape ) return false;
		}
	}
	return true;
}


void CCorridor3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FCorridor3DSpawnResult& Corridor ) noexcept
{
	if ( Corridor.PositiveSideWall ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Corridor.PositiveSideWall );
	if ( Corridor.NegativeSideWall ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Corridor.NegativeSideWall );
	if ( Corridor.Floor ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Corridor.Floor );
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FRoom3DSpawnResult CRoom3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FRoom3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	// 床とZ壁が共有する、壁厚込みのX全幅。
	const f32 OuterWidth = Params.InnerSize.x + Params.WallThickness * 2.0f;
	// 床が使う、壁厚込みのZ全幅。
	const f32 OuterDepth = Params.InnerSize.y + Params.WallThickness * 2.0f;
	// 壁を床上面から上へ置くためのY中心。
	const f32 WallCenterY = Params.FloorTopPosition.y + Params.WallHeight * 0.5f;
	// 内寸を保つためにX壁中心を外へずらす距離。
	const f32 WallOffsetX = Params.InnerSize.x * 0.5f + Params.WallThickness * 0.5f;
	// 内寸を保つためにZ壁中心を外へずらす距離。
	const f32 WallOffsetZ = Params.InnerSize.y * 0.5f + Params.WallThickness * 0.5f;

	// 四隅まで覆う外寸と、床上面から下へ伸びる厚みを持つ床設定。
	FGround3DSpawnParams FloorParams = FGround3DSpawnParams::FromSize(
		FVec2{ OuterWidth, OuterDepth }, Params.FloorTopPosition );
	FloorParams.Thickness = Params.FloorThickness;
	FloorParams.Color = Params.FloorColor;
	FloorParams.Metallic = Params.FloorMetallic;
	FloorParams.Roughness = Params.FloorRoughness;
	FloorParams.bCastsShadow = Params.bFloorCastsShadow;
	FloorParams.CollisionLayer = Params.CollisionLayer;
	FloorParams.Name = FStringView( "RoomFloor" );

	// 材質と衝突設定を共有し、位置と寸法だけを各壁で差し替える設定。
	FBlock3DSpawnParams WallParams;
	WallParams.Size = FVec3{ OuterWidth, Params.WallHeight, Params.WallThickness };
	WallParams.Color = Params.WallColor;
	WallParams.Metallic = Params.WallMetallic;
	WallParams.Roughness = Params.WallRoughness;
	WallParams.bCastsShadow = Params.bWallsCastShadow;
	WallParams.CollisionLayer = Params.CollisionLayer;

	// 途中失敗時に生成済み部分を逆順で戻すため、順番に保持する結果。
	FRoom3DSpawnResult Room;
	Room.Floor = CGround3DSpawner::SpawnInto( Graph, Collision, FloorParams, Parent );
	if ( !Room.Floor ) return {};

	WallParams.Position = FVec3{ Params.FloorTopPosition.x, WallCenterY,
		Params.FloorTopPosition.z + WallOffsetZ };
	WallParams.Name = FStringView( "RoomPositiveZWall" );
	Room.PositiveZWall = CBlock3DSpawner::SpawnInto( Graph, Collision, WallParams, Parent );
	if ( !Room.PositiveZWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	WallParams.Position.z = Params.FloorTopPosition.z - WallOffsetZ;
	WallParams.Name = FStringView( "RoomNegativeZWall" );
	Room.NegativeZWall = CBlock3DSpawner::SpawnInto( Graph, Collision, WallParams, Parent );
	if ( !Room.NegativeZWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	WallParams.Position = FVec3{ Params.FloorTopPosition.x + WallOffsetX,
		WallCenterY, Params.FloorTopPosition.z };
	WallParams.Size = FVec3{ Params.WallThickness, Params.WallHeight, Params.InnerSize.y };
	WallParams.Name = FStringView( "RoomPositiveXWall" );
	Room.PositiveXWall = CBlock3DSpawner::SpawnInto( Graph, Collision, WallParams, Parent );
	if ( !Room.PositiveXWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	WallParams.Position.x = Params.FloorTopPosition.x - WallOffsetX;
	WallParams.Name = FStringView( "RoomNegativeXWall" );
	Room.NegativeXWall = CBlock3DSpawner::SpawnInto( Graph, Collision, WallParams, Parent );
	if ( !Room.NegativeXWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	return Room;
}


bool CRoom3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FRoom3DSpawnResult& Room ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, Room ) ) return false;

	// 壁から床へ片付けるための破棄順。
	FCollidableModel3DSpawnResult* const Parts[]
	{
		&Room.NegativeXWall,
		&Room.PositiveXWall,
		&Room.NegativeZWall,
		&Room.PositiveZWall,
		&Room.Floor,
	};
	for ( usize Index = 0u; Index < kPartCount; ++Index )
	{
		if ( !CModel3DSpawner::DestroyCollidable( Graph, Collision, *Parts[Index] ) ) return false;
	}
	return true;
}


bool CRoom3DSpawner::IsOwnedPart_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept
{
	if ( !Part || Part.Node == &Graph.Root() ) return false;
	// ポインターだけでなく、この場面が発行した生存中の識別子でも所有を確かめる。
	const FNodeId NodeId = Graph.IdOf( Part.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Part.Node
		&& Collision.IsRegisteredTo( Part.Shape, *Part.Node );
}


bool CRoom3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FRoom3DSpawnResult& Room ) noexcept
{
	if ( !Room ) return false;
	// 生成結果の全要素を同じ規則で所有確認するための一覧。
	const FCollidableModel3DSpawnResult* const Parts[]
	{
		&Room.Floor,
		&Room.PositiveZWall,
		&Room.NegativeZWall,
		&Room.PositiveXWall,
		&Room.NegativeXWall,
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


void CRoom3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FRoom3DSpawnResult& Room ) noexcept
{
	if ( Room.NegativeXWall ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Room.NegativeXWall );
	if ( Room.PositiveXWall ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Room.PositiveXWall );
	if ( Room.NegativeZWall ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Room.NegativeZWall );
	if ( Room.PositiveZWall ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Room.PositiveZWall );
	if ( Room.Floor ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Room.Floor );
}

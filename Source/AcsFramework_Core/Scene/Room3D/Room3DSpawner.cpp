// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FRoom3DSpawnResult CRoom3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FRoom3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	FGround3DSpawnParams FloorParams;
	FBlock3DSpawnParams PositiveZWallParams;
	FBlock3DSpawnParams NegativeZWallParams;
	FBlock3DSpawnParams PositiveXWallParams;
	FBlock3DSpawnParams NegativeXWallParams;
	if ( !TryBuildParts_Internal( Params, FloorParams,
		PositiveZWallParams, NegativeZWallParams,
		PositiveXWallParams, NegativeXWallParams ) ) return {};

	// 途中失敗時に生成済み部分を逆順で戻すため、順番に保持する結果。
	FRoom3DSpawnResult Room;
	Room.Floor = CGround3DSpawner::SpawnInto( Graph, Collision, FloorParams, Parent );
	if ( !Room.Floor ) return {};

	Room.PositiveZWall = CBlock3DSpawner::SpawnInto(
		Graph, Collision, PositiveZWallParams, Parent );
	if ( !Room.PositiveZWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	Room.NegativeZWall = CBlock3DSpawner::SpawnInto(
		Graph, Collision, NegativeZWallParams, Parent );
	if ( !Room.NegativeZWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	Room.PositiveXWall = CBlock3DSpawner::SpawnInto(
		Graph, Collision, PositiveXWallParams, Parent );
	if ( !Room.PositiveXWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	Room.NegativeXWall = CBlock3DSpawner::SpawnInto(
		Graph, Collision, NegativeXWallParams, Parent );
	if ( !Room.NegativeXWall )
	{
		Rollback_Internal( Graph, Collision, Room );
		return {};
	}

	return Room;
}


bool CRoom3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FRoom3DSpawnResult& Room,
	const FRoom3DSpawnParams& Params ) noexcept
{
	FGround3DSpawnParams FloorParams;
	FBlock3DSpawnParams PositiveZWallParams;
	FBlock3DSpawnParams NegativeZWallParams;
	FBlock3DSpawnParams PositiveXWallParams;
	FBlock3DSpawnParams NegativeXWallParams;
	if ( !TryBuildParts_Internal( Params, FloorParams,
		PositiveZWallParams, NegativeZWallParams,
		PositiveXWallParams, NegativeXWallParams )
		|| !CanApply_Internal( Graph, Collision, Room ) ) return false;

	// 全5組の事前確認後は、既存形状のレイヤーと表示値だけを更新する。
	if ( !CGround3DSpawner::TryApplyTo(
		Graph, Collision, Room.Floor, FloorParams ) ) return false;

	const FCollidableModel3DSpawnResult* const Walls[]
	{
		&Room.PositiveZWall,
		&Room.NegativeZWall,
		&Room.PositiveXWall,
		&Room.NegativeXWall,
	};
	const FBlock3DSpawnParams* const WallParams[]
	{
		&PositiveZWallParams,
		&NegativeZWallParams,
		&PositiveXWallParams,
		&NegativeXWallParams,
	};
	for ( usize Index = 0u; Index < kPartCount - 1u; ++Index )
	{
		if ( !CBlock3DSpawner::TryApplyTo(
			Graph, Collision, *Walls[Index], *WallParams[Index] ) ) return false;
	}
	return true;
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


bool CRoom3DSpawner::TryBuildParts_Internal(
	const FRoom3DSpawnParams& Params,
	FGround3DSpawnParams& OutFloor,
	FBlock3DSpawnParams& OutPositiveZWall,
	FBlock3DSpawnParams& OutNegativeZWall,
	FBlock3DSpawnParams& OutPositiveXWall,
	FBlock3DSpawnParams& OutNegativeXWall ) noexcept
{
	if ( !Params.IsValid() ) return false;

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
	FGround3DSpawnParams Floor = FGround3DSpawnParams::FromSize(
		FVec2{ OuterWidth, OuterDepth }, Params.FloorTopPosition );
	Floor.Thickness = Params.FloorThickness;
	Floor.Color = Params.FloorColor;
	Floor.Metallic = Params.FloorMetallic;
	Floor.Roughness = Params.FloorRoughness;
	Floor.bCastsShadow = Params.bFloorCastsShadow;
	Floor.CollisionLayer = Params.CollisionLayer;
	Floor.Name = FStringView( "RoomFloor" );

	// 材質と衝突設定を共有し、位置と寸法だけを各壁で差し替える。
	FBlock3DSpawnParams PositiveZWall = FBlock3DSpawnParams::FromSize(
		FVec3{ OuterWidth, Params.WallHeight, Params.WallThickness },
		FVec3{ Params.FloorTopPosition.x, WallCenterY,
			Params.FloorTopPosition.z + WallOffsetZ } );
	PositiveZWall.Color = Params.WallColor;
	PositiveZWall.Metallic = Params.WallMetallic;
	PositiveZWall.Roughness = Params.WallRoughness;
	PositiveZWall.bCastsShadow = Params.bWallsCastShadow;
	PositiveZWall.CollisionLayer = Params.CollisionLayer;
	PositiveZWall.Name = FStringView( "RoomPositiveZWall" );

	FBlock3DSpawnParams NegativeZWall = PositiveZWall;
	NegativeZWall.Position.z = Params.FloorTopPosition.z - WallOffsetZ;
	NegativeZWall.Name = FStringView( "RoomNegativeZWall" );

	FBlock3DSpawnParams PositiveXWall = PositiveZWall;
	PositiveXWall.Position = FVec3{ Params.FloorTopPosition.x + WallOffsetX,
		WallCenterY, Params.FloorTopPosition.z };
	PositiveXWall.Size = FVec3{
		Params.WallThickness, Params.WallHeight, Params.InnerSize.y };
	PositiveXWall.Name = FStringView( "RoomPositiveXWall" );

	FBlock3DSpawnParams NegativeXWall = PositiveXWall;
	NegativeXWall.Position.x = Params.FloorTopPosition.x - WallOffsetX;
	NegativeXWall.Name = FStringView( "RoomNegativeXWall" );

	OutFloor = Floor;
	OutPositiveZWall = PositiveZWall;
	OutNegativeZWall = NegativeZWall;
	OutPositiveXWall = PositiveXWall;
	OutNegativeXWall = NegativeXWall;
	return true;
}


bool CRoom3DSpawner::IsNodeAlive_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


bool CRoom3DSpawner::CanApply_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FRoom3DSpawnResult& Room ) noexcept
{
	if ( !Room || !Graph.HasRoot() ) return false;
	const FCollidableModel3DSpawnResult* const Parts[]
	{
		&Room.Floor,
		&Room.PositiveZWall,
		&Room.NegativeZWall,
		&Room.PositiveXWall,
		&Room.NegativeXWall,
	};
	const ANode* CommonParent = nullptr;
	for ( usize Index = 0u; Index < kPartCount; ++Index )
	{
		const FCollidableModel3DSpawnResult& Part = *Parts[Index];
		if ( !IsOwnedPart_Internal( Graph, Collision, Part )
			|| !IsNodeAlive_Internal( *Part.Node )
			|| Part.Node->GetComponent<AMeshComponent3D>() == nullptr ) return false;
		if ( Index == 0u ) CommonParent = Part.Node->Parent();
		if ( CommonParent == nullptr || Part.Node->Parent() != CommonParent ) return false;

		for ( usize OtherIndex = Index + 1u;
			OtherIndex < kPartCount; ++OtherIndex )
		{
			if ( Part.Node == Parts[OtherIndex]->Node
				|| Part.Shape == Parts[OtherIndex]->Shape ) return false;
		}
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

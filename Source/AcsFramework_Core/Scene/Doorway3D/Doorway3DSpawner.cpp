// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FDoorway3DSpawnResult CDoorway3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FDoorway3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	// 壁中心から幅方向外端までの距離。
	const f32 WallHalfWidth = Params.WallWidth * 0.5f;
	// 開口中心から左右端までの距離。
	const f32 OpeningHalfWidth = Params.OpeningWidth * 0.5f;
	// 幅軸負側の柱が埋める全幅。
	const f32 NegativePillarWidth = WallHalfWidth + Params.OpeningCenterOffset - OpeningHalfWidth;
	// 幅軸正側の柱が埋める全幅。
	const f32 PositivePillarWidth = WallHalfWidth - Params.OpeningCenterOffset - OpeningHalfWidth;
	// 開口上端から壁上端までを埋める上枠高。
	const f32 LintelHeight = Params.WallHeight - Params.OpeningHeight;
	// 幅軸負側の柱中心が壁中心から離れる距離。
	const f32 NegativePillarCenter = -WallHalfWidth + NegativePillarWidth * 0.5f;
	// 幅軸正側の柱中心が壁中心から離れる距離。
	const f32 PositivePillarCenter = WallHalfWidth - PositivePillarWidth * 0.5f;
	// 左右柱を床から上へ置くためのY中心。
	const f32 PillarCenterY = Params.BottomCenter.y + Params.WallHeight * 0.5f;
	// 上枠を開口上へ置くためのY中心。
	const f32 LintelCenterY = Params.BottomCenter.y + Params.OpeningHeight + LintelHeight * 0.5f;

	// X方向へ伸びる既定壁に使う幅軸負側の柱中心。
	FVec3 NegativePillarPosition{ Params.BottomCenter.x + NegativePillarCenter,
		PillarCenterY, Params.BottomCenter.z };
	// X方向へ伸びる既定壁に使う幅軸正側の柱中心。
	FVec3 PositivePillarPosition{ Params.BottomCenter.x + PositivePillarCenter,
		PillarCenterY, Params.BottomCenter.z };
	// X方向へ伸びる既定壁に使う上枠中心。
	FVec3 LintelPosition{ Params.BottomCenter.x + Params.OpeningCenterOffset,
		LintelCenterY, Params.BottomCenter.z };
	// X方向へ伸びる既定壁に使う幅軸負側の柱寸法。
	FVec3 NegativePillarSize{ NegativePillarWidth, Params.WallHeight, Params.WallThickness };
	// X方向へ伸びる既定壁に使う幅軸正側の柱寸法。
	FVec3 PositivePillarSize{ PositivePillarWidth, Params.WallHeight, Params.WallThickness };
	// X方向へ伸びる既定壁に使う上枠寸法。
	FVec3 LintelSize{ Params.OpeningWidth, LintelHeight, Params.WallThickness };

	if ( Params.Orientation == EDoorway3DOrientation::AlongZ )
	{
		NegativePillarPosition = FVec3{ Params.BottomCenter.x, PillarCenterY,
			Params.BottomCenter.z + NegativePillarCenter };
		PositivePillarPosition = FVec3{ Params.BottomCenter.x, PillarCenterY,
			Params.BottomCenter.z + PositivePillarCenter };
		LintelPosition = FVec3{ Params.BottomCenter.x, LintelCenterY,
			Params.BottomCenter.z + Params.OpeningCenterOffset };
		NegativePillarSize = FVec3{ Params.WallThickness, Params.WallHeight, NegativePillarWidth };
		PositivePillarSize = FVec3{ Params.WallThickness, Params.WallHeight, PositivePillarWidth };
		LintelSize = FVec3{ Params.WallThickness, LintelHeight, Params.OpeningWidth };
	}

	// 3部分で見た目と衝突設定を共有し、寸法、位置、名前だけを差し替える。
	FBlock3DSpawnParams PartParams = FBlock3DSpawnParams::FromSize(
		NegativePillarSize, NegativePillarPosition );
	PartParams.Color = Params.Color;
	PartParams.Metallic = Params.Metallic;
	PartParams.Roughness = Params.Roughness;
	PartParams.bCastsShadow = Params.bCastsShadow;
	PartParams.CollisionLayer = Params.CollisionLayer;
	PartParams.Name = Params.NegativePillarName;

	// 途中失敗時に生成済み部分を逆順で戻すため、順番に保持する結果。
	FDoorway3DSpawnResult Doorway;
	Doorway.NegativePillar = CBlock3DSpawner::SpawnInto(
		Graph, Collision, PartParams, Parent );
	if ( !Doorway.NegativePillar ) return {};

	PartParams.Size = PositivePillarSize;
	PartParams.Position = PositivePillarPosition;
	PartParams.Name = Params.PositivePillarName;
	Doorway.PositivePillar = CBlock3DSpawner::SpawnInto(
		Graph, Collision, PartParams, Parent );
	if ( !Doorway.PositivePillar )
	{
		Rollback_Internal( Graph, Collision, Doorway );
		return {};
	}

	PartParams.Size = LintelSize;
	PartParams.Position = LintelPosition;
	PartParams.Name = Params.LintelName;
	Doorway.Lintel = CBlock3DSpawner::SpawnInto( Graph, Collision, PartParams, Parent );
	if ( !Doorway.Lintel )
	{
		Rollback_Internal( Graph, Collision, Doorway );
		return {};
	}

	return Doorway;
}


bool CDoorway3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FDoorway3DSpawnResult& Doorway ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, Doorway ) ) return false;

	// 生成と逆の順で上枠から左右柱へ片付ける。
	FCollidableModel3DSpawnResult* const Parts[]
	{
		&Doorway.Lintel,
		&Doorway.PositivePillar,
		&Doorway.NegativePillar,
	};
	for ( usize Index = 0u; Index < kPartCount; ++Index )
	{
		if ( !CModel3DSpawner::DestroyCollidable( Graph, Collision, *Parts[Index] ) ) return false;
	}
	return true;
}


bool CDoorway3DSpawner::IsOwnedPart_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept
{
	if ( !Part || Part.Node == &Graph.Root() ) return false;
	// ポインターだけでなく、この場面が発行した生存中の識別子でも所有を確かめる。
	const FNodeId NodeId = Graph.IdOf( Part.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Part.Node
		&& Collision.IsRegisteredTo( Part.Shape, *Part.Node );
}


bool CDoorway3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FDoorway3DSpawnResult& Doorway ) noexcept
{
	if ( !Doorway ) return false;
	// 生成結果の全要素を同じ規則で所有確認するための一覧。
	const FCollidableModel3DSpawnResult* const Parts[]
	{
		&Doorway.NegativePillar,
		&Doorway.PositivePillar,
		&Doorway.Lintel,
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


void CDoorway3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FDoorway3DSpawnResult& Doorway ) noexcept
{
	if ( Doorway.Lintel ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Doorway.Lintel );
	if ( Doorway.PositivePillar ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Doorway.PositivePillar );
	if ( Doorway.NegativePillar ) (void)CModel3DSpawner::DestroyCollidable(
		Graph, Collision, Doorway.NegativePillar );
}

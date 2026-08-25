// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 通路の3部分が指定した親へ繋がっているか返す。 */
	bool AllPartsUseParent( const FCorridor3DSpawnResult& Corridor,
		const ANode& Parent ) noexcept
	{
		return Corridor.Floor.Node != nullptr && Corridor.Floor.Node->Parent() == &Parent
			&& Corridor.NegativeSideWall.Node != nullptr
			&& Corridor.NegativeSideWall.Node->Parent() == &Parent
			&& Corridor.PositiveSideWall.Node != nullptr
			&& Corridor.PositiveSideWall.Node->Parent() == &Parent;
	}

	/** 通路の3部分が破棄予定ではないか返す。 */
	bool NoPartIsPendingDestroy( const FCorridor3DSpawnResult& Corridor ) noexcept
	{
		return Corridor.Floor.Node != nullptr && !Corridor.Floor.Node->IsPendingDestroy()
			&& Corridor.NegativeSideWall.Node != nullptr
			&& !Corridor.NegativeSideWall.Node->IsPendingDestroy()
			&& Corridor.PositiveSideWall.Node != nullptr
			&& !Corridor.PositiveSideWall.Node->IsPendingDestroy();
	}

	/** 通路部品の表示Meshを返す。 */
	AMeshComponent3D* MeshOf_Internal(
		const FCollidableModel3DSpawnResult& Part ) noexcept
	{
		return Part.Node != nullptr
			? Part.Node->GetComponent<AMeshComponent3D>() : nullptr;
	}

	/** 3成分を許容誤差付きで確認する。 */
	void CheckVector_Internal( CTestHarness& Harness,
		FVec3 Actual, FVec3 Expected, const char* Message )
	{
		Harness.CheckNearF32( Actual.x, Expected.x, 0.001f, Message );
		Harness.CheckNearF32( Actual.y, Expected.y, 0.001f, Message );
		Harness.CheckNearF32( Actual.z, Expected.z, 0.001f, Message );
	}

	/** 更新成功と失敗の基準にする初期通路指定を作る。 */
	FCorridor3DSpawnParams MakeInitialUpdateParams_Internal() noexcept
	{
		FCorridor3DSpawnParams Params = FCorridor3DSpawnParams::FromDimensions(
			2.0f, 4.0f, 2.0f, FVec3{ 1.0f, 0.0f, 2.0f },
			ECorridor3DDirection::PositiveZ );
		Params.FloorColor = FVec4{ 0.20f, 0.30f, 0.40f, 1.0f };
		Params.WallColor = FVec4{ 0.50f, 0.60f, 0.70f, 1.0f };
		Params.FloorMetallic = 0.10f;
		Params.FloorRoughness = 0.80f;
		Params.WallMetallic = 0.20f;
		Params.WallRoughness = 0.60f;
		Params.CollisionLayer = 0x2u;
		Params.FloorName = FStringView( "BeforeCorridorFloor" );
		Params.NegativeWallName = FStringView( "BeforeCorridorNegativeWall" );
		Params.PositiveWallName = FStringView( "BeforeCorridorPositiveWall" );
		return Params;
	}

	/** X負方向へ向きを変える更新後の通路指定を作る。 */
	FCorridor3DSpawnParams MakeUpdatedParams_Internal() noexcept
	{
		FCorridor3DSpawnParams Params = FCorridor3DSpawnParams::FromDimensions(
			4.0f, 6.0f, 2.0f, FVec3{ 2.0f, 3.0f, 4.0f },
			ECorridor3DDirection::NegativeX );
		Params.WallThickness = 0.5f;
		Params.FloorThickness = 0.4f;
		Params.FloorColor = FVec4{ 0.10f, 0.25f, 0.75f, 1.0f };
		Params.WallColor = FVec4{ 0.80f, 0.45f, 0.20f, 1.0f };
		Params.FloorMetallic = 0.60f;
		Params.FloorRoughness = 0.20f;
		Params.WallMetallic = 0.30f;
		Params.WallRoughness = 0.55f;
		Params.bFloorCastsShadow = true;
		Params.bWallsCastShadow = false;
		Params.CollisionLayer = 0x40u;
		Params.FloorName = FStringView( "AfterCorridorFloor" );
		Params.NegativeWallName = FStringView( "AfterCorridorNegativeWall" );
		Params.PositiveWallName = FStringView( "AfterCorridorPositiveWall" );
		return Params;
	}

	/** 失敗した更新が初期通路の表示と衝突を部分変更していないか調べる。 */
	void CheckInitialUpdateState_Internal( CTestHarness& Harness,
		CSceneCollision3D& Collision,
		const FCorridor3DSpawnResult& Corridor,
		bool bPositiveMeshExpected = true )
	{
		if ( !Corridor )
		{
			Harness.Check( false, "失敗後の状態確認に有効な通路がある" );
			return;
		}

		CheckVector_Internal( Harness, Corridor.Floor.Node->Local().position,
			FVec3{ 1.0f, 0.0f, 4.0f }, "失敗時は床位置を変えない" );
		CheckVector_Internal( Harness, Corridor.Floor.Node->Local().scale,
			FVec3{ 2.5f, 0.5f, 4.0f }, "失敗時は床寸法を変えない" );
		CheckVector_Internal( Harness,
			Corridor.NegativeSideWall.Node->Local().position,
			FVec3{ -0.125f, 1.0f, 4.0f }, "失敗時は負側壁位置を変えない" );
		CheckVector_Internal( Harness,
			Corridor.PositiveSideWall.Node->Local().position,
			FVec3{ 2.125f, 1.0f, 4.0f }, "失敗時は正側壁位置を変えない" );
		CheckVector_Internal( Harness,
			Corridor.NegativeSideWall.Node->Local().scale,
			FVec3{ 0.25f, 2.0f, 4.0f }, "失敗時は側壁寸法を変えない" );
		Harness.Check( Corridor.Floor.Node->Name()
			== FStringView( "BeforeCorridorFloor" )
			&& Corridor.NegativeSideWall.Node->Name()
			== FStringView( "BeforeCorridorNegativeWall" )
			&& Corridor.PositiveSideWall.Node->Name()
			== FStringView( "BeforeCorridorPositiveWall" ),
			"失敗時は3部品の名前を変えない" );

		AMeshComponent3D* const FloorMesh = MeshOf_Internal( Corridor.Floor );
		AMeshComponent3D* const NegativeMesh = MeshOf_Internal(
			Corridor.NegativeSideWall );
		AMeshComponent3D* const PositiveMesh = MeshOf_Internal(
			Corridor.PositiveSideWall );
		Harness.Check( FloorMesh != nullptr && NegativeMesh != nullptr,
			"失敗後も床と負側壁の表示部品を保つ" );
		Harness.Check( bPositiveMeshExpected
			? PositiveMesh != nullptr : PositiveMesh == nullptr,
			"正側壁の事前表示部品状態を変えない" );
		if ( FloorMesh != nullptr && NegativeMesh != nullptr )
		{
			Harness.CheckNearF32( FloorMesh->Color().z, 0.40f, 0.001f,
				"失敗時は床色を変えない" );
			Harness.CheckNearF32( FloorMesh->Material().pbr.metallic,
				0.10f, 0.001f, "失敗時は床金属度を変えない" );
			Harness.CheckNearF32( NegativeMesh->Color().x, 0.50f, 0.001f,
				"失敗時は壁色を変えない" );
			Harness.CheckNearF32( NegativeMesh->Material().pbr.roughness,
				0.60f, 0.001f, "失敗時は壁粗さを変えない" );
		}

		FWorldCollisionShape3D FloorShape;
		FWorldCollisionShape3D NegativeShape;
		FWorldCollisionShape3D PositiveShape;
		const bool bHasShapes = Collision.TryGetWorldShape(
			Corridor.Floor.Shape, FloorShape )
			&& Collision.TryGetWorldShape(
				Corridor.NegativeSideWall.Shape, NegativeShape )
			&& Collision.TryGetWorldShape(
				Corridor.PositiveSideWall.Shape, PositiveShape );
		Harness.Check( bHasShapes, "失敗後も3形状を読み出せる" );
		if ( bHasShapes )
		{
			Harness.Check( FloorShape.Layer == 0x2u
				&& NegativeShape.Layer == 0x2u
				&& PositiveShape.Layer == 0x2u,
				"失敗時は3形状のレイヤーを変えない" );
		}
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u,
			"失敗時は3形状を作り直さない" );
	}

	/** 指定方向の床中心、床寸法、側壁位置と寸法を調べる。 */
	void CheckDirection( CTestHarness& Harness, ECorridor3DDirection Direction,
		f32 ExpectedX, f32 ExpectedZ, f32 ExpectedFloorSizeX,
		f32 ExpectedFloorSizeZ, f32 ExpectedNegativeWallX,
		f32 ExpectedNegativeWallZ, f32 ExpectedWallSizeX,
		f32 ExpectedWallSizeZ )
	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FCorridor3DSpawnParams Params = FCorridor3DSpawnParams::FromDimensions(
			2.0f, 4.0f, 3.0f, FVec3{}, Direction );
		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Corridor.Succeeded(), "指定方向へ通路を置ける" );
		if ( Corridor.Succeeded() )
		{
			Harness.CheckNearF32( Corridor.Floor.Node->Local().position.x,
				ExpectedX, 0.001f, "床中心Xを出口側へ進める" );
			Harness.CheckNearF32( Corridor.Floor.Node->Local().position.z,
				ExpectedZ, 0.001f, "床中心Zを出口側へ進める" );
			Harness.CheckNearF32( Corridor.Floor.Node->Local().scale.x,
				ExpectedFloorSizeX, 0.001f, "床のX寸法を方向へ合わせる" );
			Harness.CheckNearF32( Corridor.Floor.Node->Local().scale.z,
				ExpectedFloorSizeZ, 0.001f, "床のZ寸法を方向へ合わせる" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().position.x,
				ExpectedNegativeWallX, 0.001f, "負側壁のX位置を幅軸へ合わせる" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().position.z,
				ExpectedNegativeWallZ, 0.001f, "負側壁のZ位置を幅軸へ合わせる" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().scale.x,
				ExpectedWallSizeX, 0.001f, "側壁のX寸法を方向へ合わせる" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().scale.z,
				ExpectedWallSizeZ, 0.001f, "側壁のZ寸法を方向へ合わせる" );
		}
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"方向確認後の通路を片付けられる" );
	}
}


void RunCorridor3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FCorridor3DSpawnParams / 既定値だけで衝突付き通路になる" );

	{
		const FCorridor3DSpawnParams Corridor;
		Harness.Check( Corridor.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Corridor.InnerWidth, 3.0f, "既定の内幅" );
		Harness.CheckEqualF32( Corridor.Length, 8.0f, "既定の長さ" );
		Harness.CheckEqualF32( Corridor.WallHeight, 3.0f, "既定の壁高" );
		Harness.Check( Corridor.Direction == ECorridor3DDirection::PositiveZ,
			"既定でZ正方向へ伸びる" );
		Harness.Check( Corridor.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );

		const FCorridor3DSpawnParams Sized = FCorridor3DSpawnParams::FromDimensions(
			4.0f, 12.0f, 3.5f, FVec3{ 1.0f, 2.0f, 3.0f },
			ECorridor3DDirection::NegativeX );
		Harness.Check( Sized.IsValid(), "内幅、長さ、壁高、方向だけで設定を作れる" );
		Harness.CheckEqualF32( Sized.InnerWidth, 4.0f, "指定した内幅" );
		Harness.CheckEqualF32( Sized.Length, 12.0f, "指定した長さ" );
		Harness.CheckEqualF32( Sized.EntranceCenter.y, 2.0f, "指定した床上高さ" );
		Harness.Check( Sized.Direction == ECorridor3DDirection::NegativeX,
			"指定した方向を保つ" );
		Harness.Check( FCorridor3DSpawnResult{}.IsEmpty(), "既定の生成結果は空" );
	}

	Harness.BeginSuite( "FCorridor3DSpawnParams / 半端な通路を作る値を配置前に弾く" );

	{
		FCorridor3DSpawnParams Broken;
		Broken.InnerWidth = 0.0f;
		Harness.Check( !Broken.IsValid(), "内幅0を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.Length = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない長さを拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.WallHeight = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない壁高を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.WallThickness = -0.1f;
		Harness.Check( !Broken.IsValid(), "負の壁厚を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.FloorThickness = 0.0f;
		Harness.Check( !Broken.IsValid(), "床厚0を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.EntranceCenter.x = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない入口を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.Direction = static_cast<ECorridor3DDirection>( 0xffu );
		Harness.Check( !Broken.IsValid(), "未知の方向を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.FloorColor.x = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の床色を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.WallColor.w = -0.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の壁色を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.FloorMetallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の床金属度を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.WallRoughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない壁粗さを拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.InnerWidth = std::numeric_limits<f32>::max();
		Broken.WallThickness = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な外幅にならない値を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.Length = std::numeric_limits<f32>::max();
		Broken.EntranceCenter.z = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な出口位置にならない値を拒否する" );

		Broken = FCorridor3DSpawnParams{};
		Broken.WallHeight = std::numeric_limits<f32>::max();
		Broken.EntranceCenter.y = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な壁上端にならない値を拒否する" );
	}

	Harness.BeginSuite( "CCorridor3DSpawner / 入口から出口まで床と側壁を置く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FCorridor3DSpawnParams Params = FCorridor3DSpawnParams::FromDimensions(
			4.0f, 6.0f, 3.0f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.WallThickness = 0.2f;
		Params.FloorThickness = 0.5f;
		Params.FloorColor = FVec4{ 0.20f, 0.25f, 0.35f, 1.0f };
		Params.WallColor = FVec4{ 0.55f, 0.60f, 0.70f, 1.0f };
		Params.FloorMetallic = 0.1f;
		Params.FloorRoughness = 0.7f;
		Params.WallMetallic = 0.2f;
		Params.WallRoughness = 0.4f;
		Params.bFloorCastsShadow = true;
		Params.bWallsCastShadow = false;
		Params.CollisionLayer = 0x4u;
		Params.FloorName = FStringView( "StoneCorridorFloor" );
		Params.NegativeWallName = FStringView( "StoneCorridorNegativeWall" );
		Params.PositiveWallName = FStringView( "StoneCorridorPositiveWall" );

		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Corridor.Succeeded(), "床と側壁2枚を一括生成できる" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 3u, "表示ノードを3個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "箱型衝突を3個だけ登録する" );

		if ( Corridor.Succeeded() )
		{
			Harness.Check( Corridor.Floor.Node->Name() == FStringView( "StoneCorridorFloor" ),
				"床へ指定名を付ける" );
			Harness.Check( Corridor.NegativeSideWall.Node->Name()
				== FStringView( "StoneCorridorNegativeWall" ), "負側壁へ指定名を付ける" );
			Harness.CheckNearF32( Corridor.Floor.Node->Local().position.z, 6.0f,
				0.001f, "床中心を入口から長さ半分先へ置く" );
			Harness.CheckNearF32( Corridor.Floor.Node->Local().scale.x, 4.4f,
				0.001f, "床を両壁の外面まで広げる" );
			Harness.CheckNearF32( Corridor.Floor.Node->Local().scale.z, 6.0f,
				0.001f, "床を入口から出口まで伸ばす" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().position.x,
				-1.1f, 0.001f, "負側壁を内幅の外側へ置く" );
			Harness.CheckNearF32( Corridor.PositiveSideWall.Node->Local().position.x,
				3.1f, 0.001f, "正側壁を内幅の外側へ置く" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().position.y,
				3.5f, 0.001f, "側壁中心を床上面から壁高半分上へ置く" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().scale.y,
				3.0f, 0.001f, "側壁へ壁高を使う" );
			Harness.CheckNearF32( Corridor.NegativeSideWall.Node->Local().scale.z,
				6.0f, 0.001f, "側壁を入口から出口まで伸ばす" );
		}

		const AMeshComponent3D* const FloorMesh = Corridor.Succeeded()
			? Corridor.Floor.Node->GetComponent<AMeshComponent3D>() : nullptr;
		const AMeshComponent3D* const WallMesh = Corridor.Succeeded()
			? Corridor.NegativeSideWall.Node->GetComponent<AMeshComponent3D>() : nullptr;
		Harness.Check( FloorMesh != nullptr && FloorMesh->Primitive() == EMeshPrimitive3D::Plane,
			"床は平面表示を使う" );
		Harness.Check( WallMesh != nullptr && WallMesh->Primitive() == EMeshPrimitive3D::Cube,
			"側壁は立方体表示を使う" );
		if ( FloorMesh != nullptr && WallMesh != nullptr )
		{
			Harness.CheckEqualF32( FloorMesh->Color().z, 0.35f, "指定した床色を使う" );
			Harness.CheckEqualF32( FloorMesh->Material().pbr.metallic, 0.1f,
				"指定した床金属度を使う" );
			Harness.Check( FloorMesh->CastsShadow(), "指定した床の影設定を使う" );
			Harness.CheckEqualF32( WallMesh->Color().z, 0.70f, "指定した壁色を使う" );
			Harness.CheckEqualF32( WallMesh->Material().pbr.roughness, 0.4f,
				"指定した壁粗さを使う" );
			Harness.Check( !WallMesh->CastsShadow(), "指定した壁の影設定を使う" );
		}

		FWorldCollisionShape3D FloorShape;
		FWorldCollisionShape3D WallShape;
		Harness.Check( Corridor.Succeeded()
			&& Collision.TryGetWorldShape( Corridor.Floor.Shape, FloorShape ),
			"床のworld衝突を読める" );
		Harness.Check( Corridor.Succeeded()
			&& Collision.TryGetWorldShape( Corridor.NegativeSideWall.Shape, WallShape ),
			"側壁のworld衝突を読める" );
		Harness.Check( FloorShape.Kind == FWorldCollisionShape3D::EKind::Box
			&& WallShape.Kind == FWorldCollisionShape3D::EKind::Box, "床と側壁の衝突は箱" );
		Harness.CheckNearF32( FloorShape.Box.center.y, 1.75f, 0.001f,
			"床衝突を上面から下へ持たせる" );
		Harness.CheckNearF32( FloorShape.Box.half_size.x, 2.2f, 0.001f,
			"床衝突を壁外面まで広げる" );
		Harness.Check( FloorShape.Layer == 0x4u && WallShape.Layer == 0x4u,
			"床と側壁へ同じ衝突レイヤーを使う" );
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"確認後の通路を片付けられる" );
	}

	Harness.BeginSuite( "CCorridor3DSpawner / XとZの正負4方向へ内幅を保って伸ばす" );

	CheckDirection( Harness, ECorridor3DDirection::PositiveX,
		2.0f, 0.0f, 4.0f, 2.5f, 2.0f, -1.125f, 4.0f, 0.25f );
	CheckDirection( Harness, ECorridor3DDirection::NegativeX,
		-2.0f, 0.0f, 4.0f, 2.5f, -2.0f, -1.125f, 4.0f, 0.25f );
	CheckDirection( Harness, ECorridor3DDirection::PositiveZ,
		0.0f, 2.0f, 2.5f, 4.0f, -1.125f, 2.0f, 0.25f, 4.0f );
	CheckDirection( Harness, ECorridor3DDirection::NegativeZ,
		0.0f, -2.0f, 2.5f, 4.0f, -1.125f, -2.0f, 0.25f, 4.0f );

	Harness.BeginSuite( "CCorridor3DSpawner / 指定親の変形を3部分へ共通適用する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn( FStringView( "CorridorRoot" ) );
		Harness.Check( Parent.Succeeded(), "通路を繋ぐ親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 1.0f, 3.0f } );
		}

		const FCorridor3DSpawnParams Params = FCorridor3DSpawnParams::FromDimensions(
			2.0f, 4.0f, 3.0f, FVec3{ 1.0f, 2.0f, 3.0f } );
		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, Params, Parent.Node );
		Harness.Check( Corridor.Succeeded(), "指定親の下へ通路を置ける" );
		if ( Parent.Node != nullptr )
		{
			Harness.CheckEqualU64( Parent.Node->ChildCount(), 3u, "親の直下へ3部分を置く" );
			Harness.Check( AllPartsUseParent( Corridor, *Parent.Node ), "3部分が同じ親を使う" );
		}

		FWorldCollisionShape3D FloorShape;
		Harness.Check( Corridor.Succeeded()
			&& Collision.TryGetWorldShape( Corridor.Floor.Shape, FloorShape ),
			"親変形後の床衝突を読める" );
		Harness.CheckNearF32( FloorShape.Box.center.x, 12.0f, 0.001f,
			"親のX変形を反映する" );
		Harness.CheckNearF32( FloorShape.Box.center.y, 2.75f, 0.001f,
			"親のY変形と床厚を反映する" );
		Harness.CheckNearF32( FloorShape.Box.center.z, 10.0f, 0.001f,
			"親のZ変形を反映する" );
		Harness.CheckNearF32( FloorShape.Box.half_size.x, 2.5f, 0.001f,
			"親X尺度を外幅へ反映する" );
		Harness.CheckNearF32( FloorShape.Box.half_size.z, 6.0f, 0.001f,
			"親Z尺度を長さへ反映する" );
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"親付き通路を片付けられる" );
	}

	Harness.BeginSuite( "CCorridor3DSpawner / 既存3部品を方向変更込みで同期更新する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "MovingCorridorRoot" ) );
		Harness.Check( Parent.Succeeded(), "更新確認用の親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 1.0f, 3.0f } );
		}

		const FCorridor3DSpawnParams Initial =
			MakeInitialUpdateParams_Internal();
		const FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, Initial, Parent.Node );
		Harness.Check( Corridor.Succeeded(), "更新確認用の通路を置ける" );
		const FNodeId OriginalFloorNode = Graph.IdOf( Corridor.Floor.Node );
		const FNodeId OriginalNegativeNode = Graph.IdOf(
			Corridor.NegativeSideWall.Node );
		const FNodeId OriginalPositiveNode = Graph.IdOf(
			Corridor.PositiveSideWall.Node );
		const FCollisionShapeId3D OriginalFloorShape = Corridor.Floor.Shape;
		const FCollisionShapeId3D OriginalNegativeShape =
			Corridor.NegativeSideWall.Shape;
		const FCollisionShapeId3D OriginalPositiveShape =
			Corridor.PositiveSideWall.Shape;

		const FCorridor3DSpawnParams Updated = MakeUpdatedParams_Internal();
		Harness.Check( CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Corridor, Updated ),
			"入口と方向を含む新指定を通路全体へ反映できる" );
		Harness.Check( Graph.IdOf( Corridor.Floor.Node ) == OriginalFloorNode
			&& Graph.IdOf( Corridor.NegativeSideWall.Node ) == OriginalNegativeNode
			&& Graph.IdOf( Corridor.PositiveSideWall.Node ) == OriginalPositiveNode,
			"更新しても床と側壁のノード番号を保つ" );
		Harness.Check( Corridor.Floor.Shape == OriginalFloorShape
			&& Corridor.NegativeSideWall.Shape == OriginalNegativeShape
			&& Corridor.PositiveSideWall.Shape == OriginalPositiveShape,
			"更新しても3形状の世代付き番号を保つ" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 5u,
			"更新でノードを作り直さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u,
			"更新で衝突形状を作り直さない" );
		if ( Corridor && Parent.Node != nullptr )
		{
			Harness.Check( AllPartsUseParent( Corridor, *Parent.Node ),
				"更新後も3部品が元の親を共有する" );
			CheckVector_Internal( Harness, Corridor.Floor.Node->Local().position,
				FVec3{ -1.0f, 3.0f, 4.0f },
				"X負方向へ変えた床中心を反映する" );
			CheckVector_Internal( Harness, Corridor.Floor.Node->Local().scale,
				FVec3{ 6.0f, 0.4f, 5.0f },
				"方向変更後の床寸法を反映する" );
			CheckVector_Internal( Harness,
				Corridor.NegativeSideWall.Node->Local().position,
				FVec3{ -1.0f, 4.0f, 1.75f },
				"方向変更後の負側壁位置を反映する" );
			CheckVector_Internal( Harness,
				Corridor.PositiveSideWall.Node->Local().position,
				FVec3{ -1.0f, 4.0f, 6.25f },
				"方向変更後の正側壁位置を反映する" );
			CheckVector_Internal( Harness,
				Corridor.NegativeSideWall.Node->Local().scale,
				FVec3{ 6.0f, 2.0f, 0.5f },
				"方向変更後の側壁寸法を反映する" );
			Harness.Check( Corridor.Floor.Node->Name()
				== FStringView( "AfterCorridorFloor" )
				&& Corridor.NegativeSideWall.Node->Name()
				== FStringView( "AfterCorridorNegativeWall" )
				&& Corridor.PositiveSideWall.Node->Name()
				== FStringView( "AfterCorridorPositiveWall" ),
				"床と側壁2枚の名前を同期更新する" );
		}

		AMeshComponent3D* const FloorMesh = MeshOf_Internal( Corridor.Floor );
		AMeshComponent3D* const NegativeMesh = MeshOf_Internal(
			Corridor.NegativeSideWall );
		AMeshComponent3D* const PositiveMesh = MeshOf_Internal(
			Corridor.PositiveSideWall );
		Harness.Check( FloorMesh != nullptr && NegativeMesh != nullptr
			&& PositiveMesh != nullptr, "更新後も3表示部品を保つ" );
		if ( FloorMesh != nullptr && NegativeMesh != nullptr
			&& PositiveMesh != nullptr )
		{
			Harness.Check( FloorMesh->Primitive() == EMeshPrimitive3D::Plane
				&& NegativeMesh->Primitive() == EMeshPrimitive3D::Cube
				&& PositiveMesh->Primitive() == EMeshPrimitive3D::Cube,
				"床と側壁の表示プリミティブを保つ" );
			Harness.CheckNearF32( FloorMesh->Color().z, 0.75f, 0.001f,
				"床を新しい色へ更新する" );
			Harness.CheckNearF32( FloorMesh->Material().pbr.metallic,
				0.60f, 0.001f, "床を新しい金属度へ更新する" );
			Harness.CheckNearF32( FloorMesh->Material().pbr.roughness,
				0.20f, 0.001f, "床を新しい粗さへ更新する" );
			Harness.Check( FloorMesh->CastsShadow(),
				"床を新しい影設定へ更新する" );
			Harness.CheckNearF32( NegativeMesh->Color().x, 0.80f, 0.001f,
				"負側壁を新しい色へ更新する" );
			Harness.CheckNearF32( PositiveMesh->Material().pbr.metallic,
				0.30f, 0.001f, "正側壁を新しい金属度へ更新する" );
			Harness.CheckNearF32( PositiveMesh->Material().pbr.roughness,
				0.55f, 0.001f, "正側壁を新しい粗さへ更新する" );
			Harness.Check( !NegativeMesh->CastsShadow()
				&& !PositiveMesh->CastsShadow(),
				"側壁2枚を新しい影設定へ更新する" );
		}

		FWorldCollisionShape3D FloorShape;
		FWorldCollisionShape3D NegativeShape;
		FWorldCollisionShape3D PositiveShape;
		const bool bHasWorldShapes = Collision.TryGetWorldShape(
			Corridor.Floor.Shape, FloorShape )
			&& Collision.TryGetWorldShape(
				Corridor.NegativeSideWall.Shape, NegativeShape )
			&& Collision.TryGetWorldShape(
				Corridor.PositiveSideWall.Shape, PositiveShape );
		Harness.Check( bHasWorldShapes, "更新後の3つのworld箱を読める" );
		if ( bHasWorldShapes )
		{
			CheckVector_Internal( Harness, FloorShape.Box.center,
				FVec3{ 8.0f, 3.8f, 7.0f },
				"親変形込みの床衝突中心を反映する" );
			CheckVector_Internal( Harness, FloorShape.Box.half_size,
				FVec3{ 6.0f, 0.2f, 7.5f },
				"親変形込みの床衝突半寸法を反映する" );
			CheckVector_Internal( Harness, NegativeShape.Box.center,
				FVec3{ 8.0f, 5.0f, 0.25f },
				"親変形込みの負側壁衝突中心を反映する" );
			CheckVector_Internal( Harness, PositiveShape.Box.center,
				FVec3{ 8.0f, 5.0f, 13.75f },
				"親変形込みの正側壁衝突中心を反映する" );
			CheckVector_Internal( Harness, NegativeShape.Box.half_size,
				FVec3{ 6.0f, 1.0f, 0.75f },
				"親変形込みの側壁衝突半寸法を反映する" );
			Harness.Check( FloorShape.Layer == 0x40u
				&& NegativeShape.Layer == 0x40u
				&& PositiveShape.Layer == 0x40u,
				"3形状を新しい共通レイヤーへ更新する" );
		}
	}

	Harness.BeginSuite( "CCorridor3DSpawner / 更新前検証の失敗では3部品を変えない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "CorridorUpdateRoot" ) );
		const FScene3DSpawnResult OtherParent = Graph.TrySpawn(
			FStringView( "OtherCorridorRoot" ) );
		const FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, MakeInitialUpdateParams_Internal(), Parent.Node );
		Harness.Check( Corridor.Succeeded(), "失敗確認用の通路を置ける" );
		const FCorridor3DSpawnParams Updated = MakeUpdatedParams_Internal();

		FCorridor3DSpawnParams Invalid = Updated;
		Invalid.Length = 0.0f;
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Corridor, Invalid ), "不正な新指定を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );

		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			OtherGraph, OtherCollision, Corridor, Updated ),
			"別場面からの更新を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );

		FCorridor3DSpawnResult Forged = Corridor;
		Forged.NegativeSideWall.Shape = Corridor.PositiveSideWall.Shape;
		Forged.PositiveSideWall.Shape = Corridor.NegativeSideWall.Shape;
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Forged, Updated ),
			"異なるノードと形状を組み直した結果を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );

		FCorridor3DSpawnResult Duplicate = Corridor;
		Duplicate.PositiveSideWall = Corridor.NegativeSideWall;
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Duplicate, Updated ),
			"ノードと形状が重複する結果を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );

		FCorridor3DSpawnResult Missing = Corridor;
		Missing.PositiveSideWall = FCollidableModel3DSpawnResult{};
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Missing, Updated ),
			"側壁が欠けた結果を拒否する" );
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, FCorridor3DSpawnResult{}, Updated ),
			"空の結果を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );

		if ( Corridor.PositiveSideWall.Node != nullptr
			&& OtherParent.Node != nullptr )
			Corridor.PositiveSideWall.Node->Reparent( *OtherParent.Node );
		Graph.ResolveStructuralChanges();
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Corridor, Updated ),
			"3部品の共通親が崩れた更新を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );
		if ( Corridor.PositiveSideWall.Node != nullptr && Parent.Node != nullptr )
			Corridor.PositiveSideWall.Node->Reparent( *Parent.Node );
		Graph.ResolveStructuralChanges();
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "PendingPartRoot" ) );
		const FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, MakeInitialUpdateParams_Internal(), Parent.Node );
		Harness.Check( Graph.Destroy( Graph.IdOf(
			Corridor.NegativeSideWall.Node ) ),
			"破棄予定部品の更新確認を準備できる" );
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Corridor, MakeUpdatedParams_Internal() ),
			"破棄予定の側壁を含む更新を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Ancestor = Graph.TrySpawn(
			FStringView( "PendingCorridorAncestor" ) );
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "PendingCorridorParent" ), Ancestor.Node );
		const FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, MakeInitialUpdateParams_Internal(), Parent.Node );
		Harness.Check( Graph.Destroy( Ancestor.Id ),
			"破棄予定祖先の更新確認を準備できる" );
		Harness.Check( NoPartIsPendingDestroy( Corridor ),
			"祖先だけが破棄予定で部品自身は生存している" );
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Corridor, MakeUpdatedParams_Internal() ),
			"破棄予定の祖先を持つ通路更新を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "MissingMeshCorridorRoot" ) );
		const FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, MakeInitialUpdateParams_Internal(), Parent.Node );
		Harness.Check( Corridor.PositiveSideWall.Node != nullptr
			&& Corridor.PositiveSideWall.Node
				->RemoveComponent<AMeshComponent3D>(),
			"表示部品欠落の更新確認を準備できる" );
		Harness.Check( !CCorridor3DSpawner::TryApplyTo(
			Graph, Collision, Corridor, MakeUpdatedParams_Internal() ),
			"表示部品が欠けた通路更新を拒否する" );
		CheckInitialUpdateState_Internal( Harness, Collision, Corridor, false );
	}

	Harness.BeginSuite( "CCorridor3DSpawner / 不正入力と別場面で半端物を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FCorridor3DSpawnParams Broken;
		Broken.Length = 0.0f;
		const FCorridor3DSpawnResult Failed = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		const FCorridor3DSpawnResult Failed = CCorridor3DSpawner::SpawnInto(
			Graph, OtherCollision, FCorridor3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に生成ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}

	Harness.BeginSuite( "CCorridor3DSpawner / 全3組を検証してから一括破棄する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, FCorridor3DSpawnParams{} );
		Harness.Check( Corridor.Succeeded(), "破棄確認用の通路を置ける" );
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"床と側壁を一括破棄できる" );
		Harness.Check( Corridor.IsEmpty() && !Corridor.Succeeded(), "成功時だけ結果を空にする" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "全形状を直ちに外す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "全ノードを残さない" );
		Harness.Check( !CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"空結果の二重破棄を拒否する" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, FCorridor3DSpawnParams{} );
		const FCollisionShapeId3D OriginalShape = Corridor.PositiveSideWall.Shape;
		Corridor.PositiveSideWall.Shape = Corridor.NegativeSideWall.Shape;

		Harness.Check( !CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"重複した形状番号を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "重複結果でも形状を外さない" );
		Harness.Check( NoPartIsPendingDestroy( Corridor ), "重複結果でもノードを破棄予定にしない" );

		Corridor.PositiveSideWall.Shape = OriginalShape;
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"結果を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, FCorridor3DSpawnParams{} );

		Harness.Check( !CCorridor3DSpawner::Destroy( OtherGraph, OtherCollision, Corridor ),
			"別場面からの破棄を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "元場面の形状を保つ" );
		Harness.Check( NoPartIsPendingDestroy( Corridor ), "元場面のノードを保つ" );
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"元場面なら片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FCorridor3DSpawnResult Corridor = CCorridor3DSpawner::SpawnInto(
			Graph, Collision, FCorridor3DSpawnParams{} );
		const FNodeId FloorNode = Graph.IdOf( Corridor.Floor.Node );
		Harness.Check( Graph.Destroy( FloorNode ), "床を先に破棄予定へ移せる" );
		Harness.Check( CCorridor3DSpawner::Destroy( Graph, Collision, Corridor ),
			"床が破棄予定でも側壁と形状を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "破棄予定を含む全形状を外す" );
	}
}

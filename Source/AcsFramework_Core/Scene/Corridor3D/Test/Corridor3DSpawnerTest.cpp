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

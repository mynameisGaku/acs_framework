// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 出入口枠の3部分が指定した親へ繋がっているか返す。 */
	bool AllPartsUseParent( const FDoorway3DSpawnResult& Doorway,
		const ANode& Parent ) noexcept
	{
		return Doorway.NegativePillar.Node != nullptr
			&& Doorway.NegativePillar.Node->Parent() == &Parent
			&& Doorway.PositivePillar.Node != nullptr
			&& Doorway.PositivePillar.Node->Parent() == &Parent
			&& Doorway.Lintel.Node != nullptr && Doorway.Lintel.Node->Parent() == &Parent;
	}

	/** 出入口枠の3部分が破棄予定ではないか返す。 */
	bool NoPartIsPendingDestroy( const FDoorway3DSpawnResult& Doorway ) noexcept
	{
		return Doorway.NegativePillar.Node != nullptr
			&& !Doorway.NegativePillar.Node->IsPendingDestroy()
			&& Doorway.PositivePillar.Node != nullptr
			&& !Doorway.PositivePillar.Node->IsPendingDestroy()
			&& Doorway.Lintel.Node != nullptr && !Doorway.Lintel.Node->IsPendingDestroy();
	}
}


void RunDoorway3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FDoorway3DSpawnParams / 既定値だけで衝突付き出入口枠になる" );

	{
		const FDoorway3DSpawnParams Doorway;
		Harness.Check( Doorway.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Doorway.WallWidth, 4.0f, "既定の壁幅" );
		Harness.CheckEqualF32( Doorway.WallHeight, 3.0f, "既定の壁高" );
		Harness.CheckEqualF32( Doorway.OpeningWidth, 1.2f, "既定の開口幅" );
		Harness.CheckEqualF32( Doorway.OpeningHeight, 2.2f, "既定の開口高" );
		Harness.Check( Doorway.Orientation == EDoorway3DOrientation::AlongX,
			"既定でX方向へ壁幅を取る" );
		Harness.Check( Doorway.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );

		const FDoorway3DSpawnParams Sized = FDoorway3DSpawnParams::FromOpening(
			5.0f, 3.5f, 1.5f, 2.4f, FVec3{ 1.0f, 2.0f, 3.0f },
			EDoorway3DOrientation::AlongZ );
		Harness.Check( Sized.IsValid(), "壁と開口の寸法と向きだけで設定を作れる" );
		Harness.CheckEqualF32( Sized.WallWidth, 5.0f, "指定した壁幅" );
		Harness.CheckEqualF32( Sized.OpeningHeight, 2.4f, "指定した開口高" );
		Harness.CheckEqualF32( Sized.BottomCenter.y, 2.0f, "指定した床上高さ" );
		Harness.Check( Sized.Orientation == EDoorway3DOrientation::AlongZ,
			"指定した向きを保つ" );
		Harness.Check( FDoorway3DSpawnResult{}.IsEmpty(), "既定の生成結果は空" );
	}

	Harness.BeginSuite( "FDoorway3DSpawnParams / 開口を壊す値を配置前に弾く" );

	{
		FDoorway3DSpawnParams Broken;
		Broken.WallWidth = 0.0f;
		Harness.Check( !Broken.IsValid(), "壁幅0を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.WallHeight = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない壁高を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.OpeningWidth = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない開口幅を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.OpeningHeight = 0.0f;
		Harness.Check( !Broken.IsValid(), "開口高0を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.WallThickness = -0.1f;
		Harness.Check( !Broken.IsValid(), "負の壁厚を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.OpeningCenterOffset = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない開口ずらしを拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.BottomCenter.z = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない下辺中心を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.Orientation = static_cast<EDoorway3DOrientation>( 0xffu );
		Harness.Check( !Broken.IsValid(), "未知の向きを拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.Color.x = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の表面色を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.Metallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の金属度を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.Roughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない粗さを拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.OpeningWidth = Broken.WallWidth;
		Harness.Check( !Broken.IsValid(), "左右柱を消す全幅開口を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.OpeningHeight = Broken.WallHeight;
		Harness.Check( !Broken.IsValid(), "上枠を消す全高開口を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.OpeningCenterOffset = Broken.WallWidth;
		Harness.Check( !Broken.IsValid(), "壁外へ出る開口ずらしを拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.WallHeight = std::numeric_limits<f32>::max();
		Broken.BottomCenter.y = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な壁上端にならない値を拒否する" );

		Broken = FDoorway3DSpawnParams{};
		Broken.WallWidth = std::numeric_limits<f32>::max();
		Broken.BottomCenter.x = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な壁外端にならない値を拒否する" );
	}

	Harness.BeginSuite( "CDoorway3DSpawner / 横ずらしした開口を左右柱と上枠で囲む" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FDoorway3DSpawnParams Params = FDoorway3DSpawnParams::FromOpening(
			5.0f, 4.0f, 1.5f, 2.5f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.WallThickness = 0.3f;
		Params.OpeningCenterOffset = 0.5f;
		Params.Color = FVec4{ 0.25f, 0.40f, 0.65f, 1.0f };
		Params.Metallic = 0.15f;
		Params.Roughness = 0.35f;
		Params.bCastsShadow = false;
		Params.CollisionLayer = 0x4u;
		Params.NegativePillarName = FStringView( "StoneNegativePillar" );
		Params.PositivePillarName = FStringView( "StonePositivePillar" );
		Params.LintelName = FStringView( "StoneLintel" );

		FDoorway3DSpawnResult Doorway = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Doorway.Succeeded(), "左右柱と上枠を一括生成できる" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 3u, "表示ノードを3個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "箱型衝突を3個だけ登録する" );

		if ( Doorway.Succeeded() )
		{
			Harness.Check( Doorway.NegativePillar.Node->Name()
				== FStringView( "StoneNegativePillar" ), "負側柱へ指定名を付ける" );
			Harness.Check( Doorway.Lintel.Node->Name() == FStringView( "StoneLintel" ),
				"上枠へ指定名を付ける" );
			Harness.CheckNearF32( Doorway.NegativePillar.Node->Local().position.x,
				-0.375f, 0.001f, "負側柱を壁外端から開口左端まで置く" );
			Harness.CheckNearF32( Doorway.NegativePillar.Node->Local().scale.x,
				2.25f, 0.001f, "負側柱へ横ずらし後の幅を使う" );
			Harness.CheckNearF32( Doorway.PositivePillar.Node->Local().position.x,
				2.875f, 0.001f, "正側柱を開口右端から壁外端まで置く" );
			Harness.CheckNearF32( Doorway.PositivePillar.Node->Local().scale.x,
				1.25f, 0.001f, "正側柱へ横ずらし後の幅を使う" );
			Harness.CheckNearF32( Doorway.NegativePillar.Node->Local().position.y,
				4.0f, 0.001f, "左右柱を床から壁上端まで伸ばす" );
			Harness.CheckNearF32( Doorway.Lintel.Node->Local().position.x,
				1.5f, 0.001f, "上枠中心を開口中心へ揃える" );
			Harness.CheckNearF32( Doorway.Lintel.Node->Local().position.y,
				5.25f, 0.001f, "上枠を開口上端から壁上端まで置く" );
			Harness.CheckNearF32( Doorway.Lintel.Node->Local().scale.x,
				1.5f, 0.001f, "上枠幅を開口幅へ揃える" );
			Harness.CheckNearF32( Doorway.Lintel.Node->Local().scale.y,
				1.5f, 0.001f, "上枠高を壁高と開口高の差へ揃える" );
		}

		const AMeshComponent3D* const PillarMesh = Doorway.Succeeded()
			? Doorway.NegativePillar.Node->GetComponent<AMeshComponent3D>() : nullptr;
		Harness.Check( PillarMesh != nullptr && PillarMesh->Primitive() == EMeshPrimitive3D::Cube,
			"3部分は立方体表示を使う" );
		if ( PillarMesh != nullptr )
		{
			Harness.CheckEqualF32( PillarMesh->Color().z, 0.65f, "指定した表面色を使う" );
			Harness.CheckEqualF32( PillarMesh->Material().pbr.metallic, 0.15f,
				"指定した金属度を使う" );
			Harness.CheckEqualF32( PillarMesh->Material().pbr.roughness, 0.35f,
				"指定した粗さを使う" );
			Harness.Check( !PillarMesh->CastsShadow(), "指定した影設定を使う" );
		}

		FWorldCollisionShape3D NegativeShape;
		FWorldCollisionShape3D LintelShape;
		Harness.Check( Doorway.Succeeded()
			&& Collision.TryGetWorldShape( Doorway.NegativePillar.Shape, NegativeShape ),
			"負側柱のworld衝突を読める" );
		Harness.Check( Doorway.Succeeded()
			&& Collision.TryGetWorldShape( Doorway.Lintel.Shape, LintelShape ),
			"上枠のworld衝突を読める" );
		Harness.Check( NegativeShape.Kind == FWorldCollisionShape3D::EKind::Box
			&& LintelShape.Kind == FWorldCollisionShape3D::EKind::Box,
			"左右柱と上枠の衝突は箱" );
		Harness.CheckNearF32( LintelShape.Box.center.y, 5.25f, 0.001f,
			"上枠衝突中心を表示へ揃える" );
		Harness.CheckNearF32( LintelShape.Box.half_size.y, 0.75f, 0.001f,
			"上枠衝突高を表示へ揃える" );
		Harness.Check( NegativeShape.Layer == 0x4u && LintelShape.Layer == 0x4u,
			"3部分へ同じ衝突レイヤーを使う" );
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"確認後の出入口枠を片付けられる" );
	}

	Harness.BeginSuite( "CDoorway3DSpawner / XとZの2軸へ開口を残す" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FDoorway3DSpawnResult AlongX = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, FDoorway3DSpawnParams{} );
		Harness.Check( AlongX.Succeeded(), "X方向へ伸びる出入口枠を置ける" );
		if ( AlongX.Succeeded() )
		{
			Harness.CheckNearF32( AlongX.NegativePillar.Node->Local().position.x,
				-1.3f, 0.001f, "X負側へ柱を置く" );
			Harness.CheckNearF32( AlongX.NegativePillar.Node->Local().scale.x,
				1.4f, 0.001f, "Xへ柱幅を使う" );
			Harness.CheckNearF32( AlongX.NegativePillar.Node->Local().scale.z,
				0.25f, 0.001f, "Zへ壁厚を使う" );
		}
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, AlongX ),
			"X向き確認後に片付けられる" );

		FDoorway3DSpawnParams AlongZParams;
		AlongZParams.Orientation = EDoorway3DOrientation::AlongZ;
		FDoorway3DSpawnResult AlongZ = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, AlongZParams );
		Harness.Check( AlongZ.Succeeded(), "Z方向へ伸びる出入口枠を置ける" );
		if ( AlongZ.Succeeded() )
		{
			Harness.CheckNearF32( AlongZ.NegativePillar.Node->Local().position.z,
				-1.3f, 0.001f, "Z負側へ柱を置く" );
			Harness.CheckNearF32( AlongZ.NegativePillar.Node->Local().scale.x,
				0.25f, 0.001f, "Xへ壁厚を使う" );
			Harness.CheckNearF32( AlongZ.NegativePillar.Node->Local().scale.z,
				1.4f, 0.001f, "Zへ柱幅を使う" );
			Harness.CheckNearF32( AlongZ.Lintel.Node->Local().scale.z,
				1.2f, 0.001f, "Zへ開口幅を使う" );
		}
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, AlongZ ),
			"Z向き確認後に片付けられる" );
	}

	Harness.BeginSuite( "CDoorway3DSpawner / 指定親の変形を3部分へ共通適用する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn( FStringView( "DoorwayRoot" ) );
		Harness.Check( Parent.Succeeded(), "出入口枠を繋ぐ親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 1.0f, 3.0f } );
		}

		const FDoorway3DSpawnParams Params = FDoorway3DSpawnParams::FromOpening(
			4.0f, 3.0f, 1.2f, 2.2f, FVec3{ 1.0f, 2.0f, 3.0f } );
		FDoorway3DSpawnResult Doorway = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, Params, Parent.Node );
		Harness.Check( Doorway.Succeeded(), "指定親の下へ出入口枠を置ける" );
		if ( Parent.Node != nullptr )
		{
			Harness.CheckEqualU64( Parent.Node->ChildCount(), 3u, "親の直下へ3部分を置く" );
			Harness.Check( AllPartsUseParent( Doorway, *Parent.Node ), "3部分が同じ親を使う" );
		}

		FWorldCollisionShape3D LintelShape;
		Harness.Check( Doorway.Succeeded()
			&& Collision.TryGetWorldShape( Doorway.Lintel.Shape, LintelShape ),
			"親変形後の上枠衝突を読める" );
		Harness.CheckNearF32( LintelShape.Box.center.x, 12.0f, 0.001f,
			"親のX変形を反映する" );
		Harness.CheckNearF32( LintelShape.Box.center.y, 5.6f, 0.001f,
			"親のY変形を反映する" );
		Harness.CheckNearF32( LintelShape.Box.center.z, 4.0f, 0.001f,
			"親のZ変形を反映する" );
		Harness.CheckNearF32( LintelShape.Box.half_size.x, 1.2f, 0.001f,
			"親X尺度を開口幅へ反映する" );
		Harness.CheckNearF32( LintelShape.Box.half_size.z, 0.375f, 0.001f,
			"親Z尺度を壁厚へ反映する" );
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"親付き出入口枠を片付けられる" );
	}

	Harness.BeginSuite( "CDoorway3DSpawner / 不正入力と別場面で半端物を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FDoorway3DSpawnParams Broken;
		Broken.OpeningWidth = Broken.WallWidth;
		const FDoorway3DSpawnResult Failed = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		const FDoorway3DSpawnResult Failed = CDoorway3DSpawner::SpawnInto(
			Graph, OtherCollision, FDoorway3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に生成ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}

	Harness.BeginSuite( "CDoorway3DSpawner / 全3組を検証してから一括破棄する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FDoorway3DSpawnResult Doorway = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, FDoorway3DSpawnParams{} );
		Harness.Check( Doorway.Succeeded(), "破棄確認用の出入口枠を置ける" );
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"左右柱と上枠を一括破棄できる" );
		Harness.Check( Doorway.IsEmpty() && !Doorway.Succeeded(), "成功時だけ結果を空にする" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "全形状を直ちに外す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "全ノードを残さない" );
		Harness.Check( !CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"空結果の二重破棄を拒否する" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FDoorway3DSpawnResult Doorway = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, FDoorway3DSpawnParams{} );
		const FCollisionShapeId3D OriginalShape = Doorway.Lintel.Shape;
		Doorway.Lintel.Shape = Doorway.PositivePillar.Shape;

		Harness.Check( !CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"重複した形状番号を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "重複結果でも形状を外さない" );
		Harness.Check( NoPartIsPendingDestroy( Doorway ), "重複結果でもノードを破棄予定にしない" );

		Doorway.Lintel.Shape = OriginalShape;
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"結果を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FDoorway3DSpawnResult Doorway = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, FDoorway3DSpawnParams{} );

		Harness.Check( !CDoorway3DSpawner::Destroy( OtherGraph, OtherCollision, Doorway ),
			"別場面からの破棄を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "元場面の形状を保つ" );
		Harness.Check( NoPartIsPendingDestroy( Doorway ), "元場面のノードを保つ" );
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"元場面なら片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FDoorway3DSpawnResult Doorway = CDoorway3DSpawner::SpawnInto(
			Graph, Collision, FDoorway3DSpawnParams{} );
		const FNodeId PillarNode = Graph.IdOf( Doorway.NegativePillar.Node );
		Harness.Check( Graph.Destroy( PillarNode ), "負側柱を先に破棄予定へ移せる" );
		Harness.Check( CDoorway3DSpawner::Destroy( Graph, Collision, Doorway ),
			"柱が破棄予定でも残りと形状を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "破棄予定を含む全形状を外す" );
	}
}

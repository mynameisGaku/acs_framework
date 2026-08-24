// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 全段が指定した親へ繋がっているか返す。 */
	bool AllStepsUseParent( const FStairs3DSpawnResult& Stairs, const ANode& Parent ) noexcept
	{
		if ( Stairs.Steps.IsEmpty() ) return false;
		for ( usize Index = 0u; Index < Stairs.Steps.Num(); ++Index )
		{
			if ( Stairs.Steps[Index].Node == nullptr
				|| Stairs.Steps[Index].Node->Parent() != &Parent ) return false;
		}
		return true;
	}

	/** 全段が破棄予定ではないか返す。 */
	bool NoStepIsPendingDestroy( const FStairs3DSpawnResult& Stairs ) noexcept
	{
		if ( Stairs.Steps.IsEmpty() ) return false;
		for ( usize Index = 0u; Index < Stairs.Steps.Num(); ++Index )
		{
			if ( Stairs.Steps[Index].Node == nullptr
				|| Stairs.Steps[Index].Node->IsPendingDestroy() ) return false;
		}
		return true;
	}

	/** 2段の最上段が指定軸方向と寸法になるか調べる。 */
	void CheckDirection( CTestHarness& Harness, EStairs3DDirection Direction,
		f32 ExpectedX, f32 ExpectedZ, f32 ExpectedSizeX, f32 ExpectedSizeZ )
	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FStairs3DSpawnParams Params = FStairs3DSpawnParams::FromSteps(
			2u, 2.0f, 0.5f, 0.25f, FVec3{}, Direction );
		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto( Graph, Collision, Params );
		Harness.Check( Stairs.Succeeded() && Stairs.StepCount() == 2u,
			"指定方向へ2段を置ける" );
		if ( Stairs.StepCount() == 2u && Stairs.Steps[1u].Node != nullptr )
		{
			const ANode& TopStep = *Stairs.Steps[1u].Node;
			Harness.CheckNearF32( TopStep.Local().position.x, ExpectedX, 0.001f,
				"最上段のX中心を上る方向へ置く" );
			Harness.CheckNearF32( TopStep.Local().position.z, ExpectedZ, 0.001f,
				"最上段のZ中心を上る方向へ置く" );
			Harness.CheckNearF32( TopStep.Local().scale.x, ExpectedSizeX, 0.001f,
				"X方向へ正しい幅または奥行きを使う" );
			Harness.CheckNearF32( TopStep.Local().scale.z, ExpectedSizeZ, 0.001f,
				"Z方向へ正しい幅または奥行きを使う" );
		}
		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"方向確認後の2段を片付けられる" );
	}
}


void RunStairs3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FStairs3DSpawnParams / 既定値だけで衝突付き階段になる" );

	{
		const FStairs3DSpawnParams Stairs;
		Harness.Check( Stairs.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualU64( Stairs.StepCount, 6u, "既定の段数" );
		Harness.CheckEqualF32( Stairs.Width, 2.0f, "既定の階段幅" );
		Harness.CheckEqualF32( Stairs.StepDepth, 0.32f, "既定の踏面奥行き" );
		Harness.CheckEqualF32( Stairs.StepHeight, 0.18f, "既定の段差" );
		Harness.Check( Stairs.Direction == EStairs3DDirection::PositiveZ,
			"既定でZ正方向へ上る" );
		Harness.Check( Stairs.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );

		const FStairs3DSpawnParams Sized = FStairs3DSpawnParams::FromSteps(
			8u, 2.4f, 0.30f, 0.17f, FVec3{ 1.0f, 2.0f, 3.0f },
			EStairs3DDirection::NegativeX );
		Harness.Check( Sized.IsValid(), "段数と寸法と方向だけで設定を作れる" );
		Harness.CheckEqualU64( Sized.StepCount, 8u, "指定した段数" );
		Harness.CheckEqualF32( Sized.Width, 2.4f, "指定した幅" );
		Harness.CheckEqualF32( Sized.BottomEdgeCenter.y, 2.0f, "指定した床上高さ" );
		Harness.Check( Sized.Direction == EStairs3DDirection::NegativeX,
			"指定した方向を保つ" );
		Harness.Check( FStairs3DSpawnResult{}.IsEmpty(), "既定の生成結果は空" );
	}

	Harness.BeginSuite( "FStairs3DSpawnParams / 半端な階段を作る値を配置前に弾く" );

	{
		FStairs3DSpawnParams Broken;
		Broken.StepCount = 0u;
		Harness.Check( !Broken.IsValid(), "段数0を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.StepCount = FStairs3DSpawnParams::kMaximumStepCount + 1u;
		Harness.Check( !Broken.IsValid(), "上限を超える段数を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.Width = 0.0f;
		Harness.Check( !Broken.IsValid(), "幅0を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.StepDepth = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない踏面奥行きを拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.StepHeight = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない段差を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.BottomEdgeCenter.x = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない基準点を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.Direction = static_cast<EStairs3DDirection>( 0xffu );
		Harness.Check( !Broken.IsValid(), "未知の方向を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.Color.x = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の表面色を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.Metallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の金属度を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.Roughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない粗さを拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.StepCount = FStairs3DSpawnParams::kMaximumStepCount;
		Broken.StepDepth = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な全奥行きにならない値を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.StepCount = 1u;
		Broken.StepHeight = std::numeric_limits<f32>::max();
		Broken.BottomEdgeCenter.y = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な最上段高さにならない基準点を拒否する" );

		Broken = FStairs3DSpawnParams{};
		Broken.StepCount = 1u;
		Broken.StepDepth = std::numeric_limits<f32>::max();
		Broken.BottomEdgeCenter.x = std::numeric_limits<f32>::max();
		Broken.Direction = EStairs3DDirection::PositiveX;
		Harness.Check( !Broken.IsValid(), "有限な終端位置にならない基準点を拒否する" );
	}

	Harness.BeginSuite( "CStairs3DSpawner / 床面から段差ずつ高くなる直方体を置く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FStairs3DSpawnParams Params = FStairs3DSpawnParams::FromSteps(
			3u, 2.0f, 0.4f, 0.25f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.Color = FVec4{ 0.20f, 0.35f, 0.55f, 1.0f };
		Params.Metallic = 0.15f;
		Params.Roughness = 0.30f;
		Params.bCastsShadow = false;
		Params.CollisionLayer = 0x4u;
		Params.StepName = FStringView( "StoneStep" );

		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Stairs.Succeeded(), "3段と衝突を一括生成できる" );
		Harness.CheckEqualU64( Stairs.StepCount(), 3u, "結果へ3段を低い順に保持する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 3u, "表示ノードを3個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 3u, "箱型衝突を3個だけ登録する" );

		if ( Stairs.StepCount() == 3u )
		{
			const ANode* const Bottom = Stairs.Steps[0u].Node;
			const ANode* const Middle = Stairs.Steps[1u].Node;
			const ANode* const Top = Stairs.Steps[2u].Node;
			Harness.Check( Bottom != nullptr && Middle != nullptr && Top != nullptr,
				"全段の表示ノードを返す" );
			if ( Bottom != nullptr && Middle != nullptr && Top != nullptr )
			{
				Harness.Check( Bottom->Name() == FStringView( "StoneStep" ), "全段用の指定名を付ける" );
				Harness.CheckNearF32( Bottom->Local().position.z, 3.2f, 0.001f,
					"最下段を基準端から奥行き半分先へ置く" );
				Harness.CheckNearF32( Middle->Local().position.z, 3.6f, 0.001f,
					"中段を踏面1個ぶん先へ置く" );
				Harness.CheckNearF32( Top->Local().position.z, 4.0f, 0.001f,
					"最上段を踏面2個ぶん先へ置く" );
				Harness.CheckNearF32( Bottom->Local().position.y, 2.125f, 0.001f,
					"最下段の下端を床面へ揃える" );
				Harness.CheckNearF32( Middle->Local().position.y, 2.25f, 0.001f,
					"中段の下端を床面へ揃える" );
				Harness.CheckNearF32( Top->Local().position.y, 2.375f, 0.001f,
					"最上段の下端を床面へ揃える" );
				Harness.CheckNearF32( Bottom->Local().scale.y, 0.25f, 0.001f,
					"最下段を1段差の高さにする" );
				Harness.CheckNearF32( Middle->Local().scale.y, 0.50f, 0.001f,
					"中段を2段差の高さにする" );
				Harness.CheckNearF32( Top->Local().scale.y, 0.75f, 0.001f,
					"最上段を3段差の高さにする" );
				Harness.CheckNearF32( Top->Local().scale.x, 2.0f, 0.001f,
					"Z方向階段のXへ全幅を使う" );
				Harness.CheckNearF32( Top->Local().scale.z, 0.4f, 0.001f,
					"Z方向階段のZへ踏面奥行きを使う" );
			}
		}

		const AMeshComponent3D* const Mesh = Stairs.StepCount() > 0u
			&& Stairs.Steps[0u].Node != nullptr
			? Stairs.Steps[0u].Node->GetComponent<AMeshComponent3D>() : nullptr;
		Harness.Check( Mesh != nullptr && Mesh->Primitive() == EMeshPrimitive3D::Cube,
			"各段は立方体表示を使う" );
		if ( Mesh != nullptr )
		{
			Harness.CheckEqualF32( Mesh->Color().z, 0.55f, "指定した表面色を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.metallic, 0.15f, "指定した金属度を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.roughness, 0.30f, "指定した粗さを使う" );
			Harness.Check( !Mesh->CastsShadow(), "指定した影設定を反映する" );
		}

		FWorldCollisionShape3D TopShape;
		Harness.Check( Collision.TryGetWorldShape( Stairs.Steps[2u].Shape, TopShape ),
			"最上段のworld衝突を読める" );
		Harness.Check( TopShape.Kind == FWorldCollisionShape3D::EKind::Box,
			"各段の衝突は箱" );
		Harness.CheckNearF32( TopShape.Box.center.y, 2.375f, 0.001f,
			"最上段衝突の中心を表示へ揃える" );
		Harness.CheckNearF32( TopShape.Box.half_size.y, 0.375f, 0.001f,
			"最上段衝突を床面から上面まで埋める" );
		Harness.Check( TopShape.Layer == 0x4u && TopShape.bQueryable,
			"全段を指定レイヤーで問い合わせ可能にする" );

		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"確認後の3段を片付けられる" );
	}

	Harness.BeginSuite( "CStairs3DSpawner / XとZの正負4方向へ正しい幅で伸ばす" );

	CheckDirection( Harness, EStairs3DDirection::PositiveX, 0.75f, 0.0f, 0.5f, 2.0f );
	CheckDirection( Harness, EStairs3DDirection::NegativeX, -0.75f, 0.0f, 0.5f, 2.0f );
	CheckDirection( Harness, EStairs3DDirection::PositiveZ, 0.0f, 0.75f, 2.0f, 0.5f );
	CheckDirection( Harness, EStairs3DDirection::NegativeZ, 0.0f, -0.75f, 2.0f, 0.5f );

	Harness.BeginSuite( "CStairs3DSpawner / 指定親の変形を全段へ共通適用する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn( FStringView( "StairsRoot" ) );
		Harness.Check( Parent.Succeeded(), "階段を繋ぐ親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 1.0f, 3.0f } );
		}

		const FStairs3DSpawnParams Params = FStairs3DSpawnParams::FromSteps(
			2u, 2.0f, 0.5f, 0.25f, FVec3{ 1.0f, 2.0f, 3.0f } );
		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto(
			Graph, Collision, Params, Parent.Node );
		Harness.Check( Stairs.Succeeded(), "指定親の下へ2段を置ける" );
		if ( Parent.Node != nullptr )
		{
			Harness.CheckEqualU64( Parent.Node->ChildCount(), 2u, "親の直下へ全段を置く" );
			Harness.Check( AllStepsUseParent( Stairs, *Parent.Node ), "全段が同じ親を使う" );
		}

		FWorldCollisionShape3D TopShape;
		Harness.Check( Collision.TryGetWorldShape( Stairs.Steps[1u].Shape, TopShape ),
			"親変形後の最上段衝突を読める" );
		Harness.CheckNearF32( TopShape.Box.center.x, 12.0f, 0.001f, "親のX変形を反映する" );
		Harness.CheckNearF32( TopShape.Box.center.y, 3.25f, 0.001f, "親のY変形を反映する" );
		Harness.CheckNearF32( TopShape.Box.center.z, 6.25f, 0.001f, "親のZ変形を反映する" );
		Harness.CheckNearF32( TopShape.Box.half_size.x, 2.0f, 0.001f,
			"親X尺度を階段幅へ反映する" );
		Harness.CheckNearF32( TopShape.Box.half_size.z, 0.75f, 0.001f,
			"親Z尺度を踏面奥行きへ反映する" );
		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"親付き階段を片付けられる" );
	}

	Harness.BeginSuite( "CStairs3DSpawner / 不正入力と別場面で半端物を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FStairs3DSpawnParams Broken;
		Broken.StepCount = 0u;
		const FStairs3DSpawnResult Failed = CStairs3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		const FStairs3DSpawnResult Failed = CStairs3DSpawner::SpawnInto(
			Graph, OtherCollision, FStairs3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に生成済み段を巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}

	Harness.BeginSuite( "CStairs3DSpawner / 全段を検証してから一括破棄する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto(
			Graph, Collision, FStairs3DSpawnParams{} );
		Harness.Check( Stairs.Succeeded(), "破棄確認用の階段を置ける" );
		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"全段を高い側から一括破棄できる" );
		Harness.Check( Stairs.IsEmpty() && !Stairs.Succeeded(), "成功時だけ結果を空にする" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "全形状を直ちに外す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "全ノードを残さない" );
		Harness.Check( !CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"空結果の二重破棄を拒否する" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto(
			Graph, Collision, FStairs3DSpawnParams{} );
		const FCollisionShapeId3D OriginalShape = Stairs.Steps[1u].Shape;
		Stairs.Steps[1u].Shape = Stairs.Steps[0u].Shape;

		Harness.Check( !CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"重複した形状番号を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 6u, "重複結果でも形状を外さない" );
		Harness.Check( NoStepIsPendingDestroy( Stairs ), "重複結果でもノードを破棄予定にしない" );

		Stairs.Steps[1u].Shape = OriginalShape;
		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"結果を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto(
			Graph, Collision, FStairs3DSpawnParams{} );

		Harness.Check( !CStairs3DSpawner::Destroy( OtherGraph, OtherCollision, Stairs ),
			"別場面からの破棄を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 6u, "元場面の形状を保つ" );
		Harness.Check( NoStepIsPendingDestroy( Stairs ), "元場面のノードを保つ" );
		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"元場面なら片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FStairs3DSpawnResult Stairs = CStairs3DSpawner::SpawnInto(
			Graph, Collision, FStairs3DSpawnParams{} );
		const FNodeId BottomNode = Graph.IdOf( Stairs.Steps[0u].Node );
		Harness.Check( Graph.Destroy( BottomNode ), "最下段を先に破棄予定へ移せる" );
		Harness.Check( CStairs3DSpawner::Destroy( Graph, Collision, Stairs ),
			"一部が破棄予定でも残りと形状を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "破棄予定を含む全形状を外す" );
	}
}

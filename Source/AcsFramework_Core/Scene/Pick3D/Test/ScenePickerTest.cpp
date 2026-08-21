// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Pick3D/ScenePicker.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/**
	 * 距離の一致を、誤差を許して見る。
	 *
	 * @details
	 * 交差の計算は割り算を通るので、**厳密な一致を期待すると環境で落ちる**。
	 *
	 * @param Harness 記録先。
	 * @param Actual 実測。
	 * @param Expected 期待。
	 * @param Label 表示名。
	 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 1.0e-3f;
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference < kTolerance, Label );
	}


	/**
	 * 立方体を 1 つ置く。
	 *
	 * @param Parent 置く先。
	 * @param Position 置く場所。
	 * @param Scale 大きさ。
	 * @return 置いたノード。
	 */
	ANode* PlaceCube( ANode& Parent, FVec3 Position, f32 Scale = 1.0f ) noexcept
	{
		FModel3DSpawnParams Params = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, Position );
		Params.Scale = FVec3{ Scale, Scale, Scale };
		return CModel3DSpawner::SpawnInto( Parent, Params );
	}
}


void RunScenePickerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FSceneRay / 壊れた線を作らせない" );

	{
		const FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 5.0f } );

		Harness.CheckEqualF32( Ray.Direction.z, 1.0f, "向きは正規化される" );
		Harness.Check( Ray.IsValid(), "正規化された線は使える" );
	}

	{
		// 向きが 0 のまま通すと «全部当たる» か «全部外れる» になり、原因が読めない。
		const FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 0.0f } );

		Harness.CheckEqualF32( Ray.Direction.z, 1.0f, "向きが 0 なら +Z へ倒す" );
		Harness.Check( Ray.IsValid(), "倒したあとは使える" );
	}

	{
		FSceneRay Ray = FSceneRay::Down( FVec3{ 0.0f, 10.0f, 0.0f } );
		Harness.CheckEqualF32( Ray.Direction.y, -1.0f, "Down は真下を向く" );

		Ray.MaxDistance = 0.0f;
		Harness.Check( !Ray.IsValid(), "届く距離が 0 の線は使えない" );
	}

	{
		FSceneRay Ray;
		Ray.Origin.x = std::numeric_limits<f32>::infinity();
		Harness.Check( !Ray.IsValid(), "有限でない始点を弾く" );

		Ray = FSceneRay{};
		Ray.MaxDistance = std::numeric_limits<f32>::infinity();
		Harness.Check( !Ray.IsValid(), "有限でない距離を弾く" );
	}

	Harness.BeginSuite( "CScenePicker / いちばん手前を返す" );

	TObjectPtr<ANode> Root = NewObject<ANode>();

	{
		// z = +4 と z = +8 に置いて、原点から +Z へ飛ばす。
		ANode* const Near = PlaceCube( *Root, FVec3{ 0.0f, 0.0f, 4.0f } );
		ANode* const Far = PlaceCube( *Root, FVec3{ 0.0f, 0.0f, 8.0f } );

		const FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		const FSceneRayHit Hit = CScenePicker::Raycast( *Root, Ray );

		Harness.Check( Hit.IsHit(), "置いた立方体に当たる" );
		Harness.Check( Hit.Node == Near, "奥ではなく手前が返る" );
		CheckNear( Harness, Hit.Distance, 3.5f, "距離は手前の面まで (4 - 0.5)" );

		// 手前を消したら奥が返る。**見えないものを掴めてしまう** のがいちばん困る。
		Near->SetVisible( false );
		const FSceneRayHit Second = CScenePicker::Raycast( *Root, Ray );

		Harness.Check( Second.Node == Far, "見えないものは飛ばして奥が返る" );

		Near->SetVisible( true );
	}

	{
		// 届く距離を手前の物より短くすると、何も返らない。
		FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		Ray.MaxDistance = 1.0f;

		const FSceneRayHit Hit = CScenePicker::Raycast( *Root, Ray );
		Harness.Check( !Hit.IsHit(), "届く距離より遠いものは当たらない" );
	}

	{
		// 逆向きへ飛ばすと当たらない。
		const FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, -1.0f } );

		const FSceneRayHit Hit = CScenePicker::Raycast( *Root, Ray );
		Harness.Check( !Hit.IsHit(), "後ろにあるものは当たらない" );
	}

	Harness.BeginSuite( "CScenePicker / 重なったものを手前から並べる" );

	{
		const FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );

		TArray<FSceneRayHit> Hits;
		const usize Count = CScenePicker::RaycastAll( *Root, Ray, Hits );

		Harness.CheckEqualU64( Count, 2u, "貫いた 2 つとも拾う" );
		if ( Hits.Num() >= 2u )
		{
			Harness.Check( Hits[0].Distance < Hits[1].Distance, "手前から順に並ぶ" );
		}
	}

	Harness.BeginSuite( "CScenePicker / 大きさと回転を見る" );

	{
		TObjectPtr<ANode> Scaled = NewObject<ANode>();
		PlaceCube( *Scaled, FVec3{ 0.0f, 0.0f, 10.0f }, 4.0f );

		const FSceneRay Ray = FSceneRay::FromDirection( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		const FSceneRayHit Hit = CScenePicker::Raycast( *Scaled, Ray );

		Harness.Check( Hit.IsHit(), "大きくした立方体に当たる" );
		CheckNear( Harness, Hit.Distance, 8.0f, "大きさが効く (10 - 4/2)" );
	}

	{
		// 板は厚みが 0。薄い厚みを足していないと、平らなものが掴めない。
		TObjectPtr<ANode> Flat = NewObject<ANode>();
		FModel3DSpawnParams Plane =
			FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{ 0.0f, 0.0f, 0.0f } );
		Plane.Scale = FVec3{ 10.0f, 1.0f, 10.0f };
		CModel3DSpawner::SpawnInto( *Flat, Plane );

		const FSceneRay Ray = FSceneRay::Down( FVec3{ 1.0f, 5.0f, 1.0f } );
		const FSceneRayHit Hit = CScenePicker::Raycast( *Flat, Ray );

		Harness.Check( Hit.IsHit(), "厚みの無い板にも当たる" );
		CheckNear( Harness, Hit.Distance, 5.0f, "板の高さまでの距離" );
	}

	Harness.BeginSuite( "CScenePicker / 実際の3D形状へ当てる" );

	{
		CSceneNodeGraph Graph;
		FModel3DSpawnParams Sphere =
			FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 0.0f, 0.0f, 4.0f } );
		ANode* const SphereNode = CModel3DSpawner::SpawnInto( Graph, Sphere );

		const FSceneRay CornerRay = FSceneRay::FromDirection(
			FVec3{ 0.49f, 0.49f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		const FSceneRayHit BoundsHit = CScenePicker::Raycast( Graph.Root(), CornerRay );
		const FSceneRayHit GeometryMiss = CScenePicker::RaycastGeometry( Graph, CornerRay );
		Harness.Check( BoundsHit.Node == SphereNode, "境界の箱なら球の角も拾う" );
		Harness.Check( !GeometryMiss.IsHit(), "実形状なら球の外側を拾わない" );

		const FSceneRay SurfaceRay = FSceneRay::FromDirection(
			FVec3{ 0.30f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		const FSceneRayHit SurfaceHit = CScenePicker::RaycastGeometry( Graph, SurfaceRay );
		Harness.Check( SurfaceHit.Node == SphereNode, "球の実表面へ当たる" );
		CheckNear( Harness, SurfaceHit.Distance, 3.60f, "球面までの距離を返す" );
		CheckNear( Harness, SurfaceHit.Normal.x, 0.60f, "球面の横向きを返す" );
		CheckNear( Harness, SurfaceHit.Normal.z, -0.80f, "球面の手前向きを返す" );
	}

	{
		CSceneNodeGraph Graph;
		TSharedPtr<AMeshAsset> Triangle = MakeShared<AMeshAsset>();
		Harness.Check( Triangle.IsValid(), "三角形メッシュを作れる" );
		if ( !Triangle ) return;
		Triangle->Vertices().Add( FMeshVertex{ FVec3{ -1.0f, -1.0f, 0.0f }, FVec3{ 0.0f, 0.0f, -1.0f }, 0.0f, 0.0f } );
		Triangle->Vertices().Add( FMeshVertex{ FVec3{ 1.0f, -1.0f, 0.0f }, FVec3{ 0.0f, 0.0f, -1.0f }, 1.0f, 0.0f } );
		Triangle->Vertices().Add( FMeshVertex{ FVec3{ -1.0f, 1.0f, 0.0f }, FVec3{ 0.0f, 0.0f, -1.0f }, 0.0f, 1.0f } );

		FScene3DSpawnResult Spawned = Graph.TrySpawn( FStringView( "Triangle" ) );
		Harness.Check( Spawned.Succeeded(), "三角形ノードを置ける" );
		if ( Spawned.Node != nullptr )
		{
			Spawned.Node->SetPosition( FVec3{ 0.0f, 0.0f, 4.0f } );
			AMeshComponent3D& Mesh = Spawned.Node->AddComponent<AMeshComponent3D>( EMeshPrimitive3D::Mesh );
			Mesh.SetMeshAsset( TSharedPtr<AAsset>( Triangle ) );
		}

		const FSceneRay OutsideTriangle = FSceneRay::FromDirection(
			FVec3{ 0.75f, 0.75f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		Harness.Check( CScenePicker::Raycast( Graph.Root(), OutsideTriangle ).IsHit(), "境界の箱には入る" );
		Harness.Check( !CScenePicker::RaycastGeometry( Graph, OutsideTriangle ).IsHit(), "三角形の外側は拾わない" );

		const FSceneRay InsideTriangle = FSceneRay::FromDirection(
			FVec3{ -0.50f, -0.50f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } );
		const FSceneRayHit TriangleHit = CScenePicker::RaycastGeometry( Graph, InsideTriangle );
		Harness.Check( TriangleHit.Node == Spawned.Node, "読み込みメッシュの三角形へ当たる" );
		CheckNear( Harness, TriangleHit.Distance, 4.0f, "三角形面までの距離を返す" );
	}
}

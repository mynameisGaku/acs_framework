// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 指定位置へプリミティブを置く。 */
	ANode* PlacePrimitive( CSceneNodeGraph& Graph, EMeshPrimitive3D Primitive,
		FVec3 Position, FVec3 Scale = FVec3{ 1.0f, 1.0f, 1.0f } ) noexcept
	{
		FModel3DSpawnParams Params = FModel3DSpawnParams::FromPrimitive( Primitive, Position );
		Params.Scale = Scale;
		return CModel3DSpawner::SpawnInto( Graph, Params );
	}

	/** 浮動小数の小さな誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 1.0e-3f;
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference < kTolerance, Label );
	}
}


void RunSceneCollision3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSceneCollision3D / 描画境界を登録して現在位置へ同期する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const SphereNode = PlacePrimitive(
			Graph, EMeshPrimitive3D::Sphere, FVec3{ 0.0f, 0.0f, 0.0f } );
		Harness.Check( SphereNode != nullptr, "球ノードを置ける" );
		if ( SphereNode == nullptr ) return;

		const FCollisionShapeId3D Shape = Collision.TryAddBounds( *SphereNode, 0x1u );
		Harness.Check( Shape.IsValid(), "球プリミティブを登録できる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u, "登録数を確認できる" );
		Harness.Check( !Collision.TryAddBounds( *SphereNode ).IsValid(), "同じノードの二重登録を弾く" );

		TArray<ANode*> Hits;
		Harness.Check( Collision.TryOverlapSphere(
			FSphere{ FVec3{ 0.49f, 0.49f, 0.0f }, 0.01f }, Hits ), "球の重なりを調べられる" );
		Harness.Check( Hits.IsEmpty(), "境界箱の角ではなく球形で判定する" );

		SphereNode->SetPosition( FVec3{ 3.0f, 0.0f, 0.0f } );
		SphereNode->SetScale( FVec3{ 2.0f, 1.0f, 1.0f } );
		Harness.Check( Collision.TryOverlapSphere(
			FSphere{ FVec3{ 3.9f, 0.0f, 0.0f }, 0.05f }, Hits ), "移動と拡縮を自動同期できる" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == SphereNode, "非一様拡縮は最大軸の安全側で拾う" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Spawned = Graph.TrySpawn( FStringView( "RotatedBox" ) );
		Harness.Check( Spawned.Succeeded(), "明示的な箱を持つノードを置ける" );
		if ( !Spawned ) return;
		Spawned.Node->RotateDeg( FVec3{ 0.0f, 90.0f, 0.0f } );

		Harness.Check( !Collision.TryAddSphere( *Spawned.Node, FVec3{}, 0.0f ).IsValid(),
			"半径0の球を登録前に弾く" );
		Harness.Check( Collision.TryAddBox(
			*Spawned.Node, FVec3{}, FVec3{ 2.0f, 0.25f, 0.25f } ).IsValid(), "細長い箱を登録できる" );

		TArray<ANode*> Hits;
		Harness.Check( Collision.TryOverlapSphere(
			FSphere{ FVec3{ 0.0f, 0.0f, -1.9f }, 0.2f }, Hits ), "回転後の長軸を調べられる" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == Spawned.Node, "箱の回転を世界境界へ反映する" );
		Harness.Check( Collision.TryOverlapSphere(
			FSphere{ FVec3{ 1.9f, 0.0f, 0.0f }, 0.2f }, Hits ), "回転前の長軸位置も調べられる" );
		Harness.Check( Hits.IsEmpty(), "回転前の長軸には形状を残さない" );
	}

	Harness.BeginSuite( "CSceneCollision3D / レイヤーとノード状態を反映する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const First = PlacePrimitive( Graph, EMeshPrimitive3D::Cube, FVec3{} );
		ANode* const Second = PlacePrimitive( Graph, EMeshPrimitive3D::Cube, FVec3{} );
		Harness.Check( First != nullptr && Second != nullptr, "比較用ノードを置ける" );
		if ( First == nullptr || Second == nullptr ) return;

		const FCollisionShapeId3D FirstShape = Collision.TryAddBounds( *First, 0x1u );
		const FCollisionShapeId3D SecondShape = Collision.TryAddBounds( *Second, 0x2u );
		Harness.Check( FirstShape.IsValid() && SecondShape.IsValid(), "別レイヤーで登録できる" );

		TArray<ANode*> Hits;
		Harness.Check( Collision.TryOverlapBox(
			FAabb3{ FVec3{}, FVec3{ 2.0f, 2.0f, 2.0f } }, Hits, {}, 0x2u ), "箱で重なりを調べられる" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == Second, "指定レイヤーだけ返す" );

		Harness.Check( Collision.TryOverlapSphere(
			FSphere{ FVec3{}, 2.0f }, Hits, FirstShape ), "自身を除外できる" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == Second, "除外した形状を返さない" );

		Second->SetVisible( false );
		Harness.Check( Collision.TryOverlapSphere( FSphere{ FVec3{}, 2.0f }, Hits, FirstShape ),
			"非表示でも衝突を調べられる" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == Second, "非表示は衝突を消さない" );

		Second->SetEnabled( false );
		Harness.Check( Collision.TryOverlapSphere( FSphere{ FVec3{}, 2.0f }, Hits, FirstShape ),
			"無効なノードを除いて調べられる" );
		Harness.Check( Hits.IsEmpty(), "無効なノードは衝突対象から外れる" );

		Second->SetEnabled( true );
		Graph.Root().SetEnabled( false );
		Harness.Check( Collision.TryOverlapSphere( FSphere{ FVec3{}, 2.0f }, Hits, FirstShape ),
			"無効な祖先を持つノードを除いて調べられる" );
		Harness.Check( Hits.IsEmpty(), "祖先の無効状態も衝突へ反映する" );
		Graph.Root().SetEnabled( true );

		Harness.Check( Collision.TrySetLayer( SecondShape, 0x4u ), "登録後にレイヤーを変えられる" );
		Harness.Check( Collision.TryOverlapSphere(
			FSphere{ FVec3{}, 2.0f }, Hits, FirstShape, 0x4u ), "変更後のレイヤーで調べられる" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == Second, "変更後のレイヤーへ戻る" );
	}

	Harness.BeginSuite( "CSceneCollision3D / 球を動かして最初の接触を返す" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const Wall = PlacePrimitive(
			Graph, EMeshPrimitive3D::Cube, FVec3{ 0.0f, 0.0f, 5.0f } );
		Harness.Check( Wall != nullptr, "壁ノードを置ける" );
		if ( Wall == nullptr ) return;
		const FCollisionShapeId3D WallShape = Collision.TryAddBounds( *Wall );
		Harness.Check( WallShape.IsValid(), "壁の境界を登録できる" );

		const FSceneRay Ray = FSceneRay::FromDirection(
			FVec3{}, FVec3{ 0.0f, 0.0f, 1.0f }, 10.0f );
		FSceneSweepHit3D Hit;
		Harness.Check( Collision.TrySweepSphere( Ray, 0.5f, Hit ), "移動球が壁へ触れる" );
		Harness.Check( Hit.IsHit() && Hit.Node == Wall && Hit.Shape == WallShape, "壁ノードと形状を返す" );
		CheckNear( Harness, Hit.Distance, 4.0f, "球表面が壁へ届く距離を返す" );
		CheckNear( Harness, Hit.Center.z, 4.0f, "接触時の球中心を返す" );
		CheckNear( Harness, Hit.Normal.z, -1.0f, "壁から球へ向く法線を返す" );

		const FSceneSweepHit3D Before = Hit;
		Harness.Check( !Collision.TrySweepSphere( Ray, 0.5f, Hit, WallShape ), "除外すれば外れる" );
		Harness.Check( Hit.Node == Before.Node && Hit.Shape == Before.Shape,
			"外れた場合は以前の結果を変えない" );
	}

	Harness.BeginSuite( "CSceneCollision3D / 不正入力と破棄を安全に扱う" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		TObjectPtr<ANode> Outside = NewObject<ANode>();
		Harness.Check( Outside.IsValid(), "グラフ外ノードを作れる" );
		if ( !Outside ) return;
		Outside->AddComponent<AMeshComponent3D>();
		Harness.Check( !Collision.TryAddBounds( *Outside ).IsValid(), "グラフ外ノードを弾く" );

		ANode* const Node = PlacePrimitive( Graph, EMeshPrimitive3D::Cube, FVec3{} );
		Harness.Check( Node != nullptr, "破棄確認用ノードを置ける" );
		if ( Node == nullptr ) return;
		const FNodeId NodeId = Graph.IdOf( Node );
		Harness.Check( Collision.TryAddBounds( *Node ).IsValid(), "破棄確認用の形状を登録できる" );

		TArray<ANode*> Hits;
		Hits.TryAdd( &Graph.Root() );
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		Harness.Check( !Collision.TryOverlapSphere( FSphere{ FVec3{}, -1.0f }, Hits ),
			"負の半径を弾く" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == &Graph.Root(), "不正入力で結果を変えない" );

		Node->SetPosition( FVec3{ Infinity, 0.0f, 0.0f } );
		Harness.Check( !Collision.TryOverlapSphere( FSphere{ FVec3{}, 2.0f }, Hits ),
			"有限でないTransformを弾く" );
		Harness.Check( Hits.Num() == 1u && Hits[0] == &Graph.Root(), "同期失敗でも結果を変えない" );

		Node->SetPosition( FVec3{} );
		Harness.Check( Graph.Destroy( NodeId ), "ノードを破棄予定にできる" );
		Harness.Check( Collision.TryOverlapSphere( FSphere{ FVec3{}, 2.0f }, Hits ),
			"破棄予定ノードを除いて問い合わせできる" );
		Harness.Check( Hits.IsEmpty(), "破棄予定ノードを返さない" );

		Graph.ResolveStructuralChanges();
		Harness.Check( Collision.Sync(), "破棄後の登録を掃除できる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "破棄済みノードの形状を残さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		ANode* const OldNode = PlacePrimitive( Graph, EMeshPrimitive3D::Cube, FVec3{} );
		Harness.Check( OldNode != nullptr && Collision.TryAddBounds( *OldNode ).IsValid(),
			"交換前のグラフへ形状を登録できる" );

		CSceneNodeGraph Replacement;
		ANode* const NewNode = PlacePrimitive( Replacement, EMeshPrimitive3D::Sphere, FVec3{} );
		Harness.Check( NewNode != nullptr, "交換用グラフへ別ノードを置ける" );
		Graph.SwapContents( Replacement );

		TArray<ANode*> Hits;
		Harness.Check( Collision.TryOverlapSphere( FSphere{ FVec3{}, 2.0f }, Hits ),
			"グラフ全置換後も安全に問い合わせできる" );
		Harness.Check( Hits.IsEmpty(), "交換前の形状を別ノードへ誤接続しない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "全置換時に旧登録を自動で外す" );
	}
}

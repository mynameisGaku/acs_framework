// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3D.h"
#include "Common/Test/TestHarness.h"

#include <limits>


void RunProximityTrigger3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FProximityTrigger3DParams / 球と箱の範囲を検証する" );

	{
		FProximityTrigger3DParams Params = FProximityTrigger3DParams::Around( 3.0f, 0x2u );
		Harness.Check( Params.IsValid(), "有限な半径とレイヤーで近接範囲を作る" );
		Harness.CheckEqualF32( Params.LocalRadius, 3.0f, "指定したローカル半径を保つ" );
		Harness.CheckEqualU64( Params.CollisionMask, 0x2u, "指定した対象レイヤーを保つ" );

		Params.LocalRadius = 0.0f;
		Harness.Check( !Params.IsValid(), "半径0を問い合わせ前に拒否する" );
		Params.LocalRadius = FProximityTrigger3DParams::kMaximumLocalRadius + 1.0f;
		Harness.Check( !Params.IsValid(), "極端に大きい半径を問い合わせ前に拒否する" );
		Params.LocalRadius = 1.0f;
		Params.LocalCenter.x = std::numeric_limits<f32>::infinity();
		Harness.Check( !Params.IsValid(), "有限でないローカル中心を拒否する" );
		Params.LocalCenter = FVec3{};
		Params.CollisionMask = 0u;
		Harness.Check( !Params.IsValid(), "対象の無いレイヤーマスクを拒否する" );
	}

	{
		FProximityTrigger3DParams Params = FProximityTrigger3DParams::Box(
			FVec3{ 2.0f, 1.0f, 3.0f }, 0x4u );
		Harness.Check( Params.IsValid(), "有限な半サイズとレイヤーで箱範囲を作る" );
		Harness.Check( Params.Kind == FProximityTrigger3DParams::EKind::Box,
			"箱の形状種別を明示する" );
		Harness.CheckEqualF32( Params.LocalHalfSize.z, 3.0f,
			"指定したローカル半サイズを保つ" );
		Harness.CheckEqualU64( Params.CollisionMask, 0x4u,
			"箱でも指定した対象レイヤーを保つ" );

		Params.LocalHalfSize.y = 0.0f;
		Harness.Check( !Params.IsValid(), "厚み0の箱を問い合わせ前に拒否する" );
		Params.LocalHalfSize.y = 1.0f;
		Params.LocalHalfSize.z = FProximityTrigger3DParams::kMaximumLocalHalfSize + 1.0f;
		Harness.Check( !Params.IsValid(), "極端に大きい箱を問い合わせ前に拒否する" );
		Params.LocalHalfSize.z = 1.0f;
		Params.Kind = static_cast<FProximityTrigger3DParams::EKind>( 255u );
		Harness.Check( !Params.IsValid(), "未定義の近接形状を拒否する" );
	}

	Harness.BeginSuite( "CProximityTrigger3D / 進入、滞在、退出を追跡する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		Harness.Check( Collision.IsBoundTo( Graph )
			&& !Collision.IsBoundTo( OtherGraph ),
			"衝突集合が接続中の場面を確認できる" );

		const FScene3DSpawnResult OriginSpawn = Graph.TrySpawn( FStringView( "TriggerOrigin" ) );
		const FScene3DSpawnResult FirstSpawn = Graph.TrySpawn( FStringView( "FirstTarget" ) );
		const FScene3DSpawnResult SecondSpawn = Graph.TrySpawn( FStringView( "SecondTarget" ) );
		const FScene3DSpawnResult IgnoredSpawn = Graph.TrySpawn( FStringView( "IgnoredTarget" ) );
		Harness.Check( OriginSpawn && FirstSpawn && SecondSpawn && IgnoredSpawn,
			"近接追跡に使う4ノードを置ける" );
		if ( !OriginSpawn || !FirstSpawn || !SecondSpawn || !IgnoredSpawn ) return;

		FirstSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		SecondSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 5.0f } );
		IgnoredSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		Harness.Check( Collision.TryAddSphere(
			*OriginSpawn.Node, FVec3{}, 0.25f, 0x2u ).IsValid(),
			"基準ノード自身の形状を自己除外確認用に登録する" );
		Harness.Check( Collision.TryAddSphere(
			*FirstSpawn.Node, FVec3{}, 0.25f, 0x2u ).IsValid(),
			"最初の対象形状を登録する" );
		Harness.Check( Collision.TryAddSphere(
			*SecondSpawn.Node, FVec3{}, 0.25f, 0x2u ).IsValid(),
			"次の対象形状を登録する" );
		Harness.Check( Collision.TryAddSphere(
			*IgnoredSpawn.Node, FVec3{}, 0.25f, 0x4u ).IsValid(),
			"別レイヤーの対象形状を登録する" );

		CProximityTrigger3D Trigger;
		Harness.Check( !Trigger.Bind( Graph, OtherCollision, *OriginSpawn.Node,
			FProximityTrigger3DParams::Around( 2.0f, 0x2u ) ),
			"別場面の衝突集合へ接続しない" );
		Harness.Check( Trigger.Bind( Graph, Collision, *OriginSpawn.Node,
			FProximityTrigger3DParams::Around( 2.0f, 0x2u ) ),
			"自場面の基準ノードと衝突集合へ接続する" );
		Harness.Check( Trigger.Origin() == OriginSpawn.Node,
			"接続中の基準ノードを世代付き識別子から解決する" );
		Harness.Check( !Trigger.Bind( Graph, Collision, *FirstSpawn.Node ),
			"接続中の二重接続を拒否する" );

		FProximityTrigger3DUpdateResult Result;
		Harness.Check( Trigger.Update( Result ), "最初の近接状態を求める" );
		Harness.Check( Result.DidEnter( FirstSpawn.Id )
			&& Result.IsInside( FirstSpawn.Id ) && !Result.DidExit( FirstSpawn.Id ),
			"最初から範囲内の対象を進入と滞在で返す" );
		Harness.Check( !Result.IsInside( OriginSpawn.Id ),
			"基準ノード自身の衝突形状を結果から外す" );
		Harness.Check( !Result.IsInside( IgnoredSpawn.Id ),
			"対象外レイヤーの形状を結果から外す" );
		Harness.CheckEqualU64( Trigger.InsideCount(), 1u,
			"直前の範囲内状態を1件保持する" );

		Harness.Check( Trigger.Update( Result ), "同じ配置をもう1回更新する" );
		Harness.CheckEqualU64( Result.EnteredNodes.Num(), 0u,
			"滞在中の対象を進入として繰り返さない" );
		Harness.CheckEqualU64( Result.InsideNodes.Num(), 1u,
			"同じ対象を滞在として返す" );
		Harness.CheckEqualU64( Result.ExitedNodes.Num(), 0u,
			"同じ配置では退出を返さない" );

		FirstSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 4.0f } );
		SecondSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		Harness.Check( Trigger.Update( Result ), "2対象を入れ替えて更新する" );
		Harness.Check( Result.DidEnter( SecondSpawn.Id )
			&& Result.DidExit( FirstSpawn.Id )
			&& Result.IsInside( SecondSpawn.Id ),
			"同じ更新で進入と退出を分けて返す" );

		SecondSpawn.Node->SetEnabled( false );
		Harness.Check( Trigger.Update( Result ) && Result.DidExit( SecondSpawn.Id ),
			"無効化した対象を同じ更新で退出にする" );
		SecondSpawn.Node->SetEnabled( true );
		Harness.Check( Trigger.Update( Result ) && Result.DidEnter( SecondSpawn.Id ),
			"再び有効にした範囲内対象を進入にする" );

		FProximityTrigger3DParams LayerParams = Trigger.Params();
		LayerParams.CollisionMask = 0x4u;
		Harness.Check( Trigger.SetParams( LayerParams ),
			"次回から調べる対象レイヤーを変更する" );
		Harness.Check( Trigger.Update( Result )
			&& Result.DidEnter( IgnoredSpawn.Id )
			&& Result.DidExit( SecondSpawn.Id ),
			"レイヤー変更を進入と退出へ反映する" );

		FProximityTrigger3DParams InvalidParams = LayerParams;
		InvalidParams.LocalRadius = 0.0f;
		Harness.Check( !Trigger.SetParams( InvalidParams )
			&& Trigger.Params().CollisionMask == 0x4u,
			"不正設定では以前の対象レイヤーを保つ" );
		Trigger.ResetState();
		Harness.Check( Trigger.Update( Result ) && Result.DidEnter( IgnoredSpawn.Id ),
			"状態初期化後は現在対象を改めて進入にする" );

		FProximityTrigger3DUpdateResult Unchanged;
		Harness.Check( Unchanged.EnteredNodes.TryAdd( FirstSpawn.Id ),
			"失敗時非変更を調べる出力を準備する" );
		CSceneNodeGraph Replacement;
		Graph.SwapContents( Replacement );
		Harness.Check( !Trigger.Update( Unchanged ) && !Trigger.IsBound(),
			"場面内容の全差し替えで古い接続を解除する" );
		Harness.CheckEqualU64( Unchanged.EnteredNodes.Num(), 1u,
			"場面差し替え失敗時は呼出側の出力を変えない" );
		Harness.Check( Unchanged.EnteredNodes[0] == FirstSpawn.Id,
			"場面差し替え失敗時は出力内容も保つ" );
		Harness.Check( Trigger.Origin() == nullptr,
			"場面差し替え後に旧基準ノードを解決しない" );
	}

	Harness.BeginSuite( "CProximityTrigger3D / 親変形をworld球へ反映する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult OriginSpawn = Graph.TrySpawn( FStringView( "ScaledOrigin" ) );
		const FScene3DSpawnResult TargetSpawn = Graph.TrySpawn( FStringView( "ScaledTarget" ) );
		Harness.Check( OriginSpawn && TargetSpawn,
			"world球変換に使う基準と対象を置ける" );
		if ( !OriginSpawn || !TargetSpawn ) return;

		OriginSpawn.Node->SetPosition( FVec3{ 10.0f, 0.0f, 0.0f } );
		OriginSpawn.Node->SetScale( FVec3{ 2.0f, 1.0f, 1.0f } );
		TargetSpawn.Node->SetPosition( FVec3{ 13.5f, 0.0f, 0.0f } );
		Harness.Check( Collision.TryAddSphere(
			*TargetSpawn.Node, FVec3{}, 0.1f, 0x1u ).IsValid(),
			"変換後の球へ入る対象形状を登録する" );

		FProximityTrigger3DParams Params = FProximityTrigger3DParams::Around( 1.0f, 0x1u );
		Params.LocalCenter = FVec3{ 1.0f, 0.0f, 0.0f };
		CProximityTrigger3D Trigger;
		Harness.Check( Trigger.Bind( Graph, Collision, *OriginSpawn.Node, Params ),
			"ローカル中心を持つ近接トリガーを接続する" );
		FProximityTrigger3DUpdateResult Result;
		Harness.Check( Trigger.Update( Result ) && Result.IsInside( TargetSpawn.Id ),
			"位置、ローカル中心、最大拡縮率をworld球へ反映する" );

		FSphere UnchangedSphere{ FVec3{ 9.0f, 8.0f, 7.0f }, 6.0f };
		Harness.Check( !CSceneCollision3D::TryMakeWorldSphere(
			*OriginSpawn.Node, FVec3{}, 0.0f, UnchangedSphere ),
			"半径0を公開変換アダプターで拒否する" );
		Harness.CheckEqualF32( UnchangedSphere.center.x, 9.0f,
			"球変換失敗時は呼出側の中心を変えない" );
		Harness.CheckEqualF32( UnchangedSphere.radius, 6.0f,
			"球変換失敗時は呼出側の半径を変えない" );
	}

	Harness.BeginSuite( "CProximityTrigger3D / 回転するローカル箱をworld軸平行箱へ変換する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult OriginSpawn = Graph.TrySpawn( FStringView( "RotatedBoxOrigin" ) );
		const FScene3DSpawnResult RotatedAxisSpawn = Graph.TrySpawn( FStringView( "RotatedAxisTarget" ) );
		const FScene3DSpawnResult OldAxisSpawn = Graph.TrySpawn( FStringView( "OldAxisTarget" ) );
		Harness.Check( OriginSpawn && RotatedAxisSpawn && OldAxisSpawn,
			"箱変換に使う基準と2対象を置ける" );
		if ( !OriginSpawn || !RotatedAxisSpawn || !OldAxisSpawn ) return;

		OriginSpawn.Node->RotateDeg( FVec3{ 0.0f, 90.0f, 0.0f } );
		RotatedAxisSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, -1.8f } );
		OldAxisSpawn.Node->SetPosition( FVec3{ 1.8f, 0.0f, 0.0f } );
		Harness.Check( Collision.TryAddSphere(
			*RotatedAxisSpawn.Node, FVec3{}, 0.05f, 0x1u ).IsValid(),
			"回転後の長軸に入る対象を登録する" );
		Harness.Check( Collision.TryAddSphere(
			*OldAxisSpawn.Node, FVec3{}, 0.05f, 0x1u ).IsValid(),
			"回転前の長軸にだけ入る比較対象を登録する" );

		CProximityTrigger3D Trigger;
		Harness.Check( Trigger.Bind( Graph, Collision, *OriginSpawn.Node,
			FProximityTrigger3DParams::Box( FVec3{ 2.0f, 0.25f, 0.25f }, 0x1u ) ),
			"ローカル箱を持つ近接トリガーを接続する" );
		FProximityTrigger3DUpdateResult Result;
		Harness.Check( Trigger.Update( Result ), "回転後のworld箱で近接状態を求める" );
		Harness.Check( Result.IsInside( RotatedAxisSpawn.Id ),
			"回転後の長軸にいる対象を箱範囲へ含める" );
		Harness.Check( !Result.IsInside( OldAxisSpawn.Id ),
			"回転前の長軸位置を箱範囲へ残さない" );

		FAabb3 Unchanged{ FVec3{ 9.0f, 8.0f, 7.0f }, FVec3{ 6.0f, 5.0f, 4.0f } };
		Harness.Check( !CSceneCollision3D::TryMakeWorldBox(
			*OriginSpawn.Node, FVec3{}, FVec3{ -1.0f, 1.0f, 1.0f }, Unchanged ),
			"負の半サイズを公開変換アダプターで拒否する" );
		Harness.CheckEqualF32( Unchanged.center.x, 9.0f,
			"箱変換失敗時は呼出側の出力を変えない" );
	}
}

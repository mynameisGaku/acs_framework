// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawner.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DQueue.h"
#include "Common/Test/TestHarness.h"

#include <limits>


void RunCheckpoint3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FCheckpoint3DParams / 球と箱の発火方法をまとめる" );

	{
		const FCheckpoint3DParams Sphere = FCheckpoint3DParams::Around(
			3.0f, 0x2u, false );
		Harness.Check( Sphere.IsValid(), "有限な半径で球型チェックポイントを作る" );
		Harness.CheckEqualF32( Sphere.Range.LocalRadius, 3.0f,
			"指定した球半径を近接範囲へ渡す" );
		Harness.CheckEqualU64( Sphere.Range.CollisionMask, 0x2u,
			"指定した対象レイヤーを近接範囲へ渡す" );
		Harness.Check( !Sphere.bActivateOnce,
			"再進入で発火する設定を保持する" );

		FCheckpoint3DParams Box = FCheckpoint3DParams::Box(
			FVec3{ 2.0f, 1.0f, 3.0f }, 0x4u );
		Harness.Check( Box.IsValid()
			&& Box.Range.Kind == FProximityTrigger3DParams::EKind::Box,
			"有限な半サイズで箱型チェックポイントを作る" );
		Box.Range.LocalHalfSize.y = 0.0f;
		Harness.Check( !Box.IsValid(), "厚み0の箱を接続前に拒否する" );
	}

	Harness.BeginSuite( "CCheckpoint3D / 指定した1形状の進入だけを発火する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult OriginSpawn = Graph.TrySpawn(
			FStringView( "CheckpointOrigin" ) );
		const FScene3DSpawnResult TargetSpawn = Graph.TrySpawn(
			FStringView( "CheckpointTarget" ) );
		const FScene3DSpawnResult OtherSpawn = Graph.TrySpawn(
			FStringView( "OtherTarget" ) );
		Harness.Check( OriginSpawn && TargetSpawn && OtherSpawn,
			"範囲基準、追跡対象、比較対象を置ける" );
		if ( !OriginSpawn || !TargetSpawn || !OtherSpawn ) return;

		TargetSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 5.0f } );
		OtherSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*TargetSpawn.Node, FVec3{}, 0.25f, 0x2u );
		const FCollisionShapeId3D OtherShape = Collision.TryAddSphere(
			*OtherSpawn.Node, FVec3{}, 0.25f, 0x2u );
		Harness.Check( TargetShape.IsValid() && OtherShape.IsValid(),
			"追跡と比較に使う衝突形状を登録する" );

		CCheckpoint3D SameNodeCheckpoint;
		Harness.Check( !SameNodeCheckpoint.Bind(
			Graph, Collision, *TargetSpawn.Node, TargetShape,
			FCheckpoint3DParams::Around( 2.0f, 0x2u ) ),
			"対象形状自身を範囲基準にはしない" );

		CCheckpoint3D Checkpoint;
		Harness.Check( !Checkpoint.Bind(
			Graph, Collision, *OriginSpawn.Node, TargetShape,
			FCheckpoint3DParams::Around( 2.0f, 0x4u ) ),
			"対象形状と交わらないレイヤー設定を拒否する" );
		Harness.Check( Checkpoint.Bind(
			Graph, Collision, *OriginSpawn.Node, TargetShape,
			FCheckpoint3DParams::Around( 2.0f, 0x2u ) ),
			"自場面の基準ノードと追跡対象へ接続する" );
		Harness.Check( Checkpoint.IsBoundTo( Graph, Collision )
			&& Checkpoint.Origin() == OriginSpawn.Node
			&& Checkpoint.Target() == TargetSpawn.Node
			&& Checkpoint.TargetShape() == TargetShape,
			"接続中の場面、基準、対象形状を確認できる" );

		FCheckpoint3DUpdateResult Result;
		Harness.Check( Checkpoint.Update( Result ),
			"比較対象だけが範囲内の状態を更新する" );
		Harness.Check( !Result.bActivatedThisUpdate
			&& !Result.bTargetInside && !Result.bHasActivated,
			"追跡していない形状の進入では発火しない" );

		TargetSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		Harness.Check( Checkpoint.Update( Result ), "追跡対象を範囲へ入れる" );
		Harness.Check( Result.bActivatedThisUpdate
			&& Result.bTargetInside && Result.bHasActivated,
			"最初の進入で一度限りのチェックポイントを発火する" );
		Harness.Check( Checkpoint.HasActivated() && Checkpoint.IsTargetInside(),
			"直前の範囲内状態と発火済み状態を保持する" );

		Harness.Check( Checkpoint.Update( Result )
			&& !Result.bActivatedThisUpdate && Result.bTargetInside,
			"滞在中は発火を繰り返さない" );
		TargetSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 5.0f } );
		Harness.Check( Checkpoint.Update( Result ) && !Result.bTargetInside,
			"退出を範囲外状態へ反映する" );
		TargetSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		Harness.Check( Checkpoint.Update( Result )
			&& !Result.bActivatedThisUpdate && Result.bHasActivated,
			"一度限りでは再進入を再発火にしない" );

		Checkpoint.ResetActivation();
		Harness.Check( Checkpoint.Update( Result )
			&& Result.bActivatedThisUpdate && Result.bTargetInside,
			"初期化後は現在範囲内の対象を新しい進入として発火する" );

		FCheckpoint3DParams RepeatParams = Checkpoint.Params();
		RepeatParams.bActivateOnce = false;
		Harness.Check( Checkpoint.SetParams( RepeatParams ),
			"再進入ごとに発火する方法へ変更する" );
		TargetSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 5.0f } );
		Harness.Check( Checkpoint.Update( Result ) && !Result.bTargetInside,
			"再発火の前に対象の退出を記録する" );
		TargetSpawn.Node->SetPosition( FVec3{ 0.0f, 0.0f, 1.0f } );
		Harness.Check( Checkpoint.Update( Result )
			&& Result.bActivatedThisUpdate && Result.bHasActivated,
			"再進入可能なら発火済みでも改めて発火する" );

		FCheckpoint3DParams WrongMask = RepeatParams;
		WrongMask.Range.CollisionMask = 0x4u;
		Harness.Check( !Checkpoint.SetParams( WrongMask )
			&& Checkpoint.Params().Range.CollisionMask == 0x2u,
			"対象形状と交わらない設定変更では以前の設定を保つ" );

		CDebugDraw3DQueue Queue( 24u );
		Harness.Check( TryQueueProximityTrigger3D(
			Checkpoint.Range(), Queue, FVec4{ 0.2f, 1.0f, 0.3f, 1.0f }, 8u ),
			"判定と同じ球範囲をGPUなしでデバッグ線へ変換する" );
		Harness.CheckEqualU64( Queue.Num(), 24u,
			"指定した8分割の3円を24本の線として追加する" );

		FCheckpoint3DUpdateResult Unchanged;
		Unchanged.bActivatedThisUpdate = true;
		Unchanged.bTargetInside = true;
		Unchanged.bHasActivated = true;
		Harness.Check( Collision.Remove( TargetShape ),
			"追跡対象形状を場面衝突から外す" );
		Harness.Check( !Checkpoint.Update( Unchanged ) && !Checkpoint.IsBound(),
			"対象形状が消えた更新で古い接続を解除する" );
		Harness.Check( Unchanged.bActivatedThisUpdate
			&& Unchanged.bTargetInside && Unchanged.bHasActivated,
			"対象消失による失敗では呼出側の出力を変えない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult OriginSpawn = Graph.TrySpawn(
			FStringView( "ReplacementOrigin" ) );
		const FScene3DSpawnResult TargetSpawn = Graph.TrySpawn(
			FStringView( "ReplacementTarget" ) );
		Harness.Check( OriginSpawn && TargetSpawn,
			"場面差し替え試験の基準と対象を置ける" );
		if ( !OriginSpawn || !TargetSpawn ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*TargetSpawn.Node, FVec3{}, 0.25f, 0x1u );

		CCheckpoint3D Checkpoint;
		Harness.Check( Checkpoint.Bind(
			Graph, Collision, *OriginSpawn.Node, TargetShape ),
			"場面差し替え前の対象へ接続する" );
		FCheckpoint3DUpdateResult Unchanged;
		Unchanged.bHasActivated = true;
		CSceneNodeGraph Replacement;
		Graph.SwapContents( Replacement );
		Harness.Check( !Checkpoint.Update( Unchanged ) && !Checkpoint.IsBound(),
			"場面内容の全差し替えで古いチェックポイント接続を解除する" );
		Harness.Check( Unchanged.bHasActivated,
			"場面差し替え失敗時は呼出側の出力を変えない" );
	}

	Harness.BeginSuite( "CCheckpoint3DSpawner / 生成、接続、破棄を巻き戻せる" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult TargetSpawn = Graph.TrySpawn(
			FStringView( "SpawnTarget" ) );
		Harness.Check( static_cast<bool>( TargetSpawn ), "生成試験の追跡対象を置ける" );
		if ( !TargetSpawn ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*TargetSpawn.Node, FVec3{}, 0.5f, 0x8u );
		Harness.Check( TargetShape.IsValid(), "生成試験の対象形状を登録する" );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult Spawned = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape,
			FVec3{ 1.0f, 2.0f, 3.0f },
			FCheckpoint3DParams::Box(
				FVec3{ 2.0f, 1.0f, 3.0f }, 0x8u, false ) );
		Harness.Check( Spawned && Checkpoint.IsBoundTo( Graph, Collision ),
			"範囲基準ノードの生成と対象形状への接続を完了する" );
		if ( Spawned )
		{
			Harness.CheckEqualF32( Spawned.Origin()->Local().position.x, 1.0f,
				"指定した範囲基準位置Xを使う" );
			Harness.CheckEqualF32( Spawned.Origin()->Local().position.y, 2.0f,
				"指定した範囲基準位置Yを使う" );
			Harness.CheckEqualF32( Spawned.Origin()->Local().position.z, 3.0f,
				"指定した範囲基準位置Zを使う" );
		}
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 2u,
			"対象と範囲基準の2ノードだけを場面へ置く" );

		CCheckpoint3D Duplicate;
		const FCheckpoint3DSpawnResult Mismatched = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Duplicate, TargetShape, FVec3{},
			FCheckpoint3DParams::Around( 1.0f, 0x1u ) );
		Harness.Check( !Mismatched && !Duplicate.IsBound(),
			"接続時にレイヤー不一致なら空の結果を返す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 2u,
			"接続失敗時に生成済みの範囲基準ノードを巻き戻す" );

		const f32 NotFinite = std::numeric_limits<f32>::quiet_NaN();
		const FCheckpoint3DSpawnResult InvalidPosition =
			CCheckpoint3DSpawner::SpawnInto( Graph, Collision, Duplicate,
				TargetShape, FVec3{ NotFinite, 0.0f, 0.0f } );
		Harness.Check( !InvalidPosition, "有限でない位置を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 2u,
			"不正位置では場面へノードを足さない" );

		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FCheckpoint3DSpawnResult WrongSceneResult = Spawned;
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			OtherGraph, OtherCollision, Checkpoint, WrongSceneResult ),
			"別場面からチェックポイントを破棄しない" );
		Harness.Check( WrongSceneResult.Origin() == Spawned.Origin()
			&& Checkpoint.IsBoundTo( Graph, Collision ),
			"別場面の破棄失敗では結果と接続を保つ" );

		CCheckpoint3D WrongCheckpoint;
		FCheckpoint3DSpawnResult WrongCheckpointResult = Spawned;
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, Collision, WrongCheckpoint, WrongCheckpointResult ),
			"別の未接続チェックポイントでは生成ノードを破棄しない" );
		Harness.Check( WrongCheckpointResult.Origin() == Spawned.Origin()
			&& Checkpoint.IsBoundTo( Graph, Collision ),
			"所有違いの破棄失敗では結果と元の接続を保つ" );

		FCheckpoint3DSpawnResult HandMadeResult;
		Harness.Check( !HandMadeResult,
			"検証付き構築を通らない手製結果を成功扱いしない" );
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, HandMadeResult ),
			"生成所有情報のない結果では無関係なノードを破棄しない" );

		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, Spawned ),
			"生成時の場面から範囲基準と接続をまとめて破棄する" );
		Harness.Check( !Spawned && !Checkpoint.IsBound(),
			"破棄成功後は結果とチェックポイントを空にする" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"破棄後は追跡対象ノードだけを場面へ残す" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u,
			"追跡対象の衝突形状は破棄しない" );
	}

	Harness.BeginSuite( "CCheckpoint3DSpawner / 失効後も生成所有権から後始末する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "RemovedShapeTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"形状失効試験の追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x10u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult Spawned = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{},
			FCheckpoint3DParams::Around( 2.0f, 0x10u ) );
		Harness.Check( Spawned && Checkpoint.IsBound(),
			"形状失効前のチェックポイントを生成する" );
		Harness.Check( Collision.Remove( TargetShape ),
			"更新前に追跡対象形状を登録解除する" );
		Harness.Check( Checkpoint.IsBound(),
			"形状失効だけでは明示更新前の接続情報を保持する" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, Spawned ),
			"対象形状が失効していても生成元を後始末する" );
		Graph.ResolveStructuralChanges();
		Harness.Check( !Spawned && !Checkpoint.IsBound(),
			"形状失効後の成功でも結果と接続を空にする" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"形状失効後も追跡対象ノードは残す" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "PendingOriginTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"破棄予定試験の追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x20u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult Spawned = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{},
			FCheckpoint3DParams::Around( 2.0f, 0x20u ) );
		Harness.Check( Spawned && Graph.Destroy( Spawned.OriginId() ),
			"生成した範囲基準を先に破棄予定へ移す" );
		Harness.Check( Spawned.Origin() == nullptr,
			"破棄予定の範囲基準を生ポインタとして返さない" );
		Graph.ResolveStructuralChanges();
		Harness.Check( Spawned && Spawned.Origin() == nullptr,
			"構造変更確定後も所有結果を保ちながら失効ポインタを隠す" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, Spawned ),
			"範囲基準の破棄確定後も接続と結果を後始末する" );
		Harness.Check( !Spawned && !Checkpoint.IsBound(),
			"破棄予定後の成功でも結果と接続を空にする" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"破棄予定後も追跡対象ノードは残す" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "SwappedResultTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"結果失効試験の追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x40u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult Spawned = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{},
			FCheckpoint3DParams::Around( 2.0f, 0x40u ) );
		Harness.Check( Spawned && Spawned.Origin() != nullptr,
			"場面差し替え前は現在の範囲基準を返す" );
		CSceneNodeGraph Replacement;
		Graph.SwapContents( Replacement );
		Harness.Check( Spawned && Spawned.Origin() == nullptr,
			"場面内容差し替え後は別グラフへ移った生ポインタを返さない" );
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, Spawned ),
			"場面内容が変わった所有結果から別rootを破棄しない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "NestedTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"対象巻き込み試験の追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x80u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult Spawned = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{},
			FCheckpoint3DParams::Around( 2.0f, 0x80u ) );
		ANode* const Origin = Spawned.Origin();
		Harness.Check( Origin != nullptr, "対象巻き込み試験の範囲基準を置ける" );
		if ( Origin == nullptr ) return;
		Target.Node->Reparent( *Origin );
		Graph.ResolveStructuralChanges();
		Harness.Check( Target.Node->Parent() == Origin,
			"追跡対象を生成後の範囲基準配下へ付け替える" );
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, Spawned ),
			"追跡対象を子孫に持つ範囲基準は先に破棄しない" );
		Harness.Check( Spawned && Checkpoint.IsBound()
			&& Graph.Get( Target.Id ) == Target.Node,
			"対象巻き込みの拒否で結果、接続、対象を保つ" );
		Target.Node->Reparent( Graph.Root() );
		Graph.ResolveStructuralChanges();
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, Spawned ),
			"対象を基準配下から外した後は生成結果を片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"対象を巻き込まず範囲基準だけを破棄する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u,
			"対象の衝突形状も登録したまま残す" );
	}

	Harness.BeginSuite( "CCheckpoint3DSpawner / 再接続後も各生成結果を独立して片付ける" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "ActiveRebindTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"再接続中の破棄試験へ追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x40u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult OldSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{ -4.0f, 0.0f, 0.0f },
			FCheckpoint3DParams::Around( 2.0f, 0x40u ) );
		Checkpoint.Unbind();
		FCheckpoint3DSpawnResult CurrentSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{ 4.0f, 0.0f, 0.0f },
			FCheckpoint3DParams::Around( 2.0f, 0x40u ) );
		ANode* const CurrentOrigin = CurrentSpawn.Origin();
		Harness.Check( OldSpawn && CurrentSpawn
			&& OldSpawn.BindingRevision() != CurrentSpawn.BindingRevision(),
			"同じチェックポイントの再接続を別世代の結果として保持する" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, OldSpawn ),
			"現在接続より古い生成ノードだけを片付ける" );
		Harness.Check( Checkpoint.IsBoundTo( Graph, Collision )
			&& Checkpoint.Origin() == CurrentOrigin,
			"古い結果の破棄では現在の接続を解除しない" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 2u,
			"古い範囲基準だけを場面から外す" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, CurrentSpawn ),
			"現在世代の生成結果も続けて片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"再接続中の全基準を片付けて対象だけ残す" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "FinishedRebindTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"再接続後の古い結果試験へ追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x80u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult OldSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{ -3.0f, 0.0f, 0.0f },
			FCheckpoint3DParams::Around( 2.0f, 0x80u ) );
		Checkpoint.Unbind();
		FCheckpoint3DSpawnResult NewSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{ 3.0f, 0.0f, 0.0f },
			FCheckpoint3DParams::Around( 2.0f, 0x80u ) );
		Harness.Check( OldSpawn && NewSpawn,
			"新旧2つの正規生成結果を用意する" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, NewSpawn ),
			"新しい生成結果を先に片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.Check( !Checkpoint.IsBound() && OldSpawn,
			"新しい接続の解除後も古い正規結果を保持する" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, OldSpawn ),
			"現在世代より古い正規結果も後から片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"新旧の破棄順に依存せず対象だけ残す" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D OldCollision{ Graph };
		CSceneCollision3D NewCollision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "SameOriginRebindTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"同じ基準への再接続試験へ追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D OldTargetShape = OldCollision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x100u );
		const FCollisionShapeId3D NewTargetShape = NewCollision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x100u );
		const FCheckpoint3DParams Params = FCheckpoint3DParams::Around(
			2.0f, 0x100u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult Spawned = CCheckpoint3DSpawner::SpawnInto(
			Graph, OldCollision, Checkpoint, OldTargetShape, FVec3{}, Params );
		ANode* const Origin = Spawned.Origin();
		const u64 SpawnRevision = Spawned.BindingRevision();
		Checkpoint.Unbind();
		Harness.Check( Origin != nullptr && Checkpoint.Bind(
			Graph, NewCollision, *Origin, NewTargetShape, Params )
			&& Checkpoint.BindingRevision() != SpawnRevision,
			"同じ基準ノードへ別の衝突集合と接続世代で繋ぎ直す" );
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, OldCollision, Checkpoint, Spawned ),
			"別衝突集合でも同じ基準を使う新しい接続を破棄しない" );
		Harness.Check( Spawned && Checkpoint.Origin() == Origin,
			"同じ基準の破棄拒否で結果と新しい接続を保つ" );
		Checkpoint.Unbind();
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, OldCollision, Checkpoint, Spawned ),
			"新しい接続を外した後は古い生成結果を片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"同じ基準への再接続でも対象だけ残す" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult OldTarget = Graph.TrySpawn(
			FStringView( "OldBindingTarget" ) );
		const FScene3DSpawnResult NewTarget = Graph.TrySpawn(
			FStringView( "NewBindingTarget" ) );
		const FScene3DSpawnResult NewOrigin = Graph.TrySpawn(
			FStringView( "NewBindingOrigin" ) );
		Harness.Check( OldTarget && NewTarget && NewOrigin,
			"別対象への再接続試験へ対象2つと新基準を置ける" );
		if ( !OldTarget || !NewTarget || !NewOrigin ) return;
		const FCollisionShapeId3D OldTargetShape = Collision.TryAddSphere(
			*OldTarget.Node, FVec3{}, 0.5f, 0x400u );
		const FCollisionShapeId3D NewTargetShape = Collision.TryAddSphere(
			*NewTarget.Node, FVec3{}, 0.5f, 0x400u );
		const FCheckpoint3DParams Params = FCheckpoint3DParams::Around(
			2.0f, 0x400u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult OldSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, OldTargetShape, FVec3{}, Params );
		ANode* const OldOrigin = OldSpawn.Origin();
		Checkpoint.Unbind();
		Harness.Check( OldOrigin != nullptr, "古い生成結果の基準を取得する" );
		if ( OldOrigin == nullptr ) return;
		NewTarget.Node->Reparent( *OldOrigin );
		Graph.ResolveStructuralChanges();
		Harness.Check( Checkpoint.Bind(
			Graph, Collision, *NewOrigin.Node, NewTargetShape, Params ),
			"古い基準配下の別対象へ新しい基準から接続する" );
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, OldSpawn ),
			"現在追跡中の別対象を子孫に持つ古い基準は破棄しない" );
		Harness.Check( OldSpawn && Checkpoint.Target() == NewTarget.Node
			&& Graph.Get( NewTarget.Id ) == NewTarget.Node,
			"別対象の巻き込み拒否で結果、接続、対象を保つ" );
		Checkpoint.Unbind();
		NewTarget.Node->Reparent( Graph.Root() );
		Graph.ResolveStructuralChanges();
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, OldSpawn ),
			"現在対象を基準配下から外した後は古い結果を片付ける" );
		Harness.Check( Graph.Destroy( NewOrigin.Id ),
			"試験用の新しい基準も片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 2u,
			"新旧の追跡対象をどちらも場面へ残す" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Target = Graph.TrySpawn(
			FStringView( "NestedRebindTarget" ) );
		Harness.Check( static_cast<bool>( Target ),
			"親子再接続の破棄試験へ追跡対象を置ける" );
		if ( !Target ) return;
		const FCollisionShapeId3D TargetShape = Collision.TryAddSphere(
			*Target.Node, FVec3{}, 0.5f, 0x200u );

		CCheckpoint3D Checkpoint;
		FCheckpoint3DSpawnResult ParentSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{},
			FCheckpoint3DParams::Around( 2.0f, 0x200u ) );
		Checkpoint.Unbind();
		FCheckpoint3DSpawnResult ChildSpawn = CCheckpoint3DSpawner::SpawnInto(
			Graph, Collision, Checkpoint, TargetShape, FVec3{ 1.0f, 0.0f, 0.0f },
			FCheckpoint3DParams::Around( 2.0f, 0x200u ), ParentSpawn.Origin() );
		Harness.Check( ParentSpawn && ChildSpawn
			&& ChildSpawn.Origin()->Parent() == ParentSpawn.Origin(),
			"新しい範囲基準を古い範囲基準の子として生成する" );
		Harness.Check( !CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, ParentSpawn ),
			"現在接続中の基準を子に持つ古い結果は先に破棄しない" );
		Harness.Check( ParentSpawn && ChildSpawn
			&& Checkpoint.Origin() == ChildSpawn.Origin(),
			"親側の破棄拒否で新旧結果と現在接続を保つ" );
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, ChildSpawn ),
			"現在接続中の子を先に片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.Check( CCheckpoint3DSpawner::Destroy(
			Graph, Collision, Checkpoint, ParentSpawn ),
			"子を外した後は古い親基準も片付ける" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u,
			"親子の破棄順を守って追跡対象だけ残す" );
	}
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3D.h"
#include "Common/Test/TestHarness.h"


void RunCheckpointRoute3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FCheckpointRoute3DParams / 件数と周回数だけで順序ルートを作る" );

	{
		const FCheckpointRoute3DParams Defaults;
		Harness.Check( Defaults.IsValid(), "既定値は1件を1周するルートになる" );
		Harness.CheckEqualU64( Defaults.CheckpointCount, 1u,
			"既定のチェックポイント数" );
		Harness.CheckEqualU64( Defaults.LapCount, 1u, "既定の周回数" );

		const FCheckpointRoute3DParams Race =
			FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u );
		Harness.Check( Race.IsValid(), "3件を2周する設定を短く作れる" );
		Harness.CheckEqualU64( Race.CheckpointCount, 3u,
			"指定したチェックポイント数" );
		Harness.CheckEqualU64( Race.LapCount, 2u, "指定した周回数" );

		FCheckpointRoute3DParams Broken = Race;
		Broken.CheckpointCount = 0u;
		Harness.Check( !Broken.IsValid(), "チェックポイント0件を拒否する" );
		Broken = Race;
		Broken.LapCount = 0u;
		Harness.Check( !Broken.IsValid(), "周回0回を拒否する" );
	}

	Harness.BeginSuite( "FCheckpointRoute3D / 順番違いを進めず複数周を完了する" );

	{
		FCheckpointRoute3D OneCheckpoint;
		FCheckpointRoute3DAdvanceResult OneResult;
		Harness.Check( OneCheckpoint.Advance( 0u, OneResult )
			&& OneResult.bAccepted && OneResult.bLapCompletedThisAdvance
			&& OneResult.bRouteCompletedThisAdvance
			&& OneResult.bRouteCompleted && !OneResult.bHasNextCheckpoint,
			"既定値だけなら先頭の1回で1周を完了する" );
		Harness.Check( !OneCheckpoint.IsExpectedCheckpoint( 0u ),
			"完了後は有効番号も次の順番として扱わない" );

		FCheckpointRoute3D Route;
		Harness.Check( Route.SetParams(
			FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u ) ),
			"3件を2周する進行へ設定する" );

		u32 Next = 99u;
		Harness.Check( Route.TryGetNextCheckpointIndex( Next ) && Next == 0u,
			"開始時は先頭番号を受け付ける" );
		Harness.Check( Route.IsExpectedCheckpoint( 0u )
			&& !Route.IsExpectedCheckpoint( 1u ),
			"現在受け付ける番号だけを確認できる" );

		FCheckpointRoute3DAdvanceResult Unchanged;
		Unchanged.NextCheckpointIndex = 77u;
		Unchanged.CompletedLapCount = 88u;
		Unchanged.bAccepted = true;
		Harness.Check( !Route.Advance( 3u, Unchanged ),
			"範囲外の番号を拒否する" );
		Harness.Check( Unchanged.NextCheckpointIndex == 77u
			&& Unchanged.CompletedLapCount == 88u && Unchanged.bAccepted,
			"拒否時は呼出側の結果を変えない" );

		FCheckpointRoute3DAdvanceResult Result;
		Harness.Check( Route.Advance( 1u, Result ),
			"範囲内の順番違いを正常に処理する" );
		Harness.Check( !Result.bAccepted && Result.bOutOfOrder
			&& Result.bHasNextCheckpoint && Result.NextCheckpointIndex == 0u,
			"順番違いを明示して先頭のまま保つ" );
		Harness.CheckEqualU64( Route.CompletedLapCount(), 0u,
			"順番違いでは周回を進めない" );

		Harness.Check( Route.Advance( 0u, Result ) && Result.bAccepted
			&& !Result.bOutOfOrder && Result.NextCheckpointIndex == 1u,
			"先頭を受理して次を1へ進める" );
		Harness.Check( Route.Advance( 0u, Result )
			&& !Result.bAccepted && Result.bOutOfOrder
			&& Result.NextCheckpointIndex == 1u,
			"同じ発火を重ねても二重に進めない" );
		Harness.Check( Route.Advance( 1u, Result ) && Result.bAccepted
			&& Result.NextCheckpointIndex == 2u,
			"中央を受理して末尾を待つ" );
		Harness.Check( Route.Advance( 2u, Result ) && Result.bAccepted
			&& Result.bLapCompletedThisAdvance
			&& !Result.bRouteCompletedThisAdvance
			&& Result.CompletedLapCount == 1u
			&& Result.bHasNextCheckpoint && Result.NextCheckpointIndex == 0u,
			"1周目の末尾で先頭へ戻す" );

		Harness.Check( Route.Advance( 0u, Result ) && Result.bAccepted,
			"2周目の先頭を受理する" );
		Harness.Check( Route.Advance( 1u, Result ) && Result.bAccepted,
			"2周目の中央を受理する" );
		Harness.Check( Route.Advance( 2u, Result ) && Result.bAccepted
			&& Result.bLapCompletedThisAdvance
			&& Result.bRouteCompletedThisAdvance
			&& Result.bRouteCompleted && !Result.bHasNextCheckpoint
			&& Result.CompletedLapCount == 2u,
			"2周目の末尾でルートを一度だけ完了する" );
		Harness.Check( Route.IsComplete()
			&& Route.CompletedLapCount() == 2u,
			"完了状態と完了周回数を保持する" );

		Next = 99u;
		Harness.Check( !Route.TryGetNextCheckpointIndex( Next ) && Next == 99u,
			"完了後は次番号を返さず出力を保つ" );
		Harness.Check( Route.Advance( 0u, Result )
			&& !Result.bAccepted && !Result.bOutOfOrder
			&& !Result.bRouteCompletedThisAdvance
			&& Result.bRouteCompleted && !Result.bHasNextCheckpoint,
			"完了後の発火は再完了させない" );

		Route.Reset();
		Harness.Check( !Route.IsComplete()
			&& Route.CompletedLapCount() == 0u
			&& Route.TryGetNextCheckpointIndex( Next ) && Next == 0u,
			"設定を保って先頭からやり直す" );
	}

	Harness.BeginSuite( "FCheckpointRoute3D / 不正な再設定で進行を保つ" );

	{
		FCheckpointRoute3D Route;
		Harness.Check( Route.SetParams(
			FCheckpointRoute3DParams::ForCheckpoints( 2u, 3u ) ),
			"保持確認用の設定を作る" );
		FCheckpointRoute3DAdvanceResult Result;
		Harness.Check( Route.Advance( 0u, Result ) && Result.bAccepted,
			"先頭を進めてから再設定する" );

		FCheckpointRoute3DParams Broken = Route.Params();
		Broken.LapCount = 0u;
		Harness.Check( !Route.SetParams( Broken ),
			"不正な周回数を拒否する" );
		u32 Next = 0u;
		Harness.Check( Route.Params().CheckpointCount == 2u
			&& Route.Params().LapCount == 3u
			&& Route.CompletedLapCount() == 0u
			&& Route.TryGetNextCheckpointIndex( Next ) && Next == 1u,
			"拒否時は設定と途中進行を保つ" );

		Harness.Check( Route.SetParams(
			FCheckpointRoute3DParams::ForCheckpoints( 1u, 1u ) ),
			"有効な再設定を受け付ける" );
		Harness.Check( Route.TryGetNextCheckpointIndex( Next ) && Next == 0u
			&& Route.CompletedLapCount() == 0u && !Route.IsComplete(),
			"有効な再設定では新しい先頭へ初期化する" );
	}

	Harness.BeginSuite( "FCheckpointRoute3DProgress / 矛盾した保存値を復元前に拒否する" );

	{
		const FCheckpointRoute3DProgress Empty;
		Harness.Check( !Empty.IsValid(), "件数を持たない空の保存値を拒否する" );

		FCheckpointRoute3DProgress Progress;
		Progress.CheckpointCount = 3u;
		Progress.LapCount = 2u;
		Progress.NextCheckpointIndex = 1u;
		Progress.CompletedLapCount = 1u;
		Harness.Check( Progress.IsValid(),
			"2周中の1周完了と次番号1を有効な途中進行にする" );

		Progress.NextCheckpointIndex = 3u;
		Harness.Check( !Progress.IsValid(), "件数以上の次番号を拒否する" );
		Progress.NextCheckpointIndex = 1u;
		Progress.CompletedLapCount = 2u;
		Harness.Check( !Progress.IsValid(),
			"必要周回数へ達した未完了値を拒否する" );

		Progress.NextCheckpointIndex = 0u;
		Progress.bComplete = true;
		Harness.Check( Progress.IsValid(),
			"必要周回数と一致する完了値を受け付ける" );
		Progress.NextCheckpointIndex = 1u;
		Harness.Check( !Progress.IsValid(),
			"完了後に残った次番号を拒否する" );
		Progress.NextCheckpointIndex = 0u;
		Progress.CompletedLapCount = 1u;
		Harness.Check( !Progress.IsValid(),
			"必要周回数へ届かない完了値を拒否する" );
	}

	Harness.BeginSuite( "FCheckpointRoute3D / 設定識別付きの途中進行を復元する" );

	{
		const FCheckpointRoute3DParams Params =
			FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u );
		FCheckpointRoute3D SourceRoute;
		Harness.Check( SourceRoute.SetParams( Params ),
			"取得元を3件2周へ設定する" );
		FCheckpointRoute3DAdvanceResult Result;
		Harness.Check( SourceRoute.Advance( 0u, Result ) && Result.bAccepted,
			"1周目の先頭を進める" );
		Harness.Check( SourceRoute.Advance( 1u, Result ) && Result.bAccepted,
			"1周目の中央を進める" );
		Harness.Check( SourceRoute.Advance( 2u, Result )
			&& Result.bLapCompletedThisAdvance,
			"1周目を完了する" );
		Harness.Check( SourceRoute.Advance( 0u, Result ) && Result.bAccepted,
			"2周目の先頭まで進める" );

		const FCheckpointRoute3DProgress Saved = SourceRoute.CaptureProgress();
		Harness.Check( Saved.IsValid() && Saved.CheckpointCount == 3u
			&& Saved.LapCount == 2u && Saved.NextCheckpointIndex == 1u
			&& Saved.CompletedLapCount == 1u && !Saved.bComplete,
			"設定識別と途中の次番号を保存値へまとめる" );

		FCheckpointRoute3D RestoredRoute;
		Harness.Check( RestoredRoute.SetParams( Params )
			&& RestoredRoute.RestoreProgress( Saved ),
			"同じ設定の別ルートへ途中進行を復元する" );
		u32 Next = 0u;
		Harness.Check( RestoredRoute.CompletedLapCount() == 1u
			&& RestoredRoute.TryGetNextCheckpointIndex( Next ) && Next == 1u,
			"復元後は2周目の中央から続ける" );
		Harness.Check( RestoredRoute.Advance( 1u, Result ) && Result.bAccepted,
			"復元後の期待番号を受理する" );
		Harness.Check( RestoredRoute.Advance( 2u, Result )
			&& Result.bRouteCompletedThisAdvance,
			"復元後も残りの順番で全体を完了する" );

		const FCheckpointRoute3DProgress Completed =
			RestoredRoute.CaptureProgress();
		Harness.Check( Completed.IsValid() && Completed.bComplete,
			"完了状態も有効な保存値になる" );
		FCheckpointRoute3D CompletedRoute;
		Harness.Check( CompletedRoute.SetParams( Params )
			&& CompletedRoute.RestoreProgress( Completed )
			&& CompletedRoute.IsComplete(),
			"完了済みのルートも再完了させず復元する" );

		FCheckpointRoute3D WrongRoute;
		Harness.Check( WrongRoute.SetParams(
			FCheckpointRoute3DParams::ForCheckpoints( 2u, 2u ) ),
			"不一致確認用の設定を作る" );
		Harness.Check( WrongRoute.Advance( 0u, Result ) && Result.bAccepted,
			"不一致確認用の途中状態を作る" );
		Harness.Check( !WrongRoute.RestoreProgress( Saved ),
			"チェックポイント数が異なる保存値を拒否する" );
		Next = 0u;
		Harness.Check( WrongRoute.CompletedLapCount() == 0u
			&& WrongRoute.TryGetNextCheckpointIndex( Next ) && Next == 1u,
			"復元拒否時は以前の途中進行を保つ" );

		FCheckpointRoute3D WrongLapRoute;
		Harness.Check( WrongLapRoute.SetParams(
			FCheckpointRoute3DParams::ForCheckpoints( 3u, 3u ) ),
			"周回数不一致の確認用設定を作る" );
		Harness.Check( !WrongLapRoute.RestoreProgress( Saved ),
			"必要周回数が異なる保存値も拒否する" );
	}
}

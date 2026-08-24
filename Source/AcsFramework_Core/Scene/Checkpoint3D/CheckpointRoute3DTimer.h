// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DAdvanceResult.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DTimingResult.h"

/**
 * 3Dチェックポイント順序ルートの区間、周回、合計時間を明示時間だけで計測する状態。
 *
 * @details 場面、時計、入力、描画は所有しない。呼出側が渡す経過秒と
 * `FCheckpointRoute3D::Advance`の受理結果だけを使うため、同じ入力列から同じ計測結果を返す。
 */
class FCheckpointRoute3DTimer
{
public:
	/** 全時間を0へ戻し、停止した未完了状態にする。 */
	void Reset() noexcept;

	/**
	 * 未完了の計測を開始または再開する。
	 *
	 * @return 開始または再開できたらtrue。ルート完了後は`Reset`するまでfalse。
	 */
	bool Start() noexcept;

	/** 現在値を保ったまま計測を停止する。 */
	void Pause() noexcept;

	/**
	 * 呼出側が決めた経過秒だけ計測を進める。
	 *
	 * @param DeltaSeconds 有限かつ0以上の経過秒。停止中と完了後は有効値を受けても進めない。
	 * @return 入力と加算後の時間が有効ならtrue。失敗時は全時間を変えない。
	 */
	bool Tick( f64 DeltaSeconds ) noexcept;

	/**
	 * ルートが受理した1地点を区間境界として記録する。
	 *
	 * @param AdvanceResult `FCheckpointRoute3D::Advance`が返した今回の受理結果。
	 * @param OutResult 合計、現在周回、直前区間の時間。失敗時は変更しない。
	 * @return 計測中で、受理済み結果が矛盾なく記録できたらtrue。
	 */
	bool RecordAdvance( const FCheckpointRoute3DAdvanceResult& AdvanceResult,
		FCheckpointRoute3DTimingResult& OutResult ) noexcept;

	/** 計測開始から現在までの合計秒を返す。 */
	f64 TotalElapsedSeconds() const noexcept { return m_TotalElapsedSeconds; }

	/** 現在周回の開始から現在までの秒を返す。 */
	f64 CurrentLapElapsedSeconds() const noexcept { return m_CurrentLapElapsedSeconds; }

	/** 前回受理地点または計測開始から現在までの区間秒を返す。 */
	f64 CurrentSegmentElapsedSeconds() const noexcept { return m_CurrentSegmentElapsedSeconds; }

	/** 計測中ならtrue。 */
	bool IsRunning() const noexcept { return m_bRunning; }

	/** ルート全体の完了を記録済みならtrue。 */
	bool IsComplete() const noexcept { return m_bComplete; }

private:
	/** 受理結果のフラグと周回境界が既存ルートの契約に沿うか返す。 */
	static bool IsAdvanceResultValid_Internal(
		const FCheckpointRoute3DAdvanceResult& Result ) noexcept;

	/** 計測開始からの合計秒。 */
	f64 m_TotalElapsedSeconds = 0.0;

	/** 現在周回の開始からの秒。周回完了時に0へ戻す。 */
	f64 m_CurrentLapElapsedSeconds = 0.0;

	/** 前回受理地点または計測開始からの秒。通過受理時に0へ戻す。 */
	f64 m_CurrentSegmentElapsedSeconds = 0.0;

	/** 有効な時間入力を現在値へ加えるならtrue。 */
	bool m_bRunning = false;

	/** ルート全体の完了を記録済みならtrue。 */
	bool m_bComplete = false;
};

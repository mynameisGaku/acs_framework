// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DAdvanceResult.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DParams.h"

/**
 * 3Dチェックポイントの発火番号を、決めた順番と周回数で受け付ける状態。
 *
 * @details チェックポイント、場面、入力、時間、描画は所有しない。呼出側が
 * `CCheckpoint3D::Update`で発火した番号だけを`Advance`へ渡すため、同じ入力列から
 * 同じ進行結果を返す。
 */
class FCheckpointRoute3D
{
public:
	/** 1チェックポイントを1周すると完了する既定ルートを作る。 */
	FCheckpointRoute3D() noexcept = default;

	/**
	 * 順番に受け付ける件数と必要周回数を置き換え、先頭から開始する。
	 *
	 * @param Params 1周のチェックポイント数と必要周回数。
	 * @return 有効な設定へ置き換えられたらtrue。失敗時は設定と進行を変えない。
	 */
	bool SetParams( const FCheckpointRoute3DParams& Params ) noexcept;

	/** 現在の設定を保ち、次に受け付ける番号、周回数、完了状態を先頭へ戻す。 */
	void Reset() noexcept;

	/**
	 * 発火したチェックポイント番号を1件処理する。
	 *
	 * @details 範囲内だが順番が異なる番号は正常な結果として`bOutOfOrder`を返し、進行しない。
	 * 完了後の番号も正常な無変更結果になる。範囲外の番号だけを失敗として拒否する。
	 * @param CheckpointIndex 0から`CheckpointCount - 1`までの発火番号。
	 * @param OutResult 受理、順番違い、周回完了、次番号の受け取り先。失敗時は変更しない。
	 * @return 設定と番号が有効で処理できたらtrue。
	 */
	bool Advance( u32 CheckpointIndex,
		FCheckpointRoute3DAdvanceResult& OutResult ) noexcept;

	/**
	 * 次に受け付ける番号を返す。
	 *
	 * @param OutCheckpointIndex 番号の受け取り先。完了時は変更しない。
	 * @return 未完了で次番号があるならtrue。
	 */
	bool TryGetNextCheckpointIndex( u32& OutCheckpointIndex ) const noexcept;

	/** 指定番号が現在受け付ける順番と一致するならtrue。 */
	bool IsExpectedCheckpoint( u32 CheckpointIndex ) const noexcept;

	/** 現在のチェックポイント数と必要周回数を返す。 */
	const FCheckpointRoute3DParams& Params() const noexcept { return m_Params; }

	/** 完了済みの周回数を返す。 */
	u32 CompletedLapCount() const noexcept { return m_CompletedLapCount; }

	/** 必要周回数へ到達していればtrue。 */
	bool IsComplete() const noexcept { return m_bComplete; }

private:
	/** 1周の中で次に受け付ける0始まりの番号。完了後は0へ戻したまま使わない。 */
	u32 m_NextCheckpointIndex = 0u;

	/** 末尾チェックポイントまで受理した周回数。 */
	u32 m_CompletedLapCount = 0u;

	/** 必要周回数へ到達していればtrue。 */
	bool m_bComplete = false;

	/** 現在のチェックポイント数と必要周回数。 */
	FCheckpointRoute3DParams m_Params;
};

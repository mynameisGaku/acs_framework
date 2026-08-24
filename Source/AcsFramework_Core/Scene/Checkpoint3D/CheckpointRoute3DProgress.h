// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3Dチェックポイント順序ルートを保存・復元するための進行値。 */
struct FCheckpointRoute3DProgress
{
	/** 取得時に設定されていた1周のチェックポイント数。 */
	u32 CheckpointCount = 0u;

	/** 取得時に設定されていた必要周回数。 */
	u32 LapCount = 0u;

	/** 未完了時に次に受け付ける0始まりの番号。完了時は0。 */
	u32 NextCheckpointIndex = 0u;

	/** 末尾チェックポイントまで受理した周回数。 */
	u32 CompletedLapCount = 0u;

	/** 必要周回数へ到達済みならtrue。 */
	bool bComplete = false;

	/**
	 * 件数、次番号、周回数、完了状態が互いに矛盾しないか返す。
	 *
	 * @return 未完了なら次番号と途中周回が範囲内、完了なら必要周回数と一致していればtrue。
	 */
	bool IsValid() const noexcept;
};

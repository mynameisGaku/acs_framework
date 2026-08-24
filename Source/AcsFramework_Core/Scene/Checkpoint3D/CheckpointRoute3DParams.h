// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 順番に通過する3Dチェックポイント数と周回数の指定。 */
struct FCheckpointRoute3DParams
{
	/** 1周で順番に受け付けるチェックポイント数。 */
	u32 CheckpointCount = 1u;

	/** ルート完了までに必要な周回数。 */
	u32 LapCount = 1u;

	/**
	 * チェックポイント数と周回数だけで順序ルート設定を作る。
	 *
	 * @param InCheckpointCount 1周で順番に受け付ける件数。
	 * @param InLapCount 完了までに必要な周回数。
	 * @return そのまま`FCheckpointRoute3D::SetParams`へ渡せる設定。不正値は`IsValid`で拒否される。
	 */
	static FCheckpointRoute3DParams ForCheckpoints( u32 InCheckpointCount,
		u32 InLapCount = 1u ) noexcept;

	/** チェックポイントと周回がそれぞれ1件以上ならtrue。 */
	bool IsValid() const noexcept;
};

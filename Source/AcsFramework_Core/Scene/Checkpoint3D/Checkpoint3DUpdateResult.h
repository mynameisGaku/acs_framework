// SPDX-License-Identifier: Apache-2.0
#pragma once

/** 3Dチェックポイントを1回更新した後の対象状態と発火事象。 */
struct FCheckpoint3DUpdateResult
{
	/** 今回の進入が設定上の発火条件を満たしたならtrue。 */
	bool bActivatedThisUpdate = false;

	/** 対象形状が今回の更新後に範囲内ならtrue。 */
	bool bTargetInside = false;

	/** 接続または最後の初期化後に1回以上発火していればtrue。 */
	bool bHasActivated = false;
};

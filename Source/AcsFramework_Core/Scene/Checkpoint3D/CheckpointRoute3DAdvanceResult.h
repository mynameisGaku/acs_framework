// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3Dチェックポイントの順序ルートへ1件の発火を渡した結果。 */
struct FCheckpointRoute3DAdvanceResult
{
	/** 次に受け付けるチェックポイント番号。`bHasNextCheckpoint`がfalseなら使わない。 */
	u32 NextCheckpointIndex = 0u;

	/** 今回の処理後に完了している周回数。 */
	u32 CompletedLapCount = 0u;

	/** 渡した番号が期待した順番と一致し、進行したならtrue。 */
	bool bAccepted = false;

	/** 未完了のルートへ期待と異なる番号を渡したならtrue。 */
	bool bOutOfOrder = false;

	/** 今回の受理で1周の最後へ到達したならtrue。 */
	bool bLapCompletedThisAdvance = false;

	/** 今回の受理で必要周回数へ初めて到達したならtrue。 */
	bool bRouteCompletedThisAdvance = false;

	/** 今回の処理後にルート全体が完了済みならtrue。 */
	bool bRouteCompleted = false;

	/** 今回の処理後に受け付ける次のチェックポイントがあるならtrue。 */
	bool bHasNextCheckpoint = false;
};

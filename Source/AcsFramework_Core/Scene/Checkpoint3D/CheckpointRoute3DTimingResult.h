// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 受理された3Dチェックポイント通過時点の区間・周回・合計タイム。 */
struct FCheckpointRoute3DTimingResult
{
	/** 計測開始から今回の通過までの合計秒。 */
	f64 TotalElapsedSeconds = 0.0;

	/** 現在周回の開始から今回の通過までの秒。 */
	f64 LapElapsedSeconds = 0.0;

	/** 前回受理地点または計測開始から今回の通過までの区間秒。 */
	f64 SegmentElapsedSeconds = 0.0;

	/** 今回の通過後に完了している周回数。 */
	u32 CompletedLapCount = 0u;

	/** 今回の通過で1周を完了したならtrue。 */
	bool bLapCompletedThisAdvance = false;

	/** 今回の通過でルート全体を完了したならtrue。 */
	bool bRouteCompletedThisAdvance = false;
};

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Simulation/ActionInput.h"

/** 押下猶予バッファを途中状態から再開するための保存値。 */
struct FActionInputBufferState
{
	/** 今後の押下を保持する猶予秒。 */
	f32 WindowSeconds = 0.12f;

	/** アクションごとの残り保持秒。 */
	f64 RemainingSeconds[kActionButtonCount] = {};

	/** 各アクションを保持した時点の猶予秒。失効境界の精度を再現する。 */
	f32 CapturedWindowSeconds[kActionButtonCount] = {};

	/** 全秒数が有限で、保持中と空の状態が矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};

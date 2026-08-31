// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 自動回復する整数チャージを同じ時点から再開するための保存値。 */
struct FGameplayChargePoolState
{
	/** 1以上のチャージ上限。 */
	u32 MaximumCharges = 1u;

	/** 0以上MaximumCharges以下の現在チャージ数。 */
	u32 CurrentCharges = 1u;

	/** 今後の回復開始時に使う1チャージ当たりの秒数。 */
	f32 RechargeSeconds = 1.0f;

	/** 現在進行中の1チャージ回復を始めた時点で固定した秒数。 */
	f32 ActiveRechargeSeconds = 1.0f;

	/** 次の回復判定へ持ち越している秒数。未処理の複数回ぶんを含められる。 */
	f64 AccumulatedSeconds = 0.0;

	/** チャージ不足中の自動回復を一時停止しているならtrue。 */
	bool bIsPaused = false;

	/** 上限、現在数、秒数と停止状態が有限かつ矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};

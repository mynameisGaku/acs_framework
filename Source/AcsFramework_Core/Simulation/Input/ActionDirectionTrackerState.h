// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/Input/ActionDirection2D.h"
#include "AcsFramework_Core/Simulation/Input/ActionDirectionQuantizer.h"

using namespace acs;

/** 離散方向の設定、現在値、今回の変化を同じ更新から再開するための保存値。 */
struct FActionDirectionTrackerState
{
	/** 2軸を離散方向へ変換する設定。 */
	FActionDirectionQuantizer Quantizer;

	/** 最後に成功した更新で決まった現在方向。 */
	EActionDirection2D Direction = EActionDirection2D::None;

	/** 最後に成功した更新より前の方向。今回だけの変化判定に使う。 */
	EActionDirection2D PreviousDirection = EActionDirection2D::None;

	/** 量子化設定と現在・前回方向が公開値として有効ならtrue。 */
	bool IsValid() const noexcept;
};

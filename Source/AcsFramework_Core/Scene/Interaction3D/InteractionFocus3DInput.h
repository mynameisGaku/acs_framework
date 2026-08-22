// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs::game;

/** 1回の3D視線フォーカス判定から状態遷移へ渡す明示入力。 */
struct FInteractionFocus3DInput
{
	/** 今回の視線が捉えた登録対象。何も無ければ無効。 */
	FNodeId CandidateNode;

	/** 今回の対象へ決定操作を要求するならtrue。 */
	bool bActivateRequested = false;
};

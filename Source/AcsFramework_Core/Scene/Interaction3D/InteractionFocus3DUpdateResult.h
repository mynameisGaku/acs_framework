// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs::game;

/** 3D視線フォーカスの次状態と、この更新で生じたイベント。 */
struct FInteractionFocus3DUpdateResult
{
	/** 更新前に捉えていた対象。 */
	FNodeId PreviousNode;

	/** 更新後に捉えている対象。 */
	FNodeId FocusedNode;

	/** 決定操作が成立した対象。成立しなければ無効。 */
	FNodeId ActivatedNode;

	/** 対象が切り替わったか、対象ありとなしの間を移ったらtrue。 */
	bool FocusChanged() const noexcept { return PreviousNode != FocusedNode; }

	/** 新しい対象へ入った更新ならtrue。対象切替時もtrue。 */
	bool FocusEntered() const noexcept { return FocusedNode.IsValid() && FocusChanged(); }

	/** 以前の対象から離れた更新ならtrue。対象切替時もtrue。 */
	bool FocusLeft() const noexcept { return PreviousNode.IsValid() && FocusChanged(); }

	/** 有効な対象への決定操作が成立したらtrue。 */
	bool Activated() const noexcept { return ActivatedNode.IsValid(); }
};

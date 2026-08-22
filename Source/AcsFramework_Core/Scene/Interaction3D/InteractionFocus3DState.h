// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs::game;

/** 3D視線フォーカスが次回まで保持する決定論的な状態。 */
struct FInteractionFocus3DState
{
	/** 前回更新で視線が捉えていた登録対象。 */
	FNodeId FocusedNode;
};

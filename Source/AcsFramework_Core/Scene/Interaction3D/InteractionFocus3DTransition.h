// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DInput.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DState.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DUpdateResult.h"

/**
 * 現状態と今回入力だけから、次の3D視線フォーカスとイベントを計算する。
 *
 * @param State 更新前の値状態。
 * @param Input 今回の候補と決定要求。
 * @return 次の対象、対象出入り、成立した決定対象。外部状態は変更しない。
 */
FInteractionFocus3DUpdateResult AdvanceInteractionFocus3D( const FInteractionFocus3DState& State, const FInteractionFocus3DInput& Input ) noexcept;

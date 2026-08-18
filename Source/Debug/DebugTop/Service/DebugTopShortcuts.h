// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class ADebugTopHUD;

/**
 * 押されたキーに割り当てられた行を、メニュー全体から探して実行する。
 *
 * @details
 * ページを跨いで探すので、どのページを開いていても効く。行まで辿らずに実行したい操作
 * (再読み込み・表示の切替など) のためのもの。
 *
 * 割り当ては行の側が持つ (CDebugTopElement::SetShortcut)。ここは押されたキーと突き合わせて
 * OnDecide を呼ぶだけで、どの行に何が割り当たっているかは知らない。
 *
 * @param HUD 探す対象のメニュー。
 * @return 1 つでも実行したら true (その入力を他へ流さないため)。
 */
bool DebugTopRunShortcuts( ADebugTopHUD& HUD );

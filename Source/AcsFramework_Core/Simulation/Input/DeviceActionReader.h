// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/Input/IActionDeviceReader.h"

using namespace acs;

/**
 * 実機の装置を読む実装。
 *
 * @details
 * `acs::CInput` を呼ぶのは**このクラスだけ**にする。ここを 1 か所に閉じておけば、
 * 割り当て表も規則も «装置がある世界» を知らずに済み、そのままテストへ持って行ける。
 *
 * 状態を持たない。作る場所も寿命も気にしなくてよい。
 */
class CDeviceActionReader final : public IActionDeviceReader
{
public:
	/** キーが押されているかを返す。 */
	bool IsKeyDown( EKey Key ) const noexcept override;

	/** パッドのボタンが押されているかを返す。 */
	bool IsGamepadButtonDown( u32 PlayerIndex, EGamepadButton Button ) const noexcept override;

	/** パッドの軸の値を返す。 */
	f32 GetGamepadAxis( u32 PlayerIndex, EGamepadAxis Axis ) const noexcept override;
};

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

	/**
	 * 今回新しく押されたゲームパッドボタンを列挙順で1つ返す。
	 *
	 * @param PlayerIndex 何番目のパッドか。
	 * @param OutButton 見つかった実ボタン。見つからない場合は変更しない。
	 * @return 押下開始のボタンが見つかればtrue。
	 */
	bool TryReadPressedGamepadButton( u32 PlayerIndex, EGamepadButton& OutButton ) const noexcept;

	/**
	 * 指定しきい値以上に動いているゲームパッド軸のうち、絶対値が最大のものを返す。
	 *
	 * @param PlayerIndex 何番目のパッドか。
	 * @param MinimumMagnitude 軸を選んだとみなす最小絶対値。0より大きく1以下。
	 * @param OutAxis 見つかった実軸。見つからない場合は変更しない。
	 * @return 条件を満たす軸が見つかればtrue。
	 */
	bool TryReadActiveGamepadAxis( u32 PlayerIndex, f32 MinimumMagnitude, EGamepadAxis& OutAxis ) const noexcept;

	/**
	 * すべてのゲームパッド軸が指定範囲内の中立位置ならtrueを返す。
	 *
	 * @param PlayerIndex 何番目のパッドか。
	 * @param MaximumMagnitude 中立とみなす最大絶対値。0以上1以下。
	 * @return 全軸が有限値かつ指定範囲内ならtrue。
	 */
	bool AreGamepadAxesCentered( u32 PlayerIndex, f32 MaximumMagnitude ) const noexcept;
};

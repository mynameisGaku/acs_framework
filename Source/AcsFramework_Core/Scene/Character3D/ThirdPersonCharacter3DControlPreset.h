// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DActionSet.h"
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"

/**
 * 第三者視点操作で使いやすいキーボードとゲームパッドの既定割り当て。
 *
 * @details キーボードはWASD移動、矢印視点、Spaceジャンプ、E/Qズームを使う。
 * ゲームパッドは左右スティック、下側ボタン、左右バンパーを同じ操作へ重ねる。
 */
struct FThirdPersonCharacter3DControlPreset
{
	/** 読み取るゲームパッド番号。0から3を指定する。 */
	u32 GamepadPlayerIndex = 0u;

	/** スティック入力を0として扱う中心付近の幅。 */
	f32 GamepadDeadZone = 0.15f;

	/**
	 * ゲームパッド番号と中心付近の幅が利用可能か返す。
	 *
	 * @return 番号が0から3で、幅が有限かつ0から1ならtrue。
	 */
	bool IsValid() const noexcept;

	/**
	 * キーボードとゲームパッドの既定操作を持つ新しい割り当て表を作る。
	 *
	 * @details 成功時はOutBindingsの既存内容を置き換える。設定不正または領域確保失敗時は
	 * OutBindingsを変更しない。
	 * @param OutBindings 成功時に既定操作を受け取る割り当て表。
	 * @param Actions 各操作を格納する軸番号とアクション番号。
	 * @return 全ての割り当てを作れたらtrue。
	 */
	bool TryBuildBindings( CActionBindingTable& OutBindings, const FThirdPersonCharacter3DActionSet& Actions = FThirdPersonCharacter3DActionSet{} ) const noexcept;
};

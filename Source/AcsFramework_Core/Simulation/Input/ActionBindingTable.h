// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionBinding.h"
#include "AcsFramework_Core/Simulation/Input/IActionDeviceReader.h"

using namespace acs;

/**
 * 「どの操作が、どのアクションになるか」の対応表。
 *
 * @details
 * 装置の状態を 1 つの `FActionInput` にまとめる。**読む相手は引数で渡す**ので、実機では
 * 本物の装置を、テストでは偽の装置を差せる。キーやパッドが 1 つも無い場所でも
 * 「この組み合わせを押したらこうなる」を確かめられる。
 *
 * ACS にも `acs::game::FInputMap` があるが、あちらは内部で `acs::CInput` を直接 poll する
 * 作りで、差し替えができない。またシーンのサービス (`ESvc::Input`) として生えるため、
 * シーンを起動しないと触れない。記録・再生のために欲しいのは「差し替えできて、
 * 1 ティックぶんの値として取り出せる」形なので、そこだけを別に持つ。
 *
 * @code
 * Table.BindKey( kActionFire, EKey::Space );
 * Table.BindGamepadButton( kActionFire, EGamepadButton::A );
 * Table.BindAxisKeys( 0u, EKey::A, EKey::D );
 *
 * const FActionInput Input = Table.Resolve( Reader );
 * @endcode
 */
class CActionBindingTable
{
public:
	/**
	 * キーをアクションへ割り当てる。
	 *
	 * @param ActionIndex アクション番号。
	 * @param Key 割り当てるキー。
	 * @return 足せたら true。
	 */
	bool BindKey( u32 ActionIndex, EKey Key ) noexcept;

	/**
	 * アクションのキーボード割り当てを1つへ置き換える。
	 *
	 * @details 同じアクションに複数のキーがあれば1つへまとめる。ゲームパッドの割り当てと
	 * 他のアクションは維持する。新規追加時に領域を確保できなければ、表は変えない。
	 * @param ActionIndex アクション番号。
	 * @param Key 新しく割り当てるキー。
	 * @return 置き換えられたらtrue。
	 */
	bool ReplaceKeyBinding( u32 ActionIndex, EKey Key ) noexcept;

	/**
	 * アクションへ最初に割り当てられたキーボードのキーを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @param OutKey 見つかったキー。見つからない場合は変更しない。
	 * @return キーボード割り当てが見つかればtrue。
	 */
	bool TryGetKeyBinding( u32 ActionIndex, EKey& OutKey ) const noexcept;

	/**
	 * パッドのボタンをアクションへ割り当てる。
	 *
	 * @param ActionIndex アクション番号。
	 * @param Button 割り当てるボタン。
	 * @param PlayerIndex 何番目のパッドか。
	 * @return 足せたら true。
	 */
	bool BindGamepadButton( u32 ActionIndex, EGamepadButton Button, u32 PlayerIndex = 0u ) noexcept;

	/**
	 * キー 2 つで軸を作る。
	 *
	 * @details 両方押されていれば 0 になる (打ち消し合う)。
	 * @param AxisIndex 軸番号。
	 * @param NegativeKey -1 側のキー。
	 * @param PositiveKey +1 側のキー。
	 * @return 足せたら true。
	 */
	bool BindAxisKeys( u32 AxisIndex, EKey NegativeKey, EKey PositiveKey ) noexcept;

	/**
	 * パッドの軸を割り当てる。
	 *
	 * @param AxisIndex 軸番号。
	 * @param Axis 割り当てるパッドの軸。
	 * @param PlayerIndex 何番目のパッドか。
	 * @param DeadZone これを下回る入力を 0 にする。
	 * @param Scale 得られた値へ掛ける倍率。
	 * @return 足せたら true。
	 */
	bool BindGamepadAxis( u32 AxisIndex, EGamepadAxis Axis, u32 PlayerIndex = 0u, f32 DeadZone = 0.15f, f32 Scale = 1.0f ) noexcept;

	/**
	 * 装置を読んで 1 ティックぶんの入力を作る。
	 *
	 * @details
	 * ボタンは「どれか 1 つでも押されていれば押されている」、軸は「絶対値の大きいほうが残る」。
	 * @param Reader 読む相手。
	 * @return まとめた入力。
	 */
	FActionInput Resolve( const IActionDeviceReader& Reader ) const noexcept;

	/** 割り当ての数 (ボタン) を返す。 */
	usize GetButtonBindingCount() const noexcept { return m_ButtonBindings.Num(); }

	/** 割り当ての数 (軸) を返す。 */
	usize GetAxisBindingCount() const noexcept { return m_AxisBindings.Num(); }

	/** 全ての割り当てを捨てる。 */
	void Clear() noexcept;

private:
	/** ボタンの割り当て。 */
	TArray<FActionButtonBinding> m_ButtonBindings;

	/** 軸の割り当て。 */
	TArray<FActionAxisBinding> m_AxisBindings;
};

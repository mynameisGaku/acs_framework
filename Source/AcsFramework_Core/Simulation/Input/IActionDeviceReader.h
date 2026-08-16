// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 装置の «いまの状態» を答える口。
 *
 * @details
 * 割り当て表 (CActionBindingTable) がキーやパッドを直接読むと、装置が無い場所では
 * 一切試せなくなる。**読む相手を差し替えられる**ようにしておくと、テストでは偽の装置を
 * 差して「この組み合わせを押したらこの入力になる」を机上で確かめられる。
 *
 * 実機では CDeviceActionReader が `acs::CInput` を読む。
 *
 * ここで «押された/離された» は扱わない。差分は前ティックとの突き合わせで決まるので、
 * FSimulationContext が答える。ここが答えるのは「いま押されているか」だけ。
 */
class IActionDeviceReader
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IActionDeviceReader() noexcept = default;

	/**
	 * キーが押されているかを返す。
	 *
	 * @param Key 調べるキー。
	 * @return 押されていれば true。
	 */
	virtual bool IsKeyDown( EKey Key ) const noexcept = 0;

	/**
	 * パッドのボタンが押されているかを返す。
	 *
	 * @param PlayerIndex 何番目のパッドか。
	 * @param Button 調べるボタン。
	 * @return 押されていれば true。
	 */
	virtual bool IsGamepadButtonDown( u32 PlayerIndex, EGamepadButton Button ) const noexcept = 0;

	/**
	 * パッドの軸の値を返す。
	 *
	 * @param PlayerIndex 何番目のパッドか。
	 * @param Axis 調べる軸。
	 * @return 軸の値 (-1..1)。
	 */
	virtual f32 GetGamepadAxis( u32 PlayerIndex, EGamepadAxis Axis ) const noexcept = 0;
};

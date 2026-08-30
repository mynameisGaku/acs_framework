// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"

using namespace acs;

/**
 * 通常の場面更新で、現在と前フレームのアクション入力をまとめて扱う。
 *
 * @details
 * `Update()`を1フレームに1回呼ぶと、押下中、押した瞬間、離した瞬間、軸値を同じ場所から
 * 取得できる。実機以外の`IActionDeviceReader`や完成済みの`FActionInput`も渡せるため、
 * AI、リプレイ、単体テストでも同じ判定を使える。
 *
 * 固定ステップのゲーム規則では、描画フレームとティックの境界が異なるため、
 * `FSimulationContext`の入力判定を使う。
 *
 * @code
 * CActionInputTracker Input;
 * Input.GetBindings().BindKey( kActionJump, EKey::Space );
 *
 * // 毎フレーム
 * Input.Update();
 * if ( Input.WasPressed( kActionJump ) ) Jump();
 * @endcode
 */
class CActionInputTracker
{
public:
	/** キーとゲームパッドの割り当て表を返す。 */
	CActionBindingTable& GetBindings() noexcept { return m_Bindings; }

	/** キーとゲームパッドの割り当て表をconstで返す。 */
	const CActionBindingTable& GetBindings() const noexcept { return m_Bindings; }

	/**
	 * 実機のキーとゲームパッドを読み、1フレーム進める。
	 *
	 * @details 場面の`OnUpdate`などから1フレームに1回だけ呼ぶ。
	 */
	void Update() noexcept;

	/**
	 * 指定した装置状態を割り当て表で変換し、1フレーム進める。
	 *
	 * @param Reader 読む装置。テスト用の偽装置も渡せる。
	 */
	void Update( const IActionDeviceReader& Reader ) noexcept;

	/**
	 * 完成済みのアクション入力で1フレーム進める。
	 *
	 * @details AI、リプレイ、ネットワーク入力など、実機操作以外も同じ履歴判定へ渡せる。
	 * @param Input このフレームの入力。
	 */
	void Update( const FActionInput& Input ) noexcept;

	/** 現在と前フレームの入力を中立へ戻す。割り当て表は維持する。 */
	void Reset() noexcept;

	/** 現在のアクション入力を返す。 */
	const FActionInput& GetCurrentInput() const noexcept { return m_CurrentInput; }

	/** 前フレームのアクション入力を返す。 */
	const FActionInput& GetPreviousInput() const noexcept { return m_PreviousInput; }

	/**
	 * 現在押されているかを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 現在押されていればtrue。範囲外ならfalse。
	 */
	bool IsDown( u32 ActionIndex ) const noexcept;

	/**
	 * このフレームで押されたかを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 前は押されておらず、現在押されていればtrue。
	 */
	bool WasPressed( u32 ActionIndex ) const noexcept;

	/**
	 * このフレームで離されたかを返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 前は押されており、現在押されていなければtrue。
	 */
	bool WasReleased( u32 ActionIndex ) const noexcept;

	/**
	 * 現在の軸値を返す。
	 *
	 * @param AxisIndex 軸番号。
	 * @return 現在の軸値。範囲外なら0。
	 */
	f32 GetAxis( u32 AxisIndex ) const noexcept;

	/**
	 * 前フレームの軸値を返す。
	 *
	 * @param AxisIndex 軸番号。
	 * @return 前フレームの軸値。範囲外なら0。
	 */
	f32 GetPreviousAxis( u32 AxisIndex ) const noexcept;

private:
	/** 現在値を前回へ送り、指定入力を現在値にする。 */
	void ApplyInput_Internal( const FActionInput& Input ) noexcept;

	/** キーとゲームパッドからアクションを作る割り当て表。 */
	CActionBindingTable m_Bindings;

	/** このフレームのアクション入力。 */
	FActionInput m_CurrentInput;

	/** 1フレーム前のアクション入力。 */
	FActionInput m_PreviousInput;
};

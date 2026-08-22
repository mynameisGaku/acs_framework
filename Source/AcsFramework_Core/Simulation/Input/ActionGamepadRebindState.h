// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * ゲームパッド割り当ての入力待ちと確定値を持つ状態。
 *
 * @details UI、実機入力、設定保存、割り当て表には依存しない。呼び出し側が新しく押された
 * ボタンまたは選ばれた軸を1件渡し、適用結果を受けて表示と保存を行う。同じ入力列なら
 * 常に同じ結果になる。
 */
class FActionGamepadRebindState
{
public:
	/** 現在待っている操作の種類。 */
	enum class ECaptureKind : u8
	{
		/** 入力待ちではない。 */
		None,

		/** 次に押されたボタンを待つ。 */
		Button,

		/** 次に大きく動かされた軸を待つ。 */
		Axis,
	};

	/** 明示入力を処理した結果。 */
	enum class EResult : u8
	{
		/** 入力待ちではないため何も変えなかった。 */
		Ignored,

		/** 新しいボタンまたは軸を確定した。 */
		Applied,

		/** 待っている種類と違う入力または無効値を拒否した。 */
		Rejected,
	};

	/** ボタンと軸が未設定の状態を作る。 */
	FActionGamepadRebindState() noexcept = default;

	/**
	 * 現在のボタン割り当てを設定し、進行中の入力待ちを終える。
	 *
	 * @param Button 現在の実ボタン。
	 * @return 実ボタンを設定できたらtrue。失敗時は状態を変えない。
	 */
	bool SetCurrentButton( EGamepadButton Button ) noexcept;

	/**
	 * 現在の軸割り当てを設定し、進行中の入力待ちを終える。
	 *
	 * @param Axis 現在の実軸。
	 * @return 実軸を設定できたらtrue。失敗時は状態を変えない。
	 */
	bool SetCurrentAxis( EGamepadAxis Axis ) noexcept;

	/**
	 * 次に押されたゲームパッドボタンを待つ。
	 *
	 * @return 現在ボタンが設定済みで待機を開始できたらtrue。
	 */
	bool BeginButtonCapture() noexcept;

	/**
	 * 次に大きく動かされたゲームパッド軸を待つ。
	 *
	 * @return 現在軸が設定済みで待機を開始できたらtrue。
	 */
	bool BeginAxisCapture() noexcept;

	/**
	 * 軸待機の開始後に、すべての軸が中立へ戻ったことを通知する。
	 *
	 * @return 軸入力を受け付ける状態へ進めたらtrue。
	 */
	bool ConfirmAxesCentered() noexcept;

	/**
	 * 押下開始のゲームパッドボタンを1件処理する。
	 *
	 * @param Button 新しく押されたボタン。
	 * @return 状態がどう変わったか。
	 */
	EResult HandlePressedButton( EGamepadButton Button ) noexcept;

	/**
	 * 入力しきい値を越えたゲームパッド軸を1件処理する。
	 *
	 * @param Axis 大きく動かされた軸。
	 * @return 状態がどう変わったか。
	 */
	EResult HandleActiveAxis( EGamepadAxis Axis ) noexcept;

	/**
	 * 入力待ちを明示的に取り消す。
	 *
	 * @return 待機中の状態を取り消したらtrue。
	 */
	bool CancelCapture() noexcept;

	/** 現在のボタンを返す。未設定なら_Count。 */
	EGamepadButton CurrentButton() const noexcept { return m_CurrentButton; }

	/** 現在の軸を返す。未設定なら_Count。 */
	EGamepadAxis CurrentAxis() const noexcept { return m_CurrentAxis; }

	/** 現在待っている操作の種類を返す。 */
	ECaptureKind CaptureKind() const noexcept { return m_CaptureKind; }

	/** 入力待ちならtrueを返す。 */
	bool IsCapturing() const noexcept { return m_CaptureKind != ECaptureKind::None; }

	/** 中立確認を終え、次の軸入力を受け付けられるならtrueを返す。 */
	bool IsAxisCaptureReady() const noexcept { return m_CaptureKind == ECaptureKind::Axis && !m_bAxisCenterRequired; }

	/** 指定値が実際に割り当てられるゲームパッドボタンならtrueを返す。 */
	static bool IsValidButton( EGamepadButton Button ) noexcept;

	/** 指定値が実際に割り当てられるゲームパッド軸ならtrueを返す。 */
	static bool IsValidAxis( EGamepadAxis Axis ) noexcept;

private:
	/** 現在確定しているボタン。 */
	EGamepadButton m_CurrentButton = EGamepadButton::_Count;

	/** 現在確定している軸。 */
	EGamepadAxis m_CurrentAxis = EGamepadAxis::_Count;

	/** 現在待っている操作の種類。 */
	ECaptureKind m_CaptureKind = ECaptureKind::None;

	/** 待機開始前から倒れていた軸を採用しないため、中立確認が必要ならtrue。 */
	bool m_bAxisCenterRequired = false;
};

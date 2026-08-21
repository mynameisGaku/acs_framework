// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * キーボード割り当ての入力待ちと確定結果だけを持つ状態。
 *
 * @details UI、実機入力、設定保存、割り当て表には依存しない。呼び出し側が押されたキーを
 * 1件ずつ渡し、適用結果を受けて表示と保存を行う。同じ入力列なら常に同じ結果になる。
 */
class FActionKeyRebindState
{
public:
	/** 押されたキーを処理した結果。 */
	enum class EResult : u8
	{
		/** 入力待ちではないため何も変えなかった。 */
		Ignored,

		/** 新しいキーを確定した。 */
		Applied,

		/** 取消キーを受け、現在の割り当てを維持した。 */
		Cancelled,

		/** 実キーではない値を拒否し、入力待ちを継続した。 */
		Rejected,
	};

	/** 未設定の状態を作る。SetCurrentKeyで現在値を与えるまで入力待ちは開始できない。 */
	FActionKeyRebindState() noexcept = default;

	/**
	 * 現在の割り当てを設定し、進行中の入力待ちを終える。
	 *
	 * @param Key 現在の実キー。
	 * @return 実キーを設定できたらtrue。失敗時は状態を変えない。
	 */
	bool SetCurrentKey( EKey Key ) noexcept;

	/**
	 * 次に押された実キーを待つ。
	 *
	 * @details CancelKeyをUnknownにすると取消キーを使わず、Escapeも割り当てられる。
	 * @param CancelKey 現在値を維持して入力待ちを終えるキー。
	 * @return 入力待ちを開始できたらtrue。未設定または既に待機中ならfalse。
	 */
	bool BeginCapture( EKey CancelKey = EKey::Escape ) noexcept;

	/**
	 * 押下開始のキーを1件処理する。
	 *
	 * @details OSのキーリピートではなく、押下開始だけを渡す。無効値を受けても待機を続ける。
	 * @param Key 押されたキー。
	 * @return 状態がどう変わったか。
	 */
	EResult HandlePressedKey( EKey Key ) noexcept;

	/**
	 * 入力待ちを明示的に取り消す。
	 *
	 * @return 待機中の状態を取り消したらtrue。
	 */
	bool CancelCapture() noexcept;

	/** 現在の割り当てを返す。未設定ならUnknown。 */
	EKey CurrentKey() const noexcept { return m_CurrentKey; }

	/** 次のキーを待っているかを返す。 */
	bool IsCapturing() const noexcept { return m_bCapturing; }

	/**
	 * 値が実際に割り当てられるキーかを返す。
	 *
	 * @param Key 検査する値。
	 * @return Unknownと_Countを除く実キーならtrue。
	 */
	static bool IsValidKey( EKey Key ) noexcept;

private:
	/** 現在確定しているキー。 */
	EKey m_CurrentKey = EKey::Unknown;

	/** 入力待ちを取り消すキー。Unknownなら取消キーを使わない。 */
	EKey m_CancelKey = EKey::Unknown;

	/** 次に押されたキーを待っているか。 */
	bool m_bCapturing = false;
};

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputBufferState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 押した瞬間を短時間だけ保持し、ゲーム側が受け付けられる時に1回だけ消費する。
 *
 * @details
 * 着地直前のジャンプ、硬直終了直前の回避、対象へ届く直前の決定操作など、入力した瞬間と
 * 実行できる瞬間が少しずれた場合の取りこぼしを防ぐ。入力装置やゲーム規則は所有せず、
 * 場面や操作対象がfieldとして持つローカル状態である。
 *
 * `Update()`は押下開始だけを保持するため、長押しで同じ操作が再装填されることはない。
 * 新しい押下にはそのフレームの経過時間を差し引かず、設定した猶予を丸ごと与える。
 *
 * @code
 * CActionInputTracker Input;
 * FActionInputBuffer InputBuffer{ 0.12f };
 *
 * // 毎フレーム
 * Input.Update();
 * InputBuffer.Update( Input, DeltaSeconds );
 * if ( CanJump() && InputBuffer.Consume( kActionJump ) ) Jump();
 * @endcode
 */
class FActionInputBuffer
{
public:
	/** 0.12秒の猶予で構築する。 */
	FActionInputBuffer() noexcept = default;

	/**
	 * 猶予を指定して構築する。
	 *
	 * @details 有限の正数でなければ既定の0.12秒を使う。
	 * @param WindowSeconds 押下を保持する秒数。
	 */
	explicit FActionInputBuffer( f32 WindowSeconds ) noexcept;

	/**
	 * 今後の押下を保持する猶予を変更する。
	 *
	 * @details 既に保持している操作の残り時間は変えない。
	 * @param WindowSeconds 有限かつ0より大きい秒数。
	 * @return 設定を反映できたらtrue。失敗時は従来値を保つ。
	 */
	bool SetWindowSeconds( f32 WindowSeconds ) noexcept;

	/** 押下を保持する猶予を秒で返す。 */
	f32 GetWindowSeconds() const noexcept { return m_WindowSeconds; }

	/**
	 * 通常フレームの入力履歴から新しい押下を取り込み、残り時間を進める。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param DeltaSeconds 前回更新からの有限かつ0以上の経過秒。
	 * @return 更新できたらtrue。不正な時間なら状態を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input, f32 DeltaSeconds ) noexcept;

	/**
	 * 明示した現在と前回の入力から新しい押下を取り込み、残り時間を進める。
	 *
	 * @details AI、入力再生、単体テストなど、装置を使わず入力履歴を渡す場合に使う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param DeltaSeconds 前回更新からの有限かつ0以上の経過秒。
	 * @return 更新できたらtrue。不正な時間なら状態を変えずfalse。
	 */
	bool Update( const FActionInput& CurrentInput, const FActionInput& PreviousInput, f32 DeltaSeconds ) noexcept;

	/**
	 * 指定操作を現在の猶予で明示的に保持する。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 範囲内ならtrue。範囲外なら状態を変えずfalse。
	 */
	bool BufferAction( u32 ActionIndex ) noexcept;

	/**
	 * 指定操作をまだ消費できるか返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 猶予が残っていればtrue。範囲外ならfalse。
	 */
	bool IsBuffered( u32 ActionIndex ) const noexcept;

	/**
	 * 指定操作を1回だけ受け取り、保持状態を空にする。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 猶予内の操作を受け取れたらtrue。未保持または範囲外ならfalse。
	 */
	bool Consume( u32 ActionIndex ) noexcept;

	/**
	 * 指定操作だけを破棄する。
	 *
	 * @param ActionIndex アクション番号。範囲外なら何もしない。
	 */
	void Clear( u32 ActionIndex ) noexcept;

	/** 全操作の保持状態を空にする。猶予設定は維持する。 */
	void Reset() noexcept;

	/** 現在の猶予設定と全操作の残り時間を保存可能な値として返す。 */
	FActionInputBufferState CaptureState() const noexcept;

	/**
	 * 保存した猶予設定と全操作の残り時間を復元する。
	 *
	 * @param State `CaptureState`で取得した有限かつ矛盾のない状態。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FActionInputBufferState& State ) noexcept;

	/**
	 * 指定操作を保持できる残り秒数を返す。
	 *
	 * @param ActionIndex アクション番号。
	 * @return 残り秒数。未保持または範囲外なら0。
	 */
	f32 GetRemainingSeconds( u32 ActionIndex ) const noexcept;

private:
	/** 有効な経過秒だけ、保持中の全操作から差し引く。 */
	void Advance_Internal( f32 DeltaSeconds ) noexcept;

	/** 現在だけ押されている操作を、設定済みの猶予で保持する。 */
	void CapturePressed_Internal( const FActionInput& CurrentInput, const FActionInput& PreviousInput ) noexcept;

	/** 新しい押下を保持する秒数。 */
	f32 m_WindowSeconds = 0.12f;

	/** アクションごとの残り保持秒数。 */
	f64 m_RemainingSeconds[kActionButtonCount] = {};

	/** アクションを保持した時点の猶予。失効判定のf32精度基準に使う。 */
	f32 m_CapturedWindowSeconds[kActionButtonCount] = {};
};

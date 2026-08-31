// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionTapSequenceTrackerState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 1つのアクションを押した間隔から、ダブルタップや複数回タップを判定する局所状態。
 *
 * @details
 * 回避、走行切り替え、特殊入力などに使う。押し続けは1回だけ数え、必要回数へ届いた更新だけ
 * `WasCompleted()`がtrueになる。入力装置や時計は所有せず、通常フレームまたは固定ステップの
 * 入力履歴と明示経過秒だけで進む。
 */
class FActionTapSequenceTracker
{
public:
	/** 0.25秒以内の2回押下を完了条件として構築する。 */
	FActionTapSequenceTracker() noexcept = default;

	/**
	 * 必要回数と押下間隔を指定して構築する。
	 *
	 * @details 2未満の回数または有限の正数でない秒数なら、既定設定を使う。
	 * @param RequiredTapCount 完了に必要な押下回数。
	 * @param MaximumIntervalSeconds 連続する押下間に許す最大秒数。
	 */
	FActionTapSequenceTracker(
		u32 RequiredTapCount, f32 MaximumIntervalSeconds ) noexcept;

	/**
	 * 今後開始するタップ列の必要回数と最大間隔を変更する。
	 *
	 * @details 既に待機中のタップ列は、最初の押下時点の設定を使い続ける。
	 * @param RequiredTapCount 2以上の完了回数。
	 * @param MaximumIntervalSeconds 有限かつ0より大きい秒数。
	 * @return 両方を反映できたらtrue。不正値では従来設定を保ちfalse。
	 */
	bool Configure( u32 RequiredTapCount, f32 MaximumIntervalSeconds ) noexcept;

	/** 今後開始するタップ列の必要回数を返す。 */
	u32 GetRequiredTapCount() const noexcept { return m_RequiredTapCount; }

	/** 今後開始するタップ列で許す最大間隔を返す。 */
	f32 GetMaximumIntervalSeconds() const noexcept { return m_MaximumIntervalSeconds; }

	/**
	 * 通常フレームの入力履歴から指定アクションを進める。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param ActionIndex 追跡するアクション番号。
	 * @param DeltaSeconds 前回の入力標本から進んだ有限かつ0以上の秒数。
	 * @return 更新できたらtrue。範囲外または待機中と異なる番号、不正時間では全状態を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input,
		u32 ActionIndex, f32 DeltaSeconds ) noexcept;

	/**
	 * 明示した現在と前回の入力から指定アクションを進める。
	 *
	 * @details AI、入力再生、固定ステップ、単体テストなど、装置を使わない経路で使う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param ActionIndex 追跡するアクション番号。
	 * @param DeltaSeconds 前回の入力標本から進んだ有限かつ0以上の秒数。
	 * @return 更新できたらtrue。範囲外または待機中と異なる番号、不正時間では全状態を変えずfalse。
	 */
	bool Update( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput,
		u32 ActionIndex, f32 DeltaSeconds ) noexcept;

	/** 途中のタップ列と今回だけの完了結果を空にする。設定は維持する。 */
	void Reset() noexcept;

	/** 設定、途中回数、経過秒、追跡番号と今回結果を保存可能な値として返す。 */
	FActionTapSequenceTrackerState CaptureState() const noexcept;

	/**
	 * 保存した複数回タップ追跡状態を復元する。
	 *
	 * @param State `CaptureState`で取得した有限かつ矛盾のない状態。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FActionTapSequenceTrackerState& State ) noexcept;

	/** 次の押下を待っている途中のタップ列があればtrue。 */
	bool IsWaitingForNextTap() const noexcept { return m_TapCount > 0u; }

	/** 現在の途中タップ回数を返す。待機していなければ0。 */
	u32 GetTapCount() const noexcept { return m_TapCount; }

	/** 直前の押下から次の押下を受理できる残り秒を返す。待機していなければ0。 */
	f32 GetRemainingSeconds() const noexcept;

	/** 今回の更新で必要回数へ到達したならtrue。 */
	bool WasCompleted() const noexcept { return m_bWasCompleted; }

private:
	/** 待機中なら時間を進め、最大間隔を超えたタップ列を空にする。 */
	void Advance_Internal( f32 DeltaSeconds ) noexcept;

	/** 指定アクションの最初の押下から、新しいタップ列を始める。 */
	void StartSequence_Internal( u32 ActionIndex ) noexcept;

	/** 途中のタップ列を空にし、開始時設定を今後の設定へ揃える。 */
	void ClearSequence_Internal() noexcept;

	/** 今後開始するタップ列で許す最大間隔。 */
	f32 m_MaximumIntervalSeconds = 0.25f;

	/** 現在のタップ列を始めた時点で固定した最大間隔。 */
	f32 m_ActiveMaximumIntervalSeconds = 0.25f;

	/** 直前の押下から経過した秒数。累積誤差を避けるため倍精度で持つ。 */
	f64 m_ElapsedSinceLastTapSeconds = 0.0;

	/** 今後開始するタップ列を完了するために必要な押下回数。 */
	u32 m_RequiredTapCount = 2u;

	/** 現在のタップ列を始めた時点で固定した必要回数。 */
	u32 m_ActiveRequiredTapCount = 2u;

	/** 現在のタップ列で数えた押下回数。 */
	u32 m_TapCount = 0u;

	/** 追跡中のアクション番号。待機していなければ`kActionButtonCount`。 */
	u32 m_ActiveActionIndex = kActionButtonCount;

	/** 今回の更新で必要回数へ到達したならtrue。 */
	bool m_bWasCompleted = false;
};

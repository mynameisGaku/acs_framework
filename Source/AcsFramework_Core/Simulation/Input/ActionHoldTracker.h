// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionHoldTrackerState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 1つのアクションを押している時間から、短押しと長押しを判定する局所状態。
 *
 * @details
 * チャージ、長押しインタラクト、短押しと長押しで異なる操作などに使う。入力装置、場面、
 * 時計は所有せず、通常フレームまたは固定ステップの入力履歴と明示経過秒だけで進む。
 *
 * 閾値へ初めて届いた更新だけ`WasThresholdReached()`がtrueになり、押したままなら
 * `HasReachedThreshold()`がtrueを保つ。閾値へ届く前に離すと`WasTapped()`、届いた後に
 * 離すと`WasHeldAndReleased()`が、その更新だけtrueになる。
 */
class FActionHoldTracker
{
public:
	/** 0.4秒を長押し閾値として構築する。 */
	FActionHoldTracker() noexcept = default;

	/**
	 * 長押し閾値を指定して構築する。
	 *
	 * @details 有限の正数でなければ既定の0.4秒を使う。
	 * @param ThresholdSeconds 長押しとして確定するまでの秒数。
	 */
	explicit FActionHoldTracker( f32 ThresholdSeconds ) noexcept;

	/**
	 * 今後の押下に使う長押し閾値を変更する。
	 *
	 * @details 既に押している操作は、押し始めた時点の閾値を使い続ける。
	 * @param ThresholdSeconds 有限かつ0より大きい秒数。
	 * @return 反映できたらtrue。不正値では従来値を保ちfalse。
	 */
	bool SetThresholdSeconds( f32 ThresholdSeconds ) noexcept;

	/** 今後の押下に使う長押し閾値を返す。 */
	f32 GetThresholdSeconds() const noexcept { return m_ThresholdSeconds; }

	/**
	 * 通常フレームの入力履歴から指定アクションを進める。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param ActionIndex 追跡するアクション番号。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @return 更新できたらtrue。範囲外または追跡中と異なる番号、不正時間では全状態を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input, u32 ActionIndex, f32 DeltaSeconds ) noexcept;

	/**
	 * 明示した現在と前回の入力から指定アクションを進める。
	 *
	 * @details AI、入力再生、固定ステップ、単体テストなど、装置を使わない経路で使う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param ActionIndex 追跡するアクション番号。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @return 更新できたらtrue。範囲外または追跡中と異なる番号、不正時間では全状態を変えずfalse。
	 */
	bool Update( const FActionInput& CurrentInput, const FActionInput& PreviousInput,
		u32 ActionIndex, f32 DeltaSeconds ) noexcept;

	/** 押下時間と今回だけの判定を空にする。閾値設定は維持する。 */
	void Reset() noexcept;

	/** 閾値、連続押下秒、追跡番号と今回判定を保存可能な値として返す。 */
	FActionHoldTrackerState CaptureState() const noexcept;

	/**
	 * 保存した短押し・長押し追跡状態を復元する。
	 *
	 * @param State `CaptureState`で取得した有限かつ矛盾のない状態。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FActionHoldTrackerState& State ) noexcept;

	/** 現在追跡中のアクションを押しているならtrue。 */
	bool IsHolding() const noexcept { return m_bIsHolding; }

	/** 現在の押下が長押し閾値へ到達済みならtrue。 */
	bool HasReachedThreshold() const noexcept { return m_bHasReachedThreshold; }

	/** 今回の更新で初めて長押し閾値へ到達したならtrue。 */
	bool WasThresholdReached() const noexcept { return m_bWasThresholdReached; }

	/** 今回の更新で閾値到達前に離したならtrue。 */
	bool WasTapped() const noexcept { return m_bWasTapped; }

	/** 今回の更新で閾値到達後に離したならtrue。 */
	bool WasHeldAndReleased() const noexcept { return m_bWasHeldAndReleased; }

	/** 現在の連続押下秒を返す。押していなければ0。 */
	f32 GetHeldSeconds() const noexcept;

	/** 現在の押下が長押し閾値へ進んだ割合を0から1で返す。 */
	f32 GetProgress() const noexcept;

private:
	/** 今後の押下に使う長押し閾値。 */
	f32 m_ThresholdSeconds = 0.4f;

	/** 現在の押下を始めた時点の長押し閾値。 */
	f32 m_ActiveThresholdSeconds = 0.4f;

	/** 現在の連続押下秒。累積減算誤差を避けるため倍精度で持つ。 */
	f64 m_HeldSeconds = 0.0;

	/** 追跡中のアクション番号。未追跡時は`kActionButtonCount`。 */
	u32 m_ActiveActionIndex = kActionButtonCount;

	/** 現在押している状態を追跡中ならtrue。 */
	bool m_bIsHolding = false;

	/** 現在の押下が閾値へ到達済みならtrue。 */
	bool m_bHasReachedThreshold = false;

	/** 今回の更新で初めて閾値へ到達したならtrue。 */
	bool m_bWasThresholdReached = false;

	/** 今回の更新で閾値到達前に離したならtrue。 */
	bool m_bWasTapped = false;

	/** 今回の更新で閾値到達後に離したならtrue。 */
	bool m_bWasHeldAndReleased = false;
};

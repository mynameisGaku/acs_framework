// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionRepeatTrackerState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 1つのアクションを押した瞬間と、押し続けた後の一定間隔を発火回数へ変える局所状態。
 *
 * @details
 * メニュー移動、段階調整、格子移動、連続操作などに使う。入力装置、場面、時計、発火時の処理は
 * 所有せず、通常フレームまたは固定ステップの入力履歴と明示経過秒だけで進む。
 */
class FActionRepeatTracker
{
public:
	/** 呼出側が上限を省略したとき、1更新で返す最大発火回数。 */
	static constexpr u32 kDefaultMaximumCatchUpCount = 64u;

	/** 最初の待ち0.4秒、以後0.1秒間隔として構築する。 */
	FActionRepeatTracker() noexcept = default;

	/**
	 * 最初の待ちとrepeat間隔を指定して構築する。
	 *
	 * @details どちらかが有限の正数でなければ既定設定を使う。
	 */
	FActionRepeatTracker( f32 InitialDelaySeconds,
		f32 RepeatIntervalSeconds ) noexcept;

	/**
	 * 今後開始する押下の最初の待ちとrepeat間隔を変更する。
	 *
	 * @details 追跡中の押下は、押し始めた時点の設定を使い続ける。
	 * @return 両方を反映できたらtrue。不正値では従来状態を一切変えずfalse。
	 */
	bool Configure( f32 InitialDelaySeconds,
		f32 RepeatIntervalSeconds ) noexcept;

	/** 今後の押下で最初のrepeatまで待つ秒数を返す。 */
	f32 GetInitialDelaySeconds() const noexcept
	{
		return m_InitialDelaySeconds;
	}

	/** 今後の押下で2回目以降のrepeat間に待つ秒数を返す。 */
	f32 GetRepeatIntervalSeconds() const noexcept
	{
		return m_RepeatIntervalSeconds;
	}

	/** 現在追跡中の押下で最初のrepeatまで待つ秒数を返す。 */
	f32 GetActiveInitialDelaySeconds() const noexcept
	{
		return m_ActiveInitialDelaySeconds;
	}

	/** 現在追跡中の押下で2回目以降に使うrepeat間隔秒を返す。 */
	f32 GetActiveRepeatIntervalSeconds() const noexcept
	{
		return m_ActiveRepeatIntervalSeconds;
	}

	/**
	 * 通常フレームの入力履歴から指定アクションを進める。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param ActionIndex 追跡するアクション番号。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @param OutTriggerCount 押下開始を含む今回の発火回数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で返す1以上の最大発火回数。押下開始も含む。
	 * @return 更新できたらtrue。範囲外、追跡中と異なる番号、不正時間や上限ではfalse。
	 */
	bool Update( const CActionInputTracker& Input, u32 ActionIndex,
		f32 DeltaSeconds, u32& OutTriggerCount,
		u32 MaximumCatchUpCount = kDefaultMaximumCatchUpCount ) noexcept;

	/**
	 * 明示した現在と前回の入力から指定アクションを進める。
	 *
	 * @details AI、入力再生、固定ステップ、単体テストなど、装置を使わない経路で使う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param ActionIndex 追跡するアクション番号。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @param OutTriggerCount 押下開始を含む今回の発火回数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で返す1以上の最大発火回数。押下開始も含む。
	 * @return 更新できたらtrue。範囲外、追跡中と異なる番号、不正時間や上限ではfalse。
	 */
	bool Update( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput, u32 ActionIndex,
		f32 DeltaSeconds, u32& OutTriggerCount,
		u32 MaximumCatchUpCount = kDefaultMaximumCatchUpCount ) noexcept;

	/** 追跡中の押下と持越し秒を空にする。今後の設定は維持する。 */
	void Reset() noexcept;

	/** 設定、持越し秒、追跡番号とrepeat段階を保存可能な値として返す。 */
	FActionRepeatTrackerState CaptureState() const noexcept;

	/**
	 * 保存した押下repeat状態を復元する。
	 *
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FActionRepeatTrackerState& State ) noexcept;

	/** 現在1つの押下を追跡しているならtrue。 */
	bool IsTracking() const noexcept
	{
		return m_ActiveActionIndex < kActionButtonCount;
	}

	/** 最初の待ちを終え、repeat間隔で進んでいるならtrue。 */
	bool IsRepeating() const noexcept { return IsTracking() && m_bIsRepeating; }

	/** 追跡中のアクション番号を返す。未追跡時は`kActionButtonCount`。 */
	u32 GetActiveActionIndex() const noexcept { return m_ActiveActionIndex; }

	/** 次の発火まで持ち越している秒数を返す。未処理の複数回ぶんを含む。 */
	f64 GetAccumulatedSeconds() const noexcept { return m_AccumulatedSeconds; }

	/** 次の発火までの秒数を返す。未追跡または未処理ぶんがあれば0。 */
	f32 GetSecondsUntilNextTrigger() const noexcept;

	/** 次の発火へ進んだ割合を0から1で返す。未追跡なら0、未処理ぶんがあれば1。 */
	f32 GetProgress() const noexcept;

private:
	/** 追跡を空にし、開始時設定を今後の設定へ揃える。 */
	void ClearTracking_Internal() noexcept;

	/** 今後の押下で最初のrepeatまで待つ秒数。 */
	f32 m_InitialDelaySeconds = 0.4f;

	/** 今後の押下で2回目以降のrepeat間に待つ秒数。 */
	f32 m_RepeatIntervalSeconds = 0.1f;

	/** 現在の押下を始めた時点で固定した最初の待ち秒数。 */
	f32 m_ActiveInitialDelaySeconds = 0.4f;

	/** 現在の押下を始めた時点で固定したrepeat間隔秒。 */
	f32 m_ActiveRepeatIntervalSeconds = 0.1f;

	/** 次の発火まで持ち越している秒数。未処理の複数回ぶんを含む。 */
	f64 m_AccumulatedSeconds = 0.0;

	/** 追跡中のアクション番号。未追跡時は`kActionButtonCount`。 */
	u32 m_ActiveActionIndex = kActionButtonCount;

	/** 最初の待ちを終え、repeat間隔で進んでいるならtrue。 */
	bool m_bIsRepeating = false;
};

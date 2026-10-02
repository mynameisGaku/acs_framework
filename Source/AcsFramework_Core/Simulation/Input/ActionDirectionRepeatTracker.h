// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionDirectionRepeatTrackerState.h"
#include "AcsFramework_Core/Simulation/Input/ActionDirectionTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionRepeatTracker.h"

using namespace acs;

class CActionInputTracker;

/**
 * 2軸を離散方向へ揃え、方向変更を即時発火、同方向保持を一定間隔の発火回数へ変える局所状態。
 *
 * @details
 * メニュー、格子移動、対象選択、段階調整などに使う。入力装置、時計、操作対象を所有せず、
 * 通常フレーム、固定ステップ、AI、再生入力から同じ方向repeatを作る。
 */
class FActionDirectionRepeatTracker
{
public:
	/** 既定の8方向量子化、最初の待ち0.4秒、以後0.1秒間隔として構築する。 */
	FActionDirectionRepeatTracker() noexcept = default;

	/**
	 * 量子化設定、最初の待ち、以後のrepeat間隔を指定して構築する。
	 *
	 * @details いずれかが不正なら全項目を既定値のまま保つ。
	 */
	FActionDirectionRepeatTracker(
		const FActionDirectionQuantizer& Quantizer,
		f32 InitialDelaySeconds, f32 RepeatIntervalSeconds ) noexcept;

	/**
	 * 今後の量子化とrepeat設定を原子的に変更する。
	 *
	 * @details
	 * 量子化設定は次の更新から使う。保持中の方向は開始時のrepeat時間を使い続け、
	 * 解除または方向変更後から新しい時間を使う。
	 * @return 全設定を反映できたらtrue。不正値では全状態を変えずfalse。
	 */
	bool Configure( const FActionDirectionQuantizer& Quantizer,
		f32 InitialDelaySeconds, f32 RepeatIntervalSeconds ) noexcept;

	/** 現在使っている方向量子化設定を返す。 */
	const FActionDirectionQuantizer& GetQuantizer() const noexcept
	{
		return m_DirectionTracker.GetQuantizer();
	}

	/** 今後の方向開始で最初のrepeatまで待つ秒数を返す。 */
	f32 GetInitialDelaySeconds() const noexcept
	{
		return m_RepeatTracker.GetInitialDelaySeconds();
	}

	/** 今後の方向開始で2回目以後に待つrepeat間隔秒を返す。 */
	f32 GetRepeatIntervalSeconds() const noexcept
	{
		return m_RepeatTracker.GetRepeatIntervalSeconds();
	}

	/** 現在保持中の方向で最初のrepeatまで待つ秒数を返す。 */
	f32 GetActiveInitialDelaySeconds() const noexcept
	{
		return m_RepeatTracker.GetActiveInitialDelaySeconds();
	}

	/** 現在保持中の方向で2回目以後に待つrepeat間隔秒を返す。 */
	f32 GetActiveRepeatIntervalSeconds() const noexcept
	{
		return m_RepeatTracker.GetActiveRepeatIntervalSeconds();
	}

	/**
	 * 明示した2軸から方向とrepeatを1回進める。
	 *
	 * @param Axes X正を右、Y正を上とする有限な入力。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @param OutTriggerCount 方向開始・変更を含む今回の発火回数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で返す1以上の最大発火回数。
	 * @return 更新できたらtrue。不正入力では出力と全状態を変えずfalse。
	 */
	bool Update( FVec2 Axes, f32 DeltaSeconds, u32& OutTriggerCount,
		u32 MaximumCatchUpCount =
			FActionRepeatTracker::kDefaultMaximumCatchUpCount ) noexcept;

	/**
	 * 通常フレームの現在入力から2軸を読み、方向とrepeatを1回進める。
	 *
	 * @param Input 現在入力を保持する通常フレーム用トラッカー。
	 * @param XAxisIndex 右を正とする範囲内の軸番号。
	 * @param YAxisIndex 上を正とする、Xとは異なる範囲内の軸番号。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @param OutTriggerCount 方向開始・変更を含む今回の発火回数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で返す1以上の最大発火回数。
	 * @return 更新できたらtrue。不正入力では出力と全状態を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input,
		u32 XAxisIndex, u32 YAxisIndex, f32 DeltaSeconds,
		u32& OutTriggerCount,
		u32 MaximumCatchUpCount =
			FActionRepeatTracker::kDefaultMaximumCatchUpCount ) noexcept;

	/**
	 * 固定ステップ、AI、再生入力から2軸を読み、方向とrepeatを1回進める。
	 *
	 * @param Input 変換元の汎用アクション入力。
	 * @param XAxisIndex 右を正とする範囲内の軸番号。
	 * @param YAxisIndex 上を正とする、Xとは異なる範囲内の軸番号。
	 * @param DeltaSeconds 現在入力が続いた有限かつ0以上の経過秒。
	 * @param OutTriggerCount 方向開始・変更を含む今回の発火回数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で返す1以上の最大発火回数。
	 * @return 更新できたらtrue。不正入力では出力と全状態を変えずfalse。
	 */
	bool Update( const FActionInput& Input,
		u32 XAxisIndex, u32 YAxisIndex, f32 DeltaSeconds,
		u32& OutTriggerCount,
		u32 MaximumCatchUpCount =
			FActionRepeatTracker::kDefaultMaximumCatchUpCount ) noexcept;

	/** 方向とrepeat途中状態を空にする。量子化と今後の時間設定は維持する。 */
	void Reset() noexcept;

	/** 方向、量子化、repeat時間と持越しを保存可能な値として返す。 */
	FActionDirectionRepeatTrackerState CaptureState() const noexcept;

	/**
	 * 保存した方向repeat状態を復元する。
	 *
	 * @return 復元できたらtrue。不正または矛盾した状態では全状態を変えずfalse。
	 */
	bool RestoreState(
		const FActionDirectionRepeatTrackerState& State ) noexcept;

	/** 現在の離散方向を返す。 */
	EActionDirection2D GetDirection() const noexcept
	{
		return m_DirectionTracker.GetDirection();
	}

	/** 最後に成功した更新より前の方向を返す。 */
	EActionDirection2D GetPreviousDirection() const noexcept
	{
		return m_DirectionTracker.GetPreviousDirection();
	}

	/** 現在方向がNoneでなければtrue。 */
	bool IsActive() const noexcept { return m_DirectionTracker.IsActive(); }

	/** 今回の成功した更新で開始・変更・解除のいずれかが起きたならtrue。 */
	bool WasChanged() const noexcept
	{
		return m_DirectionTracker.WasChanged();
	}

	/** 今回の成功した更新でNoneから方向入力を始めたならtrue。 */
	bool WasStarted() const noexcept
	{
		return m_DirectionTracker.WasStarted();
	}

	/** 今回の成功した更新で方向入力をNoneへ戻したならtrue。 */
	bool WasReleased() const noexcept
	{
		return m_DirectionTracker.WasReleased();
	}

	/** 最初の待ちを終え、現在方向をrepeat間隔で追跡中ならtrue。 */
	bool IsRepeating() const noexcept
	{
		return m_RepeatTracker.IsRepeating();
	}

	/** 次の方向発火までの秒数を返す。方向がなければ0。 */
	f32 GetSecondsUntilNextTrigger() const noexcept
	{
		return m_RepeatTracker.GetSecondsUntilNextTrigger();
	}

	/** 次の方向発火へ進んだ割合を0から1で返す。方向がなければ0。 */
	f32 GetProgress() const noexcept
	{
		return m_RepeatTracker.GetProgress();
	}

private:
	/** 候補方向から既存repeatを進め、成功時だけ候補と出力を確定する。 */
	bool UpdateRepeat_Internal(
		const FActionDirectionTracker& DirectionCandidate,
		f32 DeltaSeconds, u32& OutTriggerCount,
		u32 MaximumCatchUpCount ) noexcept;

	/** 2軸の量子化と現在・前回方向。 */
	FActionDirectionTracker m_DirectionTracker;

	/** 同じ方向を保持した時間とrepeat設定。 */
	FActionRepeatTracker m_RepeatTracker;
};

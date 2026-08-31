// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/GameplayCooldownState.h"

using namespace acs;

/**
 * 使用成功から再使用可能になるまでを、明示したゲーム時間だけ進める局所状態。
 *
 * @details
 * 攻撃、回避、能力、操作装置など、連続使用を制限したいゲーム規則のfieldとして持つ。
 * callback、入力、場面、時計は所有せず、通常フレームまたは固定ステップから渡された秒数だけで
 * 進む。`CTimerSubsystem`の予約とは異なり、保存復元と決定論的な再生に使える。
 */
class FGameplayCooldown
{
public:
	/** 1秒待ちで、最初は使用可能な状態を構築する。 */
	FGameplayCooldown() noexcept = default;

	/**
	 * 再使用までの秒数を指定し、最初は使用可能な状態を構築する。
	 *
	 * @param DurationSeconds 使用成功後に待つ有限かつ正の秒数。
	 * @details 不正値なら既定の1秒を使う。
	 */
	explicit FGameplayCooldown( f32 DurationSeconds ) noexcept;

	/**
	 * 今後の使用成功後に待つ秒数を変更する。
	 *
	 * @details 現在進行中の再使用待ちは、開始時の秒数を使い続ける。
	 * @param DurationSeconds 有限かつ0より大きい秒数。
	 * @return 反映できたらtrue。不正値では従来状態を一切変えずfalse。
	 */
	bool SetDurationSeconds( f32 DurationSeconds ) noexcept;

	/** 今後の使用成功後に待つ秒数を返す。 */
	f32 GetDurationSeconds() const noexcept { return m_DurationSeconds; }

	/** 現在の待機に使っている秒数を返す。使用可能なら今後の設定値。 */
	f32 GetActiveDurationSeconds() const noexcept
	{
		return m_ActiveDurationSeconds;
	}

	/**
	 * 現在使用可能なら使用を受理し、再使用待ちを始める。
	 *
	 * @return 使用を受理して待機を始めたらtrue。待機中なら状態を変えずfalse。
	 */
	bool TryUse() noexcept;

	/**
	 * 再使用待ちを明示したゲーム時間だけ進める。
	 *
	 * @param DeltaSeconds 前回更新から進んだ有限かつ0以上の秒数。
	 * @return 更新できたらtrue。不正時間では完了結果を含む全状態を変えずfalse。
	 */
	bool Update( f32 DeltaSeconds ) noexcept;

	/** 待機を取り消して使用可能へ戻す。秒数設定は維持する。 */
	void Reset() noexcept;

	/** 設定、開始時秒数、経過秒と今回結果を保存可能な値として返す。 */
	FGameplayCooldownState CaptureState() const noexcept;

	/**
	 * 保存した再使用待ち状態を復元する。
	 *
	 * @param State `CaptureState`で取得した有限かつ矛盾のない状態。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FGameplayCooldownState& State ) noexcept;

	/** 現在使用可能ならtrue。 */
	bool IsReady() const noexcept { return !m_bIsCoolingDown; }

	/** 現在再使用を待っているならtrue。 */
	bool IsCoolingDown() const noexcept { return m_bIsCoolingDown; }

	/** 再使用可能になるまでの残り秒を返す。使用可能なら0。 */
	f32 GetRemainingSeconds() const noexcept;

	/** 現在の待機が進んだ割合を0から1で返す。使用可能なら1。 */
	f32 GetProgress() const noexcept;

	/**
	 * 今回の有効な`Update()`で再使用可能になったならtrue。
	 * 同じ更新内の`TryUse()`では消えず、次の有効な`Update()`でfalseへ戻る。
	 */
	bool WasCompleted() const noexcept { return m_bWasCompleted; }

private:
	/** 現在の待機を完了し、今後の設定で使用可能状態を正規化する。 */
	void Complete_Internal() noexcept;

	/** 今後の使用成功後に待つ秒数。 */
	f32 m_DurationSeconds = 1.0f;

	/** 現在の待機を始めた時点で固定した秒数。 */
	f32 m_ActiveDurationSeconds = 1.0f;

	/** 現在の待機を始めてから経過した秒数。 */
	f64 m_ElapsedSeconds = 0.0;

	/** 現在再使用を待っているならtrue。 */
	bool m_bIsCoolingDown = false;

	/** 次の有効な更新まで保持する、直近更新の再使用可能通知。 */
	bool m_bWasCompleted = false;
};

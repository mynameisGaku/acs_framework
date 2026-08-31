// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/GameplayTimerState.h"

using namespace acs;

/**
 * 制限時間や状態持続時間を、明示したゲーム時間だけ進める局所タイマー。
 *
 * @details
 * ゲーム規則のfieldとして持ち、開始、一時停止、再開、完了と進行率を扱う。
 * callback、場面、時計は所有せず、完了後に行う処理も利用側へ残す。
 * `CTimerSubsystem`の予約とは異なり、固定ステップで決定論的に進めて保存復元できる。
 */
class FGameplayTimer
{
public:
	/** 1秒設定で、まだ開始していないタイマーを構築する。 */
	FGameplayTimer() noexcept = default;

	/**
	 * 指定秒数で、まだ開始していないタイマーを構築する。
	 *
	 * @param DurationSeconds 有限かつ0より大きい秒数。
	 * @details 不正値なら既定の1秒を使う。
	 */
	explicit FGameplayTimer( f32 DurationSeconds ) noexcept;

	/**
	 * 今後の開始時に使う秒数を変更する。
	 *
	 * @details 実行中、一時停止中、完了済みの計測は開始時の秒数を保つ。
	 * @param DurationSeconds 有限かつ0より大きい秒数。
	 * @return 反映できたらtrue。不正値では従来状態を変えずfalse。
	 */
	bool SetDurationSeconds( f32 DurationSeconds ) noexcept;

	/** 今後の開始時に使う秒数を返す。 */
	f32 GetDurationSeconds() const noexcept { return m_DurationSeconds; }

	/** 現在または直近の計測を始めた時点で固定した秒数を返す。 */
	f32 GetActiveDurationSeconds() const noexcept
	{
		return m_ActiveDurationSeconds;
	}

	/**
	 * 未開始なら現在設定で計測を始める。
	 *
	 * @return 開始できたらtrue。開始後、一時停止中、完了後なら状態を変えずfalse。
	 */
	bool Start() noexcept;

	/**
	 * 現在状態に関係なく、現在設定の0秒地点から計測を始め直す。
	 * 直近更新の完了通知は次の有効な`Update()`まで保持する。
	 */
	void Restart() noexcept;

	/**
	 * 実行中の計測を一時停止する。
	 *
	 * @return 停止できたらtrue。実行中でなければ状態を変えずfalse。
	 */
	bool Pause() noexcept;

	/**
	 * 一時停止した未完了の計測を再開する。
	 *
	 * @return 再開できたらtrue。未開始、実行中、完了後なら状態を変えずfalse。
	 */
	bool Resume() noexcept;

	/**
	 * 実行中の計測を明示したゲーム時間だけ進める。
	 *
	 * @param DeltaSeconds 前回更新から進んだ有限かつ0以上の秒数。
	 * @return 入力を受理できたらtrue。不正時間では完了結果を含む全状態を変えずfalse。
	 */
	bool Update( f32 DeltaSeconds ) noexcept;

	/** 設定秒数を保ち、まだ開始していない0秒状態へ戻す。 */
	void Reset() noexcept;

	/** 設定、開始時秒数、経過秒と実行状態を保存可能な値として返す。 */
	FGameplayTimerState CaptureState() const noexcept;

	/**
	 * 保存したタイマー状態を復元する。
	 *
	 * @param State 有限かつ矛盾のない保存値。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FGameplayTimerState& State ) noexcept;

	/** まだ一度も開始していない状態ならtrue。 */
	bool IsIdle() const noexcept { return !m_bHasStarted; }

	/** 明示時間を受け取ると計測が進む状態ならtrue。 */
	bool IsRunning() const noexcept { return m_bIsRunning; }

	/** 開始後かつ未完了で一時停止しているならtrue。 */
	bool IsPaused() const noexcept
	{
		return m_bHasStarted && !m_bIsRunning && !m_bIsComplete;
	}

	/** 直近の計測が必要時間へ到達しているならtrue。 */
	bool IsComplete() const noexcept { return m_bIsComplete; }

	/** 現在または直近の計測で経過した秒数を返す。未開始なら0。 */
	f64 GetElapsedSeconds() const noexcept { return m_ElapsedSeconds; }

	/** 現在または次回の計測が完了するまでの残り秒を返す。 */
	f32 GetRemainingSeconds() const noexcept;

	/** 現在または直近の計測が進んだ割合を0から1で返す。未開始なら0。 */
	f32 GetProgress() const noexcept;

	/**
	 * 今回の有効な`Update()`で必要時間へ到達したならtrue。
	 * 再開始しても保持し、次の有効な`Update()`でfalseへ戻る。
	 */
	bool WasCompleted() const noexcept { return m_bWasCompleted; }

private:
	/** 現在の計測を完了境界へ揃え、今回の完了を記録する。 */
	void Complete_Internal() noexcept;

	/** 今後の開始時に使う秒数。 */
	f32 m_DurationSeconds = 1.0f;

	/** 現在または直近の計測を始めた時点で固定した秒数。 */
	f32 m_ActiveDurationSeconds = 1.0f;

	/** 現在または直近の計測で経過した秒数。 */
	f64 m_ElapsedSeconds = 0.0;

	/** 開始後または完了後の状態ならtrue。 */
	bool m_bHasStarted = false;

	/** 明示時間を受け取ると計測が進む状態ならtrue。 */
	bool m_bIsRunning = false;

	/** 直近の計測が必要時間へ到達したならtrue。 */
	bool m_bIsComplete = false;

	/** 次の有効な更新まで保持する、直近更新の完了通知。 */
	bool m_bWasCompleted = false;
};

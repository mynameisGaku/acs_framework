// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/GameplayIntervalState.h"

using namespace acs;

/**
 * 一定間隔の到達回数を、明示したゲーム時間だけから求める局所計測値。
 *
 * @details
 * ゲーム規則のfieldとして持ち、開始、一時停止、再開と複数回ぶんの追い付き処理を扱う。
 * callback、場面、時計は所有せず、到達時に行う処理も利用側へ残す。
 * `CTimerSubsystem`の繰返し予約とは異なり、固定ステップで決定論的に進めて保存復元できる。
 */
class FGameplayInterval
{
public:
	/** 呼出側が上限を省略したとき、1更新で返す最大到達回数。 */
	static constexpr u32 kDefaultMaximumCatchUpCount = 64u;

	/** 1秒間隔で、まだ開始していない計測値を構築する。 */
	FGameplayInterval() noexcept = default;

	/**
	 * 指定間隔で、まだ開始していない計測値を構築する。
	 *
	 * @param IntervalSeconds 有限かつ0より大きい間隔秒。
	 * @details 不正値なら既定の1秒を使う。
	 */
	explicit FGameplayInterval( f32 IntervalSeconds ) noexcept;

	/**
	 * 今後の開始時に使う間隔秒を変更する。
	 *
	 * @details 実行中または一時停止中の計測は開始時の間隔を保つ。
	 * @param IntervalSeconds 有限かつ0より大きい間隔秒。
	 * @return 反映できたらtrue。不正値では従来状態を変えずfalse。
	 */
	bool SetIntervalSeconds( f32 IntervalSeconds ) noexcept;

	/** 今後の開始時に使う間隔秒を返す。 */
	f32 GetIntervalSeconds() const noexcept { return m_IntervalSeconds; }

	/** 現在の計測を始めた時点で固定した間隔秒を返す。 */
	f32 GetActiveIntervalSeconds() const noexcept
	{
		return m_ActiveIntervalSeconds;
	}

	/**
	 * 未開始なら現在設定で計測を始める。
	 *
	 * @return 開始できたらtrue。開始後または一時停止中なら状態を変えずfalse。
	 */
	bool Start() noexcept;

	/** 現在状態に関係なく、現在設定の0秒地点から計測を始め直す。 */
	void Restart() noexcept;

	/**
	 * 実行中の計測を一時停止する。
	 *
	 * @return 停止できたらtrue。実行中でなければ状態を変えずfalse。
	 */
	bool Pause() noexcept;

	/**
	 * 一時停止した計測を再開する。
	 *
	 * @return 再開できたらtrue。未開始または実行中なら状態を変えずfalse。
	 */
	bool Resume() noexcept;

	/**
	 * 実行中の計測を明示したゲーム時間だけ進め、間隔へ到達した回数を返す。
	 *
	 * @details 上限を超えた回数ぶんの時間は捨てず、次回以降の`Update()`へ持ち越す。
	 * `DeltaSeconds`が0でも、持越し分から到達回数を取り出せる。
	 * @param DeltaSeconds 前回更新から進んだ有限かつ0以上の秒数。
	 * @param OutOccurrenceCount 今回処理する到達回数。失敗時は変更しない。
	 * @param MaximumCatchUpCount 1更新で返す1以上の最大到達回数。
	 * @return 入力を受理できたらtrue。不正入力では全状態と出力を変えずfalse。
	 */
	bool Update( f32 DeltaSeconds, u32& OutOccurrenceCount,
		u32 MaximumCatchUpCount = kDefaultMaximumCatchUpCount ) noexcept;

	/** 設定間隔を保ち、まだ開始していない0秒状態へ戻す。 */
	void Reset() noexcept;

	/** 設定、開始時間隔、持越し秒と実行状態を保存可能な値として返す。 */
	FGameplayIntervalState CaptureState() const noexcept;

	/**
	 * 保存した間隔状態を復元する。
	 *
	 * @param State 有限かつ矛盾のない保存値。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState( const FGameplayIntervalState& State ) noexcept;

	/** まだ一度も開始していない状態ならtrue。 */
	bool IsIdle() const noexcept { return !m_bHasStarted; }

	/** 明示時間を受け取ると計測が進む状態ならtrue。 */
	bool IsRunning() const noexcept { return m_bIsRunning; }

	/** 開始後かつ一時停止しているならtrue。 */
	bool IsPaused() const noexcept
	{
		return m_bHasStarted && !m_bIsRunning;
	}

	/** 次の到達判定へ持ち越している秒数を返す。未処理の複数回ぶんを含む。 */
	f64 GetAccumulatedSeconds() const noexcept { return m_AccumulatedSeconds; }

	/** 次の1回へ到達するまでの秒数を返す。未処理ぶんが在れば0。 */
	f32 GetSecondsUntilNextOccurrence() const noexcept;

	/** 次の1回へ進んだ割合を0から1で返す。未処理ぶんが在れば1。 */
	f32 GetProgress() const noexcept;

private:
	/** 今後の開始時に使う間隔秒。 */
	f32 m_IntervalSeconds = 1.0f;

	/** 現在の計測を始めた時点で固定した間隔秒。 */
	f32 m_ActiveIntervalSeconds = 1.0f;

	/** 次の到達判定へ持ち越している秒数。未処理の複数回ぶんを含む。 */
	f64 m_AccumulatedSeconds = 0.0;

	/** 開始後または一時停止中ならtrue。 */
	bool m_bHasStarted = false;

	/** 明示時間を受け取ると計測が進む状態ならtrue。 */
	bool m_bIsRunning = false;
};

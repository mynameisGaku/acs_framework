// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::timing;

/**
 * 実時間を、決まった幅のステップへ割り直す係。
 *
 * @details
 * 割り直しは Engine (`acs::game::CFixedStepClock`) が持っている。ここが足すのは
 * **ティック番号**と、**取りこぼしの見え方**の 2 つ。
 *
 * ロジックを実時間の dt で回すと、機械や負荷で結果が変わり、同じ操作をしても
 * 同じ盤面にならない。幅を固定して「何ステップぶん進めるか」だけを可変にすれば、
 * 記録した入力から同じ結果を作れる。
 *
 * 描画は固定ステップの外側にあるので、ステップ間の位置は `GetAlpha()` で補間する。
 *
 * @code
 * const u32 Steps = Driver.Advance( DeltaSeconds );
 * for ( u32 Step = 0u; Step < Steps; ++Step )
 * {
 *     Rule.AdvanceStep( Context );   // Context の中の Tick が 1 つずつ進む
 * }
 * @endcode
 */
class CFixedStepDriver
{
public:
	/** 既定 (1/60 秒・1 フレーム最大 8 ステップ) で構築する。 */
	CFixedStepDriver() noexcept;

	/**
	 * ステップ幅と、1 回の Advance で進める上限を決める。
	 *
	 * @details
	 * 上限があるのは、重い処理でフレームが飛んだときに «取り戻そうとして更に重くなる» のを
	 * 避けるため。上限を超えたぶんは捨てられ、GetDroppedSeconds() で分かる。
	 * @param StepSeconds 1 ステップの秒数 (60fps なら 1/60)。
	 * @param MaximumStepsPerAdvance 1 回の Advance で進める最大ステップ数。
	 * @return 設定できたら true。
	 */
	bool Configure( f64 StepSeconds, u32 MaximumStepsPerAdvance = 8u ) noexcept;

	/**
	 * 実時間を渡して、進めるべきステップ数を得る。
	 *
	 * @details ティック番号はここでは進まない。実際に進めた側が AdvanceTick() を呼ぶ。
	 * @param DeltaSeconds 前フレームからの実経過秒。
	 * @return 進めるべきステップ数。
	 */
	u32 Advance( f64 DeltaSeconds ) noexcept;

	/** 1 ステップ進んだことを記録する (ティック番号が 1 つ進む)。 */
	void AdvanceTick() noexcept { ++m_Tick; }

	/** いまのティック番号を返す。 */
	u32 GetTick() const noexcept { return m_Tick; }

	/** 1 ステップの秒数を返す。 */
	f32 GetStepSeconds() const noexcept { return m_StepSeconds; }

	/** ステップ間のどこに居るか (0..1) を返す。描画の補間に使う。 */
	f32 GetAlpha() const noexcept;

	/** これまでに進めた総ステップ数を返す。 */
	u64 GetTotalStepCount() const noexcept;

	/** 上限に達して捨てた秒数を返す。0 でなければ処理落ちしている。 */
	f64 GetDroppedSeconds() const noexcept;

	/** 直近の Advance が上限で頭打ちになったかを返す。 */
	bool WasClamped() const noexcept { return m_bWasClamped; }

	/** ティック番号と溜まりを 0 へ戻す。 */
	void Reset() noexcept;

	/**
	 * いまの進み具合を写し取る。
	 *
	 * @param OutSnapshot 写し先。
	 * @param OutTick ティック番号の入れ先。
	 * @return 写せたら true。
	 */
	bool TryCaptureSnapshot( FFixedStepClockSnapshot& OutSnapshot, u32& OutTick ) const noexcept;

	/**
	 * 写し取った進み具合へ戻す。
	 *
	 * @param Snapshot 戻す先。
	 * @param Tick そのときのティック番号。
	 * @return 戻せたら true。
	 */
	bool TryRestoreSnapshot( const FFixedStepClockSnapshot& Snapshot, u32 Tick ) noexcept;

private:
	/** 割り直しの本体。 */
	CFixedStepClock m_Clock;

	/** 1 ステップの秒数。 */
	f32 m_StepSeconds = 1.0f / 60.0f;

	/** いまのティック番号。 */
	u32 m_Tick = 0u;

	/** 直近の Advance が頭打ちになったか。 */
	bool m_bWasClamped = false;
};

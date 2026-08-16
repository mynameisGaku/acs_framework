// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/DeterministicRandom.h"
#include "AcsFramework_Core/Simulation/FixedStepDriver.h"
#include "AcsFramework_Core/Simulation/ISimulationRule.h"

using namespace acs;

/**
 * ある瞬間の «全部» を写し取ったもの。
 *
 * @details
 * 続きから同じ道を進むために要るのは 3 つで、1 つでも欠けると別の道になる。
 *
 * | 写すもの | 欠けるとどうなるか |
 * |---|---|
 * | 盤面 (ISimulationRule) | そもそも別の局面から始まる |
 * | 時計 (CFixedStepDriver) | ティック番号がずれ、テープの読み出し位置が合わない |
 * | 乱数 (CDeterministicRandom) | 同じ入力でも違う出目になる |
 *
 * 入力のテープは含めない。テープは «どう操作したか» で、こちらは «どうなっていたか»。
 * 途中から再生するときは、同じテープとこのスナップショットを組み合わせる。
 *
 * @code
 * CSimulationSnapshot Snapshot;
 * Snapshot.TryCaptureFrom( Driver, Random, Rule );   // ここまでを覚える
 * ...
 * Snapshot.TryRestoreTo( Driver, Random, Rule );     // ここへ戻る
 * @endcode
 */
class CSimulationSnapshot
{
public:
	/**
	 * いまの様子を写し取る。
	 *
	 * @details 規則が盤面を差し出せない (TrySaveState が false) 場合は写さない。
	 * @param Driver 時計。
	 * @param Random 乱数。
	 * @param Rule 盤面を持っている規則。
	 * @return 写せたら true。
	 */
	bool TryCaptureFrom( const CFixedStepDriver& Driver, const CDeterministicRandom& Random, const ISimulationRule& Rule ) noexcept;

	/**
	 * 写し取った様子へ戻す。
	 *
	 * @details
	 * 3 つのうち 1 つでも戻せなければ false を返し、**何も変えない**。
	 * 中途半端に戻った状態から進むと、原因の分からないずれ方をするため。
	 * @param OutDriver 戻す先の時計。
	 * @param OutRandom 戻す先の乱数。
	 * @param OutRule 戻す先の規則。
	 * @return 戻せたら true。
	 */
	bool TryRestoreTo( CFixedStepDriver& OutDriver, CDeterministicRandom& OutRandom, ISimulationRule& OutRule ) const noexcept;

	/** 何か写してあるかを返す。 */
	bool IsValid() const noexcept { return m_bValid; }

	/** 写した時点のティック番号を返す。 */
	u32 GetTick() const noexcept { return m_Tick; }

	/** 写した時点までに乱数を引いた回数を返す。 */
	u64 GetDrawCount() const noexcept { return m_DrawCount; }

	/** 盤面のバイト数を返す。 */
	usize GetRuleByteCount() const noexcept { return m_RuleBytes.Num(); }

	/** 写したものを捨てる。 */
	void Clear() noexcept;

	/**
	 * バイト列へ書き出す。
	 *
	 * @param Buffer 書き出し先。
	 * @param Capacity 書き出し先の大きさ。
	 * @param OutWritten 書けた大きさの入れ先。
	 * @return 書けたら true。
	 */
	bool TrySaveToBuffer( u8* Buffer, usize Capacity, usize& OutWritten ) const noexcept;

	/**
	 * バイト列から読み込む。
	 *
	 * @details 読めなかった場合、中身は空になる。
	 * @param Buffer 読み元。
	 * @param Size 読み元の大きさ。
	 * @return 読めたら true。
	 */
	bool TryLoadFromBuffer( const u8* Buffer, usize Size ) noexcept;

	/** 書き出しに必要な大きさを返す。 */
	usize GetRequiredBytes() const noexcept;

private:
	/** 写した時点のティック番号。 */
	u32 m_Tick = 0u;

	/** 写した時点までに乱数を引いた回数。 */
	u64 m_DrawCount = 0u;

	/** 乱数の内部状態。 */
	FRandomSnapshot m_RandomState{};

	/** 時計の進み具合。 */
	FFixedStepClockSnapshot m_ClockState{};

	/** 盤面。中身の意味は規則だけが知っている。 */
	TArray<u8> m_RuleBytes;

	/** 何か写してあるか。 */
	bool m_bValid = false;
};

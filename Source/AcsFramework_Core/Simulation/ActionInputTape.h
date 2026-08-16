// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/**
 * ティックごとの入力を並べて残したもの。
 *
 * @details
 * 「再現できないバグ」を再現できるようにするための中心。種と入力列さえ残っていれば、
 * 同じ盤面をもう一度作れる。
 *
 * **前と同じ入力は書かない。** 入力は大半のティックで変わらないので、変わった所だけを
 * 残す。読むときは「そのティック以前で最後に書かれたもの」を返す。
 *
 * Engine の `CInputRecorder` は**キーとマウスの状態**を記録するもので、層が違う。
 * こちらはアクションを記録するので、キー割り当てを変えても再生が壊れない。
 */
class CActionInputTape
{
public:
	/**
	 * このティックの入力を書く。
	 *
	 * @details 直前に書いたものと同じなら書かない (テープを短く保つ)。
	 * @param Tick ティック番号。前に書いたものより後であること。
	 * @param Input このティックの入力。
	 * @return 書けたら true (同じ内容で省いた場合も true)。
	 */
	bool Record( u32 Tick, const FActionInput& Input ) noexcept;

	/**
	 * そのティックの入力を読む。
	 *
	 * @details 変化が書かれていないティックは、それ以前の最後の入力が返る。
	 * @param Tick ティック番号。
	 * @param OutInput 入力の入れ先。
	 * @return 読めたら true (テープが空、または最初の記録より前なら false)。
	 */
	bool TryGet( u32 Tick, FActionInput& OutInput ) const noexcept;

	/** 書かれている変化の数を返す。 */
	usize Num() const noexcept { return m_Entries.Num(); }

	/** 記録されている最後のティックを返す (空なら 0)。 */
	u32 GetLastTick() const noexcept { return m_LastTick; }

	/** 種を覚える。 */
	void SetSeed( u64 Seed ) noexcept { m_Seed = Seed; }

	/** 覚えている種を返す。 */
	u64 GetSeed() const noexcept { return m_Seed; }

	/** 全て捨てる。 */
	void Clear() noexcept;

	/**
	 * バイト列へ書き出す。
	 *
	 * @details 先頭に種と件数が入る。必要な大きさは GetRequiredBytes() で分かる。
	 * @param Buffer 書き出し先。
	 * @param Capacity 書き出し先の大きさ。
	 * @param OutWritten 書けた大きさの入れ先。
	 * @return 書けたら true。
	 */
	bool TrySaveToBuffer( u8* Buffer, usize Capacity, usize& OutWritten ) const noexcept;

	/**
	 * バイト列から読み込む。
	 *
	 * @details 読み込みに失敗した場合、テープの中身は空になる。
	 * @param Buffer 読み元。
	 * @param Size 読み元の大きさ。
	 * @return 読めたら true。
	 */
	bool TryLoadFromBuffer( const u8* Buffer, usize Size ) noexcept;

	/** 書き出しに必要な大きさを返す。 */
	usize GetRequiredBytes() const noexcept;

private:
	/** 入力が変わったティック 1 件。 */
	struct FEntry
	{
		/** 変わったティック。 */
		u32 Tick = 0u;

		/** そのティック以降の入力。 */
		FActionInput Input;
	};

	/**
	 * そのティックに効いている記録の位置を探す。
	 *
	 * @param Tick ティック番号。
	 * @return 見つかった位置。無ければ Num()。
	 */
	usize FindEntryIndex( u32 Tick ) const noexcept;

	/** 入力が変わったティックだけを並べたもの。 */
	TArray<FEntry> m_Entries;

	/** 記録されている最後のティック。 */
	u32 m_LastTick = 0u;

	/** 記録したときの種。 */
	u64 m_Seed = 0u;
};

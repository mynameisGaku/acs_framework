// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Common/Text/InternedNamePool.h"

using namespace acs;
using namespace acs::game;

/**
 * 状態と曲の対応表。
 *
 * @details
 * 「戦闘中はこの曲」「同じ戦闘でも強さ 0.7 以上なら別の曲」を決めるだけ。鳴らす仕組みは
 * エンジン (CMusicDirector) が持っている。
 *
 * 溜める側 (Add) と流し込む側 (ApplyTo) を分けてあるので、起動時に何度足しても、
 * エンジンが書き換わるのは ApplyTo を呼んだ 1 か所になる。
 *
 * 曲のパスは名前プールへ写す。CMusicDirector は文字列を複製せずポインタで持つ。
 */
class CMusicTrackCatalog
{
public:
	/**
	 * 曲を 1 つ足す。
	 *
	 * @param State どの状態のときの曲か。
	 * @param AssetPath 曲のパス。
	 * @param IntensityMin この曲を使う強さの下限 (0..1)。
	 * @param IntensityMax この曲を使う強さの上限 (0..1)。
	 * @param bLoop 繰り返すなら true。
	 * @return 足せたら true。
	 */
	bool Add( EMusicState State, const FString& AssetPath, f32 IntensityMin = 0.0f, f32 IntensityMax = 1.0f, bool bLoop = true ) noexcept;

	/**
	 * 溜めた対応をエンジンへ流し込む。
	 *
	 * @param Director 流し込む先。
	 * @return 流し込んだ数。
	 */
	usize ApplyTo( CMusicDirector& Director ) const noexcept;

	/** 足した曲の数を返す。 */
	usize Num() const noexcept { return m_Entries.Num(); }

private:
	/** 状態と曲の組。 */
	struct FEntry
	{
		/** どの状態のときの曲か。 */
		EMusicState State = EMusicState::Silent;

		/** エンジンへ渡す曲の情報 (パスは名前プールが持つ)。 */
		FMusicTrack Track;
	};

	/** 曲のパスの実体。 */
	CInternedNamePool m_Paths;

	/** 足した対応。 */
	TArray<FEntry> m_Entries;
};

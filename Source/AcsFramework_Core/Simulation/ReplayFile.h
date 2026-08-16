// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInputTape.h"

using namespace acs;
using namespace acs::game;

/**
 * 入力のテープをファイルへ置き、ファイルから戻す係。
 *
 * @details
 * 「バグが出た入力列を残して、後で流し直す」を 1 呼び出しで済ませるためのもの。
 *
 * 置き方は Engine (`acs::game::CSaveArchive`) に任せる。一時ファイルへ書いてから
 * 差し替える方式なので、書いている途中で落ちても前のファイルは壊れない。CRC も見てくれる。
 *
 * テープの中身の形は `CActionInputTape` が決める。ここはファイルとの間を運ぶだけ。
 */
class CReplayFile
{
public:
	/**
	 * テープをファイルへ置く。
	 *
	 * @param Tape 置くテープ。
	 * @param Path 置き先のパス (UTF-8)。
	 * @return 置けたら true。
	 */
	static bool Save( const CActionInputTape& Tape, const FString& Path ) noexcept;

	/**
	 * ファイルからテープを戻す。
	 *
	 * @details 読めなかった場合、テープの中身は空になる。
	 * @param Path 読み元のパス (UTF-8)。
	 * @param OutTape 戻す先。
	 * @return 読めたら true。
	 */
	static bool Load( const FString& Path, CActionInputTape& OutTape ) noexcept;

private:
	/**
	 * ファイルに入っている大きさを問い合わせる。
	 *
	 * @param WidePath OS へ渡すパス。
	 * @param OutSize 入っている大きさの入れ先。
	 * @return 問い合わせられたら true。
	 */
	static bool TryQuerySize( const wchar_t* WidePath, u64& OutSize ) noexcept;
};

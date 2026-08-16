// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * バイト列を 1 ファイルへ安全に置き、読み戻す係。
 *
 * @details
 * 置き方は Engine (`acs::game::CSaveArchive`) に任せる。一時ファイルへ書いてから差し替える
 * ので、書いている途中で落ちても前のファイルは壊れない。CRC も見てくれる。
 *
 * ただし `CSaveArchive` は「大きさを知るために一度失敗させる」「パスを UTF-16 へ直す」という
 * 手順を毎回書かせる。同じ手順を各所で書き写すと、片方だけ読み込み前に入れ物を用意し忘れる、
 * といった食い違いが出る。ここへ 1 か所置く。
 *
 * 中身の意味は知らない。何を書くかは呼ぶ側が決める。
 */
class CAcsArchiveFile
{
public:
	/**
	 * バイト列をファイルへ置く。
	 *
	 * @param Path 置き先のパス (UTF-8)。
	 * @param Version 中身の形の版。読むときに一致しないと弾かれる。
	 * @param Data 先頭。
	 * @param Size 大きさ。
	 * @return 置けたら true。
	 */
	static bool Write( const FString& Path, u32 Version, const u8* Data, usize Size ) noexcept;

	/**
	 * ファイルからバイト列を読み戻す。
	 *
	 * @details 入れ物は必要な大きさへ広げてから読む。失敗した場合、入れ物は空になる。
	 * @param Path 読み元のパス (UTF-8)。
	 * @param Version 期待する版。
	 * @param OutBytes 読み先。
	 * @return 読めたら true。
	 */
	static bool Read( const FString& Path, u32 Version, TArray<u8>& OutBytes ) noexcept;
};

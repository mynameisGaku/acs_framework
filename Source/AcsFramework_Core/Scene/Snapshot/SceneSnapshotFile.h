// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"

using namespace acs;
using namespace acs::game;

/**
 * バイト列をファイルへ置き、ファイルから戻す係。
 *
 * @details
 * 置き方はエンジン (`CSaveArchive`) に任せる。一時ファイルへ書いてから置き換える方式なので、
 * 書いている途中で落ちても前のファイルは壊れない。CRC まで見てくれるので、自前で書くと
 * この安全性を落とすことになる。
 *
 * パスは UTF-8 で受け取り、OS が要求する UTF-16 へはここで直す。
 */
class CSceneSnapshotFile
{
public:
	/**
	 * バイト列をファイルへ置く。
	 *
	 * @param Path 置き先のパス (UTF-8)。
	 * @param Data 先頭。
	 * @param Size 大きさ。
	 * @return 置けたら true。
	 */
	static bool Write( const FString& Path, const u8* Data, usize Size ) noexcept;

	/**
	 * ファイルからバイト列を戻す。
	 *
	 * @details 先に大きさだけを問い合わせ、入れ物を用意してから読む。
	 * @param Path 読み元のパス (UTF-8)。
	 * @param OutBuffer 読み先。
	 * @param OutSize 読めた大きさの入れ先。
	 * @return 読めたら true。
	 */
	static bool Read( const FString& Path, CSceneSnapshotBuffer& OutBuffer, usize& OutSize ) noexcept;
};

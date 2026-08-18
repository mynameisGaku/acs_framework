// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * フォルダの中の 1 件。
 */
struct FDebugTopDirEntry
{
	/** 名前 (親までのパスは含まない)。 */
	FString Name;

	/** フォルダなら true、ファイルなら false。 */
	bool bDirectory = false;

	/** ファイルの大きさ (バイト)。フォルダなら 0。 */
	u64 Size = 0;
};


/**
 * フォルダの中身を読む。
 *
 * @details
 * 隠しファイルと「.」「..」は返さない。フォルダが先、その中で名前順に並べて返す
 * (種類が混ざると探しにくいため)。
 *
 * acs には列挙の口が無い (CFileSystem は Exists / FileSize までで、engine 自身も
 * 列挙が要る所では .cpp の中で直に OS を叩いている)。ここも同じやり方にしてある。
 * @param Path 読むフォルダ (空なら何も返さない)。
 * @param OutEntries 読めた中身の書き込み先 (呼ぶたびに空にしてから積む)。
 * @return 読めたら true。
 */
bool DebugTopReadDirectory( const FString& Path, TArray<FDebugTopDirEntry>& OutEntries );

/**
 * 使えるドライブを並べて返す。
 *
 * @details 一番上のフォルダより更に上へ上がったときに出す一覧。
 * @param OutDrives 「C:\\」のような文字列の書き込み先 (呼ぶたびに空にしてから積む)。
 */
void DebugTopReadDrives( TArray<FString>& OutDrives );

/**
 * パスを 1 段繋ぐ。
 *
 * @param Parent 親のパス。
 * @param Name 足す名前。
 * @return 繋いだパス。
 */
FString DebugTopJoinPath( const FString& Parent, const FString& Name );

/**
 * 親のパスを返す。
 *
 * @param Path 元のパス。
 * @return 親のパス (これ以上、上が無ければ空文字列)。
 */
FString DebugTopParentPath( const FString& Path );

/**
 * そのパスがフォルダかを返す。
 *
 * @param Path 確かめるパス。
 * @return フォルダなら true。
 */
bool DebugTopIsDirectory( const FString& Path );

#pragma once

#include <acs.h>

using namespace acs;

/**
 * 選ばせるものの種類。
 */
enum class EDebugTopPickKind : u8
{
	/** フォルダを選ばせる。 */
	Folder,

	/** ファイルを選ばせる。 */
	File,
};


/**
 * OS の選択ダイアログを開いてパスを選ばせる。
 *
 * @details
 * 開いている間はゲームが止まる (ダイアログが入力を持っていくため)。デバッグメニューから
 * 手で選ぶ用途なので、止まって困る場面では使わないこと。
 * @param Kind 選ばせるものの種類。
 * @param Title ダイアログの見出し。
 * @param Initial 最初に開いておく場所 (空なら OS 任せ)。
 * @param OutPath 選ばれたパスの書き込み先 (区切りは Windows 綴りのまま返る)。
 * @return 選ばれたら true。取り消されたら false。
 */
bool DebugTopPickPath( EDebugTopPickKind Kind, const FString& Title, const FString& Initial, FString& OutPath );

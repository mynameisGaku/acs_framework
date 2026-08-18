// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class ADebugTopHUD;

/**
 * メニューの現在値を、そのまま貼れるテキストにして返す。
 *
 * @details
 * 不具合を報告するときに「どの設定で起きたか」を添えるためのもの。保存ファイルと違って
 * 人が読む形なので、階層はインデントで表す。値を持たない行 (カテゴリ) は見出しとして残す。
 *
 * @param HUD 書き出す対象のメニュー。
 * @return 現在値を並べたテキスト。
 */
FString DebugTopMakeSnapshotText( const ADebugTopHUD& HUD );

/**
 * 文字列を OS のクリップボードへ載せる。
 *
 * @param Text 載せる文字列 (UTF-8)。
 * @return 載せられたら true。
 */
bool DebugTopCopyToClipboard( const FString& Text );

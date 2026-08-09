#pragma once

#include <acs.h>

#include "Debug/DebugTop/Settings/DebugTopSettingsFormat.h"

using namespace acs;

// 設定の保存先。シーンが最初に一度置き、ルートページがプロファイル切り替えで名前を作り直す。
// 2 か所で別々に持つと、片方だけ変えたときに保存と読み込みで行き先が食い違う。

/** 設定の置き場所 (実行ディレクトリからの相対パス)。無ければ保存時に作る。 */
constexpr const char* kSampleSettingsDirectory = "Saved/Debug";

/** 設定のファイル名 (拡張子は形式が決める)。 */
constexpr const char* kSampleSettingsFileName = "DebugTopSettings";

/** 設定の保存形式。メニューの Format 行から実行中に切り替えられる。 */
constexpr EDebugTopSettingsFormat kSampleSettingsFormat = EDebugTopSettingsFormat::Binary;

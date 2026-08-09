#pragma once

#include <acs.h>


using namespace acs;

// 設定のセット (プロファイル)。サンプルのルートページが使う。

/**
 * 設定のセット (プロファイル)。
 *
 * @details
 * 保存先のファイル名を差し替えるだけで、設定を丸ごと持ち替えられる。「通常」「ボス戦の
 * 検証用」「描画を軽くした状態」のように、状況ごとの設定を並行して持てる。
 */
ACS_ENUM()
enum class EDebugTopProfile : u8
{
	/** 既定のセット。 */
	Default,

	/** 検証用のセット。 */
	Test,

	/** 見た目を詰めるときのセット。 */
	Visual,
};

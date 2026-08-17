// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 素材の置き場 (`Assets`) を見つけて、相対の名前を実際の場所へ直す。
 *
 * @details
 * **置き場は `Assets` 1 つに決める。** どこに置いたか探し回らずに済むことの方が、
 * 自由に置けることより価値がある。
 *
 * 探し方は 4 通りで、上から順に見る。
 *
 * 1. 環境変数 `ACSFW_ASSETS`
 * 2. いま居るフォルダの `Assets`
 * 3. 実行ファイルの隣の `Assets`
 * 4. 実行ファイルから**上へ辿って**最初に見つかった `Assets`
 *
 * 4 が要る理由: 実行ファイルは `x64\Release` に出るので、素材を repo の根に置くと
 * 隣には無い。**IDE から実行しても、出来上がりを直接叩いても、同じように動く**ように
 * するために上へ辿る。素材を毎回コピーさせるのは手数として重い。
 *
 * 見つけた場所は 1 度だけ調べて覚える。**毎回ディスクを叩くと、置いた物の数だけ遅くなる。**
 */
class CAssetRoot
{
public:
	/**
	 * 置き場を返す (初回だけ探す)。
	 *
	 * @return 置き場の絶対パス。見つからなければ空。
	 */
	static const FString& Path() noexcept;

	/**
	 * 置き場からの相対名を、実際の場所へ直す。
	 *
	 * @details
	 * `..` を含む名前は**受け付けない**。置き場の外を指せると、配る段になって
	 * «自分の機械にしか無いファイル» を掴んでいたことに気付く。
	 *
	 * @param RelativePath 置き場からの相対名 (`Robot.fbx`、`Enemy/Slime.fbx`)。
	 * @param OutFullPath 絶対パスの受け取り先。
	 * @return 直せたら true。置き場が無い・名前が空・`..` を含むなら false。
	 */
	static bool Resolve( FStringView RelativePath, FString& OutFullPath ) noexcept;

	/**
	 * 置き場を明示して上書きする (試験と道具向け)。
	 *
	 * @param FullPath 使う置き場。空を渡すと探し直しに戻る。
	 */
	static void Override( FStringView FullPath ) noexcept;

private:
	/**
	 * 置き場を実際に探す。
	 *
	 * @param OutPath 見つけた場所の受け取り先。
	 * @return 見つかれば true。
	 */
	static bool Discover( FString& OutPath ) noexcept;
};

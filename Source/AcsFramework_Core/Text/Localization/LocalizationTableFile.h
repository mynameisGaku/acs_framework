// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/Localization/LocaleCatalog.h"
#include "AcsFramework_Core/Text/Localization/LocalizationTableParser.h"

using namespace acs;

/**
 * `Assets`配下の訳文表を読み、既存の解析処理へ渡すファイルアダプター。
 *
 * @details
 * パス解決とファイルI/Oだけを担当し、表の規則は`CLocalizationTableParser`へ委ねる。
 * UTF-8のBOMは取り除く。読み込みに失敗した場合は、入れ物を変更しない。
 */
class CLocalizationTableFile
{
public:
	/**
	 * `Assets`からの相対パスで訳文表を読み、入れ物へ足す。
	 *
	 * @param OutCatalog 読めた文を足す先。
	 * @param AssetPath `Assets`からの相対パス。絶対パスと`..`は受け付けない。
	 * @return 成功時は解析結果。パス解決、文字変換、ファイル読み込みの失敗時はエラー。
	 */
	static TResult<FLocalizationParseResult> LoadInto(
		CLocaleCatalog& OutCatalog, FStringView AssetPath ) noexcept;
};

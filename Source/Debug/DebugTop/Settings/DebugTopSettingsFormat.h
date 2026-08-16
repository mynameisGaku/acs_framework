// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>
#include "Common/Compat/AcsAnnotations.h"
#include "Common/Compat/AcsEnumReflection.h"

#include "Debug/DebugTop/Settings/DebugTopSetting.h"

using namespace acs;

/**
 * 保存ファイルの形式。
 *
 * @details
 * 番兵を持たない。個数と名前は acs::TEnumTraits から取得する。
 */
ACS_ENUM()
enum class EDebugTopSettingsFormat : u8
{
	/** 1 行 1 設定のテキスト (.txt)。手で読み書きするならこれ。 */
	Text,

	/** JSON (.json)。外部のツールやスクリプトから読ませるならこれ。 */
	Json,

	/** acs 独自のバイナリ (.acsset)。小さく速く、値の型をそのまま持ち運べる。 */
	Binary,
};


/**
 * 形式の表示名を返す。
 *
 * @details 名前は列挙子から自動で引くので、形式を足したときに書き足す場所が増えない。
 * @param Format 対象の形式。
 * @return "Text" / "Json" / "Binary"。
 */
inline FString DebugTopSettingsFormatName( EDebugTopSettingsFormat Format )
{
	const AcsFw::FEnumNameView Name = AcsFw::EnumToString( Format );
	return FString( FStringView( Name.Data, Name.Size ) );
}

/**
 * 形式に対応する拡張子を返す。
 *
 * @param Format 対象の形式。
 * @return ドットを含む拡張子 (".txt" / ".json" / ".acsset")。
 */
const char* DebugTopSettingsFormatExtension( EDebugTopSettingsFormat Format ) noexcept;

/**
 * 設定の配列を指定形式の byte 列へ書き出す。
 *
 * @details
 * キーと値の配列を指定形式の byte 列へ変換し、変換できない値や容量不足では false を返す。
 * @param Settings 書き出す設定の配列。
 * @param Format 書き出す形式。
 * @param OutBytes 書き出し先 (呼び出し前の内容は捨てられる)。
 * @return 書き出せたら true。
 */
bool DebugTopSerializeSettings( const TArray<FDebugTopSetting>& Settings, EDebugTopSettingsFormat Format, TArray<byte>& OutBytes );

/**
 * byte 列から設定の配列を復元する。
 *
 * @details
 * 形式は中身の先頭から見分けるので、拡張子や現在の設定に関係なく読める (形式を切り替えた
 * 後でも、前の形式で書いたファイルをそのまま読める)。
 * @param Data byte 列の先頭。
 * @param Size byte 数。
 * @param OutSettings 復元先 (呼び出し前の内容は捨てられる)。
 * @param OutFormat 見分けた形式の書き込み先 (nullptr 可)。
 * @return 復元できたら true。
 */
bool DebugTopDeserializeSettings( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings, EDebugTopSettingsFormat* OutFormat = nullptr );

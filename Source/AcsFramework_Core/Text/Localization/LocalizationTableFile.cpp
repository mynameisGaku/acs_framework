// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/LocalizationTableFile.h"

#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Text/StringConvert.h"

namespace
{
	/** Windowsの絶対パスとして受け付ける文字数の上限。 */
	constexpr usize kMaximumLocalizationPathLength = 1024u;

	/**
	 * UTF-8のBOMが先頭に在るかを返す。
	 *
	 * @param Bytes 読み込んだファイル全体。
	 * @return 先頭3バイトがUTF-8のBOMならtrue。
	 */
	bool HasUtf8Bom( const TArray<byte>& Bytes ) noexcept
	{
		return Bytes.Num() >= 3u
			&& Bytes[0] == static_cast<byte>( 0xefu )
			&& Bytes[1] == static_cast<byte>( 0xbbu )
			&& Bytes[2] == static_cast<byte>( 0xbfu );
	}
}


TResult<FLocalizationParseResult> CLocalizationTableFile::LoadInto(
	CLocaleCatalog& OutCatalog, FStringView AssetPath ) noexcept
{
	/** `Assets`から解決した実際のファイルパス。 */
	FString FullPath;
	if ( !CAssetRoot::Resolve( AssetPath, FullPath ) )
	{
		return ACS_ERR( Asset, 1u, "訳文表のパスをAssets配下へ解決できません" );
	}

	/** `CFileSystem`へ渡すWindows形式のパス。 */
	wchar_t WidePath[kMaximumLocalizationPathLength] = {};
	if ( !AcsToWide( FullPath, WidePath, kMaximumLocalizationPathLength ) )
	{
		return ACS_ERR( Asset, 2u, "訳文表のパスをWindowsパスへ変換できません" );
	}

	/** ファイルから読み込んだ未加工のバイト列。 */
	TResult<TArray<byte>> Read = CFileSystem::ReadAllBytes( WidePath );
	if ( Read.IsErr() ) return Read.Error();

	/** 解析対象となるファイル全体。 */
	const TArray<byte>& Bytes = Read.Value();
	if ( Bytes.Num() == 0u ) return FLocalizationParseResult{};

	/** BOMを除いた本文の開始位置。 */
	const usize TextOffset = HasUtf8Bom( Bytes ) ? 3u : 0u;
	/** UTF-8本文の先頭。長さはFStringViewで明示するためNUL終端は不要。 */
	const char* const Text = reinterpret_cast<const char*>( Bytes.GetData() + TextOffset );
	return CLocalizationTableParser::ParseInto(
		OutCatalog, FStringView( Text, Bytes.Num() - TextOffset ) );
}

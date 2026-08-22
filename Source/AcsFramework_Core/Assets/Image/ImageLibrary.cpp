// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Image/ImageLibrary.h"

#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Text/StringConvert.h"

namespace
{
	/** ACSの標準画像ローダへ登録されている拡張子。 */
	constexpr const char* kSupportedExtensions[] =
	{
		"png", "jpg", "jpeg", "bmp", "tga", "gif", "hdr", "pic", "pnm", "ppm", "pgm", "psd"
	};

	/** UTF-16終端を含めて変換できる最大パス長。 */
	constexpr usize kMaximumPathLength = 1024u;

	/** ASCII英大文字を小文字へ直し、それ以外はそのまま返す。 */
	char ToLowerAscii( char Value ) noexcept
	{
		return Value >= 'A' && Value <= 'Z' ? static_cast<char>( Value - 'A' + 'a' ) : Value;
	}

	/** 最後の区切りより後ろにある拡張子を返す。 */
	FStringView ExtensionOf( FStringView Path ) noexcept
	{
		usize Index = Path.Size();
		while ( Index > 0u )
		{
			--Index;
			const char Value = Path[Index];
			if ( Value == '\\' || Value == '/' ) break;
			if ( Value == '.' ) return FStringView( Path.Data() + Index + 1u, Path.Size() - Index - 1u );
		}
		return FStringView();
	}
}


bool CImageLibrary::IsSupported( FStringView RelativePath ) noexcept
{
	const FStringView Extension = ExtensionOf( RelativePath );
	if ( Extension.Size() == 0u ) return false;

	for ( const char* const Known : kSupportedExtensions )
	{
		usize Index = 0u;
		while ( Index < Extension.Size() && Known[Index] != '\0'
			&& ToLowerAscii( Extension[Index] ) == Known[Index] ) ++Index;

		if ( Index == Extension.Size() && Known[Index] == '\0' ) return true;
	}

	return false;
}


TSharedPtr<AAsset> CImageLibrary::Load( FStringView RelativePath ) noexcept
{
	const char* const PathText = RelativePath.Data() != nullptr ? RelativePath.Data() : "";
	if ( m_Registry == nullptr )
	{
		ACS_LOG_WARN( "ImageLibrary: 登録簿が渡されていません (Bindを呼んでください)" );
		return TSharedPtr<AAsset>();
	}

	if ( !IsSupported( RelativePath ) )
	{
		ACS_LOG_WARN( "ImageLibrary: 対応していない画像形式です: %.*s",
			static_cast<int>( RelativePath.Size() ), PathText );
		return TSharedPtr<AAsset>();
	}

	FString FullPath;
	if ( !CAssetRoot::Resolve( RelativePath, FullPath ) )
	{
		ACS_LOG_WARN( "ImageLibrary: 置き場から辿れません (`..`や絶対パスは不可): %.*s",
			static_cast<int>( RelativePath.Size() ), PathText );
		return TSharedPtr<AAsset>();
	}

	wchar_t Wide[kMaximumPathLength] = {};
	if ( !AcsToWide( FullPath, Wide, kMaximumPathLength ) )
	{
		ACS_LOG_WARN( "ImageLibrary: パスが長すぎます: %s", FullPath.Data() );
		return TSharedPtr<AAsset>();
	}

	if ( !CFileSystem::Exists( Wide ) )
	{
		ACS_LOG_WARN( "ImageLibrary: ファイルがありません: %s", FullPath.Data() );
		return TSharedPtr<AAsset>();
	}

	TResult<TSharedPtr<AAsset>> Loaded = m_Registry->Load( Wide );
	if ( Loaded.IsErr() || !Loaded.Value() || Loaded.Value()->Type() != AImageAsset::StaticType() )
	{
		ACS_LOG_WARN( "ImageLibrary: 画像として読み込めません: %s", FullPath.Data() );
		return TSharedPtr<AAsset>();
	}

	return Loaded.Value();
}

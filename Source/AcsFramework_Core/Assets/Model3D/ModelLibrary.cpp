// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Model3D/ModelLibrary.h"

#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Text/StringConvert.h"

namespace
{
	/**
	 * 読める拡張子。**先頭が薦める形式**。
	 *
	 * @details
	 * どれも ACS 側にローダが在るものだけ。ここに無いものを足しても、登録簿が読めない。
	 */
	constexpr const char* kSupportedExtensions[] = { "fbx", "gltf", "glb", "obj" };

	/** パスの上限。これを超える名前は素材の置き方が壊れている。 */
	constexpr usize kMaximumPathLength = 1024u;


	/**
	 * 大文字小文字を無視して 1 文字を小文字にする。
	 *
	 * @param Value 元の文字。
	 * @return 小文字。
	 */
	char ToLower( char Value ) noexcept
	{
		return ( Value >= 'A' && Value <= 'Z' ) ? static_cast<char>( Value - 'A' + 'a' ) : Value;
	}


	/**
	 * 最後の `.` から後ろを取り出す。
	 *
	 * @param Path 調べる名前。
	 * @return 拡張子 (`.` を含まない)。無ければ空。
	 */
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


bool CModelLibrary::IsSupported( FStringView RelativePath ) noexcept
{
	const FStringView Extension = ExtensionOf( RelativePath );
	if ( Extension.Size() == 0u ) return false;

	for ( const char* const Known : kSupportedExtensions )
	{
		usize Index = 0u;
		while ( Index < Extension.Size() && Known[Index] != '\0'
			&& ToLower( Extension[Index] ) == Known[Index] ) ++Index;

		if ( Index == Extension.Size() && Known[Index] == '\0' ) return true;
	}

	return false;
}


TSharedPtr<AAsset> CModelLibrary::Load( FStringView RelativePath ) noexcept
{
	if ( m_Registry == nullptr )
	{
		ACS_LOG_WARN( "ModelLibrary: 登録簿が渡されていません (Bind を呼んでください)" );
		return TSharedPtr<AAsset>();
	}

	if ( !IsSupported( RelativePath ) )
	{
		// 拡張子で弾いておくと、«読めないのか、置き場に無いのか» を混ぜずに済む。
		ACS_LOG_WARN( "ModelLibrary: 対応していない形式です (fbx / gltf / glb / obj): %.*s",
			static_cast<int>( RelativePath.Size() ), RelativePath.Data() );
		return TSharedPtr<AAsset>();
	}

	FString FullPath;
	if ( !CAssetRoot::Resolve( RelativePath, FullPath ) )
	{
		ACS_LOG_WARN( "ModelLibrary: 置き場から辿れません (`..` や絶対パスは不可): %.*s",
			static_cast<int>( RelativePath.Size() ), RelativePath.Data() );
		return TSharedPtr<AAsset>();
	}

	wchar_t Wide[kMaximumPathLength] = {};
	if ( !AcsToWide( FullPath, Wide, kMaximumPathLength ) )
	{
		ACS_LOG_WARN( "ModelLibrary: パスが長すぎます: %s", FullPath.Data() );
		return TSharedPtr<AAsset>();
	}

	if ( !CFileSystem::Exists( Wide ) )
	{
		// **これがいちばん多い失敗。** 探した «場所» まで出さないと直せない。
		ACS_LOG_WARN( "ModelLibrary: ファイルがありません: %s", FullPath.Data() );
		return TSharedPtr<AAsset>();
	}

	TResult<TSharedPtr<AAsset>> Loaded = m_Registry->Load( Wide );
	if ( Loaded.IsErr() )
	{
		ACS_LOG_WARN( "ModelLibrary: 読み込みに失敗しました: %s", FullPath.Data() );
		return TSharedPtr<AAsset>();
	}

	return Loaded.Value();
}

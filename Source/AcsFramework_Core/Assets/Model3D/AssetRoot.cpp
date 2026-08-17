// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"

#include "AcsFramework_Core/Text/StringConvert.h"

#include <cstdlib>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
	/** 置き場のフォルダ名。ここは動かさない。 */
	constexpr const char* kAssetsFolderName = "Assets";

	/** 置き場を明示するための環境変数。 */
	constexpr const wchar_t* kAssetsEnvironmentName = L"ACSFW_ASSETS";

	/** 上へ辿る段数の上限。無制限にすると、置き場が無いときにドライブの根まで登る。 */
	constexpr u32 kMaximumParentSteps = 8u;

	/** 覚えた置き場。空なら «まだ探していない» か «見つからなかった»。 */
	FString g_ResolvedRoot;

	/** 一度でも探したか。見つからなかったことも覚えて、毎回探し直さないようにする。 */
	bool g_Searched = false;

	/** 明示された置き場。空なら通常の探し方に従う。 */
	FString g_Override;


	/**
	 * 広い文字のパスを FString へ移す。
	 *
	 * @param Wide 元のパス。
	 * @param OutPath 受け取り先。
	 * @return 移せたら true。
	 */
	bool ToUtf8Path( const wchar_t* Wide, FString& OutPath ) noexcept
	{
		if ( Wide == nullptr || Wide[0] == L'\0' ) return false;

		return AcsToUtf8( Wide, OutPath );
	}


	/**
	 * `<Base>\Assets` がフォルダとして在るかを見る。
	 *
	 * @param Base 調べる親。
	 * @param OutPath 在ったときの受け取り先。
	 * @return 在れば true。
	 */
	bool TryAssetsUnder( const FString& Base, FString& OutPath ) noexcept
	{
		if ( Base.IsEmpty() ) return false;

		FString Candidate = Base;
		if ( Candidate[Candidate.Size() - 1u] != '\\' && Candidate[Candidate.Size() - 1u] != '/' )
			Candidate.Append( '\\' );
		Candidate.Append( FStringView( kAssetsFolderName ) );

		wchar_t Wide[1024] = {};
		if ( !AcsToWide( Candidate, Wide, 1024u ) ) return false;
		if ( !CFileSystem::DirectoryExists( Wide ) ) return false;

		OutPath = Candidate;
		return true;
	}


	/**
	 * 末尾のフォルダを 1 つ落とす。
	 *
	 * @param Path 対象。
	 * @return 落とせたら true。根まで来ていたら false。
	 */
	bool DropLastComponent( FString& Path ) noexcept
	{
		usize Index = Path.Size();
		while ( Index > 0u && Path[Index - 1u] != '\\' && Path[Index - 1u] != '/' ) --Index;
		if ( Index <= 1u ) return false;

		// 区切りも落とす。ただし "C:\" のような根は残す。
		--Index;
		if ( Index > 0u && Path[Index - 1u] == ':' ) return false;

		FString Trimmed;
		for ( usize Position = 0u; Position < Index; ++Position ) Trimmed.Append( Path[Position] );
		Path = Trimmed;
		return true;
	}


	/**
	 * いま居るフォルダを返す。
	 *
	 * @param OutPath 受け取り先。
	 * @return 取れたら true。
	 */
	bool CurrentDirectory( FString& OutPath ) noexcept
	{
		wchar_t Wide[1024] = {};
		const DWORD Written = ::GetCurrentDirectoryW( 1024u, Wide );
		if ( Written == 0u || Written >= 1024u ) return false;

		return ToUtf8Path( Wide, OutPath );
	}


	/**
	 * 実行ファイルの在るフォルダを返す。
	 *
	 * @param OutPath 受け取り先。
	 * @return 取れたら true。
	 */
	bool ExecutableDirectory( FString& OutPath ) noexcept
	{
		wchar_t Wide[1024] = {};
		const DWORD Written = ::GetModuleFileNameW( nullptr, Wide, 1024u );
		if ( Written == 0u || Written >= 1024u ) return false;

		FString Full;
		if ( !ToUtf8Path( Wide, Full ) ) return false;
		if ( !DropLastComponent( Full ) ) return false;

		OutPath = Full;
		return true;
	}
}


bool CAssetRoot::Discover( FString& OutPath ) noexcept
{
	// 1) 環境変数。試験や、素材を別の場所に置いた配布で使う。
	{
		wchar_t Wide[1024] = {};
		const DWORD Written = ::GetEnvironmentVariableW( kAssetsEnvironmentName, Wide, 1024u );
		if ( Written != 0u && Written < 1024u && CFileSystem::DirectoryExists( Wide ) )
		{
			if ( ToUtf8Path( Wide, OutPath ) ) return true;
		}
	}

	// 2) いま居るフォルダ。
	FString Current;
	if ( CurrentDirectory( Current ) && TryAssetsUnder( Current, OutPath ) ) return true;

	// 3) 実行ファイルの隣と、4) そこから上へ。
	//    x64\Release に出た実行ファイルから repo の根の Assets へ届かせるため。
	FString Directory;
	if ( !ExecutableDirectory( Directory ) ) return false;

	for ( u32 Step = 0u; Step <= kMaximumParentSteps; ++Step )
	{
		if ( TryAssetsUnder( Directory, OutPath ) ) return true;
		if ( !DropLastComponent( Directory ) ) break;
	}

	return false;
}


const FString& CAssetRoot::Path() noexcept
{
	if ( !g_Override.IsEmpty() ) return g_Override;

	if ( !g_Searched )
	{
		g_Searched = true;
		FString Found;
		if ( Discover( Found ) )
		{
			g_ResolvedRoot = Found;
			ACS_LOG_INFO( "Assets: %s", g_ResolvedRoot.Data() );
		}
		else
		{
			// 黙って «モデルが出ない» になるのが一番困る。探した事実を残す。
			ACS_LOG_WARN( "Assets: 'Assets' フォルダが見つかりません。"
				"ACSFW_ASSETS を設定するか、実行ファイルの上の階層に置いてください" );
		}
	}

	return g_ResolvedRoot;
}


bool CAssetRoot::Resolve( FStringView RelativePath, FString& OutFullPath ) noexcept
{
	if ( RelativePath.Data() == nullptr || RelativePath.Size() == 0u ) return false;

	// 置き場の外を指せると、配る段になって «自分の機械にしか無いファイル» を掴んでいた
	// ことに気付く。絶対パスと `..` は受け付けない。
	for ( usize Index = 0u; Index + 1u < RelativePath.Size(); ++Index )
	{
		if ( RelativePath[Index] == '.' && RelativePath[Index + 1u] == '.' ) return false;
	}
	if ( RelativePath[0] == '\\' || RelativePath[0] == '/' ) return false;
	if ( RelativePath.Size() >= 2u && RelativePath[1] == ':' ) return false;

	const FString& Root = Path();
	if ( Root.IsEmpty() ) return false;

	FString Full = Root;
	Full.Append( '\\' );
	Full.Append( RelativePath );

	OutFullPath = Full;
	return true;
}


void CAssetRoot::Override( FStringView FullPath ) noexcept
{
	g_Override = FString( FullPath );
}

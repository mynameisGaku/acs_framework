// SPDX-License-Identifier: Apache-2.0
#include "GameSettingsStore.h"

namespace
{
	/**
	 * ファイルパスの親フォルダを再帰的に作る。
	 *
	 * @param FilePath 保存先のOS形式パス。
	 * @return 親が無いパス、既存の親、作成成功ならtrue。
	 */
	bool TryEnsureParentDirectory( const wchar_t* FilePath ) noexcept
	{
		if ( FilePath == nullptr || FilePath[0] == L'\0' ) return false;

		usize LastSeparator = static_cast<usize>( -1 );
		usize Length = 0u;
		for ( ; FilePath[Length] != L'\0'; ++Length )
		{
			if ( CFileSystem::IsPathSeparator( FilePath[Length] ) ) LastSeparator = Length;
		}

		if ( LastSeparator == static_cast<usize>( -1 ) || LastSeparator == 0u ) return true;
		if ( LastSeparator == 2u && FilePath[1] == L':' ) return true;

		TArray<wchar_t> Parent;
		if ( !Parent.TryReserve( LastSeparator + 1u ) ) return false;
		Parent.SetNum( LastSeparator + 1u );
		for ( usize Index = 0u; Index < LastSeparator; ++Index ) Parent[Index] = FilePath[Index];
		Parent[LastSeparator] = L'\0';

		if ( CFileSystem::DirectoryExists( Parent.GetData() ) ) return true;
		return CFileSystem::CreateDirectory( Parent.GetData() ).IsOk();
	}
}

const char* FGameSettingsStore::Intern( const FString& Text )
{
	// 同じ内容を持つ既存領域を探すための添字。
	for ( usize Index = 0; Index < m_Interned.Num(); ++Index )
	{
		if ( m_Interned[Index] && *m_Interned[Index] == Text ) return m_Interned[Index]->Data();
	}

	// 設定値の寿命まで保持する新しい文字列領域。
	TUniquePtr<FString> Owned = MakeUnique<FString>( Text );
	// 設定の入れ物へ渡す安定した文字列の先頭。
	const char* const Data = Owned->Data();
	m_Interned.Add( Move( Owned ) );
	return Data;
}

void FGameSettingsStore::SetFloat( const FString& Key, f32 Value )
{
	m_Settings.SetF32( Intern( Key ), Value );
}

void FGameSettingsStore::SetInt( const FString& Key, i32 Value )
{
	m_Settings.SetI32( Intern( Key ), Value );
}

void FGameSettingsStore::SetBool( const FString& Key, bool bValue )
{
	m_Settings.SetBool( Intern( Key ), bValue );
}

void FGameSettingsStore::SetString( const FString& Key, const FString& Value )
{
	m_Settings.SetString( Intern( Key ), Intern( Value ) );
}

f32 FGameSettingsStore::GetFloat( const FString& Key, f32 DefaultValue ) const
{
	return m_Settings.GetF32( Key.Data(), DefaultValue );
}

i32 FGameSettingsStore::GetInt( const FString& Key, i32 DefaultValue ) const
{
	return m_Settings.GetI32( Key.Data(), DefaultValue );
}

bool FGameSettingsStore::GetBool( const FString& Key, bool bDefaultValue ) const
{
	return m_Settings.GetBool( Key.Data(), bDefaultValue );
}

FString FGameSettingsStore::GetString( const FString& Key, const FString& DefaultValue ) const
{
	// 設定の入れ物が返した非所有文字列。
	const char* const Found = m_Settings.GetString( Key.Data(), nullptr );
	if ( Found == nullptr ) return DefaultValue;

	return FString( Found );
}

TResult<void> FGameSettingsStore::SaveTo( const wchar_t* FilePath ) noexcept
{
	// ACSのCSettingsは同じフォルダ内の一時ファイルへ安全に書くが、親フォルダは作らない。
	// 作成に失敗してもSave本体を呼び、具体的なI/Oエラーを呼び出し側へ返す。
	( void )TryEnsureParentDirectory( FilePath );
	return m_Settings.Save( FilePath );
}

TResult<void> FGameSettingsStore::LoadFrom( const wchar_t* FilePath ) noexcept
{
	return m_Settings.Load( FilePath );
}

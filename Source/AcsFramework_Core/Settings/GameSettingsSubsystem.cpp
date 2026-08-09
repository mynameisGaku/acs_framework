#include "GameSettingsSubsystem.h"

#include "AcsFramework_Core/Text/StringConvert.h"

// GameInstance スコープへ登録する。シーンを跨いでも同じ設定を指す。
ACS_REGISTER_SUBSYSTEM( CGameSettingsSubsystem, ESubsystemScope::GameInstance )


bool CGameSettingsSubsystem::Configure( const FString& FilePath )
{
	m_FilePath = FilePath;
	m_bDirty = false;
	m_IdleSeconds = 0.0f;

	return Load();
}


const char* CGameSettingsSubsystem::Intern( const FString& Text )
{
	for ( usize Index = 0; Index < m_Interned.Num(); ++Index )
	{
		if ( m_Interned[Index] && *m_Interned[Index] == Text ) return m_Interned[Index]->Data();
	}

	TUniquePtr<FString> Owned = MakeUnique<FString>( Text );
	const char* const Data = Owned->Data();
	m_Interned.Add( Move( Owned ) );
	return Data;
}


void CGameSettingsSubsystem::MarkDirty() noexcept
{
	m_bDirty = true;
	m_IdleSeconds = 0.0f;
}


void CGameSettingsSubsystem::SetFloat( const FString& Key, f32 Value )
{
	m_Settings.SetF32( Intern( Key ), Value );
	MarkDirty();
}


void CGameSettingsSubsystem::SetInt( const FString& Key, i32 Value )
{
	m_Settings.SetI32( Intern( Key ), Value );
	MarkDirty();
}


void CGameSettingsSubsystem::SetBool( const FString& Key, bool bValue )
{
	m_Settings.SetBool( Intern( Key ), bValue );
	MarkDirty();
}


void CGameSettingsSubsystem::SetString( const FString& Key, const FString& Value )
{
	m_Settings.SetString( Intern( Key ), Intern( Value ) );
	MarkDirty();
}


f32 CGameSettingsSubsystem::GetFloat( const FString& Key, f32 DefaultValue ) const
{
	return m_Settings.GetF32( Key.Data(), DefaultValue );
}


i32 CGameSettingsSubsystem::GetInt( const FString& Key, i32 DefaultValue ) const
{
	return m_Settings.GetI32( Key.Data(), DefaultValue );
}


bool CGameSettingsSubsystem::GetBool( const FString& Key, bool bDefaultValue ) const
{
	return m_Settings.GetBool( Key.Data(), bDefaultValue );
}


FString CGameSettingsSubsystem::GetString( const FString& Key, const FString& DefaultValue ) const
{
	// 返ってくるのは中の領域を指すポインタ。次の書き換えで指し先が変わるので写して返す。
	const char* const Found = m_Settings.GetString( Key.Data(), nullptr );
	if ( Found == nullptr ) return DefaultValue;

	return FString( Found );
}


bool CGameSettingsSubsystem::Save()
{
	if ( m_FilePath.IsEmpty() ) return false;

	TArray<wchar_t> Wide;
	if ( !AcsToWide( m_FilePath, Wide ) ) return false;

	const auto Result = m_Settings.Save( Wide.GetData() );
	if ( !Result.IsOk() )
	{
		// 黙って書けていないと、次の起動で «設定が戻る» としか分からない。1 度だけ知らせる。
		if ( !m_bSaveWarned )
		{
			m_bSaveWarned = true;
			ACS_LOG_WARN( "CGameSettingsSubsystem: 設定を書けなかった -> %s", m_FilePath.Data() );
		}
		return false;
	}

	m_bDirty = false;
	m_IdleSeconds = 0.0f;
	m_bSaveWarned = false;
	return true;
}


bool CGameSettingsSubsystem::Load()
{
	if ( m_FilePath.IsEmpty() ) return false;

	TArray<wchar_t> Wide;
	if ( !AcsToWide( m_FilePath, Wide ) ) return false;

	// ファイルが無いのは初回起動。読めなかっただけで、持っている値はそのまま使える。
	if ( !CFileSystem::Exists( Wide.GetData() ) ) return false;

	const bool bLoaded = m_Settings.Load( Wide.GetData() ).IsOk();
	if ( bLoaded )
	{
		m_bDirty = false;
		m_IdleSeconds = 0.0f;
	}
	return bLoaded;
}


void CGameSettingsSubsystem::Update( f32 DeltaSeconds ) noexcept
{
	if ( !m_bDirty ) return;

	// 手が止まるまで書かない。スライダーを動かしている間ずっと書き続けないため。
	m_IdleSeconds += DeltaSeconds;
	if ( m_IdleSeconds < m_AutoSaveDelaySeconds ) return;

	Save();
}


void CGameSettingsSubsystem::OnDeinitialize() noexcept
{
	if ( !m_bDirty ) return;

	( void )Save();
}

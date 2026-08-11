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


void CGameSettingsSubsystem::MarkDirty() noexcept
{
	m_bDirty = true;
	m_IdleSeconds = 0.0f;
}


void CGameSettingsSubsystem::SetFloat( const FString& Key, f32 Value )
{
	m_Store.SetFloat( Key, Value );
	MarkDirty();
}


void CGameSettingsSubsystem::SetInt( const FString& Key, i32 Value )
{
	m_Store.SetInt( Key, Value );
	MarkDirty();
}


void CGameSettingsSubsystem::SetBool( const FString& Key, bool bValue )
{
	m_Store.SetBool( Key, bValue );
	MarkDirty();
}


void CGameSettingsSubsystem::SetString( const FString& Key, const FString& Value )
{
	m_Store.SetString( Key, Value );
	MarkDirty();
}


f32 CGameSettingsSubsystem::GetFloat( const FString& Key, f32 DefaultValue ) const
{
	return m_Store.GetFloat( Key, DefaultValue );
}


i32 CGameSettingsSubsystem::GetInt( const FString& Key, i32 DefaultValue ) const
{
	return m_Store.GetInt( Key, DefaultValue );
}


bool CGameSettingsSubsystem::GetBool( const FString& Key, bool bDefaultValue ) const
{
	return m_Store.GetBool( Key, bDefaultValue );
}


FString CGameSettingsSubsystem::GetString( const FString& Key, const FString& DefaultValue ) const
{
	return m_Store.GetString( Key, DefaultValue );
}


bool CGameSettingsSubsystem::Save()
{
	if ( m_FilePath.IsEmpty() ) return false;

	// 設定保存先へ渡す広い文字のパス。
	TArray<wchar_t> Wide;
	if ( !AcsToWide( m_FilePath, Wide ) ) return false;

	// 設定値を書き込んだ結果。
	const auto Result = m_Store.SaveTo( Wide.GetData() );
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

	// 設定読込元へ渡す広い文字のパス。
	TArray<wchar_t> Wide;
	if ( !AcsToWide( m_FilePath, Wide ) ) return false;

	// ファイルが無いのは初回起動。読めなかっただけで、持っている値はそのまま使える。
	if ( !CFileSystem::Exists( Wide.GetData() ) ) return false;

	// 現在値を置き換えられたかを示す読込結果。
	const bool bLoaded = m_Store.LoadFrom( Wide.GetData() ).IsOk();
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

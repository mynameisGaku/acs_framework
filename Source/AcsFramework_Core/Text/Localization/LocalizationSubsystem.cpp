// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/LocalizationSubsystem.h"

#include "AcsFramework_Core/Text/Localization/TextFormatter.h"

ACS_REGISTER_SUBSYSTEM( CLocalizationSubsystem, ESubsystemScope::GameInstance )

void CLocalizationSubsystem::OnDeinitialize() noexcept
{
	// 相手より先に消えることがあるので、参照を残さない。
	m_Broadcaster.Clear();
}


bool CLocalizationSubsystem::RegisterText( ELocale Locale, const FString& Key, const FString& Text ) noexcept
{
	return m_Catalog.Register( Locale, Key, Text );
}


FLocalizationParseResult CLocalizationSubsystem::LoadTable( FStringView Text ) noexcept
{
	const FLocalizationParseResult Result = CLocalizationTableParser::ParseInto( m_Catalog, Text );

	// 黙って落とさない。翻訳が出ない原因のほとんどはここ。
	if ( Result.bMissingLocaleHeader )
	{
		ACS_LOG_WARN( "CLocalizationSubsystem: 言語の見出し ([ja] など) より前に書かれた行がある" );
	}
	if ( Result.Skipped != 0u )
	{
		ACS_LOG_WARN( "CLocalizationSubsystem: 表の %zu 行を読めずに飛ばした (%zu 行は読めた)",
			Result.Skipped, Result.Registered );
	}

	return Result;
}


FString CLocalizationSubsystem::GetText( const FString& Key ) const noexcept
{
	const char* const Found = m_Catalog.Find( Key );

	FString Text;
	if ( Found != nullptr ) Text.TryAppend( FStringView( Found ) );

	return Text;
}


FString CLocalizationSubsystem::FormatText( const FString& Key, const FTextArgument* Arguments,
	usize ArgumentCount ) const noexcept
{
	const char* const Found = m_Catalog.Find( Key );
	if ( Found == nullptr ) return FString();

	return CTextFormatter::Format( FStringView( Found ), Arguments, ArgumentCount );
}


bool CLocalizationSubsystem::HasText( const FString& Key ) const noexcept
{
	return m_Catalog.Has( Key );
}


void CLocalizationSubsystem::SetLocale( ELocale Locale ) noexcept
{
	if ( m_Catalog.GetLocale() == Locale ) return;

	m_Catalog.SetLocale( Locale );
	m_Broadcaster.Broadcast( Locale );
}


ELocale CLocalizationSubsystem::GetLocale() const noexcept
{
	return m_Catalog.GetLocale();
}


usize CLocalizationSubsystem::KeyCount( ELocale Locale ) const noexcept
{
	return m_Catalog.KeyCount( Locale );
}


bool CLocalizationSubsystem::AddLocaleListener( ILocaleChangeListener& Listener ) noexcept
{
	return m_Broadcaster.Add( Listener );
}


void CLocalizationSubsystem::RemoveLocaleListener( ILocaleChangeListener& Listener ) noexcept
{
	m_Broadcaster.Remove( Listener );
}


void CLocalizationSubsystem::ClearTexts() noexcept
{
	m_Catalog.Clear();
}

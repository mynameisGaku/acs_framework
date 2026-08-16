// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/LocaleCatalog.h"

bool CLocaleCatalog::Register( ELocale Locale, const FString& Key, const FString& Text ) noexcept
{
	if ( Key.IsEmpty() ) return false;

	// 辞書は指すだけなので、先に写しを作る。どちらか片方でも写せなければ登録しない。
	const char* const StableKey = m_Strings.Intern( Key );
	if ( StableKey == nullptr ) return false;

	const char* const StableText = m_Strings.Intern( Text );
	if ( StableText == nullptr ) return false;

	m_Director.RegisterString( Locale, StableKey, StableText );

	return true;
}


const char* CLocaleCatalog::Find( const FString& Key ) const noexcept
{
	return m_Director.Get( Key.Data() );
}


const char* CLocaleCatalog::FindForLocale( ELocale Locale, const FString& Key ) const noexcept
{
	return m_Director.GetForLocale( Locale, Key.Data() );
}


bool CLocaleCatalog::Has( const FString& Key ) const noexcept
{
	return m_Director.Has( Key.Data() );
}


void CLocaleCatalog::SetLocale( ELocale Locale ) noexcept
{
	m_Director.SetLocale( Locale );
}


ELocale CLocaleCatalog::GetLocale() const noexcept
{
	return m_Director.CurrentLocale();
}


usize CLocaleCatalog::KeyCount( ELocale Locale ) const noexcept
{
	return static_cast<usize>( m_Director.KeyCount( Locale ) );
}


void CLocaleCatalog::Clear() noexcept
{
	// 順番が要る。辞書が指すのをやめてから、指されていた実体を捨てる。
	m_Director.Clear();
	m_Strings.Clear();
}

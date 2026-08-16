// SPDX-License-Identifier: Apache-2.0
#include "DebugTopSettingsPath.h"

#include <cstdlib>

namespace
{
	/** パスの区切り。 */
	constexpr char kSeparator = '/';

	/** ファイル名を空にしたときへ戻す既定名。 */
	constexpr const char* kDefaultFileName = "DebugTopSettings";

	/** UTF-16 の代用対を作る境界。 */
	constexpr u32 kSurrogateBase = 0x10000u;
}


void CDebugTopSettingsPath::SetDirectory( const FString& Directory )
{
	m_Directory = Directory;

	// 末尾の区切りは組み立て側で足すので、ここで落としておく。
	while ( !m_Directory.IsEmpty() )
	{
		const char Last = m_Directory[m_Directory.Size() - 1];
		if ( Last != '/' && Last != '\\' ) break;

		m_Directory = FString( FStringView( m_Directory.Data(), m_Directory.Size() - 1 ) );
	}
}

void CDebugTopSettingsPath::SetFileName( const FString& FileName )
{
	m_FileName = FileName.IsEmpty() ? FString( kDefaultFileName ) : FileName;
}

void CDebugTopSettingsPath::SetFormat( EDebugTopSettingsFormat Format ) noexcept
{
	if ( !AcsFw::IsValidEnum( Format ) ) return;

	m_Format = Format;
}

FString CDebugTopSettingsPath::Build() const
{
	FString Path;
	if ( !m_Directory.IsEmpty() )
	{
		Path.Append( m_Directory.View() );
		Path.Append( kSeparator );
	}
	Path.Append( m_FileName.View() );
	Path.Append( DebugTopSettingsFormatExtension( m_Format ) );
	return Path;
}

FString CDebugTopSettingsPath::BuildAbsolute() const
{
	const FString Path = Build();

	TArray<wchar_t> WidePath;
	if ( !DebugTopToWidePath( Path, WidePath ) ) return Path;

	// 相対パスのままではどこへ書いたか分からないので、実行ディレクトリ基準で解決して返す。
	wchar_t* const Full = _wfullpath( nullptr, WidePath.GetData(), 0 );
	if ( Full == nullptr ) return Path;

	FString Absolute;
	for ( const wchar_t* Cursor = Full; *Cursor != L'\0'; ++Cursor )
	{
		// ログへ出すだけなので、ASCII 以外はそのまま UTF-8 へ畳んでおく。
		const u32 CodePoint = static_cast<u32>( *Cursor );
		if ( CodePoint < 0x80u )
		{
			Absolute.Append( static_cast<char>( CodePoint ) );
		}
		else if ( CodePoint < 0x800u )
		{
			Absolute.Append( static_cast<char>( 0xC0u | ( CodePoint >> 6 ) ) );
			Absolute.Append( static_cast<char>( 0x80u | ( CodePoint & 0x3Fu ) ) );
		}
		else
		{
			Absolute.Append( static_cast<char>( 0xE0u | ( CodePoint >> 12 ) ) );
			Absolute.Append( static_cast<char>( 0x80u | ( ( CodePoint >> 6 ) & 0x3Fu ) ) );
			Absolute.Append( static_cast<char>( 0x80u | ( CodePoint & 0x3Fu ) ) );
		}
	}

	::free( Full );
	return Absolute;
}


bool DebugTopToWidePath( const FString& Path, TArray<wchar_t>& OutPath )
{
	OutPath.Reset();
	if ( Path.IsEmpty() ) return false;

	const char* Cursor = Path.Data();
	while ( true )
	{
		const u32 CodePoint = DecodeUtf8( &Cursor );
		if ( CodePoint == 0 ) break;

		if ( CodePoint < kSurrogateBase )
		{
			OutPath.Add( static_cast<wchar_t>( CodePoint ) );
			continue;
		}

		// BMP の外は代用対で表す。
		const u32 Shifted = CodePoint - kSurrogateBase;
		OutPath.Add( static_cast<wchar_t>( 0xD800u + ( Shifted >> 10 ) ) );
		OutPath.Add( static_cast<wchar_t>( 0xDC00u + ( Shifted & 0x3FFu ) ) );
	}

	OutPath.Add( L'\0' );
	return true;
}

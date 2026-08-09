#include "DebugTopSettingsJson.h"

#include <cstdlib>

#include "Debug/DebugTop/Settings/DebugTopSettingsBytes.h"

namespace
{
	/**
	 * JSON の文字列として安全な形へ変換する。
	 *
	 * @param Text 変換元。
	 * @return 引用符・逆斜線・制御文字を退避した文字列。
	 */
	FString EscapeJson( const FString& Text )
	{
		FString Escaped;
		for ( usize Index = 0; Index < Text.Size(); ++Index )
		{
			const char Character = Text[Index];
			switch ( Character )
			{
			case '"':  Escaped.Append( "\\\"" ); break;
			case '\\': Escaped.Append( "\\\\" ); break;
			case '\n': Escaped.Append( "\\n" );  break;
			case '\r': Escaped.Append( "\\r" );  break;
			case '\t': Escaped.Append( "\\t" );  break;
			default:
				if ( static_cast<u8>( Character ) < 0x20u )
				{
					Escaped.AppendFormat( "\\u%04x", static_cast<u32>( static_cast<u8>( Character ) ) );
				}
				else
				{
					Escaped.Append( Character );
				}
				break;
			}
		}
		return Escaped;
	}

	/**
	 * JSON の退避を元へ戻す。
	 *
	 * @param Text 変換元。
	 * @return 退避を戻した文字列。
	 */
	FString UnescapeJson( FStringView Text )
	{
		FString Plain;
		for ( usize Index = 0; Index < Text.Size(); ++Index )
		{
			const char Character = Text.Data()[Index];
			if ( Character != '\\' || Index + 1 >= Text.Size() )
			{
				Plain.Append( Character );
				continue;
			}

			++Index;
			switch ( Text.Data()[Index] )
			{
			case 'n': Plain.Append( '\n' ); break;
			case 'r': Plain.Append( '\r' ); break;
			case 't': Plain.Append( '\t' ); break;
			case 'u':
				// 本形式が \u で書くのは制御文字だけなので、4 桁を 1 byte へ戻せば足りる。
				if ( Index + 4 < Text.Size() )
				{
					char Digits[5]{};
					for ( usize Digit = 0; Digit < 4; ++Digit ) Digits[Digit] = Text.Data()[Index + 1 + Digit];
					Plain.Append( static_cast<char>( std::strtol( Digits, nullptr, 16 ) ) );
					Index += 4;
				}
				break;
			default:
				Plain.Append( Text.Data()[Index] );
				break;
			}
		}
		return Plain;
	}

	/**
	 * 種類を表す名前を返す。
	 *
	 * @param Kind 対象の種類。
	 * @return "int" / "float" / "bool" / "string"。
	 */
	const char* KindName( EDebugTopSettingKind Kind ) noexcept
	{
		switch ( Kind )
		{
		case EDebugTopSettingKind::Float:  return "float";
		case EDebugTopSettingKind::Bool:   return "bool";
		case EDebugTopSettingKind::String: return "string";
		default:                           return "int";
		}
	}

	/**
	 * 値を JSON の右辺として書ける文字列にする。
	 *
	 * @param Setting 対象の設定。
	 * @return 値の文字列。
	 */
	FString ValueText( const FDebugTopSetting& Setting )
	{
		FString Text;
		switch ( Setting.Kind )
		{
		case EDebugTopSettingKind::Float:
			Text.AppendFormat( "%.6f", static_cast<double>( Setting.FloatValue ) );
			break;

		case EDebugTopSettingKind::Bool:
			Text.Append( Setting.bBoolValue ? "true" : "false" );
			break;

		case EDebugTopSettingKind::String:
			Text.Append( '"' );
			Text.Append( EscapeJson( Setting.StringValue ).View() );
			Text.Append( '"' );
			break;

		default:
			Text.AppendFormat( "%d", Setting.IntValue );
			break;
		}
		return Text;
	}

	/**
	 * 指定位置から次の "<名前>" を探し、その値の開始位置を返す。
	 *
	 * @param Cursor 走査開始位置。
	 * @param End 走査終端。
	 * @param Name 探す名前 (引用符は含めない)。
	 * @return 値の開始位置 (見つからなければ nullptr)。
	 */
	const char* FindValue( const char* Cursor, const char* End, const char* Name )
	{
		if ( Cursor == nullptr ) return nullptr;

		const usize NameLength = FStringView( Name ).Size();
		for ( const char* Scan = Cursor; Scan + NameLength + 2 < End; ++Scan )
		{
			if ( Scan[0] != '"' ) continue;

			bool bMatched = true;
			for ( usize Index = 0; Index < NameLength; ++Index )
			{
				if ( Scan[1 + Index] != Name[Index] ) { bMatched = false; break; }
			}
			if ( !bMatched || Scan[1 + NameLength] != '"' ) continue;

			// 名前の後ろの ':' と空白を飛ばす。
			const char* Value = Scan + 2 + NameLength;
			while ( Value < End && ( *Value == ':' || *Value == ' ' || *Value == '\t' ) ) ++Value;
			return Value;
		}
		return nullptr;
	}

	/**
	 * JSON の文字列値を読み出す。
	 *
	 * @param Cursor 開始位置 (先頭の引用符を指すこと)。
	 * @param End 走査終端。
	 * @param OutText 読み出し先。
	 * @return 閉じ引用符の次の位置 (読めなければ nullptr)。
	 */
	const char* ReadString( const char* Cursor, const char* End, FString& OutText )
	{
		if ( Cursor == nullptr || Cursor >= End || *Cursor != '"' ) return nullptr;

		const char* const Begin = Cursor + 1;
		const char* Scan = Begin;
		while ( Scan < End && *Scan != '"' )
		{
			// 退避された引用符は終端とみなさない。
			if ( *Scan == '\\' && Scan + 1 < End ) ++Scan;
			++Scan;
		}
		if ( Scan >= End ) return nullptr;

		OutText = UnescapeJson( FStringView( Begin, static_cast<usize>( Scan - Begin ) ) );
		return Scan + 1;
	}
}


void DebugTopWriteSettingsJson( const TArray<FDebugTopSetting>& Settings, TArray<byte>& OutBytes )
{
	FString Text( "{\n  \"format\": \"acs_debugtop\",\n  \"version\": 1,\n  \"settings\": [\n" );

	for ( usize Index = 0; Index < Settings.Num(); ++Index )
	{
		const FDebugTopSetting& Setting = Settings[Index];

		Text.Append( "    { \"key\": \"" );
		Text.Append( EscapeJson( Setting.Key ).View() );
		Text.Append( "\", \"kind\": \"" );
		Text.Append( KindName( Setting.Kind ) );
		Text.Append( "\", \"value\": " );
		Text.Append( ValueText( Setting ).View() );
		Text.Append( " }" );
		if ( Index + 1 < Settings.Num() ) Text.Append( ',' );
		Text.Append( '\n' );
	}

	Text.Append( "  ]\n}\n" );
	DebugTopAppendString( OutBytes, Text );
}

bool DebugTopReadSettingsJson( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings )
{
	if ( Data == nullptr ) return false;

	const char* const Begin = reinterpret_cast<const char*>( Data );
	const char* const End = Begin + Size;

	const char* Cursor = Begin;
	while ( Cursor < End )
	{
		const char* const KeyValue = FindValue( Cursor, End, "key" );
		if ( KeyValue == nullptr ) break;

		FDebugTopSetting Setting;
		const char* Next = ReadString( KeyValue, End, Setting.Key );
		if ( Next == nullptr ) break;

		FString KindText;
		Next = ReadString( FindValue( Next, End, "kind" ), End, KindText );
		if ( Next == nullptr ) break;

		const char* const ValueBegin = FindValue( Next, End, "value" );
		if ( ValueBegin == nullptr ) break;

		if ( KindText == FStringView( "float" ) )
		{
			Setting.Kind = EDebugTopSettingKind::Float;
			Setting.FloatValue = static_cast<f32>( std::strtod( ValueBegin, nullptr ) );
			Cursor = ValueBegin;
		}
		else if ( KindText == FStringView( "bool" ) )
		{
			Setting.Kind = EDebugTopSettingKind::Bool;
			Setting.bBoolValue = ( *ValueBegin == 't' );
			Cursor = ValueBegin;
		}
		else if ( KindText == FStringView( "string" ) )
		{
			Setting.Kind = EDebugTopSettingKind::String;
			const char* const AfterValue = ReadString( ValueBegin, End, Setting.StringValue );
			if ( AfterValue == nullptr ) break;
			Cursor = AfterValue;
		}
		else
		{
			Setting.Kind = EDebugTopSettingKind::Int;
			Setting.IntValue = static_cast<i32>( std::strtol( ValueBegin, nullptr, 10 ) );
			Cursor = ValueBegin;
		}

		OutSettings.Add( Move( Setting ) );
	}
	return true;
}

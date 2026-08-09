#include "DebugTopSettingsText.h"

#include <cstdlib>

#include "Debug/DebugTop/Settings/DebugTopSettingsBytes.h"

namespace
{
	/** 先頭に置く目印。 */
	constexpr const char* kMagic = "ACS_DEBUGTOP 1";

	/** キーと値の区切り。キー自体にこの並びを含めないこと。 */
	constexpr const char* kAssign = " = ";

	/** 種類を表す 1 文字 (Int)。 */
	constexpr char kKindInt = 'i';

	/** 種類を表す 1 文字 (Float)。 */
	constexpr char kKindFloat = 'f';

	/** 種類を表す 1 文字 (Bool)。 */
	constexpr char kKindBool = 'b';

	/** 種類を表す 1 文字 (String)。 */
	constexpr char kKindString = 's';

	/** 「<種類> <キー> = <値>」として最低限必要な長さ。 */
	constexpr usize kMinLineLength = 4;
}


void DebugTopWriteSettingsText( const TArray<FDebugTopSetting>& Settings, TArray<byte>& OutBytes )
{
	FString Text( kMagic );
	Text.Append( '\n' );

	for ( usize Index = 0; Index < Settings.Num(); ++Index )
	{
		const FDebugTopSetting& Setting = Settings[Index];

		switch ( Setting.Kind )
		{
		case EDebugTopSettingKind::Float:  Text.Append( kKindFloat );  break;
		case EDebugTopSettingKind::Bool:   Text.Append( kKindBool );   break;
		case EDebugTopSettingKind::String: Text.Append( kKindString ); break;
		default:                           Text.Append( kKindInt );    break;
		}
		Text.Append( ' ' );
		Text.Append( Setting.Key.View() );
		Text.Append( kAssign );

		switch ( Setting.Kind )
		{
		case EDebugTopSettingKind::Float:
			Text.AppendFormat( "%.6f", static_cast<double>( Setting.FloatValue ) );
			break;

		case EDebugTopSettingKind::Bool:
			Text.Append( Setting.bBoolValue ? "1" : "0" );
			break;

		case EDebugTopSettingKind::String:
			Text.Append( Setting.StringValue.View() );
			break;

		default:
			Text.AppendFormat( "%d", Setting.IntValue );
			break;
		}
		Text.Append( '\n' );
	}

	DebugTopAppendString( OutBytes, Text );
}

bool DebugTopReadSettingsText( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings )
{
	if ( Data == nullptr ) return false;

	const char* const Begin = reinterpret_cast<const char*>( Data );
	const char* const End = Begin + Size;
	const char* LineBegin = Begin;

	while ( LineBegin < End && *LineBegin != '\0' )
	{
		// 1 行を切り出す (CRLF と LF の両方を受ける)。
		const char* LineEnd = LineBegin;
		while ( LineEnd < End && *LineEnd != '\n' && *LineEnd != '\0' ) ++LineEnd;

		const char* Trimmed = LineEnd;
		if ( Trimmed > LineBegin && *( Trimmed - 1 ) == '\r' ) --Trimmed;

		const char* const NextLine = ( LineEnd < End && *LineEnd == '\n' ) ? LineEnd + 1 : End;

		// 目印・コメント・空行など、形の合わない行は読み飛ばす。
		const usize Length = static_cast<usize>( Trimmed - LineBegin );
		if ( Length >= kMinLineLength && LineBegin[1] == ' ' )
		{
			const char Kind = LineBegin[0];

			const char* Separator = nullptr;
			for ( const char* Cursor = LineBegin + 2; Cursor + 2 < Trimmed; ++Cursor )
			{
				if ( Cursor[0] == kAssign[0] && Cursor[1] == kAssign[1] && Cursor[2] == kAssign[2] )
				{
					Separator = Cursor;
					break;
				}
			}

			if ( Separator != nullptr )
			{
				FDebugTopSetting Setting;
				Setting.Key = FString( FStringView( LineBegin + 2, static_cast<usize>( Separator - LineBegin - 2 ) ) );

				const char* const ValueBegin = Separator + 3;
				const FString Value( FStringView( ValueBegin, static_cast<usize>( Trimmed - ValueBegin ) ) );

				bool bValid = true;
				switch ( Kind )
				{
				case kKindFloat:
					Setting.Kind = EDebugTopSettingKind::Float;
					Setting.FloatValue = static_cast<f32>( std::strtod( Value.Data(), nullptr ) );
					break;

				case kKindBool:
					Setting.Kind = EDebugTopSettingKind::Bool;
					Setting.bBoolValue = std::strtol( Value.Data(), nullptr, 10 ) != 0;
					break;

				case kKindString:
					Setting.Kind = EDebugTopSettingKind::String;
					Setting.StringValue = Value;
					break;

				case kKindInt:
					Setting.Kind = EDebugTopSettingKind::Int;
					Setting.IntValue = static_cast<i32>( std::strtol( Value.Data(), nullptr, 10 ) );
					break;

				default:
					bValid = false;
					break;
				}

				if ( bValid ) OutSettings.Add( Move( Setting ) );
			}
		}

		LineBegin = NextLine;
	}
	return true;
}

// SPDX-License-Identifier: Apache-2.0
#include "DebugTopSettingsFormat.h"

#include "Debug/DebugTop/Settings/DebugTopSettingsBinary.h"
#include "Debug/DebugTop/Settings/DebugTopSettingsBytes.h"
#include "Debug/DebugTop/Settings/DebugTopSettingsJson.h"
#include "Debug/DebugTop/Settings/DebugTopSettingsText.h"


const char* DebugTopSettingsFormatExtension( EDebugTopSettingsFormat Format ) noexcept
{
	switch ( Format )
	{
	case EDebugTopSettingsFormat::Json:   return ".json";
	case EDebugTopSettingsFormat::Binary: return ".acsset";
	default:                              return ".txt";
	}
}

bool DebugTopSerializeSettings( const TArray<FDebugTopSetting>& Settings, EDebugTopSettingsFormat Format, TArray<byte>& OutBytes )
{
	OutBytes.Reset();

	switch ( Format )
	{
	case EDebugTopSettingsFormat::Json:   DebugTopWriteSettingsJson( Settings, OutBytes );   break;
	case EDebugTopSettingsFormat::Binary: DebugTopWriteSettingsBinary( Settings, OutBytes ); break;
	default:                              DebugTopWriteSettingsText( Settings, OutBytes );   break;
	}
	return true;
}

bool DebugTopDeserializeSettings( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings, EDebugTopSettingsFormat* OutFormat )
{
	OutSettings.Reset();
	if ( Data == nullptr || Size == 0 ) return false;

	// 形式は拡張子ではなく中身の先頭で見分ける。
	if ( Size >= sizeof( u32 ) )
	{
		u32 Magic = 0;
		usize Cursor = 0;
		if ( DebugTopReadRaw( Data, Size, Cursor, Magic ) && Magic == kDebugTopBinaryMagic )
		{
			if ( OutFormat != nullptr ) *OutFormat = EDebugTopSettingsFormat::Binary;
			return DebugTopReadSettingsBinary( Data, Size, OutSettings );
		}
	}

	// 空白を飛ばした先頭が '{' なら JSON とみなす。
	for ( usize Index = 0; Index < Size; ++Index )
	{
		const char Character = static_cast<char>( Data[Index] );
		if ( Character == ' ' || Character == '\t' || Character == '\r' || Character == '\n' ) continue;

		if ( Character == '{' )
		{
			if ( OutFormat != nullptr ) *OutFormat = EDebugTopSettingsFormat::Json;
			return DebugTopReadSettingsJson( Data, Size, OutSettings );
		}
		break;
	}

	if ( OutFormat != nullptr ) *OutFormat = EDebugTopSettingsFormat::Text;
	return DebugTopReadSettingsText( Data, Size, OutSettings );
}

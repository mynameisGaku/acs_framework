// SPDX-License-Identifier: Apache-2.0
#include "DebugTopSettingsBinary.h"

#include "Debug/DebugTop/Settings/DebugTopSettingsBytes.h"


void DebugTopWriteSettingsBinary( const TArray<FDebugTopSetting>& Settings, TArray<byte>& OutBytes )
{
	DebugTopAppendRaw( OutBytes, kDebugTopBinaryMagic );
	DebugTopAppendRaw( OutBytes, kDebugTopBinaryVersion );
	DebugTopAppendRaw( OutBytes, static_cast<u16>( 0 ) );                 // 予約 (以降を 4 byte 境界に置くため)
	DebugTopAppendRaw( OutBytes, static_cast<u32>( Settings.Num() ) );

	for ( usize Index = 0; Index < Settings.Num(); ++Index )
	{
		const FDebugTopSetting& Setting = Settings[Index];

		DebugTopAppendRaw( OutBytes, static_cast<u8>( Setting.Kind ) );
		DebugTopAppendSizedString( OutBytes, Setting.Key );

		switch ( Setting.Kind )
		{
		case EDebugTopSettingKind::Float:
			DebugTopAppendRaw( OutBytes, Setting.FloatValue );
			break;

		case EDebugTopSettingKind::Bool:
			DebugTopAppendRaw( OutBytes, static_cast<u8>( Setting.bBoolValue ? 1 : 0 ) );
			break;

		case EDebugTopSettingKind::String:
			DebugTopAppendSizedString( OutBytes, Setting.StringValue );
			break;

		default:
			DebugTopAppendRaw( OutBytes, Setting.IntValue );
			break;
		}
	}
}

bool DebugTopReadSettingsBinary( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings )
{
	if ( Data == nullptr ) return false;

	usize Cursor = 0;

	u32 Magic = 0;
	u16 Version = 0;
	u16 Reserved = 0;
	u32 Count = 0;
	if ( !DebugTopReadRaw( Data, Size, Cursor, Magic ) ) return false;
	if ( !DebugTopReadRaw( Data, Size, Cursor, Version ) ) return false;
	if ( !DebugTopReadRaw( Data, Size, Cursor, Reserved ) ) return false;
	if ( !DebugTopReadRaw( Data, Size, Cursor, Count ) ) return false;

	if ( Magic != kDebugTopBinaryMagic ) return false;
	if ( Version != kDebugTopBinaryVersion ) return false;

	OutSettings.Reserve( Count );
	for ( u32 Index = 0; Index < Count; ++Index )
	{
		u8 Kind = 0;
		if ( !DebugTopReadRaw( Data, Size, Cursor, Kind ) ) return false;

		FDebugTopSetting Setting;
		if ( !DebugTopReadSizedString( Data, Size, Cursor, Setting.Key ) ) return false;

		switch ( static_cast<EDebugTopSettingKind>( Kind ) )
		{
		case EDebugTopSettingKind::Float:
			Setting.Kind = EDebugTopSettingKind::Float;
			if ( !DebugTopReadRaw( Data, Size, Cursor, Setting.FloatValue ) ) return false;
			break;

		case EDebugTopSettingKind::Bool:
		{
			Setting.Kind = EDebugTopSettingKind::Bool;
			u8 Value = 0;
			if ( !DebugTopReadRaw( Data, Size, Cursor, Value ) ) return false;
			Setting.bBoolValue = Value != 0;
			break;
		}
		case EDebugTopSettingKind::String:
			Setting.Kind = EDebugTopSettingKind::String;
			if ( !DebugTopReadSizedString( Data, Size, Cursor, Setting.StringValue ) ) return false;
			break;

		default:
			Setting.Kind = EDebugTopSettingKind::Int;
			if ( !DebugTopReadRaw( Data, Size, Cursor, Setting.IntValue ) ) return false;
			break;
		}

		OutSettings.Add( Move( Setting ) );
	}
	return true;
}

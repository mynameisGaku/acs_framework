// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Settings/GameSettingsStore.h"
#include "Common/Test/TestHarness.h"


void RunGameSettingsStoreTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGameSettingsStore / 未作成フォルダへの保存と復元" );

	const wchar_t* const Path = L"TestOutput/GameSettings/Nested/settings.acscfg";
	FGameSettingsStore Written;
	Written.SetInt( FString( "Input.FxaaToggleKey" ), static_cast<i32>( EKey::P ) );

	Harness.Check( Written.SaveTo( Path ).IsOk(), "親フォルダごと作って保存できる" );

	FGameSettingsStore Loaded;
	Harness.Check( Loaded.LoadFrom( Path ).IsOk(), "保存した設定を読み戻せる" );
	Harness.CheckEqualU64( static_cast<u64>( Loaded.GetInt( FString( "Input.FxaaToggleKey" ), -1 ) ), static_cast<u64>( EKey::P ), "整数の設定値が一致する" );
}

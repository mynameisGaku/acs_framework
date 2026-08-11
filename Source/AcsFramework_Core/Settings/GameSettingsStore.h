// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 設定値と、設定へ渡す文字列の安定した領域を所有する通常型。 */
class FGameSettingsStore
{
public:
	/** 浮動小数の設定値を書き込む。 */
	void SetFloat( const FString& Key, f32 Value );

	/** 整数の設定値を書き込む。 */
	void SetInt( const FString& Key, i32 Value );

	/** 真偽の設定値を書き込む。 */
	void SetBool( const FString& Key, bool bValue );

	/** 文字列の設定値を書き込み、キーと値の領域を保持する。 */
	void SetString( const FString& Key, const FString& Value );

	/** 浮動小数の設定値を返し、未設定または型違いなら既定値を返す。 */
	f32 GetFloat( const FString& Key, f32 DefaultValue ) const;

	/** 整数の設定値を返し、未設定または型違いなら既定値を返す。 */
	i32 GetInt( const FString& Key, i32 DefaultValue ) const;

	/** 真偽の設定値を返し、未設定または型違いなら既定値を返す。 */
	bool GetBool( const FString& Key, bool bDefaultValue ) const;

	/** 文字列の設定値を所有する値として返し、未設定または型違いなら既定値を返す。 */
	FString GetString( const FString& Key, const FString& DefaultValue ) const;

	/** 現在の設定値を指定先へ保存し、失敗時はエラーを返す。 */
	TResult<void> SaveTo( const wchar_t* FilePath ) noexcept;

	/** 指定先の設定値を検証して読み込み、失敗時は現在値を維持してエラーを返す。 */
	TResult<void> LoadFrom( const wchar_t* FilePath ) noexcept;

private:
	/** 同じ内容を再利用し、設定値の寿命まで有効な文字列領域を返す。 */
	const char* Intern( const FString& Text );

	/** 型付き設定値と読込済み文字列を所有する入れ物。 */
	game::CSettings m_Settings;

	/** 呼出し側から設定したキーと文字列値を安定した領域で所有する。 */
	TArray<TUniquePtr<FString>> m_Interned;
};

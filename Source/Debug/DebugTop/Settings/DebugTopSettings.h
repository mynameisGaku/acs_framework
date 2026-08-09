// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Settings/DebugTopSetting.h"
#include "Debug/DebugTop/Settings/DebugTopSettingsPath.h"

using namespace acs;

class ADebugTopHUD;

/**
 * デバッグメニューで設定した値の置き場。
 *
 * @details
 * ゲーム側とデバッグメニューを直接つながずに値を渡すための、キーと値だけの保管庫。
 * GameInstance スコープのサブシステムなので、シーンを切り替えても同じ実体が残り、
 * 「メニューで設定 → 次のレベルで受け取る」がそのまま成立する。ファイルへ保存すれば
 * プロセスを跨いでも残り、形式を選べるので他のツールやゲームからも読める。
 *
 * 受け取り側 (ゲーム・システム) の書き方:
 * @code
 * if ( CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>() )
 * {
 *     const f32 Speed = Settings->GetFloat( FString( "Gameplay/MoveSpeed" ), 1.0f );
 * }
 * @endcode
 *
 * 渡す側 (デバッグメニュー) の書き方:
 * @code
 * // 行に絶対キーを付けておくと、メニューのどこへ移しても同じキーで受け取れる。
 * ChildMenu->Add<CDebugTopElementFloat>( "MoveSpeed", 1.0f, 0.0f, 8.0f, 0.25f )
 *          ->SetSaveKey( FString( "Gameplay/MoveSpeed" ) );
 *
 * CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>();
 * Settings->MutablePath().SetDirectory( FString( "Saved/Debug" ) );
 * Settings->MutablePath().SetFileName( FString( "MySettings" ) );
 * Settings->MutablePath().SetFormat( EDebugTopSettingsFormat::Binary );
 * Settings->CaptureFrom( *m_HUD );
 * Settings->Save();   // 保存先の絶対パスと件数と形式をログへ出す
 * @endcode
 *
 * 置き場所について: 本クラスは Debug モジュール側にある。acs はビルド済みの acs.h + acs.lib
 * として配布されており、この構成からはエンジンへ手を入れて配布し直せないため。形式まわりだけは
 * メニューを知らない層 (DebugTopSetting / DebugTopSettings*Format 等) に切ってあるので、
 * 配布物を更新できるようになった時点でそのまま acs 側へ移せる。
 */
class CDebugTopSettings : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CDebugTopSettings )

	/** 保存先の指定を返す。 */
	const CDebugTopSettingsPath& GetPath() const noexcept { return m_Path; }

	/**
	 * 保存先の指定を書き換えられる形で返す。
	 *
	 * @details 置き場所・ファイル名・形式はここから設定する。
	 * @return 保存先の指定。
	 */
	CDebugTopSettingsPath& MutablePath() noexcept { return m_Path; }

	/**
	 * i32 を取り出す。
	 *
	 * @param Key 引くキー。
	 * @param OutValue 取り出せた場合に値を書き込む先。
	 * @return 見つかって種類も一致すれば true。
	 */
	bool TryGetInt( const FString& Key, i32& OutValue ) const noexcept;

	/**
	 * f32 を取り出す。
	 *
	 * @param Key 引くキー。
	 * @param OutValue 取り出せた場合に値を書き込む先。
	 * @return 見つかって種類も一致すれば true。
	 */
	bool TryGetFloat( const FString& Key, f32& OutValue ) const noexcept;

	/**
	 * bool を取り出す。
	 *
	 * @param Key 引くキー。
	 * @param bOutValue 取り出せた場合に値を書き込む先。
	 * @return 見つかって種類も一致すれば true。
	 */
	bool TryGetBool( const FString& Key, bool& bOutValue ) const noexcept;

	/**
	 * 文字列を取り出す。
	 *
	 * @param Key 引くキー。
	 * @param OutValue 取り出せた場合に値を書き込む先。
	 * @return 見つかって種類も一致すれば true。
	 */
	bool TryGetString( const FString& Key, FString& OutValue ) const;

	/**
	 * i32 を取り出す (無ければ既定値)。
	 *
	 * @param Key 引くキー。
	 * @param DefaultValue 見つからなかったときに返す値。
	 * @return 保管庫の値、または既定値。
	 */
	i32 GetInt( const FString& Key, i32 DefaultValue = 0 ) const noexcept;

	/**
	 * f32 を取り出す (無ければ既定値)。
	 *
	 * @param Key 引くキー。
	 * @param DefaultValue 見つからなかったときに返す値。
	 * @return 保管庫の値、または既定値。
	 */
	f32 GetFloat( const FString& Key, f32 DefaultValue = 0.0f ) const noexcept;

	/**
	 * bool を取り出す (無ければ既定値)。
	 *
	 * @param Key 引くキー。
	 * @param bDefaultValue 見つからなかったときに返す値。
	 * @return 保管庫の値、または既定値。
	 */
	bool GetBool( const FString& Key, bool bDefaultValue = false ) const noexcept;

	/**
	 * i32 を書き込む (同じキーがあれば上書き)。
	 *
	 * @param Key 書き込むキー。
	 * @param Value 書き込む値。
	 */
	void SetInt( const FString& Key, i32 Value );

	/**
	 * f32 を書き込む (同じキーがあれば上書き)。
	 *
	 * @param Key 書き込むキー。
	 * @param Value 書き込む値。
	 */
	void SetFloat( const FString& Key, f32 Value );

	/**
	 * bool を書き込む (同じキーがあれば上書き)。
	 *
	 * @param Key 書き込むキー。
	 * @param bValue 書き込む値。
	 */
	void SetBool( const FString& Key, bool bValue );

	/**
	 * 文字列を書き込む (同じキーがあれば上書き)。
	 *
	 * @details テキスト形式では 1 行 1 設定で書くため、改行は空白へ潰して保存する。
	 * @param Key 書き込むキー。
	 * @param Value 書き込む値。
	 */
	void SetString( const FString& Key, const FString& Value );

	/**
	 * キーを 1 つ消す。
	 *
	 * @param Key 消すキー。
	 * @return 消せたら true。
	 */
	bool Remove( const FString& Key ) noexcept;

	/** 全ての値を捨てる。 */
	void Clear() noexcept;

	/** 保持している設定を返す (保存順)。 */
	const TArray<FDebugTopSetting>& GetAll() const noexcept { return m_Settings; }

	/**
	 * メニューの現在値を保管庫へ吸い出す。
	 *
	 * @details 値を持つ行 (Int / Float / Bool / Enum と配列の各要素) が対象。
	 * @param HUD 吸い出す元のメニュー。
	 */
	void CaptureFrom( const ADebugTopHUD& HUD );

	/**
	 * 保管庫の値をメニューへ書き戻す。
	 *
	 * @details 保管庫に無いキーの行は現在値のままにする。
	 * @param HUD 書き戻す先のメニュー。
	 */
	void ApplyTo( ADebugTopHUD& HUD ) const;

	/**
	 * 保存先の設定に従ってファイルへ保存する。
	 *
	 * @details 書き出した絶対パス・件数・形式をログへ出す。置き場所が無ければ 1 段だけ作る。
	 * @return 書き出せたら true。
	 */
	bool Save() const;

	/**
	 * 保存先の設定に従ってファイルから読み込む。
	 *
	 * @details
	 * 読み込む前に現在の内容を捨てる。形式はファイルの中身から見分けるので、設定と違う形式で
	 * 書かれていても読める。ファイルが無い場合は false を返すだけで警告は出さない。
	 * @return 読み込めたら true。
	 */
	bool Load();

	/**
	 * パスを明示してファイルへ保存する。
	 *
	 * @param Path 書き出し先のパス (拡張子まで含めた完全な名前)。
	 * @param Format 書き出す形式。
	 * @return 書き出せたら true。
	 */
	bool SaveAs( const FString& Path, EDebugTopSettingsFormat Format ) const;

	/**
	 * パスを明示してファイルから読み込む。
	 *
	 * @param Path 読み込むパス。
	 * @return 読み込めたら true。
	 */
	bool LoadFrom( const FString& Path );

private:
	/**
	 * キーに対応する設定を探す。
	 *
	 * @param Key 探すキー。
	 * @return 見つかった設定 (無ければ nullptr)。
	 */
	FDebugTopSetting* Find( const FString& Key ) noexcept;

	/**
	 * キーに対応する設定を探す (const 版)。
	 *
	 * @param Key 探すキー。
	 * @return 見つかった設定 (無ければ nullptr)。
	 */
	const FDebugTopSetting* Find( const FString& Key ) const noexcept;

	/**
	 * キーに対応する設定を返し、無ければ作る。
	 *
	 * @param Key 探すキー。
	 * @return 見つかった、または作った設定。
	 */
	FDebugTopSetting& FindOrAdd( const FString& Key );

	/** キーと値の一覧 (保存順を保つため配列で持つ。デバッグメニュー規模なら線形探索で足りる)。 */
	TArray<FDebugTopSetting> m_Settings;

	/** 保存先の指定 (置き場所・ファイル名・形式)。 */
	CDebugTopSettingsPath m_Path;
};

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Settings/DebugTopSettingsFormat.h"

using namespace acs;

/**
 * 保存先の指定 (置き場所・ファイル名・形式)。
 *
 * @details
 * 実際のパスは「ディレクトリ / ファイル名 + 形式ごとの拡張子」で組み立てる。ファイル名に
 * 拡張子を書く必要は無い (書いた場合はそのまま残り、後ろに形式の拡張子が付く)。
 */
class CDebugTopSettingsPath
{
public:
	/** 実行ディレクトリ直下・既定名で構築する。 */
	CDebugTopSettingsPath() noexcept = default;

	/** 置き場所を返す (空なら実行ディレクトリ)。 */
	const FString& GetDirectory() const noexcept { return m_Directory; }

	/**
	 * 置き場所を設定する。
	 *
	 * @details 末尾の区切りは付けても付けなくてもよい。無いディレクトリは保存時に 1 段だけ作る。
	 * @param Directory 置き場所 (空にすると実行ディレクトリ)。
	 */
	void SetDirectory( const FString& Directory );

	/** ファイル名 (拡張子を除く) を返す。 */
	const FString& GetFileName() const noexcept { return m_FileName; }

	/**
	 * ファイル名 (拡張子を除く) を設定する。
	 *
	 * @param FileName 設定するファイル名 (空にすると既定名へ戻る)。
	 */
	void SetFileName( const FString& FileName );

	/** 出力形式を返す。 */
	EDebugTopSettingsFormat GetFormat() const noexcept { return m_Format; }

	/**
	 * 出力形式を設定する。
	 *
	 * @details 拡張子も併せて変わる。読み込みは中身で形式を見分けるため、この設定に縛られない。
	 * @param Format 設定する形式。
	 */
	void SetFormat( EDebugTopSettingsFormat Format ) noexcept;

	/**
	 * 組み立てたパスを返す。
	 *
	 * @return 「ディレクトリ/ファイル名+拡張子」。ディレクトリが空ならファイル名+拡張子だけ。
	 */
	FString Build() const;

	/**
	 * 組み立てたパスを絶対パスへ直して返す。
	 *
	 * @details ログへ出す用。解決できない場合は Build() の結果をそのまま返す。
	 * @return 絶対パス (解決できなければ相対パスのまま)。
	 */
	FString BuildAbsolute() const;

private:
	/** 置き場所 (空なら実行ディレクトリ)。 */
	FString m_Directory;

	/** ファイル名 (拡張子を除く)。 */
	FString m_FileName{ "DebugTopSettings" };

	/** 出力形式。 */
	EDebugTopSettingsFormat m_Format = EDebugTopSettingsFormat::Text;
};


/**
 * UTF-8 のパスを Win32 API 用のワイド文字列へ変換する。
 *
 * @param Path 変換元のパス。
 * @param OutPath 変換先 (NUL 終端まで書き込む)。
 * @return 変換できたら true。
 */
bool DebugTopToWidePath( const FString& Path, TArray<wchar_t>& OutPath );

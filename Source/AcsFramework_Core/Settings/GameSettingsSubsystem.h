// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Settings/GameSettingsStore.h"

using namespace acs;
using namespace acs::game;

/**
 * GameInstanceの寿命で設定の保存先、自動保存、警告、外部窓口を所有するサブシステム。
 * 設定値と安定した文字列領域は通常型へ任せ、更新と終了時保存を管理する。
 */
class CGameSettingsSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CGameSettingsSubsystem )

	/**
	 * 置き場所を決めて、あれば読み込む。
	 *
	 * @details
	 * 保存先が未設定でも値は保持できるが、ファイルへは残らない。
	 * ファイルが無いのは初回起動なので、失敗とは扱わない。
	 * @param FilePath 置き場所 (UTF-8。実行時の作業フォルダからの相対でよい)。
	 * @return 読み込めたら true (初回起動で無かった場合は false)。
	 */
	bool Configure( const FString& FilePath );

	/**
	 * 値を書き込む。
	 *
	 * @param Key 値の名前。
	 * @param Value 書き込む値。
	 */
	void SetFloat( const FString& Key, f32 Value );

	/** 値を書き込む (整数)。 */
	void SetInt( const FString& Key, i32 Value );

	/** 値を書き込む (真偽)。 */
	void SetBool( const FString& Key, bool bValue );

	/** 値を書き込む (文字列)。 */
	void SetString( const FString& Key, const FString& Value );

	/**
	 * 値を読み出す。
	 *
	 * @param Key 値の名前。
	 * @param DefaultValue 見つからなかったときに返す値。
	 * @return 読み出した値。
	 */
	f32 GetFloat( const FString& Key, f32 DefaultValue = 0.0f ) const;

	/** 値を読み出す (整数)。 */
	i32 GetInt( const FString& Key, i32 DefaultValue = 0 ) const;

	/** 値を読み出す (真偽)。 */
	bool GetBool( const FString& Key, bool bDefaultValue = false ) const;

	/**
	 * 値を読み出す (文字列)。
	 *
	 * @details
	 * エンジンの口は中の領域を指す生ポインタを返すので、次に値を書き換えると指し先が変わる。
	 * 持ち回れるよう写して返す。
	 * @param Key 値の名前。
	 * @param DefaultValue 見つからなかったときに返す値。
	 * @return 読み出した値。
	 */
	FString GetString( const FString& Key, const FString& DefaultValue = FString() ) const;

	/**
	 * いま持っている値をファイルへ書く。
	 *
	 * @return 書けたら true。
	 */
	bool Save();

	/**
	 * ファイルから読み直す (いま持っている値は捨てられる)。
	 *
	 * @return 読めたら true。
	 */
	bool Load();

	/** 書き換えてからまだ書いていないものがあるかを返す。 */
	bool IsDirty() const noexcept { return m_bDirty; }

	/**
	 * 書き換えてから実際に書くまでの待ちを設定する。
	 *
	 * @param Seconds 待ち (秒)。0 にすると変えた次の更新で書く。
	 */
	void SetAutoSaveDelay( f32 Seconds ) noexcept { m_AutoSaveDelaySeconds = Seconds > 0.0f ? Seconds : 0.0f; }

	/**
	 * 1 フレーム進める。
	 *
	 * @details 変更後の経過時間を進め、待ち時間に達した設定を保存する。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/** シーンの退場後に、まだ書かれていない設定を最後に保存する。 */
	void OnDeinitialize() noexcept override;

private:
	/** 書き換えられたことを控える。 */
	void MarkDirty() noexcept;

	/** 設定値と設定へ渡す文字列の安定した領域。 */
	FGameSettingsStore m_Store;

	/** 置き場所 (空なら残さない)。 */
	FString m_FilePath;

	/** 書き換えてからの経過秒。 */
	f32 m_IdleSeconds = 0.0f;

	/** 書き換えてから実際に書くまでの待ち (秒)。 */
	f32 m_AutoSaveDelaySeconds = 1.0f;

	/** まだ書いていない書き換えがあるか。 */
	bool m_bDirty = false;

	/** 書けなかったことを既に知らせたか (毎秒出さないため)。 */
	bool m_bSaveWarned = false;
};

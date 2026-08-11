// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 音量や画面設定のような、ゲームの設定を持って残すサブシステム。
 *
 * @details
 * 値の入れ物と読み書きはエンジン (CSettings) が持っている。ただし**その実体は誰も持っていない**
 * ので、そのままではゲームコードから使いようがない。ここが 1 つ持ち、置き場所を決め、
 * 起動時に読み、変わったら書く、までを引き受ける。
 *
 * **変えるたびには書かない。** 音量をスライダーで動かすと毎フレーム変わるので、そのたびに
 * ファイルへ書くと無駄が大きい。手が止まってから書く。すぐ書きたいときは Save を呼ぶ。
 * アプリを畳むときにも書くので、変えた直後に落としても失われない。
 *
 * デバッグメニューの設定 (CDebugTopSettings) とは別物。あちらは開発中に触る値で、
 * こちらは製品としてプレイヤーが決める値。
 *
 * **文字列はこちらが持つ。** エンジンの入れ物はキーと文字列値を «ポインタで指すだけ» で、
 * 寿命は渡した側の責任になっている。その場で作った FString を渡すとすぐ宙を指すので、
 * ここで写して持ち、指し先が動かないようにしている。
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
	 * アプリの起動時に 1 度だけ呼ぶ。決める前に読み書きしても値は持てるが、ファイルへは残らない。
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
	 * @details 手が止まったかを測り、止まっていれば書く。アプリの更新から毎フレーム呼ぶ。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/** シーンの退場後に、まだ書かれていない設定を最後に保存する。 */
	void OnDeinitialize() noexcept override;

private:
	/** 書き換えられたことを控える。 */
	void MarkDirty() noexcept;

	/**
	 * 文字列を写し取って、動かない指し先を返す。
	 *
	 * @details
	 * 同じ中身が既にあればそれを使い回す (同じキーを繰り返し書いても増えない)。写しは
	 * 1 つずつ確保するので、後から増やしても既に配った指し先は動かない。
	 * @param Text 写す文字列。
	 * @return 写しを指すポインタ。
	 */
	const char* Intern( const FString& Text );

	/** 値の入れ物。 */
	CSettings m_Settings;

	/**
	 * 入れ物へ渡した文字列の持ち主。
	 *
	 * @details
	 * 1 つずつ確保する。まとめて 1 本の配列に置くと、増えたときに再確保されて既に配った
	 * 指し先が動いてしまう。
	 */
	TArray<TUniquePtr<FString>> m_Interned;

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

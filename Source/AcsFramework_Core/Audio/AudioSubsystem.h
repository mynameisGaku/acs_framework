// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** GameInstanceの寿命で音の出力先と音の取りまとめを所有・接続する窓口。 */
class CAudioSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CAudioSubsystem )

	/** 音の出力先を終了する。 */
	~CAudioSubsystem() noexcept override;

	/** 終了時に音を止め、非所有参照を切って出力先を終了する。 */
	void OnDeinitialize() noexcept override;

	/**
	 * 音の出力先を初期化してアセットレジストリへ接続する。
	 * @param Application 音声アセットを検索するレジストリを持つアプリケーション。
	 * @param MaxVoices 同時に鳴らせる音の数の上限。
	 * @return 初期化に成功した場合は true。失敗時は無音のまま false。
	 */
	bool Bind( CApplication& Application, u32 MaxVoices = 64 );

	/** 音の出力先が利用可能かを返す。 */
	bool IsAudible() const noexcept { return m_bBackendReady; }

	/**
	 * BGM を鳴らす。
	 * @details 既にBGM 名が鳴っていれば、指定秒数で入れ替える。名前の保持に失敗した場合だけ要求しない。出力先がない、または再生に失敗してもBGM 名と音量の状態は保持する。
	 * @param Name アセット名またはパス。
	 * @param FadeInSeconds 入りにかける秒数。
	 * @param bLoop 繰り返すなら true。
	 */
	void PlayBgm( const FString& Name, f32 FadeInSeconds = 1.0f, bool bLoop = true );

	/**
	 * BGM を止める。
	 * @param FadeOutSeconds 消えるまでの秒数。
	 */
	void StopBgm( f32 FadeOutSeconds = 0.5f );

	/** 現在のBGM名を返す。再生中でなければ空文字列を返す。 */
	FString GetCurrentBgmName() const;

	/**
	 * 効果音を 1 回鳴らす。
	 * @details 音量倍率が0 以下または名前の保持に失敗した場合だけ要求しない。出力先がない、または再生に失敗しても音量の状態は保持する。
	 * @param Name アセット名またはパス。
	 * @param VolumeScale この 1 回だけの音量の倍率。
	 */
	void PlaySfx( const FString& Name, f32 VolumeScale = 1.0f );

	/**
	 * BGM を一時的に小さくする (台詞や決定音を通すため)。
	 * @param DurationSeconds 小さくしている秒数。
	 * @param Depth どれだけ小さくするか (0..1)。
	 */
	void Duck( f32 DurationSeconds, f32 Depth );

	/**
	 * 全体の音量を設定する。
	 * @param Volume 設定する音量 (0..1)。
	 */
	void SetMasterVolume( f32 Volume );

	/** BGM の音量を設定する。 */
	void SetBgmVolume( f32 Volume );

	/** 効果音の音量を設定する。 */
	void SetSfxVolume( f32 Volume );

	/** 全体の音量を返す。 */
	f32 GetMasterVolume() const noexcept { return m_Director.GetMasterVolume(); }

	/** BGM の音量を返す。 */
	f32 GetBgmVolume() const noexcept { return m_Director.GetBgmVolume(); }

	/** 効果音の音量を返す。 */
	f32 GetSfxVolume() const noexcept { return m_Director.GetSfxVolume(); }

	/** 鳴っているものを全て止める。 */
	void StopAll();

	/** 音を止める (ポーズ中に曲まで止めたいときに明示して呼ぶ)。 */
	void Pause();

	/** 止めた音を戻す。 */
	void Resume();

	/** 止まっているかを返す。 */
	bool IsPaused() const noexcept { return m_Director.IsPaused(); }

	/**
	 * 1 フレーム進める。
	 * @param UnscaledDeltaSeconds 前フレームからの実経過秒。
	 */
	void Update( f32 UnscaledDeltaSeconds );

private:
	/** 音を止め、非所有参照を切り、未接続の状態へ戻す。 */
	void ReleaseBinding() noexcept;

	/** 音声名を保持配列へ複製し、確保または複製に失敗した場合はnullptrを返す。 */
	const char* TryInternAudioName( const FString& Name ) noexcept;

	/** 音の取りまとめへ渡す名前をサブシステムの寿命中保持する配列。 */
	TArray<TUniquePtr<FString>> m_InternedAudioNames;

	/** 音の取りまとめ。 */
	CAudioDirector m_Director;

	/** 実際に音を出す層。 */
	CXAudio2Backend m_Backend;

	/** 音を出す層まで用意できているか。 */
	bool m_bBackendReady = false;
};

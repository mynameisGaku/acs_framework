#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 音を鳴らす一式を持って配線するサブシステム。
 *
 * @details
 * 音の部品はエンジンに揃っている。ただし**どれも誰も持っておらず、繋がってもいない**。
 * 鳴らすまでには最低でも次の 4 つが要る。
 *
 * 1. 実際に音を出す層 (CXAudio2Backend) を作って初期化する
 * 2. 取りまとめ (CAudioDirector) へその層を差す
 * 3. 名前からアセットを引けるよう、アプリのレジストリを差す
 * 4. 毎フレーム進める / 終了時に畳む
 *
 * 各ゲームが同じ 4 つを書き写すことになるので、ここが引き受ける。
 *
 * **音は止めない。** 進めるのに使うのは実時間で、ゲームを止めても BGM は流れ続ける。
 * 止まった «ゲームの中» ではなく «プレイヤーの手元» で鳴っているものだからで、ポーズ中に
 * 曲まで止めたいなら Pause を明示して呼ぶ。
 *
 * 音量は 3 系統 (全体 / BGM / SE)。どこへ保存するかは知らない (プレイヤー設定として残したい
 * なら、決め所であるアプリが CGameSettingsSubsystem と繋ぐ)。
 *
 * @code
 * Audio->SetBgmVolume( 0.6f );
 * Audio->PlayBgm( FString( "Assets/Bgm/Field.wav" ) );
 * Audio->PlaySfx( FString( "Assets/Se/Decide.wav" ) );
 * @endcode
 */
class CAudioSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CAudioSubsystem )

	/** 音を出す層を畳む。 */
	~CAudioSubsystem() noexcept override;

	/** subsystem 終了時に backend と非所有参照を切る。 */
	void OnDeinitialize() noexcept override;

	/**
	 * 音を出せる状態にする。
	 *
	 * @details
	 * アプリの起動時に 1 度だけ呼ぶ。音を出す層の用意に失敗しても、音が出ないだけで
	 * 呼び出し側は同じように書ける (名前や音量の状態は持たれる)。
	 * @param Application 名前からアセットを引くためのレジストリを持っているもの。
	 * @param MaxVoices 同時に鳴らせる数の上限。
	 * @return 音を出す層まで用意できたら true。
	 */
	bool Bind( CApplication& Application, u32 MaxVoices = 64 );

	/** 音を出す層まで用意できているかを返す。 */
	bool IsAudible() const noexcept { return m_bBackendReady; }

	/**
	 * BGM を鳴らす。
	 *
	 * @details 既に鳴っていれば、指定の秒数で入れ替わる。
	 * @param Name アセットの名前 (パス)。
	 * @param FadeInSeconds 入りにかける秒数。
	 * @param bLoop 繰り返すなら true。
	 */
	void PlayBgm( const FString& Name, f32 FadeInSeconds = 1.0f, bool bLoop = true );

	/**
	 * BGM を止める。
	 *
	 * @param FadeOutSeconds 消えるまでの秒数。
	 */
	void StopBgm( f32 FadeOutSeconds = 0.5f );

	/** いま鳴っている BGM の名前を返す (無ければ空文字列)。 */
	FString GetCurrentBgmName() const;

	/**
	 * 効果音を 1 回鳴らす。
	 *
	 * @param Name アセットの名前 (パス)。
	 * @param VolumeScale この 1 回だけの音量の倍率。
	 */
	void PlaySfx( const FString& Name, f32 VolumeScale = 1.0f );

	/**
	 * BGM を一時的に小さくする (台詞や決定音を通すため)。
	 *
	 * @param DurationSeconds 小さくしている秒数。
	 * @param Depth どれだけ小さくするか (0..1)。
	 */
	void Duck( f32 DurationSeconds, f32 Depth );

	/**
	 * 全体の音量を設定する。
	 *
	 * @param Volume 0..1。
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
	 *
	 * @details
	 * 渡すのは実時間の経過秒。ゲームを止めても曲は流れ続けるので、時間の倍率は掛けない。
	 * @param UnscaledDeltaSeconds 前フレームからの実経過秒。
	 */
	void Update( f32 UnscaledDeltaSeconds );

private:
	/** backend と非所有参照を順序どおり解放し、何度呼んでも未接続へ戻す。 */
	void ReleaseBinding() noexcept;

	/** 音声名をsubsystem寿命へ複製し、失敗時はnullptrを返す。 */
	const char* TryInternAudioName( const FString& Name ) noexcept;

	/** Directorへ渡す名前をsubsystem寿命中保持する所有pool。 */
	TArray<TUniquePtr<FString>> m_InternedAudioNames;

	/** 音の取りまとめ。 */
	CAudioDirector m_Director;

	/** 実際に音を出す層。 */
	CXAudio2Backend m_Backend;

	/** 音を出す層まで用意できているか。 */
	bool m_bBackendReady = false;
};

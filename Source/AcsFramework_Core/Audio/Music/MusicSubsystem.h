// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Audio/Music/IMusicStateSource.h"
#include "AcsFramework_Core/Audio/Music/MusicStateArbiter.h"
#include "AcsFramework_Core/Audio/Music/MusicStingerPump.h"
#include "AcsFramework_Core/Audio/Music/MusicTrackCatalog.h"

using namespace acs;
using namespace acs::game;

class CAudioSubsystem;

/**
 * 状況に合わせて曲を切り替えるサブシステム。
 *
 * @details
 * 切り替えの仕組みはエンジン (CMusicDirector) が持っていて、鳴らす部分は CAudioSubsystem が
 * 既に持っている CAudioDirector へ委譲される。ここが引き受けるのは、その 2 つを繋ぐ 1 本の線と、
 * **どの状態を採るかを決める側**の分解。
 *
 * 1. 状態と曲の対応表 (CMusicTrackCatalog) を持ち、エンジンへ流し込む
 * 2. 申告 (IMusicStateSource / RequestState) を集め、1 つに決める
 * 3. 決めた状態をエンジンへ渡し、毎フレーム進め、差し込みの一音を鳴らす側へ回す
 *
 * **その場では曲は変わらない。** 申告はフレームの決まった 1 か所でまとめて処理される。
 * どこから何度申告しても、切り替えは 1 回で済む。
 *
 * **実時間で進める。** ゲームを止めても曲は流れ続ける (CAudioSubsystem と同じ理由)。
 *
 * @code
 * Music->Bind( *Audio );
 * Music->RegisterTrack( EMusicState::Calm,   FString( "Assets/Bgm/Field.wav" ) );
 * Music->RegisterTrack( EMusicState::Combat, FString( "Assets/Bgm/Battle.wav" ) );
 *
 * // 遊びの側からは «状態» だけを伝える
 * Music->RequestState( FMusicStateRequest{ EMusicState::Combat, 0.8f, 1.5f, EMusicPriority::Gameplay } );
 * @endcode
 */
class CMusicSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CMusicSubsystem )

	/** subsystem 終了時に非所有参照を切る。 */
	void OnDeinitialize() noexcept override;

	/**
	 * 鳴らす側へ繋ぐ。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡すまでは何を申告しても曲は鳴らない。
	 * @param Audio 鳴らす側。このサブシステムより長く生きること。
	 */
	void Bind( CAudioSubsystem& Audio ) noexcept;

	/**
	 * 状態と曲の対応を足す。
	 *
	 * @param State どの状態のときの曲か。
	 * @param AssetPath 曲のパス。
	 * @param IntensityMin この曲を使う強さの下限 (0..1)。
	 * @param IntensityMax この曲を使う強さの上限 (0..1)。
	 * @param bLoop 繰り返すなら true。
	 * @return 足せたら true。
	 */
	bool RegisterTrack( EMusicState State, const FString& AssetPath, f32 IntensityMin = 0.0f, f32 IntensityMax = 1.0f, bool bLoop = true ) noexcept;

	/**
	 * 状態を申告するものを受け取る。
	 *
	 * @details 渡したものの寿命はここが持つ。毎フレーム聞きに行く。
	 * @param Source 申告するもの。
	 * @return 受け取れたら true。
	 */
	bool AddSource( TUniquePtr<IMusicStateSource> Source ) noexcept;

	/**
	 * その場で 1 件申告する。
	 *
	 * @details
	 * 溜まるだけで、曲が変わるのはこのフレームの決まった 1 か所。毎フレーム申告し続けないと、
	 * 次のフレームには «申告なし» に戻る (押しっぱなしの状態を表すのに向く)。
	 * @param Request 申告。
	 * @return 溜められたら true。
	 */
	bool RequestState( const FMusicStateRequest& Request ) noexcept;

	/**
	 * 差し込みの一音を鳴らすよう頼む。
	 *
	 * @details 曲は止めない。決着や取得の合図に使う。
	 * @param AssetPath 鳴らすもののパス。
	 * @param Volume 音量。
	 */
	void PlayStinger( const FString& AssetPath, f32 Volume = 1.0f ) noexcept;

	/** 曲を止める。 */
	void Stop() noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details 渡すのは実時間の経過秒。CAudioSubsystem を進めた直後に呼ぶこと。
	 * @param UnscaledDeltaSeconds 前フレームからの実経過秒。
	 */
	void Update( f32 UnscaledDeltaSeconds ) noexcept;

	/** 切り替えの最中かを返す。 */
	bool IsTransitioning() const noexcept { return m_Director.IsTransitioning(); }

	/** 直近に決まった状態を返す。 */
	EMusicState GetCurrentState() const noexcept { return m_CurrentState; }

	/** 登録されている曲の数を返す。 */
	usize GetTrackCount() const noexcept { return m_Catalog.Num(); }

	/** 申告するものを受け取った数を返す。 */
	usize GetSourceCount() const noexcept { return m_Sources.Num(); }

	/** いま鳴っている曲のパスを返す (無ければ空)。 */
	FString GetCurrentTrackPath() const;

private:
	/** 申告するものを一巡して集める。 */
	void CollectFromSources() noexcept;

	/** 集まった申告から決めて、エンジンへ渡す。 */
	void ApplyResolvedState() noexcept;

	/** 曲の切り替えを決めるエンジン側。 */
	CMusicDirector m_Director;

	/** 状態と曲の対応表。 */
	CMusicTrackCatalog m_Catalog;

	/** 申告をまとめる係。 */
	CMusicStateArbiter m_Arbiter;

	/** 差し込みの一音を回す係。 */
	CMusicStingerPump m_StingerPump;

	/** 申告するもの。寿命をここで持つ。 */
	TArray<TUniquePtr<IMusicStateSource>> m_Sources;

	/** 鳴らす側。所有はしない。 */
	CAudioSubsystem* m_Audio = nullptr;

	/** 直近に決まった状態。 */
	EMusicState m_CurrentState = EMusicState::Silent;
};

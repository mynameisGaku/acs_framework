// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Audio/Spatial/SpatialListenerBinder.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialPlayRequest.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialSfxRouter.h"

using namespace acs;
using namespace acs::game;

class CAudioSubsystem;

/**
 * 場所のある音を扱うサブシステム。
 *
 * @details
 * 距離と向きの計算はエンジン (CSpatialAudio) が持っている。ここが引き受けるのは、
 * Engineが発行する音源番号の寿命・聴く位置の更新・鳴らす側への受け渡しをまとめる。
 * 距離減衰と左右位置は、再生を始める時点の同じvoiceへ一度に反映する。
 *
 * 同じ地点から繰り返し鳴らすものは Acquire → 必要なら UpdateSource → PlayFromSource → Release、
 * 一度きりのもの (着弾音など) は PlayOnce を使う。再生中voiceの位置追従はこの型の責務外。
 *
 * @code
 * Spatial->Bind( *Audio );
 * Spatial->SetListenerNode( PlayerNode );
 *
 * FSpatialPlayRequest Request;
 * Request.AssetPath = FString( "Audio/Hit.wav" );
 * Request.Position = HitPoint;
 * Spatial->PlayOnce( Request );
 * @endcode
 */
class CSpatialAudioSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSpatialAudioSubsystem )

	/** subsystem 終了時に非所有参照を切る。 */
	void OnDeinitialize() noexcept override;

	/**
	 * 鳴らす側へ繋ぐ。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡すまでは何を頼んでも鳴らない。
	 * @param Audio 鳴らす側。このサブシステムより長く生きること。
	 */
	void Bind( CAudioSubsystem& Audio ) noexcept { m_Audio = &Audio; }

	/**
	 * 聴く位置を、あるノードに追いかけさせる。
	 *
	 * @param Node 追いかけるノード。nullptr で追いかけるのをやめる。
	 */
	void SetListenerNode( ANode* Node ) noexcept { m_Listener.SetTarget( Node ); }

	/**
	 * 聴く位置を直接決める。
	 *
	 * @details ノードを追いかけていないときに使われる。
	 * @param Listener 聴く位置と向き。
	 */
	void SetListener( const FAudioListener& Listener ) noexcept { m_Listener.SetManualListener( Listener ); }

	/**
	 * 繰り返し効果音を鳴らす場所を登録する。
	 *
	 * @param Position 世界座標。
	 * @param Velocity 動いている速さ。
	 * @param MaxDistance この距離以上では聞こえなくする距離。
	 * @param Curve 距離から音量を求める曲線。
	 * @return 借りた番号 (借りられなければ 0)。
	 */
	u32 AcquireSource( FVec3 Position, FVec3 Velocity = FVec3::Zero(), f32 MaxDistance = 20.0f, EAttenuationCurve Curve = EAttenuationCurve::Linear ) noexcept;

	/**
	 * 登録した場所を動かす。
	 *
	 * @param SourceId 借りた番号。
	 * @param Position 世界座標。
	 * @param Velocity 動いている速さ。
	 */
	void UpdateSource( u32 SourceId, FVec3 Position, FVec3 Velocity = FVec3::Zero() ) noexcept;

	/**
	 * 登録した場所を外して番号を返す。
	 *
	 * @param SourceId 借りた番号。
	 */
	void ReleaseSource( u32 SourceId ) noexcept;

	/**
	 * 登録済みの場所から 1 回鳴らす。
	 *
	 * @param SourceId 借りた番号。
	 * @param Request 何を鳴らすか。Position、Velocity、MaxDistance、AttenuationCurveは無視し、登録時と更新後の場所を使う。
	 * @return 鳴らしたら true。
	 */
	bool PlayFromSource( u32 SourceId, const FSpatialPlayRequest& Request ) noexcept;

	/**
	 * その場かぎりの音を 1 回鳴らす。
	 *
	 * @details 番号を借りて鳴らし、すぐ返す。着弾音のように鳴らしっぱなしにしないもの向け。
	 * @param Request 何をどこで鳴らすか。
	 * @return 鳴らしたら true。
	 */
	bool PlayOnce( const FSpatialPlayRequest& Request ) noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details 聴く位置を作り直してから、エンジンを進める。実時間で渡すこと。
	 * @param UnscaledDeltaSeconds 前フレームからの実経過秒。
	 */
	void Update( f32 UnscaledDeltaSeconds ) noexcept;

	/** 登録されている場所の数を返す。 */
	u32 GetSourceCount() const noexcept { return m_Spatial.SourceCount(); }

	/** 直近に出力へ渡した «どちらから聞こえるか» を返す。 */
	f32 GetLastPan() const noexcept { return m_Router.GetLastPan(); }

	/** 直近に渡した音量を返す。 */
	f32 GetLastVolume() const noexcept { return m_Router.GetLastVolume(); }

	/** 遠すぎて鳴らさなかった数を返す。 */
	u64 GetSkippedCount() const noexcept { return m_Router.GetSkippedCount(); }

	/** 出力先や素材の問題で鳴らせなかった数を返す。 */
	u64 GetFailedCount() const noexcept { return m_Router.GetFailedCount(); }

private:
	/** ノードまたは手動指定から最新の聴取位置をACSへ反映する。 */
	void RefreshListener_Internal() noexcept;

	/** 距離と向きを計算するエンジン側。 */
	CSpatialAudio m_Spatial;

	/** 聴く位置を作る係。 */
	CSpatialListenerBinder m_Listener;

	/** 鳴らす側へ流す係。 */
	CSpatialSfxRouter m_Router;

	/** 鳴らす側。所有はしない。 */
	CAudioSubsystem* m_Audio = nullptr;
};

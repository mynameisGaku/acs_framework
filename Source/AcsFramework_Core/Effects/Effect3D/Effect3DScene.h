// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Effects/Effect3D/Effect3DPlayer.h"

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * HDR・depth・bloomへ3Dエフェクトを自動で接続する場面。
 *
 * @details 派生側は `PlayEffect3D( "Effects/hit.efkefc", Position )` のように再生するだけでよい。
 * playerの更新、透明3Dパスへの描画、GPUより先に行う終了処理はこの基底が受け持つ。
 */
class AEffect3DScene : public ALegacyScene3DAdapter
{
public:
	/** 空の3Dエフェクトplayerを持つ場面を作る。 */
	AEffect3DScene() noexcept = default;

	/** playerを先に解放してから基底を破棄する。 */
	~AEffect3DScene() noexcept override = default;

	AEffect3DScene( const AEffect3DScene& ) = delete;
	AEffect3DScene& operator=( const AEffect3DScene& ) = delete;

	/**
	 * 素材を位置だけ指定して再生する。
	 *
	 * @param AssetPath `Assets`からの相対パス。
	 * @param WorldPosition world上の位置。
	 * @return 受け付けた再生。指定が不正なら無効。
	 */
	FEffect3DHandle PlayEffect3D( FStringView AssetPath, FVec3 WorldPosition ) noexcept;

	/**
	 * 素材を詳しい指定で再生する。
	 *
	 * @param AssetPath `Assets`からの相対パス。
	 * @param Params 位置・向き・倍率・速度・開始位置。
	 * @return 受け付けた再生。指定が不正なら無効。
	 */
	FEffect3DHandle PlayEffect3D( FStringView AssetPath, const FEffect3DPlayParams& Params ) noexcept;

	/**
	 * 個別停止や追従位置の変更に使うplayerを返す。
	 *
	 * @return この場面が所有するplayer。
	 */
	CEffect3DPlayer& Effects3D() noexcept { return m_Effects; }

	/** 通常の場面更新と同じ明示秒でplayerを進める。 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

	/** playerをGPU停止前に解放してから通常の場面終了を行う。 */
	void OnExit() noexcept override;

protected:
	/**
	 * ACSのHDR透明3Dパスへplayerの描画を追加する。
	 *
	 * @param Context 現在のHDR/depth、カメラ、描画command。
	 * @return 外部commandを追加したら true。
	 */
	bool OnRenderTransparent3D( const FScene3DTransparentRenderContext& Context ) noexcept override;

private:
	/** この場面だけが所有する3Dエフェクトplayer。 */
	CEffect3DPlayer m_Effects;
};

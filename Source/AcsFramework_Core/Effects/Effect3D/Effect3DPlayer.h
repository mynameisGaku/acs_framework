// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Effects/Effect3D/Effect3DHandle.h"
#include "AcsFramework_Core/Effects/Effect3D/Effect3DPlayParams.h"

#include <acs.h>

using namespace acs;

/**
 * 3D エフェクトの読込・再生・描画を 1 つの場面内で所有する。
 *
 * @details `Play` は描画backendの準備前でも呼べる。その場合は再生指定を保持し、最初の
 * `Render` でD3D12を借りられた時点から始める。時刻・カメラ・描画先は引数からだけ受け取る。
 */
class CEffect3DPlayer
{
public:
	/** 空の player を作る。 */
	CEffect3DPlayer() noexcept;

	/** GPUが残っていれば待機してから解放する。 */
	~CEffect3DPlayer() noexcept;

	CEffect3DPlayer( const CEffect3DPlayer& ) = delete;
	CEffect3DPlayer& operator=( const CEffect3DPlayer& ) = delete;
	CEffect3DPlayer( CEffect3DPlayer&& ) = delete;
	CEffect3DPlayer& operator=( CEffect3DPlayer&& ) = delete;

	/**
	 * 素材を位置だけ指定して再生する。
	 *
	 * @param AssetPath `Assets` からの相対パス。
	 * @param WorldPosition world 上の位置。
	 * @return 受け付けた再生。パスまたは位置が不正なら無効。
	 */
	FEffect3DHandle Play( FStringView AssetPath, FVec3 WorldPosition ) noexcept;

	/**
	 * 素材を詳しい指定で再生する。
	 *
	 * @param AssetPath `Assets` からの相対パス。
	 * @param Params 位置・向き・倍率・速度・開始位置。
	 * @return 受け付けた再生。指定が不正なら無効。
	 */
	FEffect3DHandle Play( FStringView AssetPath, const FEffect3DPlayParams& Params ) noexcept;

	/**
	 * 1 つの再生を止める。準備待ちの再生も取り消せる。
	 *
	 * @param Handle 止める再生。
	 * @return 存在して止めたら true。
	 */
	bool Stop( FEffect3DHandle Handle ) noexcept;

	/** 準備待ちを含む全再生を止める。読込済み素材は保持する。 */
	void StopAll() noexcept;

	/**
	 * 再生中または準備待ちか返す。
	 *
	 * @param Handle 調べる再生。
	 * @return player が保持していれば true。
	 */
	bool IsPlaying( FEffect3DHandle Handle ) const noexcept;

	/**
	 * 再生位置を変える。
	 *
	 * @param Handle 変える再生。
	 * @param WorldPosition 新しい world 位置。
	 * @return 再生が存在し、有限値なら true。
	 */
	bool SetPosition( FEffect3DHandle Handle, FVec3 WorldPosition ) noexcept;

	/**
	 * 再生方向を度単位で変える。
	 *
	 * @param Handle 変える再生。
	 * @param RotationDeg 軸ごとの角度。
	 * @return 再生が存在し、有限値なら true。
	 */
	bool SetRotationDeg( FEffect3DHandle Handle, FVec3 RotationDeg ) noexcept;

	/**
	 * 再生倍率を変える。
	 *
	 * @param Handle 変える再生。
	 * @param Scale 0 ではない軸ごとの倍率。
	 * @return 再生が存在し、倍率が有効なら true。
	 */
	bool SetScale( FEffect3DHandle Handle, FVec3 Scale ) noexcept;

	/**
	 * 再生速度を変える。
	 *
	 * @param Handle 変える再生。
	 * @param Speed 0 より大きい倍率。1 が素材どおり。
	 * @return 再生が存在し、速度が有効なら true。
	 */
	bool SetSpeed( FEffect3DHandle Handle, f32 Speed ) noexcept;

	/**
	 * 明示された経過秒だけ再生を進める。
	 *
	 * @param DeltaSeconds 0 以上の有限な経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * ACSの透明3Dパスへエフェクト描画を追加する。
	 *
	 * @param Device D3D12資源を借りるRHIデバイス。
	 * @param Commands 外部commandを追加する現在のRHI command list。
	 * @param Camera 描画時点のカメラ。
	 * @param ColorTarget 既存内容を保持してbind済みのHDR描画先。
	 * @param DepthTarget bind済みのdepth描画先。無い場合はnullptr。
	 * @return 外部D3D12 commandを追加し、ACS側の状態復旧が必要なら true。
	 */
	bool Render( IRhiDevice& Device, IRhiCommandList& Commands, const CCamera& Camera, IRhiTexture& ColorTarget, IRhiTexture* DepthTarget ) noexcept;

	/**
	 * D3D12描画backendの準備が済んでいるか返す。
	 *
	 * @return 再生を直ちに開始できるなら true。
	 */
	bool IsReady() const noexcept;

	/** GPU完了を待ち、再生・素材・描画backendを解放する。何度呼んでもよい。 */
	void Shutdown() noexcept;

private:
	/** EffekseerとD3D12を公開headerから隠す実装。 */
	struct FImpl;

	/** 場面が単独所有する実装。 */
	TUniquePtr<FImpl> m_Impl;
};

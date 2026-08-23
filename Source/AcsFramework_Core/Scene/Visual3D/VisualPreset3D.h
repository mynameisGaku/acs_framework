// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 3D場面の見た目と描画負荷をまとめて選ぶプリセット。
 *
 * @details
 * 個別調整が必要な場合は、適用後に`AmbientOcclusion()`、`Reflections()`、
 * `GlobalIllumination()`、`PostParams()`の必要な値だけを上書きできる。
 */
enum class EVisualPreset3D : u8
{
	/** 画面空間反射と間接光を切り、軽い遮蔽とFXAAだけを使う。 */
	Performance = 0u,

	/** 遮蔽、控えめな反射と間接光、FXAAを組み合わせる標準設定。 */
	Balanced = 1u,

	/** 強めの反射と間接光、TAA、映画調の仕上げを使う高品質設定。 */
	Cinematic = 2u,
};

/**
 * 指定プリセットをACSの3D描画設定へ原子的に反映する。
 *
 * @details GPU資源への参照、フレーム時刻、行列などACSが更新する実行中の値は維持する。
 * @param Preset 適用する既知のプリセット。
 * @param AmbientOcclusion 遮蔽設定の更新先。
 * @param Reflections 画面空間反射設定の更新先。
 * @param GlobalIllumination 画面空間間接光設定の更新先。
 * @param PostProcess ポスト処理設定の更新先。
 * @return 既知のプリセットを完全に反映できた場合だけtrue。未知値では全出力を維持する。
 */
bool TryApplyVisualPreset3DSettings( EVisualPreset3D Preset,
	FScene3DAmbientOcclusion& AmbientOcclusion,
	FScene3DReflections& Reflections,
	FScene3DGlobalIllumination& GlobalIllumination,
	FPostProcessParams& PostProcess ) noexcept;

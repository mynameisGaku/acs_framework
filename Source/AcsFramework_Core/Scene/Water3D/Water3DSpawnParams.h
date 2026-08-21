// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 3D水面を1枚置くときの指定。
 *
 * @details
 * 既定値だけで青緑の波立つ水面になる。位置と広さだけ決めれば使え、光学的な吸収や
 * 散乱などの高度な値はACSの`AWaterSurface3DComponent`の既定へ任せる。
 */
struct FWater3DSpawnParams
{
	/** 水面の中心位置。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** X方向とZ方向の広さ。どちらも0より大きくする。 */
	FVec2 Size{ 4.0f, 4.0f };

	/** 浅い場所に見せる色。 */
	FVec3 ShallowColor{ 0.055f, 0.38f, 0.50f };

	/** 深い場所に見せる色。 */
	FVec3 DeepColor{ 0.008f, 0.055f, 0.16f };

	/** 水面上の流れる向き。長さ0は受け付けない。 */
	FVec2 FlowDirection{ 0.92f, 0.38f };

	/** 反射の粗さ。0に近いほど鏡に近く、1で粗くなる。 */
	f32 Roughness = 0.105f;

	/** 細かな波の法線の強さ。 */
	f32 NormalStrength = 0.82f;

	/** 1ワールド単位あたりの細かな波の繰り返し量。 */
	f32 NormalTiling = 0.075f;

	/** 大きな波の上下幅。0で平らになる。 */
	f32 WaveAmplitude = 0.105f;

	/** 大きな波の空間的な細かさ。 */
	f32 WaveScale = 0.78f;

	/** 波と法線が進む速さ。負なら逆向きに進む。 */
	f32 WaveSpeed = 0.72f;

	/** 波紋が1秒で進む距離。 */
	f32 RippleSpeed = 2.65f;

	/** 波紋の山から次の山までの距離。 */
	f32 RippleWavelength = 0.52f;

	/** 波紋が消えるまでの秒数。 */
	f32 RippleLifetime = 4.0f;

	/** 波紋が距離とともに弱くなる量。 */
	f32 RippleDamping = 0.78f;

	/** 背景を水面越しにずらす強さ。 */
	f32 RefractionStrength = 0.72f;

	/** 深度画像が無い場合に使う見かけの水深。 */
	f32 OpticalDepth = 1.35f;

	/** 波頭と接触部へ出す泡の強さ。 */
	f32 FoamIntensity = 0.82f;

	/** ノードに付ける名前。 */
	FStringView Name = FStringView( "Water" );

	/**
	 * 水面として意味のある有限値だけで構成されているか返す。
	 *
	 * @return 置ける指定ならtrue。
	 */
	bool IsValid() const noexcept;
};

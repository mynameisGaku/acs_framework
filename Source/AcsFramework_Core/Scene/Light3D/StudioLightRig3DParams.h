// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawnParams.h"

/**
 * 被写体を読みやすくする3点照明を、中心・見る方向・半径から組み立てる指定。
 *
 * @details
 * 既存または時刻連動の太陽はそのまま影の主光源として残し、キー、フィル、リムの3灯を
 * 点光源として追加する。ACSが描画する点光源4灯のうち3灯を使用する。
 */
struct FStudioLightRig3DParams
{
	/** 配置先親から見た被写体の中心。root直下ではworld位置になる。 */
	FVec3 SubjectCenter{ 0.0f, 1.0f, 0.0f };

	/** 被写体からカメラへ向かう方向。高さ成分を除いた向きで左右を決める。 */
	FVec3 ViewDirectionToCamera{ 0.0f, 0.0f, -1.0f };

	/** 被写体を覆う球のおおよその半径。3灯の位置と到達距離の基準にする。 */
	f32 SubjectRadius = 1.0f;

	/** 暖かい主光の線形RGB。 */
	FVec3 KeyColor{ 1.0f, 0.84f, 0.68f };

	/** 主光の強さ。 */
	f32 KeyIntensity = 2.0f;

	/** 陰側を持ち上げる寒色光の線形RGB。 */
	FVec3 FillColor{ 0.52f, 0.68f, 1.0f };

	/** 陰側を持ち上げる光の強さ。 */
	f32 FillIntensity = 0.75f;

	/** 背面の輪郭を分ける暖色光の線形RGB。 */
	FVec3 RimColor{ 1.0f, 0.58f, 0.32f };

	/** 背面の輪郭を分ける光の強さ。 */
	f32 RimIntensity = 1.3f;

	/** 被写体半径へ掛ける3灯共通の到達距離倍率。 */
	f32 RangeScale = 4.5f;

	/**
	 * 中心、見る方向、半径だけを指定した3点照明設定を作る。
	 *
	 * @param InSubjectCenter 配置先親から見た被写体中心。
	 * @param InViewDirectionToCamera 被写体からカメラへ向かう方向。
	 * @param InSubjectRadius 被写体を覆うおおよその半径。
	 * @return 既定の色と強さを持つ3点照明設定。不正値は`IsValid`で拒否される。
	 */
	static FStudioLightRig3DParams AroundSubject( FVec3 InSubjectCenter,
		FVec3 InViewDirectionToCamera, f32 InSubjectRadius ) noexcept;

	/**
	 * 3灯の親内位置、色、強さ、到達距離を計算する。
	 *
	 * @details 失敗時は3つの出力を変更しない。高さ成分だけの見る方向、0以下の半径、
	 * 負の色や強さ、NaN、無限大、派生値の溢れを拒否する。
	 * @param OutKey 成功時に受け取る正面左上の主光。
	 * @param OutFill 成功時に受け取る正面右側の補助光。
	 * @param OutRim 成功時に受け取る背面上側の輪郭光。
	 * @return 3灯を安全に配置できる値へ変換できた場合だけtrue。
	 */
	bool TryBuildLights( FLight3DSpawnParams& OutKey,
		FLight3DSpawnParams& OutFill,
		FLight3DSpawnParams& OutRim ) const noexcept;

	/** 中心、方向、半径と3灯の見た目から、安全な点光源3灯を作れるか返す。 */
	bool IsValid() const noexcept;
};

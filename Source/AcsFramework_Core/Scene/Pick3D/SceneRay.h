// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 世界を貫く 1 本の線。
 *
 * @details
 * 「そこに何があるか」を訊くための道具。マウスの位置から作る (`FromScreen`) のが一番よく
 * 使う形で、視点から地面へ落とす (`Down`) のが次に多い。
 *
 * **向きは正規化しておくこと。** していないと当たった距離が世界の単位でなくなり、
 * `MaxDistance` の意味も変わる。作る関数はどれも正規化して返す。
 */
struct FSceneRay
{
	/** 始点 (世界座標)。 */
	FVec3 Origin{ 0.0f, 0.0f, 0.0f };

	/** 向き (正規化済み)。 */
	FVec3 Direction{ 0.0f, 0.0f, 1.0f };

	/**
	 * ここより遠いものは当たらない。
	 *
	 * @details
	 * **無限にしないこと。** 遠くの物まで拾うと、画面外の物を «掴んだ» ことになる。
	 * 既定の 1000 は「見えている範囲」のつもりの値。
	 */
	f32 MaxDistance = 1000.0f;

	/**
	 * 始点と向きから作る。
	 *
	 * @param InOrigin 始点。
	 * @param InDirection 向き (正規化していなくてよい)。
	 * @param InMaxDistance 届く距離。
	 * @return 正規化済みの線。向きが 0 なら +Z を向く。
	 */
	static FSceneRay FromDirection( FVec3 InOrigin, FVec3 InDirection, f32 InMaxDistance = 1000.0f ) noexcept;

	/**
	 * 画面の位置から、カメラを通して世界へ伸ばす。
	 *
	 * @details
	 * **クリックした物を取るのはこれ。** 画面の左上を (0, 0)、右下を (Width, Height) とする。
	 *
	 * @param Camera 見ているカメラ。
	 * @param ScreenX 画面の横位置 (ピクセル)。
	 * @param ScreenY 画面の縦位置 (ピクセル)。
	 * @param Width 画面の幅 (ピクセル)。0 なら +Z を向いた線を返す。
	 * @param Height 画面の高さ (ピクセル)。0 なら同上。
	 * @param InMaxDistance 届く距離。
	 * @return カメラ位置から画面のその点へ向かう線。
	 */
	static FSceneRay FromScreen( const CCamera& Camera, f32 ScreenX, f32 ScreenY,
		u32 Width, u32 Height, f32 InMaxDistance = 1000.0f ) noexcept;

	/**
	 * 真下へ落とす。
	 *
	 * @details 足元に何があるかを調べるとき。
	 *
	 * @param InOrigin 始点。
	 * @param InMaxDistance 届く距離。
	 * @return 下向きの線。
	 */
	static FSceneRay Down( FVec3 InOrigin, f32 InMaxDistance = 1000.0f ) noexcept;

	/**
	 * ACS の判定関数へ渡せる形にする。
	 *
	 * @return 同じ始点と向きを持つ `FRay3`。
	 */
	FRay3 ToRay3() const noexcept;

	/**
	 * 使える線か。
	 *
	 * @return 向きが 0 でなく、届く距離が正なら true。
	 */
	bool IsValid() const noexcept;
};

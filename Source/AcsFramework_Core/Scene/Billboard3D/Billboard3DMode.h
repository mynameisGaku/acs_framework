// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3D画像板がカメラへ向くときに許す回転軸。 */
enum class EBillboard3DMode : u8
{
	/** 上下も含めてカメラ位置へ正面を向ける。 */
	FaceCamera = 0,

	/** worldのY軸を立てたまま、水平方向だけカメラ位置へ向ける。 */
	FaceCameraYAxis,
};

/** 公開列挙値として扱えるならtrueを返す。 */
constexpr bool IsBillboard3DModeValid( EBillboard3DMode Mode ) noexcept
{
	return Mode == EBillboard3DMode::FaceCamera || Mode == EBillboard3DMode::FaceCameraYAxis;
}

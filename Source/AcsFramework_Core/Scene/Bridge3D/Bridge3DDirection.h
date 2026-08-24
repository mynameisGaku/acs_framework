// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 橋が入口から出口へ伸びる、配置先親のローカルXZ平面上の軸方向。 */
enum class EBridge3DDirection : u8
{
	/** X正方向へ伸ばす。橋幅はZ方向になる。 */
	PositiveX,

	/** X負方向へ伸ばす。橋幅はZ方向になる。 */
	NegativeX,

	/** Z正方向へ伸ばす。橋幅はX方向になる。 */
	PositiveZ,

	/** Z負方向へ伸ばす。橋幅はX方向になる。 */
	NegativeZ,
};

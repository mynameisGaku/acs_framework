// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 階段が低い側から高い側へ伸びる、ローカルXZ平面上の軸方向。 */
enum class EStairs3DDirection : u8
{
	/** X正方向へ上る。階段幅はZ方向になる。 */
	PositiveX,

	/** X負方向へ上る。階段幅はZ方向になる。 */
	NegativeX,

	/** Z正方向へ上る。階段幅はX方向になる。 */
	PositiveZ,

	/** Z負方向へ上る。階段幅はX方向になる。 */
	NegativeZ,
};

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 柵が始点から終点へ伸びる、配置先親のローカルXZ平面上の軸方向。 */
enum class EFence3DDirection : u8
{
	/** X正方向へ伸びる。柵の厚みはZ方向になる。 */
	PositiveX,

	/** X負方向へ伸びる。柵の厚みはZ方向になる。 */
	NegativeX,

	/** Z正方向へ伸びる。柵の厚みはX方向になる。 */
	PositiveZ,

	/** Z負方向へ伸びる。柵の厚みはX方向になる。 */
	NegativeZ,
};

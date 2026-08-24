// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 出入口枠の壁幅を、配置先親のローカルXZ平面上で伸ばす軸。 */
enum class EDoorway3DOrientation : u8
{
	/** 壁幅をX方向、厚みをZ方向へ取る。 */
	AlongX,

	/** 壁幅をZ方向、厚みをX方向へ取る。 */
	AlongZ,
};

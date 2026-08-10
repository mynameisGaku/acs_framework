// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** シーンを切り替えるときの見せ方。 */
enum class ESceneTransition : u8
{
	/** 幕を使わず、その場で切り替える。 */
	Cut,

	/** 暗転後に切り替え、明転する。 */
	Fade,
};

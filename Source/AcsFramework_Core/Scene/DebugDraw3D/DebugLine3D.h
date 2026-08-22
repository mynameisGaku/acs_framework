// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3Dデバッグオーバーレイへ1フレームだけ描く線。 */
struct FDebugLine3D
{
	/** world座標の始点。 */
	FVec3 Start{};

	/** world座標の終点。 */
	FVec3 End{};

	/** 0から1の範囲で指定するRGBA色。 */
	FVec4 Color{ 0.20f, 0.95f, 1.0f, 1.0f };

	/** 座標と色が描画器へ安全に渡せる有限値ならtrue。 */
	bool IsValid() const noexcept;
};

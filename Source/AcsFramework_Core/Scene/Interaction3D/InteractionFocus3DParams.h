// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3D視線フォーカスが画面から飛ばす有限レイの設定。 */
struct FInteractionFocus3DParams
{
	/** 画面の左上を0、右下を1とする視線位置。既定は中央。 */
	FVec2 ScreenPosition{ 0.5f, 0.5f };

	/** カメラから対象を捉える最大world距離。 */
	f32 MaximumDistance = 4.0f;

	/** 画面位置と距離を安全なレイへ変換できる値ならtrue。 */
	bool IsValid() const noexcept;
};

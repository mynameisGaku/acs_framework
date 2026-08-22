// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractionHighlight3DParams.h"
#include "Common/Test/TestHarness.h"

#include <limits>

void RunInteractionHighlight3DParamsTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FInteractionHighlight3DParams / ACS選択輪郭の安全な表示域を保つ" );

	FInteractionHighlight3DParams Params;
	Harness.Check( Params.IsValid(), "既定値をそのまま選択輪郭へ渡せる" );

	Params.Color.x = -0.01f;
	Harness.Check( !Params.IsValid(), "負の色を拒否する" );
	Params.Color.x = 1.0f;
	Params.Intensity = 4.01f;
	Harness.Check( !Params.IsValid(), "上限を超える強さを拒否する" );
	Params.Intensity = 1.0f;
	Params.ThicknessPixels = 0.0f;
	Harness.Check( !Params.IsValid(), "見えない幅0を拒否する" );
	Params.ThicknessPixels = std::numeric_limits<f32>::quiet_NaN();
	Harness.Check( !Params.IsValid(), "有限でない輪郭幅を拒否する" );

	Params = FInteractionHighlight3DParams{};
	Params.bEnabled = false;
	Harness.Check( Params.IsValid(), "表示無効化は設定値を壊さない" );
}

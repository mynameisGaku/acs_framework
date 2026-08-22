// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DLayout.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 浮動小数の小さな誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.001f;
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference < kTolerance, Label );
	}
}

void RunInteractionReticle3DParamsTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FInteractionReticle3DParams / 読みやすい既定値と安全な寸法を保つ" );

	{
		FInteractionReticle3DParams Params;
		Harness.Check( Params.IsValid(), "既定値をそのまま描画へ渡せる" );

		Params.Thickness = 0.0f;
		Harness.Check( !Params.IsValid(), "見えない太さ0を拒否する" );
		Params.Thickness = 2.0f;
		Params.FocusedColor.x = 1.01f;
		Harness.Check( !Params.IsValid(), "色の範囲外を拒否する" );
		Params.FocusedColor.x = 1.0f;
		Params.ShadowOffset = -1.0f;
		Harness.Check( !Params.IsValid(), "負の影ずれを拒否する" );
		Params.ShadowOffset = 1.0f;
		Params.FocusedScale = std::numeric_limits<f32>::infinity();
		Harness.Check( !Params.IsValid(), "有限でない対象倍率を拒否する" );
	}

	Harness.BeginSuite( "MakeInteractionReticle3DLayout / 判定位置と対象状態だけから矩形を作る" );

	{
		const FInteractionReticle3DParams Params;
		const FInteractionReticle3DLayout Idle = MakeInteractionReticle3DLayout( Params, FVec2{ 0.5f, 0.5f }, 800u, 400u, false, true );
		Harness.Check( Idle.bVisible, "対象登録中は通常照準を作る" );
		CheckNear( Harness, Idle.Rectangles[0].x, 388.0f, "左線を画面中央の左へ置く" );
		CheckNear( Harness, Idle.Rectangles[0].y, 199.0f, "横線を画面中央の高さへ置く" );
		CheckNear( Harness, Idle.Rectangles[0].z, 7.0f, "通常時の線長を保つ" );
		CheckNear( Harness, Idle.Rectangles[4].x, 399.0f, "中央点をX中央へ置く" );
		Harness.Check( Idle.Color.x == Params.IdleColor.x && Idle.Color.y == Params.IdleColor.y, "通常色を選ぶ" );

		const FInteractionReticle3DLayout Focused = MakeInteractionReticle3DLayout( Params, FVec2{ 0.25f, 0.75f }, 800u, 400u, true, true );
		Harness.Check( Focused.bVisible, "対象を捉えた照準を作る" );
		CheckNear( Harness, Focused.Rectangles[1].x, 200.0f + Params.Gap * Params.FocusedScale, "判定位置と対象倍率を右線へ反映する" );
		CheckNear( Harness, Focused.Rectangles[2].y, 300.0f - ( Params.Gap + Params.ArmLength ) * Params.FocusedScale, "判定位置と対象倍率を上線へ反映する" );
		Harness.Check( Focused.Color.x == Params.FocusedColor.x && Focused.Color.y == Params.FocusedColor.y, "対象色を選ぶ" );

		Harness.Check( !MakeInteractionReticle3DLayout( Params, FVec2{ 0.5f, 0.5f }, 800u, 400u, false, false ).bVisible, "対象登録が0件なら隠す" );
		Harness.Check( !MakeInteractionReticle3DLayout( Params, FVec2{ 0.5f, 0.5f }, 0u, 400u, false, true ).bVisible, "幅0の描画先を拒否する" );
		Harness.Check( !MakeInteractionReticle3DLayout( Params, FVec2{ std::numeric_limits<f32>::quiet_NaN(), 0.5f }, 800u, 400u, false, true ).bVisible, "有限でない判定位置を拒否する" );
	}
}

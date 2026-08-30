// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionAxisResponse.h"
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 左スティック2軸を個別に返す検証用装置。 */
	class CActionAxisResponseDevice final : public IActionDeviceReader
	{
	public:
		/** 検証で返す左スティック値を設定する。 */
		void SetLeftStick( FVec2 Value ) noexcept { m_LeftStick = Value; }

		/** 軸応答の検証ではキーを押していない。 */
		bool IsKeyDown( EKey ) const noexcept override { return false; }

		/** 軸応答の検証ではパッドボタンを押していない。 */
		bool IsGamepadButtonDown( u32, EGamepadButton ) const noexcept override
		{
			return false;
		}

		/** 1人目の左スティック2軸だけを返す。 */
		f32 GetGamepadAxis( u32 PlayerIndex, EGamepadAxis Axis ) const noexcept override
		{
			if ( PlayerIndex != 0u ) return 0.0f;
			if ( Axis == EGamepadAxis::LeftX ) return m_LeftStick.x;
			if ( Axis == EGamepadAxis::LeftY ) return m_LeftStick.y;
			return 0.0f;
		}

	private:
		/** 検証で返す左スティックのXとY。 */
		FVec2 m_LeftStick;
	};
}


/**
 * 1軸と2軸の遊び除去、応答曲線、入力アダプターと失敗原子性を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionAxisResponseTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FActionAxisResponse / 中心を除いて最大値まで再正規化" );

	{
		const FActionAxisResponse Response;
		f32 Output = 99.0f;
		Harness.Check( Response.IsValid(), "既定設定をそのまま使える" );
		Harness.Check( Response.TryApply( 0.0f, Output ) && Output == 0.0f,
			"中心入力を0にする" );
		Harness.Check( Response.TryApply( 0.15f, Output ) && Output == 0.0f,
			"内側境界を0に含める" );
		Harness.Check( Response.TryApply( 0.575f, Output ),
			"遊びの外側を変換できる" );
		Harness.CheckNearF32( Output, 0.5f, 0.000001f,
			"残った入力範囲の中央を0.5へ詰め直す" );
		Harness.Check( Response.TryApply( -0.575f, Output ),
			"負方向も変換できる" );
		Harness.CheckNearF32( Output, -0.5f, 0.000001f,
			"負方向の符号を保つ" );
		Harness.Check( Response.TryApply( 2.0f, Output ) && Output == 1.0f,
			"範囲外の正入力を最大値へ止める" );
		Harness.Check( Response.TryApply( -2.0f, Output ) && Output == -1.0f,
			"範囲外の負入力を最小値へ止める" );
	}

	Harness.BeginSuite( "FActionAxisResponse / 外周補正と応答指数" );

	{
		FActionAxisResponse Response;
		Response.InnerDeadZone = 0.2f;
		Response.OuterDeadZone = 0.1f;
		Response.ResponseExponent = 2.0f;

		f32 Output = 0.0f;
		Harness.Check( Response.TryApply( 0.55f, Output ),
			"内外の遊びと応答指数を同時に適用できる" );
		Harness.CheckNearF32( Output, 0.25f, 0.000001f,
			"再正規化した0.5へ二乗応答を掛ける" );
		Harness.Check( Response.TryApply( 0.9f, Output ) && Output == 1.0f,
			"外周境界から最大入力として扱う" );
		Harness.Check( Response.TryApply( -0.9f, Output ) && Output == -1.0f,
			"負方向の外周境界も最大強度にする" );
	}

	Harness.BeginSuite( "FActionAxisResponse / 2軸の方向を保つ" );

	{
		FActionAxisResponse Response;
		Response.InnerDeadZone = 0.2f;
		FVec2 Output{ 9.0f, 8.0f };
		Harness.Check( Response.TryApplyRadial( FVec2{ 0.3f, 0.4f }, Output ),
			"2軸入力を長さで変換できる" );
		Harness.CheckNearF32( Output.x, 0.225f, 0.000001f,
			"元のX方向比率を保つ" );
		Harness.CheckNearF32( Output.y, 0.3f, 0.000001f,
			"元のY方向比率を保つ" );
		Harness.CheckNearF32( std::sqrt( Output.x * Output.x + Output.y * Output.y ),
			0.375f, 0.000001f, "2軸の長さだけを再正規化する" );

		Harness.Check( Response.TryApplyRadial( FVec2{ 0.1f, 0.1f }, Output )
			&& Output.x == 0.0f && Output.y == 0.0f,
			"円形の内側遊びを0にする" );
		Harness.Check( Response.TryApplyRadial( FVec2{ 1.0f, 1.0f }, Output ),
			"斜めの範囲外入力を変換できる" );
		Harness.CheckNearF32( Output.x, 0.70710677f, 0.000001f,
			"斜め入力を成分ごとに切らず正規化する" );
		Harness.CheckNearF32( Output.y, 0.70710677f, 0.000001f,
			"斜め入力の方向を最大値でも保つ" );
	}

	Harness.BeginSuite( "FActionAxisResponse / アクション入力アダプター" );

	{
		FActionInput Input;
		Input.SetAxis( 0u, 0.575f );
		Input.SetAxis( 1u, 0.3f );
		Input.SetAxis( 2u, 0.4f );
		const FActionAxisResponse Response;

		f32 AxisOutput = 9.0f;
		Harness.Check( Response.TryApply( Input, 0u, AxisOutput ),
			"FActionInputから1軸を直接変換できる" );
		Harness.CheckNearF32( AxisOutput, 0.5f, 0.000001f,
			"指定した軸だけを読む" );

		FVec2 RadialOutput{ 9.0f, 8.0f };
		Harness.Check( Response.TryApplyRadial( Input, 1u, 2u, RadialOutput ),
			"FActionInputから異なる2軸を直接変換できる" );
		Harness.Check( RadialOutput.x > 0.0f && RadialOutput.y > RadialOutput.x,
			"指定した2軸の方向を保つ" );
	}

	Harness.BeginSuite( "FActionAxisResponse / バインド表から円形応答へ接続" );

	{
		CActionAxisResponseDevice Device;
		Device.SetLeftStick( FVec2{ 0.14f, 0.20f } );
		FActionAxisResponse Response;

		CActionBindingTable ComponentDeadZoneBindings;
		ComponentDeadZoneBindings.BindGamepadAxis(
			0u, EGamepadAxis::LeftX );
		ComponentDeadZoneBindings.BindGamepadAxis(
			1u, EGamepadAxis::LeftY );
		FVec2 ComponentClippedOutput;
		Harness.Check( Response.TryApplyRadial(
				ComponentDeadZoneBindings.Resolve( Device ),
				0u, 1u, ComponentClippedOutput )
			&& ComponentClippedOutput.x == 0.0f,
			"軸別の既定遊びを先に掛けると斜め方向を失う" );

		CActionBindingTable RadialBindings;
		RadialBindings.BindGamepadAxis(
			0u, EGamepadAxis::LeftX, 0u, 0.0f );
		RadialBindings.BindGamepadAxis(
			1u, EGamepadAxis::LeftY, 0u, 0.0f );
		FVec2 RadialOutput;
		Harness.Check( Response.TryApplyRadial(
				RadialBindings.Resolve( Device ), 0u, 1u, RadialOutput ),
			"binding側の遊び0から円形応答へ接続できる" );
		Harness.CheckNearF32( RadialOutput.x, 0.06350664f, 0.000001f,
			"小さいX成分を円形応答まで保持する" );
		Harness.CheckNearF32( RadialOutput.y, 0.09072378f, 0.000001f,
			"生スティックと同じ斜め方向を保つ" );
	}

	Harness.BeginSuite( "FActionAxisResponse / 不正入力で出力を保つ" );

	{
		FActionAxisResponse Invalid;
		Invalid.InnerDeadZone = 0.8f;
		Invalid.OuterDeadZone = 0.2f;
		Harness.Check( !Invalid.IsValid(), "利用可能範囲を残さない設定を拒否する" );
		Invalid.InnerDeadZone = 0.5f;
		Invalid.OuterDeadZone = std::nextafter( 0.5f, 0.0f );
		Harness.Check( !Invalid.IsValid(),
			"f32では内外境界が同じになる設定を拒否する" );
		Invalid.InnerDeadZone = -0.1f;
		Invalid.OuterDeadZone = 0.0f;
		Harness.Check( !Invalid.IsValid(), "負の内側遊びを拒否する" );
		Invalid.InnerDeadZone = 0.1f;
		Invalid.ResponseExponent = 0.0f;
		Harness.Check( !Invalid.IsValid(), "0の応答指数を拒否する" );
		Invalid.ResponseExponent = std::numeric_limits<f32>::infinity();
		Harness.Check( !Invalid.IsValid(), "有限でない応答指数を拒否する" );

		f32 AxisOutput = 7.0f;
		FVec2 RadialOutput{ 6.0f, 5.0f };
		Harness.Check( !Invalid.TryApply( 0.5f, AxisOutput )
			&& AxisOutput == 7.0f
			&& !Invalid.TryApplyRadial( FVec2{ 0.5f, 0.0f }, RadialOutput )
			&& RadialOutput.x == 6.0f && RadialOutput.y == 5.0f,
			"不正設定では1軸と2軸の出力を変えない" );

		const FActionAxisResponse Response;
		Harness.Check( !Response.TryApply(
				std::numeric_limits<f32>::quiet_NaN(), AxisOutput )
			&& AxisOutput == 7.0f,
			"有限でない1軸入力では出力を変えない" );

		FActionInput Input;
		Input.SetAxis( 0u, 0.5f );
		Harness.Check( !Response.TryApply( Input, kActionAxisCount, AxisOutput )
			&& AxisOutput == 7.0f,
			"範囲外の1軸番号では出力を変えない" );
		Harness.Check( !Response.TryApplyRadial( Input, 0u, 0u, RadialOutput )
			&& RadialOutput.x == 6.0f && RadialOutput.y == 5.0f,
			"重複する2軸番号では出力を変えない" );
		Harness.Check( !Response.TryApplyRadial(
				FVec2{ std::numeric_limits<f32>::infinity(), 0.0f }, RadialOutput )
			&& RadialOutput.x == 6.0f && RadialOutput.y == 5.0f,
			"有限でない2軸入力では出力を変えない" );
	}
}

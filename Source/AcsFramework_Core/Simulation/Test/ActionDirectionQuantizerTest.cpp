// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionAxisResponse.h"
#include "AcsFramework_Core/Simulation/Input/ActionDirectionQuantizer.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 8方向の軸寄り境界について、直前・境界・直後を1象限ぶん確認する。 */
	void CheckEightWayBoundary_Internal( CTestHarness& Harness,
		const FActionDirectionQuantizer& Quantizer, f32 XSign, f32 YSign,
		bool bVerticalMajor, EActionDirection2D CardinalDirection,
		EActionDirection2D DiagonalDirection, const char* BelowLabel,
		const char* BoundaryLabel, const char* AboveLabel )
	{
		/** 実装と同じf32入力領域のtan(22.5度)。 */
		constexpr f32 kBoundary = 0.41421357f;
		/** 軸方向側と斜め方向側で隣接するf32値。 */
		const f32 BelowBoundary = std::nextafter( kBoundary, 0.0f );
		const f32 AboveBoundary = std::nextafter( kBoundary, 1.0f );
		/** 大きい軸を1とし、小さい軸だけを境界の前後へ動かした入力。 */
		const FVec2 BelowAxes = bVerticalMajor
			? FVec2{ XSign * BelowBoundary, YSign }
			: FVec2{ XSign, YSign * BelowBoundary };
		const FVec2 BoundaryAxes = bVerticalMajor
			? FVec2{ XSign * kBoundary, YSign }
			: FVec2{ XSign, YSign * kBoundary };
		const FVec2 AboveAxes = bVerticalMajor
			? FVec2{ XSign * AboveBoundary, YSign }
			: FVec2{ XSign, YSign * AboveBoundary };

		EActionDirection2D Direction = EActionDirection2D::None;
		Harness.Check( Quantizer.TryResolve( BelowAxes,
				EActionDirection2D::None, Direction )
			&& Direction == CardinalDirection, BelowLabel );
		Harness.Check( Quantizer.TryResolve( BoundaryAxes,
				EActionDirection2D::None, Direction )
			&& Direction == CardinalDirection, BoundaryLabel );
		Harness.Check( Quantizer.TryResolve( AboveAxes,
				EActionDirection2D::None, Direction )
			&& Direction == DiagonalDirection, AboveLabel );
	}
}


/**
 * 4/8方向変換、中心ヒステリシス、入力接続、変換ベクトルと失敗原子性を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionDirectionQuantizerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FActionDirectionQuantizer / 中心の開始と解除を分ける" );

	{
		const FActionDirectionQuantizer Quantizer;
		EActionDirection2D Direction = EActionDirection2D::Left;
		Harness.Check( Quantizer.IsValid(), "既定設定をそのまま使える" );
		Harness.Check( Quantizer.TryResolve( FVec2{},
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::None,
			"中心入力をNoneにする" );
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.5f, 0.0f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::None,
			"開始閾値の上だけで方向入力を始める" );
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.6f, 0.0f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Right,
			"開始閾値を超えた方向を返す" );
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.4f, 0.0f },
				EActionDirection2D::Right, Direction )
			&& Direction == EActionDirection2D::Right,
			"開始閾値より下でも入力中の方向を保つ" );
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.35f, 0.0f },
				EActionDirection2D::Right, Direction )
			&& Direction == EActionDirection2D::None,
			"解除閾値をNoneへ含める" );
	}

	Harness.BeginSuite( "FActionDirectionQuantizer / 8方向と4方向を選べる" );

	{
		FActionDirectionQuantizer Quantizer;
		Quantizer.ActivationThreshold = 0.0f;
		Quantizer.ReleaseThreshold = 0.0f;
		EActionDirection2D Direction = EActionDirection2D::None;
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.2f, 0.8f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Up,
			"縦寄りの入力を上へまとめる" );
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.8f, 0.8f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::UpRight,
			"斜め入力を右上へまとめる" );
		Harness.Check( Quantizer.TryResolve( FVec2{ -0.8f, -0.6f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::DownLeft,
			"負の2軸を左下へまとめる" );

		Quantizer.bAllowDiagonal = false;
		Harness.Check( Quantizer.TryResolve( FVec2{ 0.9f, 0.8f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Right,
			"4方向では大きいX軸を選ぶ" );
		Harness.Check( Quantizer.TryResolve( FVec2{ -0.8f, 0.8f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Up,
			"4方向の同値ではY軸を決定的に選ぶ" );
	}

	Harness.BeginSuite( "FActionDirectionQuantizer / 8方向境界を4象限で固定する" );

	{
		FActionDirectionQuantizer Quantizer;
		Quantizer.ActivationThreshold = 0.0f;
		Quantizer.ReleaseThreshold = 0.0f;
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			1.0f, 1.0f, false, EActionDirection2D::Right,
			EActionDirection2D::UpRight, "右上22.5度の直前は右",
			"右上22.5度の境界は右", "右上22.5度の直後は右上" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			1.0f, 1.0f, true, EActionDirection2D::Up,
			EActionDirection2D::UpRight, "右上67.5度の直前は上",
			"右上67.5度の境界は上", "右上67.5度の直後は右上" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			-1.0f, 1.0f, false, EActionDirection2D::Left,
			EActionDirection2D::UpLeft, "左上22.5度の直前は左",
			"左上22.5度の境界は左", "左上22.5度の直後は左上" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			-1.0f, 1.0f, true, EActionDirection2D::Up,
			EActionDirection2D::UpLeft, "左上67.5度の直前は上",
			"左上67.5度の境界は上", "左上67.5度の直後は左上" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			1.0f, -1.0f, false, EActionDirection2D::Right,
			EActionDirection2D::DownRight, "右下22.5度の直前は右",
			"右下22.5度の境界は右", "右下22.5度の直後は右下" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			1.0f, -1.0f, true, EActionDirection2D::Down,
			EActionDirection2D::DownRight, "右下67.5度の直前は下",
			"右下67.5度の境界は下", "右下67.5度の直後は右下" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			-1.0f, -1.0f, false, EActionDirection2D::Left,
			EActionDirection2D::DownLeft, "左下22.5度の直前は左",
			"左下22.5度の境界は左", "左下22.5度の直後は左下" );
		CheckEightWayBoundary_Internal( Harness, Quantizer,
			-1.0f, -1.0f, true, EActionDirection2D::Down,
			EActionDirection2D::DownLeft, "左下67.5度の直前は下",
			"左下67.5度の境界は下", "左下67.5度の直後は左下" );
	}

	Harness.BeginSuite( "EActionDirection2D / 単位方向へ変換する" );

	{
		/** 9状態と、変換後に期待する各軸成分。 */
		struct FDirectionVectorCase
		{
			/** 変換元の離散方向。 */
			EActionDirection2D Direction;
			/** 期待するX成分。 */
			f32 X;
			/** 期待するY成分。 */
			f32 Y;
		};
		/** 斜め単位方向の各成分。 */
		constexpr f32 kDiagonal = 0.70710677f;
		constexpr FDirectionVectorCase kCases[] = {
			{ EActionDirection2D::None, 0.0f, 0.0f },
			{ EActionDirection2D::Up, 0.0f, 1.0f },
			{ EActionDirection2D::UpRight, kDiagonal, kDiagonal },
			{ EActionDirection2D::Right, 1.0f, 0.0f },
			{ EActionDirection2D::DownRight, kDiagonal, -kDiagonal },
			{ EActionDirection2D::Down, 0.0f, -1.0f },
			{ EActionDirection2D::DownLeft, -kDiagonal, -kDiagonal },
			{ EActionDirection2D::Left, -1.0f, 0.0f },
			{ EActionDirection2D::UpLeft, -kDiagonal, kDiagonal }
		};
		for ( const FDirectionVectorCase& TestCase : kCases )
		{
			FVec2 DirectionVector{ 9.0f, 8.0f };
			Harness.Check( TryGetActionDirection2DVector(
					TestCase.Direction, DirectionVector ),
				"全9状態をベクトルへ変換できる" );
			Harness.CheckNearF32( DirectionVector.x, TestCase.X, 0.000001f,
				"全9状態のX符号を保つ" );
			Harness.CheckNearF32( DirectionVector.y, TestCase.Y, 0.000001f,
				"全9状態のY符号を保つ" );
		}

		FVec2 DirectionVector{ 9.0f, 8.0f };
		Harness.Check( !TryGetActionDirection2DVector(
				static_cast<EActionDirection2D>( 0xffu ), DirectionVector )
			&& DirectionVector.x == 9.0f && DirectionVector.y == 8.0f,
			"未知の方向では出力を変えない" );
	}

	Harness.BeginSuite( "FActionDirectionQuantizer / アクション軸と応答へ接続" );

	{
		FActionInput Input;
		Input.SetAxis( 1u, -0.8f );
		Input.SetAxis( 3u, 0.8f );
		const FActionDirectionQuantizer Quantizer;
		EActionDirection2D Direction = EActionDirection2D::None;
		Harness.Check( Quantizer.TryResolve( Input, 1u, 3u,
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::UpLeft,
			"FActionInputの異なる2軸を直接変換する" );

		FActionAxisResponse Response;
		Response.InnerDeadZone = 0.1f;
		FVec2 ShapedAxes;
		Harness.Check( Response.TryApplyRadial(
				FVec2{ 0.45f, -0.60f }, ShapedAxes )
			&& Quantizer.TryResolve( ShapedAxes,
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::DownRight,
			"円形軸応答の結果を離散方向へ接続する" );
	}

	Harness.BeginSuite( "FActionDirectionQuantizer / 不正入力で出力を保つ" );

	{
		FActionDirectionQuantizer Invalid;
		Invalid.ReleaseThreshold = 0.6f;
		Harness.Check( !Invalid.IsValid(), "開始より大きい解除閾値を拒否する" );
		Invalid.ReleaseThreshold = 0.0f;
		Invalid.ActivationThreshold = 1.0f;
		Harness.Check( !Invalid.IsValid(), "通常の最大入力を使えない開始閾値を拒否する" );
		Invalid.ActivationThreshold = -0.1f;
		Harness.Check( !Invalid.IsValid(), "負の開始閾値を拒否する" );
		Invalid.ActivationThreshold = std::numeric_limits<f32>::infinity();
		Harness.Check( !Invalid.IsValid(), "有限でない開始閾値を拒否する" );

		const FActionDirectionQuantizer Quantizer;
		EActionDirection2D Direction = EActionDirection2D::Down;
		Harness.Check( !Quantizer.TryResolve(
				FVec2{ std::numeric_limits<f32>::quiet_NaN(), 0.0f },
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Down,
			"有限でない軸では出力を変えない" );
		Harness.Check( !Quantizer.TryResolve( FVec2{ 1.0f, 0.0f },
				static_cast<EActionDirection2D>( 0xffu ), Direction )
			&& Direction == EActionDirection2D::Down,
			"未知の前回方向では出力を変えない" );

		FActionInput Input;
		Input.SetAxis( 0u, 1.0f );
		Harness.Check( !Quantizer.TryResolve( Input, 0u, 0u,
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Down,
			"重複する軸番号では出力を変えない" );
		Harness.Check( !Quantizer.TryResolve( Input, 0u, kActionAxisCount,
				EActionDirection2D::None, Direction )
			&& Direction == EActionDirection2D::Down,
			"範囲外の軸番号では出力を変えない" );
	}
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/CameraRelativeMovement3D.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 小さな浮動小数誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		/** 期待値からの絶対差。 */
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference <= 1.0e-4f, Label );
	}

	/** 失敗時に出力値が一切変わらないことを確かめる。 */
	void CheckOutputPreserved( CTestHarness& Harness, const CCamera& Camera, FVec2 MoveAxes, f32 MaximumSpeed, const char* Label ) noexcept
	{
		/** 失敗前後を比較するための番兵値。 */
		FVec2 Velocity{ 17.0f, -23.0f };
		/** 対象入力を拒否したかどうか。 */
		const bool bSucceeded = TryCalculateCameraRelativeVelocity3D( Camera, MoveAxes, MaximumSpeed, Velocity );
		Harness.Check( !bSucceeded && Velocity.x == 17.0f && Velocity.y == -23.0f, Label );
	}
}


void RunCameraRelativeMovement3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "TryCalculateCameraRelativeVelocity3D / カメラ基準の水平速度を作る" );

	{
		/** world前方を見る基準カメラ。 */
		CCamera Camera;
		Camera.SetLookAt( FVec3{ 0.0f, 2.0f, -5.0f }, FVec3{ 0.0f, 2.0f, 0.0f } );

		/** 各入力の計算結果を受け取る速度。 */
		FVec2 Velocity{ 9.0f, 9.0f };
		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 0.0f, 1.0f }, 4.0f, Velocity ), "正面入力を計算できる" );
		CheckNear( Harness, Velocity.x, 0.0f, "正面入力でworld Xを保つ" );
		CheckNear( Harness, Velocity.y, 4.0f, "正面入力をworld Z速度へ変える" );

		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 1.0f, 0.0f }, 4.0f, Velocity ), "右入力を計算できる" );
		CheckNear( Harness, Velocity.x, 4.0f, "右入力をworld X速度へ変える" );
		CheckNear( Harness, Velocity.y, 0.0f, "右入力でworld Zを保つ" );

		Camera.SetLookAt( FVec3{ -5.0f, 6.0f, 0.0f }, FVec3{} );
		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 0.0f, 1.0f }, 3.0f, Velocity ), "上下角のある横向きカメラを使える" );
		CheckNear( Harness, Velocity.x, 3.0f, "上下角を除いたカメラ前方Xを使う" );
		CheckNear( Harness, Velocity.y, 0.0f, "上下角をworld Z速度へ混ぜない" );

		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 1.0f, 0.0f }, 2.0f, Velocity ), "横向きカメラの右入力を計算できる" );
		CheckNear( Harness, Velocity.x, 0.0f, "横向きカメラの右入力でworld Xを保つ" );
		CheckNear( Harness, Velocity.y, -2.0f, "横向きカメラの右入力をworld負Zへ変える" );
	}

	Harness.BeginSuite( "TryCalculateCameraRelativeVelocity3D / 入力長を保って制限する" );

	{
		/** world前方を見る入力長確認用カメラ。 */
		CCamera Camera;
		Camera.SetLookAt( FVec3{ 0.0f, 1.0f, -4.0f }, FVec3{ 0.0f, 1.0f, 0.0f } );

		/** 入力長ごとの計算結果を受け取る速度。 */
		FVec2 Velocity{};
		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 0.5f, 0.0f }, 6.0f, Velocity ), "半分の入力を計算できる" );
		CheckNear( Harness, Velocity.x, 3.0f, "半分の入力は半分の速度になる" );

		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 3.0f, 4.0f }, 10.0f, Velocity ), "最大長を超える入力を計算できる" );
		CheckNear( Harness, Velocity.x, 6.0f, "最大長を超えても入力方向Xを保つ" );
		CheckNear( Harness, Velocity.y, 8.0f, "最大長を超えても入力方向Zを保つ" );
		CheckNear( Harness, Length( Velocity ), 10.0f, "斜め入力を最大速度へ制限する" );

		Harness.Check( TryCalculateCameraRelativeVelocity3D( Camera, FVec2{ 1.0f, 1.0f }, 0.0f, Velocity ), "最大速度0を停止として受け付ける" );
		Harness.Check( Velocity.x == 0.0f && Velocity.y == 0.0f, "最大速度0は停止速度になる" );
	}

	Harness.BeginSuite( "TryCalculateCameraRelativeVelocity3D / 不正入力で出力を保つ" );

	{
		/** 不正値の判定に使う通常カメラ。 */
		CCamera Camera;
		Camera.SetLookAt( FVec3{ 0.0f, 1.0f, -4.0f }, FVec3{ 0.0f, 1.0f, 0.0f } );
		/** 有限入力ではないことを表す値。 */
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		/** 数値ではない入力を表す値。 */
		const f32 NotANumber = std::numeric_limits<f32>::quiet_NaN();
		/** 単独では有限だが二乗和が表現範囲を超える値。 */
		const f32 MaximumFinite = std::numeric_limits<f32>::max();

		CheckOutputPreserved( Harness, Camera, FVec2{ Infinity, 0.0f }, 4.0f, "無限大の左右入力を拒否する" );
		CheckOutputPreserved( Harness, Camera, FVec2{ 0.0f, NotANumber }, 4.0f, "NaNの前後入力を拒否する" );
		CheckOutputPreserved( Harness, Camera, FVec2{ MaximumFinite, MaximumFinite }, 4.0f, "二乗和が溢れる有限入力を拒否する" );
		CheckOutputPreserved( Harness, Camera, FVec2{}, -1.0f, "負の最大速度を拒否する" );
		CheckOutputPreserved( Harness, Camera, FVec2{}, Infinity, "無限大の最大速度を拒否する" );
		CheckOutputPreserved( Harness, Camera, FVec2{}, NotANumber, "NaNの最大速度を拒否する" );

		/** 水平な前方向を一意に決められない真上向きカメラ。 */
		CCamera VerticalCamera;
		VerticalCamera.SetLookAt( FVec3{}, FVec3{ 0.0f, 1.0f, 0.0f }, FVec3::Forward() );
		CheckOutputPreserved( Harness, VerticalCamera, FVec2{ 0.0f, 1.0f }, 4.0f, "真上向きカメラを拒否する" );
	}
}

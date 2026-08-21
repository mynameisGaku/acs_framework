// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimation3DProfile.h"
#include "Common/Test/TestHarness.h"

#include <limits>


void RunCharacterAnimation3DProfileTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FCharacterAnimation3DProfile / 既定の4状態" );

	{
		const FCharacterAnimation3DProfile Profile;
		Harness.Check( Profile.IsValid(), "既定値をそのまま使える" );
		Harness.Check( Profile.ClipFor( EAnimationGraphState::Idle ) == FStringView( "Idle" ), "待機名が入る" );
		Harness.Check( Profile.ClipFor( EAnimationGraphState::Run ) == FStringView( "Run" ), "走り名が入る" );
		Harness.Check( Profile.LoopsFor( EAnimationGraphState::Walk ), "歩きは繰り返す" );
		Harness.Check( !Profile.LoopsFor( EAnimationGraphState::Jump ), "ジャンプは既定で1回" );
		Harness.Check( Profile.ClipFor( EAnimationGraphState::Attack ).IsEmpty(), "対象外状態へ名前を返さない" );
	}

	Harness.BeginSuite( "FCharacterAnimation3DProfile / 速度境界で状態を選ぶ" );

	{
		const FCharacterAnimation3DProfile Profile;
		FCharacterAnimation3DInput Input;
		EAnimationGraphState State = EAnimationGraphState::Idle;

		Input.HorizontalSpeed = 0.14f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "待機を評価できる" );
		Harness.Check( State == EAnimationGraphState::Idle, "歩き開始未満は待機" );

		Input.HorizontalSpeed = 0.15f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "歩き開始を評価できる" );
		Harness.Check( State == EAnimationGraphState::Walk, "開始値で歩く" );

		Input.HorizontalSpeed = 4.0f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "走り開始を評価できる" );
		Harness.Check( State == EAnimationGraphState::Run, "開始値で走る" );

		Input.HorizontalSpeed = 3.2f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "走り維持を評価できる" );
		Harness.Check( State == EAnimationGraphState::Run, "終了値ではまだ走る" );

		Input.HorizontalSpeed = 3.19f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "走り終了を評価できる" );
		Harness.Check( State == EAnimationGraphState::Walk, "終了値未満で歩きへ戻る" );

		State = EAnimationGraphState::Run;
		Input.HorizontalSpeed = 0.09f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "走りから低速への減速を評価できる" );
		Harness.Check( State == EAnimationGraphState::Walk, "歩き終了値より上なら歩きを経由する" );

		Input.HorizontalSpeed = 0.09f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "歩き維持を評価できる" );
		Harness.Check( State == EAnimationGraphState::Walk, "開始値未満でも終了値より上なら歩き続ける" );

		Input.HorizontalSpeed = 0.08f;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "歩き終了を評価できる" );
		Harness.Check( State == EAnimationGraphState::Idle, "終了値で待機へ戻る" );
	}

	Harness.BeginSuite( "FCharacterAnimation3DProfile / 空中を優先して着地時に戻す" );

	{
		const FCharacterAnimation3DProfile Profile;
		FCharacterAnimation3DInput Input;
		Input.HorizontalSpeed = 5.0f;
		Input.bGrounded = false;

		EAnimationGraphState State = EAnimationGraphState::Run;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "空中を評価できる" );
		Harness.Check( State == EAnimationGraphState::Jump, "速度より空中を優先する" );

		Input.HorizontalSpeed = 0.0f;
		Input.bGrounded = true;
		Harness.Check( Profile.TrySelectState( Input, State, State ), "着地を評価できる" );
		Harness.Check( State == EAnimationGraphState::Idle, "止まって着地したら待機へ戻る" );
	}

	Harness.BeginSuite( "FCharacterAnimation3DProfile / 壊れた値では出力を変えない" );

	{
		FCharacterAnimation3DProfile BrokenProfile;
		BrokenProfile.RunExitSpeed = BrokenProfile.RunEnterSpeed + 1.0f;
		Harness.Check( !BrokenProfile.IsValid(), "速度境界の逆転を弾く" );

		FCharacterAnimation3DProfile EmptyClip;
		EmptyClip.WalkClip = FString();
		Harness.Check( !EmptyClip.IsValid(), "空のクリップ名を弾く" );

		FCharacterAnimation3DProfile BrokenBlend;
		BrokenBlend.BlendSeconds = std::numeric_limits<f32>::infinity();
		Harness.Check( !BrokenBlend.IsValid(), "有限でない補間時間を弾く" );

		FCharacterAnimation3DInput BrokenInput;
		BrokenInput.HorizontalSpeed = std::numeric_limits<f32>::quiet_NaN();
		EAnimationGraphState Output = EAnimationGraphState::Attack;
		Harness.Check( !FCharacterAnimation3DProfile{}.TrySelectState(
			BrokenInput, EAnimationGraphState::Idle, Output ), "有限でない速度を弾く" );
		Harness.Check( Output == EAnimationGraphState::Attack, "失敗時は既存出力を保つ" );

		BrokenInput.HorizontalSpeed = -0.01f;
		Harness.Check( !BrokenInput.IsValid(), "負の速度を弾く" );

		FCharacterAnimation3DInput ValidInput;
		Harness.Check( !FCharacterAnimation3DProfile{}.TrySelectState(
			ValidInput, EAnimationGraphState::Attack, Output ), "対象外の現在状態を弾く" );
	}
}

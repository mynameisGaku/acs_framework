// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimator3D.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 指定名のテスト用クリップをモデルへ追加する。 */
	void AddAnimation( ASkinnedMeshAsset& Mesh, const char* Name ) noexcept
	{
		FAnimation Animation;
		Animation.name = FString( Name );
		Animation.duration = 2.0f;
		Mesh.Animations().Add( Move( Animation ) );
	}

	/** 4状態を持つ最小の描画可能な骨付きモデルを作る。 */
	TSharedPtr<ASkinnedMeshAsset> MakeCharacterMesh() noexcept
	{
		TSharedPtr<ASkinnedMeshAsset> Mesh = MakeShared<ASkinnedMeshAsset>();
		if ( !Mesh ) return Mesh;

		FSkinnedVertex Vertex{};
		Vertex.weights[0] = 1.0f;
		Mesh->Vertices().Add( Vertex );
		Mesh->Indices().Add( 0u );
		Mesh->Bones().Add( FBone{} );
		AddAnimation( *Mesh, "Idle" );
		AddAnimation( *Mesh, "Walk" );
		AddAnimation( *Mesh, "Run" );
		AddAnimation( *Mesh, "Jump" );
		return Mesh;
	}
}


void RunCharacterAnimator3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CCharacterAnimator3D / ノードへ接続して待機を始める" );

	{
		TObjectPtr<ANode> Node = NewObject<ANode>();
		Harness.Check( static_cast<bool>( Node ), "接続先ノードを作れる" );
		if ( !Node ) return;

		CCharacterAnimator3D Animator;
		Harness.Check( !Animator.Bind( *Node ), "骨付き部品のないノードを弾く" );

		ASkinnedMeshComponent3D& Skin = Node->AddComponent<ASkinnedMeshComponent3D>();
		Skin.SetMeshAsset( MakeCharacterMesh() );
		Harness.Check( Animator.Bind( *Node ), "ノードから骨付き部品を見つけて接続できる" );
		Harness.Check( Animator.IsBound(), "接続状態を確認できる" );
		Harness.Check( Animator.CurrentState() == EAnimationGraphState::Idle, "待機状態から始まる" );
		Harness.Check( Skin.Player().IsPlaying(), "待機クリップを再生する" );
		Harness.CheckEqualF32( Skin.Player().Time(), 0.0f, "待機クリップの先頭から始まる" );
	}

	Harness.BeginSuite( "CCharacterAnimator3D / 必要なときだけ滑らかに切り替える" );

	{
		ASkinnedMeshComponent3D Skin;
		Skin.SetMeshAsset( MakeCharacterMesh() );
		CCharacterAnimator3D Animator;
		const FCharacterAnimation3DProfile Profile;
		Harness.Check( Animator.Bind( Skin, Profile ), "4クリップへ接続できる" );

		Skin.OnUpdate( 0.30f );
		FCharacterAnimation3DInput Input;
		Harness.Check( Animator.Update( Input ), "同じ待機状態を処理できる" );
		Harness.CheckEqualF32( Skin.Player().Time(), 0.30f, "同じ状態を先頭から再生し直さない" );

		Input.HorizontalSpeed = Profile.WalkEnterSpeed;
		Harness.Check( Animator.Update( Input ), "歩きへの姿勢遷移を要求できる" );
		Harness.Check( Animator.CurrentState() == EAnimationGraphState::Walk, "受理後に歩き状態へ進む" );

		Input.HorizontalSpeed = Profile.RunEnterSpeed;
		Harness.Check( !Animator.Update( Input ), "進行中の姿勢遷移へ重ねる要求を保留する" );
		Harness.Check( Animator.CurrentState() == EAnimationGraphState::Walk, "拒否された走り状態へ進まない" );

		Skin.OnUpdate( Profile.BlendSeconds + 0.01f );
		Harness.Check( Animator.Update( Input ), "既存遷移の完了後に走りを再試行できる" );
		Harness.Check( Animator.CurrentState() == EAnimationGraphState::Run, "受理後に走り状態へ進む" );
	}

	Harness.BeginSuite( "CCharacterAnimator3D / 失敗時は再生と状態を保つ" );

	{
		ASkinnedMeshComponent3D Skin;
		Skin.SetMeshAsset( MakeCharacterMesh() );
		Harness.Check( Skin.PlayByName( FStringView( "Walk" ) ), "比較用の歩きを始められる" );
		Skin.OnUpdate( 0.40f );

		FCharacterAnimation3DProfile MissingClip;
		MissingClip.JumpClip = FString( "Missing" );
		CCharacterAnimator3D Animator;
		Harness.Check( !Animator.Bind( Skin, MissingClip ), "不足クリップを再生前に弾く" );
		Harness.Check( !Animator.IsBound(), "失敗した接続を残さない" );
		Harness.CheckEqualF32( Skin.Player().Time(), 0.40f, "失敗前の再生時刻を保つ" );
		Harness.Check( Skin.Player().IsPlaying(), "失敗前の再生状態を保つ" );

		Harness.Check( Animator.Bind( Skin ), "正常な規則なら接続できる" );
		FCharacterAnimation3DInput InvalidInput;
		InvalidInput.HorizontalSpeed = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Animator.Update( InvalidInput ), "有限でない入力を弾く" );
		Harness.Check( Animator.CurrentState() == EAnimationGraphState::Idle, "不正入力で状態を変えない" );

		Animator.Unbind();
		Harness.Check( !Animator.IsBound(), "明示的に接続を解除できる" );
		Harness.Check( !Animator.Update( FCharacterAnimation3DInput{} ), "未接続では入力を処理しない" );
	}
}

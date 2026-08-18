// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Text/StringConvert.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/**
	 * 置き場から FBX を読み込む。
	 *
	 * @param RelativePath `Assets` からの相対名。
	 * @param OutBytes 受け取り先。
	 * @return 読めたら true。
	 */
	bool ReadAsset( FStringView RelativePath, TArray<byte>& OutBytes ) noexcept
	{
		FString Full;
		if ( !CAssetRoot::Resolve( RelativePath, Full ) ) return false;

		wchar_t Wide[1024] = {};
		if ( !AcsToWide( Full, Wide, 1024u ) ) return false;

		TResult<TArray<byte>> Read = CFileSystem::ReadAllBytes( Wide );
		if ( Read.IsErr() ) return false;

		OutBytes = Move( Read.Value() );
		return true;
	}
}


void RunSkinnedModelTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "LoadSkinnedMeshFromFbxMemory / 壊れた入力で落ちない" );

	{
		Harness.Check( LoadSkinnedMeshFromFbxMemory( nullptr, 0u ).IsErr(), "空の入力はエラー" );

		const byte Garbage[16] = {};
		Harness.Check( LoadSkinnedMeshFromFbxMemory( Garbage, sizeof( Garbage ) ).IsErr(),
			"FBX でないバイト列はエラー" );
	}

	{
		// 骨の無い FBX を «骨付き» として読もうとしたら、黙って空を返さずエラーにする。
		TArray<byte> Bytes;
		if ( ReadAsset( FStringView( "Models/MergedSphere.fbx" ), Bytes ) )
		{
			Harness.Check( LoadSkinnedMeshFromFbxMemory( Bytes.GetData(), Bytes.Num() ).IsErr(),
				"スキンの無い FBX はエラー" );
		}
	}

	Harness.BeginSuite( "LoadSkinnedMeshFromFbxMemory / 骨付きを読む" );

	TArray<byte> Bytes;
	if ( !ReadAsset( FStringView( "Models/SkinnedCube.fbx" ), Bytes ) )
	{
		Harness.Check( false, "Assets/Models/SkinnedCube.fbx を読めない" );
		return;
	}

	TResult<TSharedPtr<ASkinnedMeshAsset>> Loaded =
		LoadSkinnedMeshFromFbxMemory( Bytes.GetData(), Bytes.Num() );
	if ( Loaded.IsErr() )
	{
		Harness.Check( false, "骨付き FBX の読み込みが失敗した" );
		return;
	}

	const TSharedPtr<ASkinnedMeshAsset> Mesh = Loaded.Value();
	Harness.Check( static_cast<bool>( Mesh ), "アセットが返る" );
	if ( !Mesh ) return;

	Harness.Check( Mesh->Vertices().Num() > 0u, "頂点がある" );
	Harness.Check( Mesh->Indices().Num() == Mesh->Vertices().Num(), "index は頂点と同数" );
	Harness.Check( Mesh->Bones().Num() > 0u, "骨がある" );

	{
		// **親が自分より小さい番号** であること。CAnimationPlayer が world を組む前提。
		// 崩れると、腕だけ別の場所にある、のような壊れ方をする。
		bool bOrdered = true;
		for ( usize Index = 0u; Index < Mesh->Bones().Num(); ++Index )
		{
			const i32 Parent = Mesh->Bones()[Index].parent;
			if ( Parent >= static_cast<i32>( Index ) ) bOrdered = false;
		}
		Harness.Check( bOrdered, "親は必ず子より小さい番号" );
	}

	{
		// 重みが 1 に均されていること。均していないと、切り捨てた分だけ頂点が縮む。
		bool bNormalized = true;
		usize Influenced = 0u;
		for ( usize Index = 0u; Index < Mesh->Vertices().Num(); ++Index )
		{
			const FSkinnedVertex& Vertex = Mesh->Vertices()[Index];
			f32 Total = 0.0f;
			for ( u32 Slot = 0u; Slot < 4u; ++Slot )
			{
				Total += Vertex.weights[Slot];
				if ( Vertex.weights[Slot] > 0.0f
					&& static_cast<usize>( Vertex.bones[Slot] ) >= Mesh->Bones().Num() )
				{
					bNormalized = false;
				}
			}
			if ( Total < 0.999f || Total > 1.001f ) bNormalized = false;
			if ( Vertex.weights[0] > 0.0f ) ++Influenced;
		}
		Harness.Check( bNormalized, "重みの合計は 1、骨の番号は範囲内" );
		Harness.CheckEqualU64( Influenced, Mesh->Vertices().Num(), "全頂点がどこかの骨に付く" );
	}

	Harness.BeginSuite( "LoadSkinnedMeshFromFbxMemory / アニメーションを焼く" );

	{
		// SkinnedCube.fbx にはクリップが無い。焼く経路はこちらで確かめる。
		TArray<byte> AnimatedBytes;
		if ( !ReadAsset( FStringView( "Models/SkinnedAnimated.fbx" ), AnimatedBytes ) )
		{
			Harness.Check( false, "Assets/Models/SkinnedAnimated.fbx を読めない" );
		}
		else
		{
			TResult<TSharedPtr<ASkinnedMeshAsset>> AnimatedResult =
				LoadSkinnedMeshFromFbxMemory( AnimatedBytes.GetData(), AnimatedBytes.Num() );
			Harness.Check( AnimatedResult.IsOk(), "クリップ付きの骨付き FBX を読める" );

			if ( AnimatedResult.IsOk() && AnimatedResult.Value() )
			{
				const TSharedPtr<ASkinnedMeshAsset> Animated = AnimatedResult.Value();
				Harness.Check( Animated->Animations().Num() > 0u, "クリップが取れる" );

				if ( Animated->Animations().Num() > 0u )
				{
					const FAnimation& Clip = Animated->Animations()[0];
					Harness.Check( Clip.duration > 0.0f, "長さがある" );
					Harness.CheckEqualU64( Clip.channels.Num(), Animated->Bones().Num(),
						"骨の数だけチャネルがある" );

					bool bAscending = true;
					for ( usize Index = 0u; Index < Clip.channels.Num(); ++Index )
					{
						const FAnimationChannel& Channel = Clip.channels[Index];
						if ( Channel.keys.IsEmpty() ) { bAscending = false; continue; }
						for ( usize Key = 1u; Key < Channel.keys.Num(); ++Key )
						{
							// 時刻が昇順でないと、CAnimationPlayer の補間が隣を取り違える。
							if ( Channel.keys[Key].time < Channel.keys[Key - 1u].time ) bAscending = false;
						}
					}
					Harness.Check( bAscending, "キーは時刻の昇順で、空のチャネルが無い" );

					// 30 回 / 秒で焼いているので、キー数は長さから決まる。
					const usize Expected = static_cast<usize>( Clip.duration * 30.0f ) + 1u;
					Harness.CheckEqualU64( Clip.channels[0].keys.Num(), Expected, "既定の 30 回 / 秒で焼く" );
				}
			}
		}
	}

	Harness.BeginSuite( "CAnimationPlayer / 読んだ骨で動く" );

	{
		CAnimationPlayer Player;
		Player.SetMesh( Mesh.Get() );

		TArray<FMat4> Palette;
		Harness.Check( Palette.TrySetNum( Mesh->Bones().Num() ), "パレットを確保できる" );

		// アニメーションを選んでいない状態でも、バインド姿勢のパレットが出る。
		const u32 Written = Player.WritePalette( Palette.GetData(), static_cast<u32>( Palette.Num() ) );
		Harness.CheckEqualU64( Written, Mesh->Bones().Num(), "骨の数だけ書かれる" );

		// **バインド姿勢のパレットは単位行列に近いはず。**
		// world_at_bind * inverse_bind = I が成り立つのが «正しく組めている» の定義で、
		// ここが崩れていると、画面では «真っ黒» や «消える» としか分からない。
		f32 WorstOffDiagonal = 0.0f;
		f32 WorstDiagonal = 0.0f;
		for ( usize Index = 0u; Index < Palette.Num(); ++Index )
		{
			for ( u32 Row = 0u; Row < 4u; ++Row )
			{
				for ( u32 Column = 0u; Column < 4u; ++Column )
				{
					const f32 Value = Palette[Index].m[Row][Column];
					const f32 Expected = ( Row == Column ) ? 1.0f : 0.0f;
					const f32 Difference = Value > Expected ? Value - Expected : Expected - Value;
					if ( Row == Column ) { if ( Difference > WorstDiagonal ) WorstDiagonal = Difference; }
					else if ( Difference > WorstOffDiagonal ) WorstOffDiagonal = Difference;
				}
			}
		}
		Harness.Check( WorstDiagonal < 0.01f, "バインド姿勢のパレットは対角が 1" );
		Harness.Check( WorstOffDiagonal < 0.01f, "バインド姿勢のパレットは非対角が 0" );

		bool bFinite = true;
		for ( usize Index = 0u; Index < Palette.Num(); ++Index )
		{
			for ( u32 Row = 0u; Row < 4u; ++Row )
			{
				for ( u32 Column = 0u; Column < 4u; ++Column )
				{
					const f32 Value = Palette[Index].m[Row][Column];
					if ( !( Value == Value ) || Value > 1.0e30f || Value < -1.0e30f ) bFinite = false;
				}
			}
		}
		Harness.Check( bFinite, "パレットに NaN も無限も無い" );
	}
}

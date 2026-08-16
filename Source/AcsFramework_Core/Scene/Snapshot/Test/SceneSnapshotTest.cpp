// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFile.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotReader.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotStatus.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotWriter.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/**
	 * 試験用の木を作る。
	 *
	 * @details Root - (Left, Right) - Right の下に Leaf。名前と位置で見分けられるようにする。
	 */
	TObjectPtr<ANode> MakeTree() noexcept
	{
		TObjectPtr<ANode> Root = NewObject<ANode>();
		if ( Root.Get() == nullptr ) return Root;

		Root->SetName( FStringView( "Root" ) );
		Root->Local().position = FVec3{ 1.0f, 2.0f, 3.0f };

		TObjectPtr<ANode> Left = NewObject<ANode>();
		Left->SetName( FStringView( "Left" ) );
		Left->Local().position = FVec3{ -1.0f, 0.0f, 0.0f };
		Root->AddChild( Move( Left ) );

		TObjectPtr<ANode> Right = NewObject<ANode>();
		Right->SetName( FStringView( "Right" ) );
		Right->Local().position = FVec3{ 1.0f, 0.0f, 0.0f };

		TObjectPtr<ANode> Leaf = NewObject<ANode>();
		Leaf->SetName( FStringView( "Leaf" ) );
		Leaf->Local().scale = FVec3{ 2.0f, 2.0f, 2.0f };
		Right->AddChild( Move( Leaf ) );

		Root->AddChild( Move( Right ) );

		return Root;
	}
}


void RunSceneSnapshotTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSceneSnapshotBuffer / 広げて使い回す" );

	{
		CSceneSnapshotBuffer Buffer;

		Harness.CheckEqualU64( Buffer.Size(), 0u, "最初は空" );
		Harness.Check( Buffer.Data() == nullptr, "空なら nullptr" );

		Harness.Check( Buffer.EnsureSize( 128u ), "広げられる" );
		Harness.Check( Buffer.Size() >= 128u, "要求した大きさは確保される" );
		Harness.Check( Buffer.Data() != nullptr, "先頭が取れる" );

		const usize Before = Buffer.Size();
		Harness.Check( Buffer.EnsureSize( 64u ), "小さい要求も通る" );
		Harness.CheckEqualU64( Buffer.Size(), Before, "縮まない (使い回すため)" );

		Buffer.Release();
		Harness.CheckEqualU64( Buffer.Size(), 0u, "手放せる" );
	}

	Harness.BeginSuite( "CSceneSnapshotWriter / 木をバイト列にする" );

	{
		TObjectPtr<ANode> Root = MakeTree();
		Harness.Check( Root.Get() != nullptr, "木を作れる" );

		CSceneSnapshotBuffer Buffer;
		const FSceneSaveResult Result = CSceneSnapshotWriter::WriteTo( *Root, Buffer );

		Harness.Check( Result.Succeeded(), "書き出せる" );
		Harness.Check( CSceneSnapshotStatus::IsSuccess( Result.Error ), "結果は成功" );
		Harness.CheckEqualU64( Result.NodeCount, 4u, "4 ノードぶん" );
		Harness.Check( Result.BytesWritten != 0u, "中身がある" );
	}

	Harness.BeginSuite( "CSceneSnapshotReader / 起こし直す" );

	{
		TObjectPtr<ANode> Root = MakeTree();

		CSceneSnapshotBuffer Buffer;
		const FSceneSaveResult Saved = CSceneSnapshotWriter::WriteTo( *Root, Buffer );
		Harness.Check( Saved.Succeeded(), "書き出せる" );

		const FSceneLoadResult Loaded = CSceneSnapshotReader::ReadFrom( Buffer.Data(), static_cast<usize>( Saved.BytesWritten ) );

		Harness.Check( Loaded.Succeeded(), "起こせる" );

		if ( Loaded.Succeeded() )
		{
			ANode* const NewRoot = Loaded.Root.Get();

			Harness.CheckEqualU64( NewRoot->ChildCount(), 2u, "子の数" );
			Harness.CheckEqualF32( NewRoot->Local().position.y, 2.0f, "根の位置" );

			// 名前は Engine の形式 (version 4) に含まれていない。ここが通らなくなったら
			// Engine が名前を持つようになったということなので、README の但し書きを消すこと。
			Harness.Check( NewRoot->Name().Size() == 0u, "名前は復元されない (Engine の形式に無い)" );

			ANode* const NewLeft = NewRoot->Child( 0u );
			ANode* const NewRight = NewRoot->Child( 1u );

			Harness.Check( NewLeft != nullptr, "左の子が居る" );
			Harness.Check( NewRight != nullptr, "右の子が居る" );

			// 名前が無いぶん、並び順と位置が «どれがどれか» を決める唯一の手がかりになる。
			if ( NewLeft != nullptr )  Harness.CheckEqualF32( NewLeft->Local().position.x, -1.0f, "並び順が保たれる (左の位置)" );
			if ( NewRight != nullptr ) Harness.CheckEqualF32( NewRight->Local().position.x, 1.0f, "並び順が保たれる (右の位置)" );

			if ( NewRight != nullptr )
			{
				Harness.CheckEqualU64( NewRight->ChildCount(), 1u, "孫が居る" );

				ANode* const NewLeaf = NewRight->Child( 0u );
				Harness.Check( NewLeaf != nullptr, "孫を辿れる" );

				if ( NewLeaf != nullptr ) Harness.CheckEqualF32( NewLeaf->Local().scale.x, 2.0f, "孫の大きさ" );
			}
		}
	}

	Harness.BeginSuite( "CSceneSnapshotReader / 壊れたものを弾く" );

	{
		TObjectPtr<ANode> Root = MakeTree();

		CSceneSnapshotBuffer Buffer;
		const FSceneSaveResult Saved = CSceneSnapshotWriter::WriteTo( *Root, Buffer );

		Harness.Check( !CSceneSnapshotReader::ReadFrom( nullptr, 16u ).Succeeded(), "nullptr は起こせない" );
		Harness.Check( !CSceneSnapshotReader::ReadFrom( Buffer.Data(), 0u ).Succeeded(), "0 バイトは起こせない" );

		const FSceneLoadResult Truncated = CSceneSnapshotReader::ReadFrom( Buffer.Data(), 8u );
		Harness.Check( !Truncated.Succeeded(), "途中で切れたものは起こせない" );
		Harness.Check( CSceneSnapshotStatus::IsCorruptData( Truncated.Error ) || Truncated.Error == ESceneSerializeError::NullInput,
			"壊れている扱いになる" );
	}

	Harness.BeginSuite( "CSceneSnapshot / ファイルへの往復" );

	{
		TObjectPtr<ANode> Root = MakeTree();

		CSceneSnapshotBuffer Buffer;
		const FSceneSaveResult Saved = CSceneSnapshotWriter::WriteTo( *Root, Buffer );
		Harness.Check( Saved.Succeeded(), "書き出せる" );

		// 掘っていない場所を指す。親フォルダが自動で作られること込みで確かめる。
		const FString Path( "TestOutput/Scene/tree.acssave" );

		Harness.Check( CSceneSnapshotFile::Write( Path, Buffer.Data(), static_cast<usize>( Saved.BytesWritten ) ), "ファイルへ置ける" );

		CSceneSnapshotBuffer ReadBuffer;
		usize ReadSize = 0u;
		Harness.Check( CSceneSnapshotFile::Read( Path, ReadBuffer, ReadSize ), "ファイルから戻せる" );
		Harness.CheckEqualU64( ReadSize, static_cast<u64>( Saved.BytesWritten ), "大きさが一致" );

		const FSceneLoadResult Loaded = CSceneSnapshotReader::ReadFrom( ReadBuffer.Data(), ReadSize );
		Harness.Check( Loaded.Succeeded(), "ファイル経由でも起こせる" );

		if ( Loaded.Succeeded() )
		{
			Harness.CheckEqualU64( Loaded.Root->ChildCount(), 2u, "同じ木になる (子の数)" );
			Harness.CheckEqualF32( Loaded.Root->Local().position.x, 1.0f, "同じ木になる (位置)" );
		}

		usize Ignored = 0u;
		CSceneSnapshotBuffer Unused;
		Harness.Check( !CSceneSnapshotFile::Read( FString( "TestOutput/Scene/missing.acssave" ), Unused, Ignored ), "無いファイルは読めない" );
	}
}

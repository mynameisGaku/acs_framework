// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFile.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFormat.h"
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
		Leaf->SetName( FStringView( "葉" ) );
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
		Harness.Check( FSceneSnapshotFormat::IsEnvelope( Buffer.Data(), Result.BytesWritten ), "Framework 形式の目印がある" );
	}

	Harness.BeginSuite( "CSceneSnapshotReader / 起こし直す" );

	{
		TObjectPtr<ANode> Root = MakeTree();

		CSceneSnapshotBuffer Buffer;
		const FSceneSaveResult Saved = CSceneSnapshotWriter::WriteTo( *Root, Buffer );
		Harness.Check( Saved.Succeeded(), "書き出せる" );

		const FSceneLoadResult Loaded = CSceneSnapshotReader::ReadFrom( Buffer.Data(), static_cast<usize>( Saved.BytesWritten ) );

		Harness.Check( Loaded.Succeeded(), "起こせる" );
		const FSceneLoadResult LoadedFromReusableBuffer = CSceneSnapshotReader::ReadFrom( Buffer );
		Harness.Check( LoadedFromReusableBuffer.Succeeded(), "余剰容量を持つ再利用バッファからも起こせる" );

		if ( Loaded.Succeeded() )
		{
			ANode* const NewRoot = Loaded.Root.Get();

			Harness.CheckEqualU64( NewRoot->ChildCount(), 2u, "子の数" );
			Harness.CheckEqualF32( NewRoot->Local().position.y, 2.0f, "根の位置" );

			Harness.Check( NewRoot->Name() == FStringView( "Root" ), "根の名前が戻る" );

			ANode* const NewLeft = NewRoot->Child( 0u );
			ANode* const NewRight = NewRoot->Child( 1u );

			Harness.Check( NewLeft != nullptr, "左の子が居る" );
			Harness.Check( NewRight != nullptr, "右の子が居る" );

			if ( NewLeft != nullptr )
			{
				Harness.Check( NewLeft->Name() == FStringView( "Left" ), "左の名前が戻る" );
				Harness.CheckEqualF32( NewLeft->Local().position.x, -1.0f, "並び順が保たれる (左の位置)" );
			}
			if ( NewRight != nullptr )
			{
				Harness.Check( NewRight->Name() == FStringView( "Right" ), "右の名前が戻る" );
				Harness.CheckEqualF32( NewRight->Local().position.x, 1.0f, "並び順が保たれる (右の位置)" );
			}

			if ( NewRight != nullptr )
			{
				Harness.CheckEqualU64( NewRight->ChildCount(), 1u, "孫が居る" );

				ANode* const NewLeaf = NewRight->Child( 0u );
				Harness.Check( NewLeaf != nullptr, "孫を辿れる" );

				if ( NewLeaf != nullptr )
				{
					Harness.Check( NewLeaf->Name() == FStringView( "葉" ), "UTF-8 の孫名が戻る" );
					Harness.CheckEqualF32( NewLeaf->Local().scale.x, 2.0f, "孫の大きさ" );
				}
			}
		}
	}

	Harness.BeginSuite( "CSceneSnapshotReader / 従来の ACS 形式も読む" );

	{
		TObjectPtr<ANode> Root = MakeTree();
		const FSceneSaveResult Measured = TrySaveNodeTree( Root.Get(), nullptr, 0u );
		Harness.Check( CSceneSnapshotStatus::IsBufferTooSmall( Measured.Error ), "従来形式の大きさを測れる" );

		CSceneSnapshotBuffer LegacyBuffer;
		Harness.Check( LegacyBuffer.EnsureSize( Measured.RequiredBytes ), "従来形式の入れ物を用意できる" );
		const FSceneSaveResult Saved = TrySaveNodeTree( Root.Get(), LegacyBuffer.Data(), static_cast<u32>( LegacyBuffer.Size() ) );
		Harness.Check( Saved.Succeeded(), "従来形式を書ける" );
		Harness.Check( !FSceneSnapshotFormat::IsEnvelope( LegacyBuffer.Data(), Saved.BytesWritten ), "従来形式には Framework の目印が無い" );

		const FSceneLoadResult Loaded = CSceneSnapshotReader::ReadFrom( LegacyBuffer.Data(), Saved.BytesWritten );
		Harness.Check( Loaded.Succeeded(), "従来形式も読み込める" );
		if ( Loaded.Succeeded() )
		{
			Harness.CheckEqualU64( Loaded.Root->ChildCount(), 2u, "従来形式でも木が戻る" );
			Harness.Check( Loaded.Root->Name().IsEmpty(), "従来形式に無い名前は空のまま" );
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
		Harness.Check( CSceneSnapshotStatus::IsCorruptData( Truncated.Error ) || Truncated.Error == ESceneSerializeError::NullInput, "壊れている扱いになる" );

		TArray<u8> Unsupported;
		Unsupported.SetNum( Saved.BytesWritten );
		MemCopy( Unsupported.GetData(), Buffer.Data(), Saved.BytesWritten );
		const u32 UnsupportedVersion = FSceneSnapshotFormat::Version + 1u;
		MemCopy( Unsupported.GetData() + sizeof( u32 ), &UnsupportedVersion, sizeof( UnsupportedVersion ) );
		const FSceneLoadResult VersionMismatch = CSceneSnapshotReader::ReadFrom( Unsupported.GetData(), Unsupported.Num() );
		Harness.Check( VersionMismatch.Error == ESceneSerializeError::UnsupportedVersion, "未対応の Framework 版を弾く" );

		u32 EnvelopeVersion = 0u;
		u32 EngineBytes = 0u;
		u32 NodeCount = 0u;
		u32 NameBytes = 0u;
		Harness.Check( FSceneSnapshotFormat::ReadHeader( Buffer.Data(), Saved.BytesWritten, EnvelopeVersion, EngineBytes, NodeCount, NameBytes ), "検証用にヘッダーを読める" );

		TArray<u8> WrongNameCount;
		WrongNameCount.SetNum( Saved.BytesWritten );
		MemCopy( WrongNameCount.GetData(), Buffer.Data(), Saved.BytesWritten );
		Harness.Check( FSceneSnapshotFormat::WriteHeader( WrongNameCount.GetData(), WrongNameCount.Num(), EngineBytes, NodeCount + 1u, NameBytes ), "検証用に名前数を改ざんできる" );
		const FSceneLoadResult CountMismatch = CSceneSnapshotReader::ReadFrom( WrongNameCount.GetData(), WrongNameCount.Num() );
		Harness.Check( !CountMismatch.Succeeded() && CountMismatch.Root.Get() == nullptr, "名前数が合わない木は返さない" );

		TArray<u8> OversizedName;
		OversizedName.SetNum( Saved.BytesWritten );
		MemCopy( OversizedName.GetData(), Buffer.Data(), Saved.BytesWritten );
		u8* NameCursor = OversizedName.GetData() + FSceneSnapshotFormat::HeaderBytes + EngineBytes;
		const u8* const NameEnd = NameCursor + NameBytes;
		Harness.Check( FSceneSnapshotFormat::WriteU32( NameCursor, NameEnd, FSceneSnapshotFormat::MaxNodeNameBytes + 1u ), "検証用に巨大な名前長へ改ざんできる" );
		const FSceneLoadResult NameLimit = CSceneSnapshotReader::ReadFrom( OversizedName.GetData(), OversizedName.Num() );
		Harness.Check( NameLimit.Error == ESceneSerializeError::SerializedSizeOverflow && NameLimit.Root.Get() == nullptr, "巨大な名前を木へ反映する前に弾く" );
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
			Harness.Check( Loaded.Root->Name() == FStringView( "Root" ), "ファイル経由でも名前が戻る" );
		}

		usize Ignored = 0u;
		CSceneSnapshotBuffer Unused;
		Harness.Check( !CSceneSnapshotFile::Read( FString( "TestOutput/Scene/missing.acssave" ), Unused, Ignored ), "無いファイルは読めない" );
	}
}

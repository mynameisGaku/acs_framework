// SPDX-License-Identifier: Apache-2.0
#include "Common/File/AcsArchiveFile.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 試験に使う版。 */
	constexpr u32 kVersion = 42u;

	/** 中身を作る。 */
	void FillBytes( TArray<u8>& OutBytes, usize Count ) noexcept
	{
		OutBytes.Reset();
		OutBytes.SetNum( Count );

		for ( usize Index = 0u; Index < Count; ++Index )
		{
			OutBytes[Index] = static_cast<u8>( ( Index * 7u ) & 0xFFu );
		}
	}

	/** 2 つのバイト列が同じかを返す。 */
	bool SameBytes( const TArray<u8>& A, const TArray<u8>& B ) noexcept
	{
		if ( A.Num() != B.Num() ) return false;

		for ( usize Index = 0u; Index < A.Num(); ++Index )
		{
			if ( A[Index] != B[Index] ) return false;
		}

		return true;
	}
}


void RunAcsArchiveFileTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CAcsArchiveFile / 往復" );

	{
		TArray<u8> Written;
		FillBytes( Written, 256u );

		const FString Path( "archive_roundtrip.acssave" );

		Harness.Check( CAcsArchiveFile::Write( Path, kVersion, Written.GetData(), Written.Num() ), "書ける" );

		TArray<u8> ReadBack;
		Harness.Check( CAcsArchiveFile::Read( Path, kVersion, ReadBack ), "読める" );
		Harness.Check( SameBytes( Written, ReadBack ), "中身が一致する" );
	}

	Harness.BeginSuite( "CAcsArchiveFile / 掘っていないフォルダへも置ける" );

	{
		// ここが効かないと «Saved/Replay/ を先に作っておくこと» という覚え書きが要る。
		TArray<u8> Written;
		FillBytes( Written, 32u );

		const FString Path( "TestOutput/Nested/Deeper/archive.acssave" );

		Harness.Check( CAcsArchiveFile::Write( Path, kVersion, Written.GetData(), Written.Num() ), "親フォルダごと作って書ける" );

		TArray<u8> ReadBack;
		Harness.Check( CAcsArchiveFile::Read( Path, kVersion, ReadBack ), "読み戻せる" );
		Harness.Check( SameBytes( Written, ReadBack ), "中身が一致する" );
	}

	Harness.BeginSuite( "CAcsArchiveFile / 受け付けないもの" );

	{
		TArray<u8> Written;
		FillBytes( Written, 16u );

		Harness.Check( !CAcsArchiveFile::Write( FString(), kVersion, Written.GetData(), Written.Num() ), "空のパスは書かない" );
		Harness.Check( !CAcsArchiveFile::Write( FString( "x.acssave" ), kVersion, nullptr, 8u ), "nullptr は書かない" );
		Harness.Check( !CAcsArchiveFile::Write( FString( "x.acssave" ), kVersion, Written.GetData(), 0u ), "0 バイトは書かない" );

		TArray<u8> ReadBack;
		Harness.Check( !CAcsArchiveFile::Read( FString(), kVersion, ReadBack ), "空のパスからは読まない" );
		Harness.Check( !CAcsArchiveFile::Read( FString( "no_such_file_here.acssave" ), kVersion, ReadBack ), "無いファイルは読めない" );
		Harness.CheckEqualU64( ReadBack.Num(), 0u, "失敗したら空になる" );
	}

	Harness.BeginSuite( "CAcsArchiveFile / 版が違えば読まない" );

	{
		// 版を見ずに読むと、形の違う中身を «読めた» ことにしてしまう。
		TArray<u8> Written;
		FillBytes( Written, 64u );

		const FString Path( "archive_version.acssave" );
		Harness.Check( CAcsArchiveFile::Write( Path, kVersion, Written.GetData(), Written.Num() ), "書ける" );

		TArray<u8> ReadBack;
		Harness.Check( !CAcsArchiveFile::Read( Path, kVersion + 1u, ReadBack ), "違う版では読めない" );
		Harness.Check( CAcsArchiveFile::Read( Path, kVersion, ReadBack ), "同じ版なら読める" );
	}
}

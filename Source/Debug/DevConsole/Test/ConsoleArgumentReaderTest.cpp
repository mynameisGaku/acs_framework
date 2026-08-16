// SPDX-License-Identifier: Apache-2.0
#include "Common/Test/TestHarness.h"
#include "Debug/DevConsole/ConsoleArgumentReader.h"

namespace
{
	/** 文字列の並びから引数リーダーを作る補助。 */
	class CArgs
	{
	public:
		void Add( const char* Text ) noexcept
		{
			FConsoleArg Arg;
			Arg.str = Text;
			m_Args.TryAdd( Arg );
		}

		CConsoleArgumentReader MakeReader() const noexcept
		{
			return CConsoleArgumentReader( static_cast<u32>( m_Args.Num() ), m_Args.Num() != 0u ? m_Args.GetData() : nullptr );
		}

	private:
		TArray<FConsoleArg> m_Args;
	};

	/**
	 * その文字列が小数として読めるか、読めた値は何かを確かめる。
	 *
	 * @param Harness 確かめる相手。
	 * @param Text 読ませる文字列。
	 * @param bExpectOk 読めるべきなら true。
	 * @param Expected 読めるときの期待値。
	 */
	void CheckFloat( CTestHarness& Harness, const char* Text, bool bExpectOk, f32 Expected )
	{
		CArgs Args;
		Args.Add( Text );
		const CConsoleArgumentReader Reader = Args.MakeReader();

		f32 Value = -12345.0f;
		const bool bOk = Reader.TryGetFloat( 0u, Value );

		if ( bOk != bExpectOk )
		{
			Harness.Check( false, Text );
			return;
		}

		Harness.Check( true, Text );

		if ( bExpectOk ) Harness.CheckEqualF32( Value, Expected, Text );
	}
}


void RunConsoleArgumentReaderTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CConsoleArgumentReader / 数の読み取り" );

	// 読めるべきもの。
	CheckFloat( Harness, "0", true, 0.0f );
	CheckFloat( Harness, "1", true, 1.0f );
	CheckFloat( Harness, "-1", true, -1.0f );
	CheckFloat( Harness, "+2", true, 2.0f );
	CheckFloat( Harness, "0.5", true, 0.5f );
	CheckFloat( Harness, "-0.25", true, -0.25f );
	CheckFloat( Harness, "12.0", true, 12.0f );

	// 読めてはいけないもの。ここを許すと、打ち間違いが «0» として通ってしまう。
	CheckFloat( Harness, "", false, 0.0f );
	CheckFloat( Harness, "abc", false, 0.0f );
	CheckFloat( Harness, "-", false, 0.0f );
	CheckFloat( Harness, "+", false, 0.0f );
	CheckFloat( Harness, ".", false, 0.0f );
	CheckFloat( Harness, "1.2.3", false, 0.0f );
	CheckFloat( Harness, "1x", false, 0.0f );
	CheckFloat( Harness, "1 ", false, 0.0f );
	CheckFloat( Harness, "1e5", false, 0.0f );

	Harness.BeginSuite( "CConsoleArgumentReader / 整数と範囲" );

	{
		CArgs Args;
		Args.Add( "42" );
		Args.Add( "-7" );
		Args.Add( "zzz" );
		const CConsoleArgumentReader Reader = Args.MakeReader();

		i32 Value = 0;
		Harness.Check( Reader.TryGetInt( 0u, Value ), "42 を読める" );
		Harness.CheckEqualU64( static_cast<u64>( Value ), 42u, "42" );

		Harness.Check( Reader.TryGetInt( 1u, Value ), "-7 を読める" );
		Harness.Check( Value == -7, "-7 の値" );

		Harness.Check( !Reader.TryGetInt( 2u, Value ), "zzz は読めない" );
		Harness.Check( !Reader.TryGetInt( 9u, Value ), "範囲外は読めない" );

		Harness.CheckEqualU64( Reader.Num(), 3u, "引数の数" );
		Harness.Check( Reader.GetString( 9u ) == nullptr, "範囲外の文字列は nullptr" );
	}

	Harness.BeginSuite( "CConsoleArgumentReader / 空の並び" );

	{
		const CConsoleArgumentReader Empty( 0u, nullptr );

		f32 Value = 0.0f;
		Harness.CheckEqualU64( Empty.Num(), 0u, "空なら 0 件" );
		Harness.Check( !Empty.TryGetFloat( 0u, Value ), "空からは読めない" );
		Harness.Check( Empty.JoinFrom( 0u ).IsEmpty(), "空を繋ぐと空" );
	}

	Harness.BeginSuite( "CConsoleArgumentReader / 繋ぎ" );

	{
		CArgs Args;
		Args.Add( "Assets/Bgm" );
		Args.Add( "My" );
		Args.Add( "Song.wav" );
		const CConsoleArgumentReader Reader = Args.MakeReader();

		const FString Joined = Reader.JoinFrom( 0u );
		Harness.Check( Joined == FString( "Assets/Bgm My Song.wav" ), "空白で繋がる" );

		const FString Tail = Reader.JoinFrom( 1u );
		Harness.Check( Tail == FString( "My Song.wav" ), "途中から繋がる" );
	}
}

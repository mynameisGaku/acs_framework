// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/ActionInputTape.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 軸とボタンを 1 つずつ持つ入力を作る。 */
	FActionInput MakeInput( f32 Axis, bool bButton ) noexcept
	{
		FActionInput Input;
		Input.SetAxis( 0u, Axis );
		Input.SetDown( 0u, bButton );
		return Input;
	}
}


void RunActionInputTapeTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CActionInputTape / 変わったときだけ書く" );

	{
		CActionInputTape Tape;

		Tape.Record( 0u, MakeInput( 1.0f, false ) );
		Tape.Record( 1u, MakeInput( 1.0f, false ) );   // 同じ -> 書かない
		Tape.Record( 2u, MakeInput( 1.0f, false ) );   // 同じ -> 書かない
		Tape.Record( 3u, MakeInput( -1.0f, false ) );  // 変わった -> 書く

		Harness.CheckEqualU64( Tape.Num(), 2u, "書かれた件数" );
		Harness.CheckEqualU64( Tape.GetLastTick(), 3u, "最後のティック" );

		FActionInput Read;
		Harness.Check( Tape.TryGet( 0u, Read ), "0 を読める" );
		Harness.CheckEqualF32( Read.GetAxis( 0u ), 1.0f, "tick 0" );

		Harness.Check( Tape.TryGet( 2u, Read ), "書かれていない 2 も読める" );
		Harness.CheckEqualF32( Read.GetAxis( 0u ), 1.0f, "tick 2 は直前の値" );

		Harness.Check( Tape.TryGet( 3u, Read ), "3 を読める" );
		Harness.CheckEqualF32( Read.GetAxis( 0u ), -1.0f, "tick 3" );

		Harness.Check( Tape.TryGet( 99u, Read ), "終端より後も読める" );
		Harness.CheckEqualF32( Read.GetAxis( 0u ), -1.0f, "終端より後は最後の値" );
	}

	Harness.BeginSuite( "CActionInputTape / 空と、最初より前" );

	{
		CActionInputTape Tape;

		FActionInput Read;
		Harness.Check( !Tape.TryGet( 0u, Read ), "空からは読めない" );

		Tape.Record( 10u, MakeInput( 1.0f, true ) );
		Harness.Check( !Tape.TryGet( 9u, Read ), "最初の記録より前は読めない" );
		Harness.Check( Tape.TryGet( 10u, Read ), "最初の記録は読める" );
	}

	Harness.BeginSuite( "CActionInputTape / 逆行を拒む" );

	{
		// 並びが崩れたテープを許すと、再生が静かにずれる。
		CActionInputTape Tape;

		Tape.Record( 5u, MakeInput( 1.0f, false ) );
		const bool bAccepted = Tape.Record( 4u, MakeInput( -1.0f, false ) );

		Harness.Check( !bAccepted, "前のティックへは書けない" );
		Harness.CheckEqualU64( Tape.Num(), 1u, "件数は増えない" );
	}

	Harness.BeginSuite( "CActionInputTape / バイト列の往復" );

	{
		CActionInputTape Tape;
		Tape.SetSeed( 777u );
		Tape.Record( 0u, MakeInput( 0.25f, true ) );
		Tape.Record( 4u, MakeInput( -0.75f, false ) );
		Tape.Record( 9u, MakeInput( 1.0f, true ) );

		TArray<u8> Bytes;
		Bytes.SetNum( Tape.GetRequiredBytes() );

		usize Written = 0u;
		Harness.Check( Tape.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written ), "書き出せる" );
		Harness.CheckEqualU64( Written, Tape.GetRequiredBytes(), "書けた大きさ" );

		CActionInputTape Loaded;
		Harness.Check( Loaded.TryLoadFromBuffer( Bytes.GetData(), Written ), "読み込める" );
		Harness.CheckEqualU64( Loaded.Num(), Tape.Num(), "件数が一致" );
		Harness.CheckEqualU64( Loaded.GetSeed(), 777u, "種が一致" );
		Harness.CheckEqualU64( Loaded.GetLastTick(), 9u, "最後のティックが一致" );

		bool bSame = true;
		for ( u32 Tick = 0u; Tick <= 12u; ++Tick )
		{
			FActionInput A;
			FActionInput B;
			const bool bGotA = Tape.TryGet( Tick, A );
			const bool bGotB = Loaded.TryGet( Tick, B );

			if ( bGotA != bGotB ) { bSame = false; break; }
			if ( bGotA && !A.Equals( B ) ) { bSame = false; break; }
		}

		Harness.Check( bSame, "全ティックで同じ入力が返る" );
	}

	Harness.BeginSuite( "CActionInputTape / 壊れたバイト列を弾く" );

	{
		CActionInputTape Source;
		Source.Record( 0u, MakeInput( 1.0f, true ) );

		TArray<u8> Bytes;
		Bytes.SetNum( Source.GetRequiredBytes() );
		usize Written = 0u;
		Source.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written );

		CActionInputTape Loaded;

		Harness.Check( !Loaded.TryLoadFromBuffer( nullptr, 0u ), "nullptr は弾く" );
		Harness.Check( !Loaded.TryLoadFromBuffer( Bytes.GetData(), 4u ), "短すぎるものは弾く" );
		Harness.Check( !Loaded.TryLoadFromBuffer( Bytes.GetData(), Written - 1u ), "途中で切れたものは弾く" );

		// 目印を壊す。
		TArray<u8> Broken;
		Broken.SetNum( Written );
		MemCopy( Broken.GetData(), Bytes.GetData(), Written );
		Broken[0] = static_cast<u8>( Broken[0] ^ 0xFFu );

		Harness.Check( !Loaded.TryLoadFromBuffer( Broken.GetData(), Written ), "目印が違うものは弾く" );
		Harness.CheckEqualU64( Loaded.Num(), 0u, "失敗したら空になる" );

		// 入れ物が足りない書き出しも拒む。
		usize Ignored = 0u;
		Harness.Check( !Source.TrySaveToBuffer( Bytes.GetData(), 4u, Ignored ), "入れ物が足りなければ書かない" );
	}
}

// SPDX-License-Identifier: Apache-2.0
#include "Common/Compat/AcsEnumReflection.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 0 から連番の、いちばん普通の形。 */
	enum class ETestColor
	{
		Red,
		Green,
		Blue,
	};

	/** 途中が抜けている形。位置と値がずれることを確かめる。 */
	enum class ETestSparse
	{
		Zero = 0,
		Five = 5,
		Nine = 9,
	};

	/** 列挙子が 1 つだけの形。 */
	enum class ETestSingle
	{
		Only,
	};

	/** 走査の上限 (64) を越える値。表から漏れることを確かめる。 */
	enum class ETestBeyondScan
	{
		Inside = 3,
		Outside = 200,
	};

	/** 名前が一致するかを見る。 */
	bool NameIs( const AcsFw::FEnumNameView& Name, const char* Expected ) noexcept
	{
		if ( Name.IsEmpty() ) return false;

		return FStringView( Name.Data, Name.Size ) == FStringView( Expected );
	}
}


void RunEnumReflectionTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "AcsFw 列挙反映 / 名前を引く" );

	{
		Harness.CheckEqualU64( AcsFw::EnumCount<ETestColor>(), 3u, "列挙子の数" );

		Harness.Check( NameIs( AcsFw::EnumToString( ETestColor::Red ), "Red" ), "先頭の名前" );
		Harness.Check( NameIs( AcsFw::EnumToString( ETestColor::Green ), "Green" ), "途中の名前" );
		Harness.Check( NameIs( AcsFw::EnumToString( ETestColor::Blue ), "Blue" ), "末尾の名前" );

		// 型名まで含んでいたら切り出しに失敗している。
		const AcsFw::FEnumNameView Name = AcsFw::EnumToString( ETestColor::Red );
		Harness.CheckEqualU64( Name.Size, 3u, "型名が混ざっていない" );

		const AcsFw::FEnumNameView* Names = AcsFw::EnumNames<ETestColor>();
		Harness.Check( Names != nullptr, "名前表が取れる" );
		Harness.Check( NameIs( Names[0], "Red" ), "表の並びは宣言順 (先頭)" );
		Harness.Check( NameIs( Names[2], "Blue" ), "表の並びは宣言順 (末尾)" );
	}

	Harness.BeginSuite( "AcsFw 列挙反映 / 位置と値の行き来" );

	{
		Harness.CheckEqualU64( AcsFw::EnumToIndex( ETestColor::Green ), 1u, "値から位置" );
		Harness.Check( AcsFw::EnumFromIndex<ETestColor>( 2u ) == ETestColor::Blue, "位置から値" );

		// 画面の行は「位置」で選ぶので、往復して元へ戻らないと選択がずれる。
		for ( usize Index = 0u; Index < AcsFw::EnumCount<ETestColor>(); ++Index )
		{
			const ETestColor Value = AcsFw::EnumFromIndex<ETestColor>( Index );
			Harness.CheckEqualU64( AcsFw::EnumToIndex( Value ), Index, "位置 → 値 → 位置 で戻る" );
		}
	}

	Harness.BeginSuite( "AcsFw 列挙反映 / 値が飛んでいても引ける" );

	{
		Harness.CheckEqualU64( AcsFw::EnumCount<ETestSparse>(), 3u, "抜けは数に入らない" );

		Harness.Check( NameIs( AcsFw::EnumToString( ETestSparse::Five ), "Five" ), "飛んだ先の名前" );
		Harness.Check( NameIs( AcsFw::EnumToString( ETestSparse::Nine ), "Nine" ), "さらに飛んだ先の名前" );

		// 値 5 は「3 番目の選択肢」ではなく「2 番目」。ここを取り違えると選択が飛ぶ。
		Harness.CheckEqualU64( AcsFw::EnumToIndex( ETestSparse::Five ), 1u, "位置は値ではなく並び順" );
		Harness.CheckEqualU64( AcsFw::EnumToIndex( ETestSparse::Nine ), 2u, "位置は詰めて数える" );
		Harness.Check( AcsFw::EnumFromIndex<ETestSparse>( 1u ) == ETestSparse::Five, "位置から飛んだ値へ戻る" );
	}

	Harness.BeginSuite( "AcsFw 列挙反映 / 定義されていない値を弾く" );

	{
		Harness.Check( AcsFw::IsValidEnum( ETestColor::Green ), "定義された値は通る" );
		Harness.Check( !AcsFw::IsValidEnum( static_cast<ETestColor>( 7 ) ), "定義されていない値は弾く" );
		Harness.Check( !AcsFw::IsValidEnum( static_cast<ETestColor>( -1 ) ), "負の値も弾く" );

		Harness.Check( !AcsFw::IsValidEnum( static_cast<ETestSparse>( 1 ) ), "抜けている値は弾く" );
		Harness.Check( AcsFw::IsValidEnum( ETestSparse::Zero ), "0 は «未設定» ではなく列挙子として扱う" );

		Harness.Check( AcsFw::EnumToString( static_cast<ETestColor>( 7 ) ).IsEmpty(), "名前は引けない" );
	}

	Harness.BeginSuite( "AcsFw 列挙反映 / 端の形" );

	{
		Harness.CheckEqualU64( AcsFw::EnumCount<ETestSingle>(), 1u, "1 つだけでも引ける" );
		Harness.Check( NameIs( AcsFw::EnumToString( ETestSingle::Only ), "Only" ), "その名前" );

		// 範囲の外を渡されても落ちない (画面の選択位置は外から来る)。
		Harness.Check( AcsFw::EnumFromIndex<ETestColor>( 99u ) == ETestColor::Red, "範囲外は先頭へ寄せる" );
		Harness.CheckEqualU64( AcsFw::EnumToIndex( static_cast<ETestColor>( 7 ) ), 0u, "引けない値は 0 を返す" );
	}

	Harness.BeginSuite( "AcsFw 列挙反映 / 走査の上限" );

	{
		// 0 以上 64 未満しか見ない。この前提が崩れると、画面から選択肢が黙って消える。
		Harness.CheckEqualU64( AcsFw::EnumCount<ETestBeyondScan>(), 1u, "上限を越えた列挙子は表に入らない" );
		Harness.Check( NameIs( AcsFw::EnumToString( ETestBeyondScan::Inside ), "Inside" ), "上限内は引ける" );
		Harness.Check( AcsFw::EnumToString( ETestBeyondScan::Outside ).IsEmpty(), "上限外は引けない (既知の限界)" );
		Harness.Check( !AcsFw::IsValidEnum( ETestBeyondScan::Outside ), "上限外は «定義されていない» と見える" );
	}
}

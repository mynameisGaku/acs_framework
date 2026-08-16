// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * @file
 * 列挙型から「列挙子の名前」を引く口。配布物の世代差をここで吸収する。
 *
 * @details
 * 新しい ACS は `acs::EnumNames<T>()` / `acs::EnumCount<T>` などのコンパイル時反映を持つが、
 * 手元の配布物 (2026-08-03 生成) には無い。代わりに `ACS_REGISTER_ENUM` による**登録制の
 * 実行時反映**がある。ただしそちらは列挙子を足すたびに登録も書き足す必要があり、
 * 「列挙子を足せば画面の選択肢も自動で増える」という枠組み側の売りを失う。
 *
 * そこでこのヘッダが、どちらの配布物でも同じ形で使える口を出す。
 * **列挙型を使う側はここだけを見ればよく、配布物が変わっても書き換えは要らない。**
 *
 * 対応する配布物へ移ったら `ACSFW_USE_ACS_ENUM_REFLECTION` を 1 にするだけで、
 * 中身がエンジン側の実装へ切り替わる (使う側の変更は不要)。
 */

/**
 * エンジン側のコンパイル時 enum 反映を使うかどうか。
 *
 * @details 0 = このヘッダが自前で解決する (2026-08-03 配布物)。1 = `acs::` のものへ委ねる。
 */
#if !defined( ACSFW_USE_ACS_ENUM_REFLECTION )
	#define ACSFW_USE_ACS_ENUM_REFLECTION 0
#endif

namespace AcsFw
{
	/**
	 * 列挙子の名前 1 つ。
	 *
	 * @details 文字列は実行ファイルの静的領域を指すので、寿命を気にせず持ち回してよい。
	 */
	struct FEnumNameView
	{
		/** 先頭。名前が引けなかったときは nullptr。 */
		const char* Data = nullptr;

		/** 長さ (終端の 0 を含まない)。 */
		usize Size = 0u;

		/** 名前が入っていなければ true。 */
		constexpr bool IsEmpty() const noexcept { return Data == nullptr || Size == 0u; }
	};
}


#if ACSFW_USE_ACS_ENUM_REFLECTION

// エンジン側へ委ねる形。呼び出しの形は `dev` に残っていた使用箇所から起こしたもので、
// 対応する配布物が手に入るまで**一度もコンパイルされていない**。切り替えたら真っ先にここを疑うこと。
namespace AcsFw
{
	template<typename TEnum> inline const FEnumNameView* EnumNames() noexcept
	{
		return reinterpret_cast<const FEnumNameView*>( acs::EnumNames<TEnum>().Items );
	}

	template<typename TEnum> inline usize EnumCount() noexcept { return acs::EnumCount<TEnum>; }
	template<typename TEnum> inline bool IsValidEnum( TEnum Value ) noexcept { return acs::IsValidEnum( Value ); }
	template<typename TEnum> inline usize EnumToIndex( TEnum Value ) noexcept { return acs::EnumToIndex( Value ); }
	template<typename TEnum> inline TEnum EnumFromIndex( usize Index ) noexcept { return acs::EnumFromIndex<TEnum>( Index ); }

	template<typename TEnum> inline FEnumNameView EnumToString( TEnum Value ) noexcept
	{
		const acs::FEnumName Name = acs::ToString( Value );

		return FEnumNameView{ Name.Data, Name.Size };
	}
}

#else

namespace AcsFwEnumDetail
{
	/**
	 * 走査する値の上限。
	 *
	 * @details
	 * 0 から順に「その値が列挙子かどうか」を調べる。**0 以上 kScanLimit 未満に居る列挙子しか
	 * 見つからない。** 枠組みが画面へ出す列挙型はすべて 0 から連番なので足りている。
	 * 負の値や大きな値を持つ列挙型をここへ通すと、その列挙子は表から漏れる。
	 */
	constexpr usize kScanLimit = 64u;

	/** 終端までの長さを数える。 */
	constexpr usize TextLength( const char* Text ) noexcept
	{
		usize Length = 0u;
		while ( Text[Length] != '\0' ) ++Length;

		return Length;
	}

	/**
	 * その値を型引数に持つ関数の名前を、コンパイラから文字列として受け取る。
	 *
	 * @details
	 * MSVC の `__FUNCSIG__` には型引数がそのまま入る。列挙子であれば `EFoo::Bar` と出て、
	 * 列挙子でない値なら `(enum EFoo)0x5` のように括弧付きで出る。この差で在否を判定する。
	 */
	template<typename TEnum, TEnum Value>
	constexpr const char* RawSignature() noexcept
	{
		return __FUNCSIG__;
	}

	/**
	 * `__FUNCSIG__` から列挙子の名前だけを切り出す。
	 *
	 * @param Signature 切り出し元。
	 * @return 列挙子なら名前。そうでなければ空。
	 */
	constexpr AcsFw::FEnumNameView ParseEnumeratorName( const char* Signature ) noexcept
	{
		const usize Length = TextLength( Signature );

		// 型引数の終わり ">(" を後ろから探す。
		usize End = 0u;
		bool bFoundEnd = false;
		for ( usize Index = Length; Index >= 2u; --Index )
		{
			if ( Signature[Index - 2u] == '>' && Signature[Index - 1u] == '(' )
			{
				End = Index - 2u;
				bFoundEnd = true;
				break;
			}
		}
		if ( !bFoundEnd ) return AcsFw::FEnumNameView{};

		// 直前の ',' の後ろが値の側。
		usize Begin = End;
		while ( Begin > 0u && Signature[Begin - 1u] != ',' ) --Begin;
		if ( Begin == 0u || Begin >= End ) return AcsFw::FEnumNameView{};

		// 列挙子でない値は "(enum EFoo)0x5" と出る。
		if ( Signature[Begin] == '(' ) return AcsFw::FEnumNameView{};

		// "EFoo::Bar" なら "Bar" だけを名前とする。
		usize NameBegin = Begin;
		for ( usize Index = Begin; Index + 1u < End; ++Index )
		{
			if ( Signature[Index] == ':' && Signature[Index + 1u] == ':' ) NameBegin = Index + 2u;
		}
		if ( NameBegin >= End ) return AcsFw::FEnumNameView{};

		return AcsFw::FEnumNameView{ Signature + NameBegin, End - NameBegin };
	}

	/** 1 つの列挙型ぶんの名前表。 */
	template<typename TEnum>
	struct TEnumTable
	{
		/** 見つかった順に並ぶ名前。`Count` 個だけが有効。 */
		AcsFw::FEnumNameView Names[kScanLimit] = {};

		/** 名前と同じ並びの値。 */
		i64 Values[kScanLimit] = {};

		/** 見つかった個数。 */
		usize Count = 0u;
	};

	/**
	 * 値を 1 つ調べ、列挙子であれば表へ足す。
	 *
	 * @details 値ごとに型引数を変える必要があるので、番号を進めながら自分を呼ぶ形にしている。
	 */
	template<typename TEnum, usize Index>
	constexpr void ScanValue( TEnumTable<TEnum>& Table ) noexcept
	{
		if constexpr ( Index < kScanLimit )
		{
			constexpr AcsFw::FEnumNameView Name =
				ParseEnumeratorName( RawSignature<TEnum, static_cast<TEnum>( Index )>() );

			if constexpr ( !Name.IsEmpty() )
			{
				Table.Names[Table.Count] = Name;
				Table.Values[Table.Count] = static_cast<i64>( Index );
				++Table.Count;
			}

			ScanValue<TEnum, Index + 1u>( Table );
		}
	}

	/** 名前表を作る。 */
	template<typename TEnum>
	constexpr TEnumTable<TEnum> MakeTable() noexcept
	{
		TEnumTable<TEnum> Table{};
		ScanValue<TEnum, 0u>( Table );

		return Table;
	}

	/** 列挙型ごとに 1 つだけ作られる名前表 (すべてコンパイル時に決まる)。 */
	template<typename TEnum>
	inline constexpr TEnumTable<TEnum> kTable = MakeTable<TEnum>();
}


namespace AcsFw
{
	/**
	 * 列挙子の名前表を返す。
	 *
	 * @tparam TEnum 対象の列挙型。
	 * @return 先頭の名前へのポインタ。個数は `EnumCount<TEnum>()`。
	 */
	template<typename TEnum>
	constexpr const FEnumNameView* EnumNames() noexcept
	{
		return AcsFwEnumDetail::kTable<TEnum>.Names;
	}

	/** 列挙子の個数を返す。 */
	template<typename TEnum>
	constexpr usize EnumCount() noexcept
	{
		return AcsFwEnumDetail::kTable<TEnum>.Count;
	}

	/**
	 * その値が列挙子として定義されているかを返す。
	 *
	 * @details 設定ファイルなど、外から来た値をそのまま信じないための関門。
	 */
	template<typename TEnum>
	constexpr bool IsValidEnum( TEnum Value ) noexcept
	{
		const auto& Table = AcsFwEnumDetail::kTable<TEnum>;
		for ( usize Index = 0u; Index < Table.Count; ++Index )
		{
			if ( Table.Values[Index] == static_cast<i64>( Value ) ) return true;
		}

		return false;
	}

	/**
	 * 列挙子を名前表の中の位置へ直す。
	 *
	 * @return 見つかった位置。列挙子でなければ 0。
	 */
	template<typename TEnum>
	constexpr usize EnumToIndex( TEnum Value ) noexcept
	{
		const auto& Table = AcsFwEnumDetail::kTable<TEnum>;
		for ( usize Index = 0u; Index < Table.Count; ++Index )
		{
			if ( Table.Values[Index] == static_cast<i64>( Value ) ) return Index;
		}

		return 0u;
	}

	/**
	 * 名前表の中の位置を列挙子へ戻す。
	 *
	 * @return その位置の列挙子。範囲の外なら先頭の列挙子。
	 */
	template<typename TEnum>
	constexpr TEnum EnumFromIndex( usize Index ) noexcept
	{
		const auto& Table = AcsFwEnumDetail::kTable<TEnum>;
		if ( Table.Count == 0u ) return static_cast<TEnum>( 0 );
		if ( Index >= Table.Count ) return static_cast<TEnum>( Table.Values[0] );

		return static_cast<TEnum>( Table.Values[Index] );
	}

	/**
	 * 列挙子の名前を返す。
	 *
	 * @return 名前。列挙子でなければ空。
	 */
	template<typename TEnum>
	constexpr FEnumNameView EnumToString( TEnum Value ) noexcept
	{
		const auto& Table = AcsFwEnumDetail::kTable<TEnum>;
		for ( usize Index = 0u; Index < Table.Count; ++Index )
		{
			if ( Table.Values[Index] == static_cast<i64>( Value ) ) return Table.Names[Index];
		}

		return FEnumNameView{};
	}
}

#endif

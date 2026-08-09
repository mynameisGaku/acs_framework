// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

// =============================================================================
// 列挙値を DebugTop の行で表示する名前と選択肢へ変換する。
// 入力は列挙型と列挙子名で、名前を取得できない値は変換に失敗する。
//
// 使い方:
//   ACS_ENUM()
//   enum class EMyColor : u8 { Red, Green, Blue };
//
//   TDebugTopEnum<EMyColor>::kCount;                    // 3
//   DebugTopToString( EMyColor::Green );                 // "Green"
//   DebugTopEnumValues<EMyColor>();                      // 範囲 for で回せる表
//   DebugTopMakeEnumOptions<EMyColor>();                 // Enum 行へ渡す選択肢
// =============================================================================

/**
 * 列挙子の名前 (コンパイラのシグネチャ内を指す参照)。
 *
 * @details NUL 終端ではない。参照先は静的な文字列なので寿命は気にしなくてよい。
 */
struct FDebugTopEnumName
{
	/** 名前の先頭 (NUL 終端ではない)。 */
	const char* Data = nullptr;

	/** 名前のバイト数。 */
	usize Size = 0;

	/** 名前を引けなかった (未定義の値) なら true。 */
	constexpr bool IsEmpty() const noexcept { return Size == 0; }

	/**
	 * NUL 終端文字列と一致するかを返す。
	 *
	 * @param Text 比較する NUL 終端文字列。
	 * @return 長さも中身も一致すれば true。
	 */
	constexpr bool Equals( const char* Text ) const noexcept
	{
		if ( Text == nullptr || Data == nullptr ) return false;

		for ( usize Index = 0; Index < Size; ++Index )
		{
			if ( Text[Index] == '\0' || Text[Index] != Data[Index] ) return false;
		}
		return Text[Size] == '\0';
	}

	/** FString へ写す (NUL 終端が要る場面で使う)。 */
	FString ToString() const { return FString( FStringView( Data, Size ) ); }
};


namespace DebugTopEnumDetail
{
	/** 自動で走査する列挙子の上限 (0 .. kScanMax-1 を見る)。 */
	inline constexpr usize kScanMax = 64;

	/** コンパイル時に添字を展開するための列。 */
	template<usize... Indices>
	struct TIndexSeq {};

	/** kScanMax 個の添字列を作るための再帰。 */
	template<usize N, usize... Indices>
	struct TMakeIndexSeq : TMakeIndexSeq<N - 1, N - 1, Indices...> {};

	/** 添字列の再帰終端。 */
	template<usize... Indices>
	struct TMakeIndexSeq<0, Indices...> { using FType = TIndexSeq<Indices...>; };

	/**
	 * 列挙子 1 つ分の名前をコンパイラのシグネチャから切り出す。
	 *
	 * @details
	 * MSVC は有効な列挙子を `...Fn<EMyColor::Green>(void)`、未定義の値を
	 * `...Fn<(enum EMyColor)0xc8>(void)` と綴るので、括弧で始まるものを未定義として弾く。
	 * @tparam Value 名前を引きたい列挙子。
	 * @return 切り出した名前。未定義の値なら空。
	 */
	template<auto Value>
	constexpr FDebugTopEnumName NameFromSignature() noexcept
	{
#if defined( _MSC_VER )
		const char* const Signature = __FUNCSIG__;
		constexpr char kCloser = '>';
		constexpr char kOpener = '<';
#else
		const char* const Signature = __PRETTY_FUNCTION__;
		constexpr char kCloser = ']';
		constexpr char kOpener = '=';
#endif

		usize Length = 0;
		while ( Signature[Length] != '\0' ) ++Length;

		usize Close = 0;
		bool bClosed = false;
		for ( usize Index = Length; Index > 0; --Index )
		{
			if ( Signature[Index - 1] != kCloser ) continue;

			Close = Index - 1;
			bClosed = true;
			break;
		}
		if ( !bClosed ) return FDebugTopEnumName{};

		usize Begin = 0;
		bool bOpened = false;
		for ( usize Index = 0; Index < Close; ++Index )
		{
			if ( Signature[Index] != kOpener ) continue;

			Begin = Index + 1;
			bOpened = true;
			break;
		}
		if ( !bOpened ) return FDebugTopEnumName{};

		while ( Begin < Close && Signature[Begin] == ' ' ) ++Begin;
		if ( Begin >= Close ) return FDebugTopEnumName{};

		// キャスト表記で綴られるのは列挙子が割り当たっていない値。
		if ( Signature[Begin] == '(' ) return FDebugTopEnumName{};

		// 「型名::列挙子」で来るので、最後の :: の後ろだけを取る。
		for ( usize Index = Close; Index > Begin + 1; --Index )
		{
			if ( Signature[Index - 1] != ':' || Signature[Index - 2] != ':' ) continue;

			Begin = Index;
			break;
		}
		return FDebugTopEnumName{ Signature + Begin, Close - Begin };
	}

	/** 走査範囲ぶんの名前表。 */
	template<usize N>
	struct TNameTable
	{
		/** 添字 = 列挙子の値。名前を引けなかった位置は空。 */
		FDebugTopEnumName Items[N];
	};

	/**
	 * 走査範囲の名前をまとめて引く。
	 *
	 * @tparam TEnum 対象の列挙型。
	 * @param Indices 走査する添字列。
	 * @return 添字ぶんの名前表。
	 */
	template<typename TEnum, usize... Indices>
	constexpr TNameTable<sizeof...( Indices )> MakeNameTable( TIndexSeq<Indices...> Indices_ ) noexcept
	{
		(void)Indices_;
		return TNameTable<sizeof...( Indices )>{ { NameFromSignature<static_cast<TEnum>( Indices )>()... } };
	}
}


/**
 * 列挙型の名前表。
 *
 * @details 型を渡すだけで使える。飛び番の列挙でも値から名前を引ける。
 * @tparam TEnum 対象の列挙型。
 */
template<typename TEnum>
struct TDebugTopEnum
{
	/** 走査範囲ぶんの名前表 (添字 = 列挙子の値)。 */
	static constexpr auto kTable = DebugTopEnumDetail::MakeNameTable<TEnum>( typename DebugTopEnumDetail::TMakeIndexSeq<DebugTopEnumDetail::kScanMax>::FType{} );

	/** 名前を引けた列挙子の個数。 */
	static constexpr usize kCount = []
	{
		usize Count = 0;
		for ( usize Index = 0; Index < DebugTopEnumDetail::kScanMax; ++Index )
		{
			if ( !kTable.Items[Index].IsEmpty() ) ++Count;
		}
		return Count;
	}();

	/**
	 * 名前を返す。
	 *
	 * @param Value 対象の列挙子。
	 * @return 名前。走査範囲外や未定義の値なら空。
	 */
	static constexpr FDebugTopEnumName Name( TEnum Value ) noexcept
	{
		const usize Index = static_cast<usize>( Value );
		return Index < DebugTopEnumDetail::kScanMax ? kTable.Items[Index] : FDebugTopEnumName{};
	}

	/**
	 * 添字から列挙子を作る (宣言順で数える)。
	 *
	 * @param Index 0 起点の添字。
	 * @return 対応する列挙子。範囲外なら先頭。
	 */
	static constexpr TEnum FromIndex( i32 Index ) noexcept
	{
		if ( Index < 0 ) return TEnum{};

		usize Remaining = static_cast<usize>( Index );
		for ( usize Slot = 0; Slot < DebugTopEnumDetail::kScanMax; ++Slot )
		{
			if ( kTable.Items[Slot].IsEmpty() ) continue;
			if ( Remaining == 0 ) return static_cast<TEnum>( Slot );

			--Remaining;
		}
		return TEnum{};
	}

	/**
	 * 列挙子を添字にする (宣言順で数える)。
	 *
	 * @param Value 対象の列挙子。
	 * @return 0 起点の添字。名前を引けない値なら -1。
	 */
	static constexpr i32 ToIndex( TEnum Value ) noexcept
	{
		const usize Target = static_cast<usize>( Value );
		if ( Target >= DebugTopEnumDetail::kScanMax || kTable.Items[Target].IsEmpty() ) return -1;

		i32 Index = 0;
		for ( usize Slot = 0; Slot < Target; ++Slot )
		{
			if ( !kTable.Items[Slot].IsEmpty() ) ++Index;
		}
		return Index;
	}

	/**
	 * 名前から列挙子へ戻す。
	 *
	 * @param Name 探す名前 (NUL 終端)。
	 * @param OutValue 見つかった列挙子の書き込み先。
	 * @return 見つかれば true。
	 */
	static constexpr bool TryParse( const char* Name_, TEnum& OutValue ) noexcept
	{
		for ( usize Index = 0; Index < DebugTopEnumDetail::kScanMax; ++Index )
		{
			if ( !kTable.Items[Index].Equals( Name_ ) ) continue;

			OutValue = static_cast<TEnum>( Index );
			return true;
		}
		return false;
	}
};


/**
 * 列挙子の名前を返す。
 *
 * @tparam TEnum 対象の列挙型 (引数から推論される)。
 * @param Value 対象の列挙子。
 * @return 名前。未定義の値なら空。
 */
template<typename TEnum>
constexpr FDebugTopEnumName DebugTopToString( TEnum Value ) noexcept
{
	return TDebugTopEnum<TEnum>::Name( Value );
}

/**
 * 名前から列挙子へ戻す (見つからなければ既定値)。
 *
 * @tparam TEnum 対象の列挙型。
 * @param Name 探す名前 (NUL 終端)。
 * @param Fallback 見つからなかったときに返す値。
 * @return 見つかった列挙子、または Fallback。
 */
template<typename TEnum>
constexpr TEnum DebugTopFromString( const char* Name, TEnum Fallback = TEnum{} ) noexcept
{
	TEnum Value{};
	return TDebugTopEnum<TEnum>::TryParse( Name, Value ) ? Value : Fallback;
}

/**
 * 全ての列挙子から Enum 行用の選択肢を作る。
 *
 * @tparam TEnum 対象の列挙型。
 * @return 宣言順に並べた名前の配列。
 */
template<typename TEnum>
TArray<FString> DebugTopMakeEnumOptions()
{
	TArray<FString> Options;
	Options.Reserve( TDebugTopEnum<TEnum>::kCount );
	for ( usize Index = 0; Index < DebugTopEnumDetail::kScanMax; ++Index )
	{
		const FDebugTopEnumName& Name = TDebugTopEnum<TEnum>::kTable.Items[Index];
		if ( Name.IsEmpty() ) continue;

		Options.Add( Name.ToString() );
	}
	return Options;
}


/**
 * 列挙をリフレクション対象として印付ける (UE の UENUM 相当)。
 *
 * @details
 * 名前と個数はコンパイラのシグネチャから自動で引くため、この印自体は何も展開しない。
 * 列挙の意図をコード上に残すための目印。
 */
#define ACS_ENUM( ... )

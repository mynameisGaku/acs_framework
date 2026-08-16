// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 文へ差し込む値 1 つ。
 *
 * @details
 * 「所持金: {0} G」の `{0}` に入るもの。文字列・整数・実数のどれかを持つ。
 *
 * **文字列を複製しない。** 指している元が先に消えると壊れるので、差し込みが終わるまで
 * 生きている文字列を渡すこと (その場で組み立てて渡すのが普通)。
 */
struct FTextArgument
{
	/** 何が入っているか。 */
	enum class EKind : u8
	{
		/** 文字列。 */
		Text,

		/** 整数。 */
		Integer,

		/** 実数。 */
		Real,
	};

	/** 入っているものの種類。 */
	EKind Kind = EKind::Text;

	/** `Text` のときの中身。 */
	FStringView TextValue;

	/** `Integer` のときの中身。 */
	i64 IntegerValue = 0;

	/** `Real` のときの中身。 */
	f64 RealValue = 0.0;

	/** `Real` のときに小数点以下を何桁出すか。 */
	i32 Decimals = 2;

	/** 文字列から作る。 */
	static FTextArgument FromText( FStringView Value ) noexcept;

	/** 整数から作る。 */
	static FTextArgument FromInteger( i64 Value ) noexcept;

	/**
	 * 実数から作る。
	 *
	 * @param Value 中身。
	 * @param DecimalPlaces 小数点以下の桁数 (0 なら小数点も出さない)。
	 */
	static FTextArgument FromReal( f64 Value, i32 DecimalPlaces = 2 ) noexcept;

	/**
	 * 自分を文字へ直して末尾へ足す。
	 *
	 * @param OutText 足す先。
	 * @return 足せたら true (確保に失敗したら false)。
	 */
	bool AppendTo( FString& OutText ) const noexcept;
};

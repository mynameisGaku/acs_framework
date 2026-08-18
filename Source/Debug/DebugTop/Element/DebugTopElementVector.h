// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Element/DebugTopElementNumber.h"

using namespace acs;

// ベクトルを 1 行にまとめ、展開すると成分ごとに編集できる行。
// 位置・速度・拡縮のように「まとめて見たいが個別に詰めたい」値のための型。

/**
 * ベクトルを編集する行。
 *
 * @details
 * 成分は X / Y / Z / W の子行が持つ。値の実体を子行に置くのは色の行 (CDebugTopElementColor)
 * と同じ作りで、こうすると保存も成分ごとの数値としてそのまま乗る。
 *
 * 畳んでいる間は右カラムに "1.00, 2.00, 3.00" と並べて出すので、開かなくても現在値が分かる。
 *
 * @tparam TVector 扱うベクトル型 (FVec2 / FVec3 / FVec4)。
 * @tparam TCount 成分の数。
 */
template<typename TVector, usize TCount>
class TDebugTopElementVector : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @details
	 * 上下限と刻みは全成分で共通。位置のように広い範囲を取るなら大きめに渡す
	 * (範囲を持つとスライダーが付く)。
	 * @param Label 左カラムへ出す表示名。
	 * @param Value 初期値。
	 * @param Min 各成分の下限。
	 * @param Max 各成分の上限。
	 * @param Step 左右キー 1 回で動く量。
	 */
	TDebugTopElementVector( const FString& Label, const TVector& Value, f32 Min = -100.0f, f32 Max = 100.0f, f32 Step = 0.1f )
		: CDebugTopElement( Label )
	{
		// 成分ごとにスライダー付きの行を持たせる。値はこの子行が持つので、保存もそのまま乗る。
		static const char* const kNames[4] = { "X", "Y", "Z", "W" };
		for ( usize Index = 0; Index < TCount; ++Index )
		{
			m_Components[Index] = Add<CDebugTopElementFloat>( FString( kNames[Index] ), GetComponent( Value, Index ), Min, Max, Step );
		}
	}

	/** 現在値を返す。 */
	TVector GetValue() const noexcept
	{
		TVector Result{};
		for ( usize Index = 0; Index < TCount; ++Index )
		{
			if ( m_Components[Index] == nullptr ) continue;

			SetComponent( Result, Index, m_Components[Index]->GetValue() );
		}
		return Result;
	}

	/**
	 * 値を設定する。
	 *
	 * @param Value 設定する値。
	 */
	void SetValue( const TVector& Value )
	{
		for ( usize Index = 0; Index < TCount; ++Index )
		{
			if ( m_Components[Index] == nullptr ) continue;

			m_Components[Index]->SetValue( GetComponent( Value, Index ) );
		}
	}

	/** 畳んでいるときへ出す、成分を並べた文字列を返す。 */
	FString GetValueText() const override
	{
		FString Text;
		for ( usize Index = 0; Index < TCount; ++Index )
		{
			if ( Index > 0 ) Text.Append( ", " );
			if ( m_Components[Index] == nullptr ) continue;

			Text.AppendFormat( "%.2f", static_cast<double>( m_Components[Index]->GetValue() ) );
		}
		return Text;
	}

private:
	/**
	 * ベクトルから成分を 1 つ取り出す。
	 *
	 * @details FVec2 に z が無いので、成分数で分岐してから触る。
	 * @param Value 取り出す元。
	 * @param Index 成分の位置。
	 * @return 成分の値。
	 */
	static f32 GetComponent( const TVector& Value, usize Index ) noexcept
	{
		if ( Index == 0 ) return Value.x;
		if ( Index == 1 ) return Value.y;
		if constexpr ( TCount >= 3 ) { if ( Index == 2 ) return Value.z; }
		if constexpr ( TCount >= 4 ) { if ( Index == 3 ) return Value.w; }
		return 0.0f;
	}

	/**
	 * ベクトルの成分を 1 つ書き込む。
	 *
	 * @param Value 書き込む先。
	 * @param Index 成分の位置。
	 * @param Component 書き込む値。
	 */
	static void SetComponent( TVector& Value, usize Index, f32 Component ) noexcept
	{
		if ( Index == 0 ) { Value.x = Component; return; }
		if ( Index == 1 ) { Value.y = Component; return; }
		if constexpr ( TCount >= 3 ) { if ( Index == 2 ) { Value.z = Component; return; } }
		if constexpr ( TCount >= 4 ) { if ( Index == 3 ) { Value.w = Component; return; } }
	}

	/** X / Y / Z / W を持つ子行。所有はこの行 (子行配列) が持つ。 */
	CDebugTopElementFloat* m_Components[TCount] {};
};


/** 2 成分のベクトルを扱う行。 */
using CDebugTopElementVec2 = TDebugTopElementVector<FVec2, 2>;

/** 3 成分のベクトルを扱う行 (位置・速度・拡縮)。 */
using CDebugTopElementVec3 = TDebugTopElementVector<FVec3, 3>;

/** 4 成分のベクトルを扱う行。 */
using CDebugTopElementVec4 = TDebugTopElementVector<FVec4, 4>;

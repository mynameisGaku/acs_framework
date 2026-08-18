// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// 配列を 1 行にまとめ、展開すると要素ごとに編集できる行。

template<typename TValue>
class TDebugTopElementArray;


/**
 * 配列行の要素 1 つを指す子行。
 *
 * @details 値は自分では持たず、親の配列を直接読み書きする。TDebugTopElementArray が
 * 要素数ぶん自動生成するので、通常は直接生成しない。
 */
template<typename TValue>
class TDebugTopElementArrayItem : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Owner 値を持っている親の配列行。
	 * @param Index 親配列の中でこの行が指す位置。
	 */
	TDebugTopElementArrayItem( const FString& Label, TDebugTopElementArray<TValue>& Owner, usize Index )
		: CDebugTopElement( Label )
		, m_Owner( &Owner )
		, m_Index( Index )
	{
	}

	/** 右カラムへ親配列の該当要素を出す。 */
	FString GetValueText() const override
	{
		return DebugTopFormatValue( m_Owner->GetValue( m_Index ) );
	}

	/** 左右キーで親配列の該当要素を増減する。 */
	void OnLeftRight( i32 Delta ) override
	{
		m_Owner->StepValue( m_Index, Delta );
	}

	/** 左右キーに反応する。 */
	bool IsLeftRightAdjustable() const noexcept override { return true; }

	/** 要素の型に応じて Int か Float を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override
	{
		if constexpr ( IsSameV<TValue, i32> ) return EDebugTopValueKind::Int;
		else if constexpr ( IsSameV<TValue, f32> ) return EDebugTopValueKind::Float;
		else return EDebugTopValueKind::None;
	}

	/** 要素が i32 なら親配列から値を取り出す。 */
	bool TryGetInt( i32& OutValue ) const noexcept override
	{
		if constexpr ( IsSameV<TValue, i32> )
		{
			OutValue = m_Owner->GetValue( m_Index );
			return true;
		}
		else
		{
			(void)OutValue;
			return false;
		}
	}

	/** 要素が f32 なら親配列から値を取り出す。 */
	bool TryGetFloat( f32& OutValue ) const noexcept override
	{
		if constexpr ( IsSameV<TValue, f32> )
		{
			OutValue = m_Owner->GetValue( m_Index );
			return true;
		}
		else
		{
			(void)OutValue;
			return false;
		}
	}

	/** 要素が i32 なら親配列の該当要素へ書き込む。 */
	bool TrySetInt( i32 Value ) override
	{
		if constexpr ( IsSameV<TValue, i32> )
		{
			m_Owner->SetValue( m_Index, Value );
			return true;
		}
		else
		{
			(void)Value;
			return false;
		}
	}

	/** 要素が f32 なら親配列の該当要素へ書き込む。 */
	bool TrySetFloat( f32 Value ) override
	{
		if constexpr ( IsSameV<TValue, f32> )
		{
			m_Owner->SetValue( m_Index, Value );
			return true;
		}
		else
		{
			(void)Value;
			return false;
		}
	}

	/** 親配列の中でこの行が指す位置を返す。 */
	usize GetIndex() const noexcept { return m_Index; }

	/** 値を持っている親の配列行を返す。 */
	TDebugTopElementArray<TValue>& GetOwner() const noexcept { return *m_Owner; }

private:
	/** 値を持っている親の配列行。所有はしない (親が自分を所有している)。 */
	TDebugTopElementArray<TValue>* m_Owner;

	/** 親配列の中でこの行が指す位置。 */
	usize m_Index;
};


/**
 * 配列を 1 行にまとめ、展開すると要素を個別に編集できる行。
 *
 * @details
 * 構築時に要素数ぶんの子行を作る。右カラムには全要素を並べた要約を出す。
 * 要素数は構築後に変わらない前提 (子行が指す添字が固定のため)。
 */
template<typename TValue>
class TDebugTopElementArray : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Values 初期値の配列。
	 * @param Min 各要素の下限。
	 * @param Max 各要素の上限。
	 * @param Step 左右キー 1 回あたりの増減量。
	 */
	TDebugTopElementArray( const FString& Label, TArray<TValue> Values, TValue Min, TValue Max, TValue Step )
		: CDebugTopElement( Label )
		, m_Values( Move( Values ) )
		, m_Min( Min )
		, m_Max( Max )
		, m_Step( Step )
	{
		if ( m_Min > m_Max )
		{
			const TValue Swapped = m_Min;
			m_Min = m_Max;
			m_Max = Swapped;
		}

		for ( usize Index = 0; Index < m_Values.Num(); ++Index )
		{
			m_Values[Index] = Clamp( m_Values[Index] );

			FString ItemLabel;
			ItemLabel.AppendFormat( "[%zu]", Index );
			AddChild( MakeShared<TDebugTopElementArrayItem<TValue>>( ItemLabel, *this, Index ) );
		}
	}

	/** 要素数を返す。 */
	usize GetCount() const noexcept { return m_Values.Size(); }

	/** 配列全体を返す。 */
	const TArray<TValue>& GetValues() const noexcept { return m_Values; }

	/**
	 * 要素を 1 つ返す。
	 *
	 * @param Index 取り出す位置 (範囲外なら下限を返す)。
	 * @return 該当要素の値。
	 */
	TValue GetValue( usize Index ) const
	{
		if ( Index >= m_Values.Num() ) return m_Min;
		return m_Values[Index];
	}

	/**
	 * 要素を 1 つ設定する (上下限へクランプし、変化したときだけ通知する)。
	 *
	 * @param Index 設定する位置 (範囲外なら何もしない)。
	 * @param Value 設定する値。
	 */
	void SetValue( usize Index, TValue Value )
	{
		if ( Index >= m_Values.Num() ) return;

		const TValue Clamped = Clamp( Value );
		if ( Clamped == m_Values[Index] ) return;

		m_Values[Index] = Clamped;
		NotifyChanged();
	}

	/**
	 * 要素を 1 つ Step 分だけ増減する。
	 *
	 * @param Index 増減する位置。
	 * @param Delta 左キーで -1、右キーで +1。
	 */
	void StepValue( usize Index, i32 Delta )
	{
		if ( Index >= m_Values.Num() ) return;
		SetValue( Index, m_Values[Index] + m_Step * static_cast<TValue>( Delta ) );
	}

	/** 右カラムへ全要素を並べて出す (長い場合は末尾を省略する)。 */
	FString GetValueText() const override
	{
		/** 省略せずに並べる最大要素数。 */
		constexpr usize kMaxShownCount = 8;

		FString Text;
		const usize ShownCount = m_Values.Num() < kMaxShownCount ? m_Values.Num() : kMaxShownCount;
		for ( usize Index = 0; Index < ShownCount; ++Index )
		{
			if ( Index > 0 ) Text.Append( ", " );
			Text.Append( DebugTopFormatValue( m_Values[Index] ) );
		}
		if ( m_Values.Num() > ShownCount )
		{
			Text.Append( ", ..." );
		}
		return Text;
	}

	/** 要素の型に応じて IntArray か FloatArray を返す。 */
	EDebugTopValueKind GetValueKind() const noexcept override
	{
		if constexpr ( IsSameV<TValue, i32> ) return EDebugTopValueKind::IntArray;
		else if constexpr ( IsSameV<TValue, f32> ) return EDebugTopValueKind::FloatArray;
		else return EDebugTopValueKind::None;
	}

private:
	/**
	 * 値を上下限へ収める。
	 *
	 * @param Value 収める値。
	 * @return クランプ後の値。
	 */
	TValue Clamp( TValue Value ) const
	{
		if ( Value < m_Min ) return m_Min;
		if ( Value > m_Max ) return m_Max;
		return Value;
	}

	/** 要素の実体。 */
	TArray<TValue> m_Values;

	/** 各要素の下限。 */
	TValue m_Min;

	/** 各要素の上限。 */
	TValue m_Max;

	/** 左右キー 1 回あたりの増減量。 */
	TValue m_Step;
};


/** i32 配列を扱う行。 */
using CDebugTopElementIntArray = TDebugTopElementArray<i32>;

/** f32 配列を扱う行。 */
using CDebugTopElementFloatArray = TDebugTopElementArray<f32>;

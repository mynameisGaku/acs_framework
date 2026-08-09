#include "DebugTopElementNumber.h"

#include <cstdlib>


CDebugTopElementInt::CDebugTopElementInt( const FString& Label, i32 Value, i32 Min, i32 Max, i32 Step )
	: CDebugTopElement( Label )
	, m_Value( Value )
	, m_Min( Min )
	, m_Max( Max )
	, m_Step( Step )
{
	// 上下限が逆に渡されても以降の比較が壊れないように正規化する。
	if ( m_Min > m_Max )
	{
		const i32 Swapped = m_Min;
		m_Min = m_Max;
		m_Max = Swapped;
	}
	if ( m_Value < m_Min ) m_Value = m_Min;
	if ( m_Value > m_Max ) m_Value = m_Max;

	m_DefaultValue = m_Value;
}


bool CDebugTopElementInt::TryGetRatio( f32& OutRatio ) const noexcept
{
	// 候補モードは範囲ではなく並びなので、スライダーには載せない。
	if ( !m_Data.IsEmpty() ) return false;
	if ( m_Max <= m_Min ) return false;

	OutRatio = static_cast<f32>( m_Value - m_Min ) / static_cast<f32>( m_Max - m_Min );
	return true;
}


bool CDebugTopElementInt::TrySetRatio( f32 Ratio )
{
	if ( !m_Data.IsEmpty() ) return false;
	if ( m_Max <= m_Min ) return false;

	if ( Ratio < 0.0f ) Ratio = 0.0f;
	if ( Ratio > 1.0f ) Ratio = 1.0f;

	// 四捨五入して整数へ落とす (切り捨てだと上限へ届かない)。
	const f32 Span = static_cast<f32>( m_Max - m_Min );
	SetValue( m_Min + static_cast<i32>( Span * Ratio + 0.5f ) );
	return true;
}


void CDebugTopElementInt::AddData( const FString& Title, i32 Value )
{
	FData Entry;
	Entry.Title = Title;
	Entry.Value = Value;
	m_Data.Add( Move( Entry ) );

	// 最初の候補が入った時点でそれを選択状態にする (構築中なので通知はしない)。
	if ( m_Data.Num() == 1 )
	{
		m_Select = 0;
		ApplySelectedData();
		m_DefaultValue = m_Value;
	}
}


i32 CDebugTopElementInt::GetSelectedDataIndex() const noexcept
{
	return m_Data.IsEmpty() ? -1 : m_Select;
}


void CDebugTopElementInt::SetSelectedDataIndex( i32 Index )
{
	if ( m_Data.IsEmpty() ) return;
	if ( Index < 0 || Index >= static_cast<i32>( m_Data.Num() ) ) return;
	if ( Index == m_Select ) return;

	m_Select = Index;
	ApplySelectedData();
	NotifyChanged();
}


void CDebugTopElementInt::ApplySelectedData()
{
	if ( m_Data.IsEmpty() ) return;

	const FData& Entry = m_Data[static_cast<usize>( m_Select )];
	m_Value = Entry.Value;
	SetSubTitle( Entry.Title );
}


void CDebugTopElementInt::SetValue( i32 Value )
{
	if ( !m_Data.IsEmpty() )
	{
		// 候補モードでは値に対応する候補へ合わせる (対応する候補が無ければ何もしない)。
		for ( usize Index = 0; Index < m_Data.Num(); ++Index )
		{
			if ( m_Data[Index].Value == Value )
			{
				SetSelectedDataIndex( static_cast<i32>( Index ) );
				return;
			}
		}
		return;
	}

	if ( Value < m_Min ) Value = m_Min;
	if ( Value > m_Max ) Value = m_Max;
	if ( Value == m_Value ) return;

	m_Value = Value;
	NotifyChanged();
}


FString CDebugTopElementInt::GetValueText() const
{
	if ( !m_Data.IsEmpty() )
	{
		const FData& Entry = m_Data[static_cast<usize>( m_Select )];
		if ( !Entry.Title.IsEmpty() ) return Entry.Title;
	}
	return DebugTopFormatValue( m_Value );
}


void CDebugTopElementInt::OnLeftRight( i32 Delta )
{
	if ( !m_Data.IsEmpty() )
	{
		SetSelectedDataIndex( m_Select + Delta );
		return;
	}
	SetValue( m_Value + m_Step * Delta );
}


bool CDebugTopElementInt::TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept
{
	if ( m_Data.IsEmpty() ) return false;

	OutIndex = m_Select;
	OutCount = static_cast<i32>( m_Data.Num() );
	return true;
}


bool CDebugTopElementInt::TryGetInt( i32& OutValue ) const noexcept
{
	OutValue = m_Value;
	return true;
}


bool CDebugTopElementInt::TrySetInt( i32 Value )
{
	SetValue( Value );
	return true;
}


FString CDebugTopElementInt::GetEditText() const
{
	FString Text;
	Text.AppendFormat( "%d", m_Value );
	return Text;
}


bool CDebugTopElementInt::CommitEditText( const FString& Text )
{
	if ( Text.IsEmpty() ) return false;

	// 数字以外で終わっている入力 (打ち間違い) は捨てて、元の値を残す。
	char* End = nullptr;
	const long Parsed = std::strtol( Text.Data(), &End, 10 );
	if ( End == Text.Data() || ( End != nullptr && *End != '\0' ) ) return false;

	SetValue( static_cast<i32>( Parsed ) );
	return true;
}


CDebugTopElementFloat::CDebugTopElementFloat( const FString& Label, f32 Value, f32 Min, f32 Max, f32 Step )
	: CDebugTopElement( Label )
	, m_Value( Value )
	, m_Min( Min )
	, m_Max( Max )
	, m_Step( Step )
{
	if ( m_Min > m_Max )
	{
		const f32 Swapped = m_Min;
		m_Min = m_Max;
		m_Max = Swapped;
	}
	if ( m_Value < m_Min ) m_Value = m_Min;
	if ( m_Value > m_Max ) m_Value = m_Max;

	m_DefaultValue = m_Value;
}


bool CDebugTopElementFloat::TryGetRatio( f32& OutRatio ) const noexcept
{
	if ( m_Max <= m_Min ) return false;

	OutRatio = ( m_Value - m_Min ) / ( m_Max - m_Min );
	return true;
}


bool CDebugTopElementFloat::TrySetRatio( f32 Ratio )
{
	if ( m_Max <= m_Min ) return false;

	if ( Ratio < 0.0f ) Ratio = 0.0f;
	if ( Ratio > 1.0f ) Ratio = 1.0f;

	SetValue( m_Min + ( m_Max - m_Min ) * Ratio );
	return true;
}


void CDebugTopElementFloat::SetValue( f32 Value )
{
	if ( Value < m_Min ) Value = m_Min;
	if ( Value > m_Max ) Value = m_Max;
	if ( Value == m_Value ) return;

	m_Value = Value;
	NotifyChanged();
}


FString CDebugTopElementFloat::GetValueText() const
{
	return DebugTopFormatValue( m_Value );
}


void CDebugTopElementFloat::OnLeftRight( i32 Delta )
{
	SetValue( m_Value + m_Step * static_cast<f32>( Delta ) );
}


bool CDebugTopElementFloat::TryGetFloat( f32& OutValue ) const noexcept
{
	OutValue = m_Value;
	return true;
}


bool CDebugTopElementFloat::TrySetFloat( f32 Value )
{
	SetValue( Value );
	return true;
}


FString CDebugTopElementFloat::GetEditText() const
{
	return DebugTopFormatValue( m_Value );
}


bool CDebugTopElementFloat::CommitEditText( const FString& Text )
{
	if ( Text.IsEmpty() ) return false;

	char* End = nullptr;
	const double Parsed = std::strtod( Text.Data(), &End );
	if ( End == Text.Data() || ( End != nullptr && *End != '\0' ) ) return false;

	SetValue( static_cast<f32>( Parsed ) );
	return true;
}

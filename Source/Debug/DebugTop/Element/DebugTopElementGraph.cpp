#include "DebugTopElementGraph.h"


namespace
{
	/** グラフに溜める標本の既定数。 */
	constexpr usize kDefaultGraphSamples = 120;
}


CDebugTopElementGraph::CDebugTopElementGraph( const FString& Label, FDebugTopValueDelegate Provider, usize SampleCount )
	: CDebugTopElement( Label )
	, m_Provider( Provider )
	, m_Capacity( SampleCount > 0 ? SampleCount : kDefaultGraphSamples )
{
	SetMarkerVisibility( EDebugTopMarkerVisibility::Never );
	m_Samples.Reserve( m_Capacity );
}


void CDebugTopElementGraph::SetRange( f32 Min, f32 Max ) noexcept
{
	m_Min = Min;
	m_Max = Max;
	m_bFixedRange = Max > Min;
}


void CDebugTopElementGraph::OnTick( f32 DeltaSeconds )
{
	(void)DeltaSeconds;

	f32 Value = 0.0f;
	if ( !m_Provider.TryExecute( Value ) ) return;

	// 溜まりきったら古いものから捨てる。標本は 120 個程度なので詰め直しでも十分に軽い。
	if ( m_Samples.Num() >= m_Capacity ) m_Samples.RemoveAt( 0 );

	m_Samples.Add( Value );
}


bool CDebugTopElementGraph::TryGetGraph( const f32*& OutSamples, usize& OutCount, f32& OutMin, f32& OutMax ) const noexcept
{
	OutSamples = m_Samples.GetData();
	OutCount = m_Samples.Num();

	if ( m_bFixedRange )
	{
		OutMin = m_Min;
		OutMax = m_Max;
		return true;
	}

	// 自動のときは溜まっているぶんに合わせる。全部同じ値だと高さが 0 になるので少し広げる。
	f32 Min = 0.0f;
	f32 Max = 0.0f;
	for ( usize Index = 0; Index < m_Samples.Num(); ++Index )
	{
		const f32 Sample = m_Samples[Index];
		if ( Index == 0 || Sample < Min ) Min = Sample;
		if ( Index == 0 || Sample > Max ) Max = Sample;
	}
	if ( !( Max > Min ) )
	{
		Min -= 1.0f;
		Max += 1.0f;
	}
	OutMin = Min;
	OutMax = Max;
	return true;
}


FString CDebugTopElementGraph::GetValueText() const
{
	if ( m_Samples.IsEmpty() ) return FString();

	return DebugTopFormatValue( m_Samples[m_Samples.Num() - 1] );
}

bool CDebugTopElementGraph::TryGetFloat( f32& OutValue ) const noexcept
{
	if ( m_Samples.IsEmpty() ) return false;

	OutValue = m_Samples[m_Samples.Num() - 1];
	return true;
}

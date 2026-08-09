#include "AcsFramework_Core/Input/InputRepeat.h"


FInputRepeat::FInputRepeat( f32 DelaySeconds, f32 IntervalSeconds ) noexcept
{
	SetTiming( DelaySeconds, IntervalSeconds );
}


void FInputRepeat::SetTiming( f32 DelaySeconds, f32 IntervalSeconds ) noexcept
{
	// 0 以下だと 1 フレームで何度も出てしまうので、下限で止める。
	m_DelaySeconds = DelaySeconds > 0.0f ? DelaySeconds : 0.0f;
	m_IntervalSeconds = IntervalSeconds > 0.0001f ? IntervalSeconds : 0.0001f;
}


i32 FInputRepeat::Step( i32 Raw, f32 DeltaSeconds ) noexcept
{
	// 離した / 向きが変わったら、その場で 1 回出して測り直す。
	if ( Raw != m_Last )
	{
		m_Last = Raw;
		m_Timer = 0.0f;
		m_bRepeating = false;
		return Raw;
	}

	if ( Raw == 0 ) return 0;

	m_Timer += DeltaSeconds;

	// 入れ始めてしばらくは動かさず、そこから先は一定間隔で連射する。
	const f32 Threshold = m_bRepeating ? m_IntervalSeconds : m_DelaySeconds;
	if ( m_Timer < Threshold ) return 0;

	m_Timer -= Threshold;
	m_bRepeating = true;
	return Raw;
}


void FInputRepeat::Reset() noexcept
{
	m_Last = 0;
	m_Timer = 0.0f;
	m_bRepeating = false;
}

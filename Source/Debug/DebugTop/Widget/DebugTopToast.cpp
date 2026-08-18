// SPDX-License-Identifier: Apache-2.0
#include "DebugTopToast.h"

namespace
{
	/** 既定で留まる秒数。 */
	constexpr f32 kDefaultDuration = 5.0f;

	/** 右から滑り込むのにかける秒数。 */
	constexpr f32 kSlideSeconds = 0.22f;

	/** 薄くなって消えるのにかける秒数。 */
	constexpr f32 kFadeSeconds = 0.45f;
}


CDebugTopToast::CDebugTopToast( EDebugTopToastKind Kind, const FString& Title, const FString& Message )
	: m_Kind( Kind )
	, m_Title( Title )
	, m_Message( Message )
	, m_Duration( kDefaultDuration )
{
}

CDebugTopToast& CDebugTopToast::AddButton( const FString& Label, FSimpleDelegate OnPressed )
{
	FDebugTopToastButton Button;
	Button.Label = Label;
	Button.OnPressed = OnPressed;
	m_Buttons.Add( Move( Button ) );
	return *this;
}

CDebugTopToast& CDebugTopToast::SetDuration( f32 Seconds ) noexcept
{
	if ( Seconds > 0.0f ) m_Duration = Seconds;
	return *this;
}

void CDebugTopToast::Update( f32 DeltaSeconds, bool bHovered )
{
	// 閉じ始めていたら、あとは薄くなるのを進めるだけ。
	if ( m_DismissElapsed >= 0.0f )
	{
		m_DismissElapsed += DeltaSeconds;
		return;
	}

	// 滑り込みの途中はマウスに関係なく進める (出きる前に止まると出し切れない)。
	if ( m_Elapsed < kSlideSeconds )
	{
		m_Elapsed += DeltaSeconds;
		return;
	}

	// 重ねている間は数えない。ボタンへ手を伸ばしている最中に消えてしまわないように。
	if ( bHovered ) return;

	m_Elapsed += DeltaSeconds;
	if ( m_Elapsed >= kSlideSeconds + m_Duration ) Dismiss();
}

void CDebugTopToast::Dismiss() noexcept
{
	if ( m_DismissElapsed < 0.0f ) m_DismissElapsed = 0.0f;
}

bool CDebugTopToast::IsFinished() const noexcept
{
	return m_DismissElapsed >= kFadeSeconds;
}

f32 CDebugTopToast::GetSlideRatio() const noexcept
{
	if ( m_Elapsed >= kSlideSeconds ) return 1.0f;

	// 終わりへ向かって緩めると、止まり方が硬く見えない。
	const f32 Linear = m_Elapsed / kSlideSeconds;
	const f32 Rest = 1.0f - Linear;
	return 1.0f - Rest * Rest * Rest;
}

f32 CDebugTopToast::GetOpacity() const noexcept
{
	if ( m_DismissElapsed < 0.0f ) return 1.0f;

	const f32 Remaining = 1.0f - m_DismissElapsed / kFadeSeconds;
	if ( Remaining < 0.0f ) return 0.0f;
	if ( Remaining > 1.0f ) return 1.0f;
	return Remaining;
}

void CDebugTopToast::SetRect( f32 X, f32 Y, f32 Width, f32 Height ) noexcept
{
	m_X = X;
	m_Y = Y;
	m_Width = Width;
	m_Height = Height;
}

void CDebugTopToast::SetCloseRect( f32 X, f32 Y, f32 Size ) noexcept
{
	m_CloseX = X;
	m_CloseY = Y;
	m_CloseSize = Size;
}

void CDebugTopToast::SetButtonRow( f32 Y, f32 Height ) noexcept
{
	m_ButtonY = Y;
	m_ButtonHeight = Height;
}

// SPDX-License-Identifier: Apache-2.0
#include "DebugTopValueEditor.h"

#include "Debug/DebugTop/Input/DebugTopCursor.h"


void CDebugTopValueEditor::Begin( CDebugTopElement& Element )
{
	m_Element = &Element;
	m_Edit.Begin( Element.GetEditText() );
}

bool CDebugTopValueEditor::Update( f32 DeltaSeconds, const CDebugTopElement* CursorElement ) noexcept
{
	if ( !m_Edit.IsActive() ) return false;

	// 対象の行が消えていたら打ち込みごと捨てる (行の入れ替えに巻き込まれないように)。
	if ( m_Element == nullptr || m_Element != CursorElement )
	{
		m_Edit.Cancel();
		m_Element = nullptr;
		return false;
	}

	// 取り消しは決定より先に見る (同じフレームで両方来たら取り消しを優先する)。
	if ( CInput::IsKeyPressed( EKey::Escape ) )
	{
		m_Edit.Cancel();
		m_Element = nullptr;
		DebugTopSetCursor( EDebugTopCursor::Arrow );
		return true;
	}

	// 欄の外を押したら、打ったところまでを反映して抜ける。わざわざ Enter を押させない。
	if ( CInput::IsMouseButtonPressed( EMouseButton::Left ) )
	{
		const FVec2 Mouse = CInput::MousePos();
		const bool bInsideField = Mouse.x >= m_FieldX && Mouse.x <= m_FieldX + m_FieldWidth && Mouse.y >= m_FieldY && Mouse.y <= m_FieldY + m_FieldHeight;

		if ( !bInsideField )
		{
			m_Element->CommitEditText( m_Edit.GetText() );
			m_Edit.Cancel();
			m_Element = nullptr;

			// この押下を握り潰さず後続へ渡す。別の欄を押したなら、そのままそちらへ
			// 移りたいのに、握り潰すと 2 回押す羽目になる。
			return false;
		}
	}

	m_Edit.Update( DeltaSeconds );

	FString Typed;
	if ( m_Edit.TryCommit( Typed ) )
	{
		// 数字以外を打った等で読めなければ、元の値をそのまま残す。
		m_Element->CommitEditText( Typed );
		m_Element = nullptr;
	}
	return true;
}

#include "DebugTopKeyNav.h"


void CDebugTopKeyNav::Update( f32 DeltaSeconds ) noexcept
{
	// テンキー側でも同じように効かせる (NumLock を切っていても動くように)。
	i32 RawVertical = 0;
	if ( CInput::IsKeyDown( EKey::Up )   || CInput::IsKeyDown( EKey::KP8 ) ) RawVertical -= 1;
	if ( CInput::IsKeyDown( EKey::Down ) || CInput::IsKeyDown( EKey::KP2 ) ) RawVertical += 1;

	i32 RawHorizontal = 0;
	if ( CInput::IsKeyDown( EKey::Left )  || CInput::IsKeyDown( EKey::KP4 ) ) RawHorizontal -= 1;
	if ( CInput::IsKeyDown( EKey::Right ) || CInput::IsKeyDown( EKey::KP6 ) ) RawHorizontal += 1;

	m_Vertical = m_VerticalRepeat.Step( RawVertical, DeltaSeconds );
	m_Horizontal = m_HorizontalRepeat.Step( RawHorizontal, DeltaSeconds );
}

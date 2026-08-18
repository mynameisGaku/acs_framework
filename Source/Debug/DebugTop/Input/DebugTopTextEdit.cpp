// SPDX-License-Identifier: Apache-2.0
#include "DebugTopTextEdit.h"

namespace
{
	/** キャレットが点滅する周期 (秒)。 */
	constexpr f32 kBlinkSeconds = 1.0f;

	/** 打ち込める文字数の上限 (これ以上は無視する)。 */
	constexpr usize kMaxLength = 128;
}


void CDebugTopTextEdit::Begin( const FString& Initial )
{
	m_Text = Initial;
	m_BlinkSeconds = 0.0f;
	m_bActive = true;
	m_bSelectAll = true;
}

void CDebugTopTextEdit::Cancel() noexcept
{
	m_Text.Clear();
	m_bActive = false;
	m_bSelectAll = false;
}

void CDebugTopTextEdit::Update( f32 DeltaSeconds )
{
	if ( !m_bActive ) return;

	m_BlinkSeconds += DeltaSeconds;
	if ( m_BlinkSeconds >= kBlinkSeconds ) m_BlinkSeconds -= kBlinkSeconds;

	// IME 確定後の文字が来る。1 フレーム分をまとめて受け取る。
	if ( const char* const Typed = CInput::TextInput() )
	{
		for ( const char* Cursor = Typed; *Cursor != '\0'; ++Cursor )
		{
			// 制御文字 (Enter や Tab) は文字として積まない。
			if ( static_cast<u8>( *Cursor ) < 0x20u ) continue;

			// 全選択の状態で打たれた最初の 1 文字は、選択ごと置き換える。
			if ( m_bSelectAll )
			{
				m_Text.Clear();
				m_bSelectAll = false;
			}
			if ( m_Text.Size() >= kMaxLength ) break;

			m_Text.Append( *Cursor );
		}
	}

	if ( CInput::IsKeyPressed( EKey::Backspace ) && m_bSelectAll )
	{
		// 選択されているものを消す操作なので、丸ごと消える。
		m_Text.Clear();
		m_bSelectAll = false;
	}
	else if ( CInput::IsKeyPressed( EKey::Backspace ) && !m_Text.IsEmpty() )
	{
		// UTF-8 の途中で切らないよう、後続バイトを跨いで 1 文字ぶん落とす。
		usize NewSize = m_Text.Size() - 1;
		while ( NewSize > 0 && ( static_cast<u8>( m_Text[NewSize] ) & 0xC0u ) == 0x80u ) --NewSize;

		m_Text = FString( FStringView( m_Text.Data(), NewSize ) );
	}
}

bool CDebugTopTextEdit::TryCommit( FString& OutText )
{
	if ( !m_bActive ) return false;
	if ( !CInput::IsKeyPressed( EKey::Enter ) ) return false;

	OutText = m_Text;
	m_Text.Clear();
	m_bActive = false;
	m_bSelectAll = false;
	return true;
}

FString CDebugTopTextEdit::MakeDisplayText() const
{
	// 全選択のうちは下敷きで選択を示すので、キャレットは出さない (二重に見えるため)。
	if ( m_bSelectAll ) return m_Text;

	FString Display = m_Text;

	// 打っている位置が分かるようにキャレットを点滅させる。周期の前半だけ出す。
	Display.Append( m_BlinkSeconds < kBlinkSeconds * 0.5f ? '_' : ' ' );
	return Display;
}

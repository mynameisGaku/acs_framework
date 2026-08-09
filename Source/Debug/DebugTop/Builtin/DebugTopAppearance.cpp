#include "DebugTopAppearance.h"

#include "Debug/DebugTop/DebugTopHUD.h"

namespace
{
	/** 文字サイズの下限 (これ以下だと読めない)。 */
	constexpr f32 kMinFontSize = 12.0f;

	/** 文字サイズの上限 (これ以上だと一覧が数行しか入らない)。 */
	constexpr f32 kMaxFontSize = 48.0f;

	/** 左右キー 1 回で動く量。 */
	constexpr f32 kFontSizeStep = 1.0f;
}


ADebugTopAppearanceEntity::ADebugTopAppearanceEntity( const FString& Name, ADebugTopHUD& HUD )
	: ADebugTopEntity( Name )
	, m_HUD( &HUD )
{
}

void ADebugTopAppearanceEntity::OnBuild() noexcept
{
	SetHeader( "Appearance" );
	SetDescription( FString( "メニュー自身の見た目を変えます\n" "変えるとその場で効きます (保存を待ちません)" ) );

	if ( m_HUD == nullptr ) return;

	m_FontSize = Add<CDebugTopElementFloat>( "FontSize", m_HUD->GetFontSize(), kMinFontSize, kMaxFontSize, kFontSizeStep );
	m_FontSize->SetUnit( FString( "px" ) );
	m_FontSize->SetOnChanged( FSimpleDelegate::CreateRaw<&ADebugTopAppearanceEntity::ApplyFontSize>( this ) );
	m_FontSize->SetDescription( FString( "行の高さも段差も右カラムの位置も、この大きさに追従します\n" "動かしている間は少しにじみ、手を止めると焼き直して鮮明になります" ) );

	m_IncludeCjk = Add<CDebugTopElementBool>( "IncludeCjk", m_HUD->IsFontIncludeCjk() );
	m_IncludeCjk->SetOnChanged( FSimpleDelegate::CreateRaw<&ADebugTopAppearanceEntity::ApplyIncludeCjk>( this ) );
	m_IncludeCjk->SetDescription( FString( "漢字をアトラスへ焼くか\n" "切ると焼き直しが速くなりますが、漢字が出なくなります\n" "英数字しか使わないメニューなら切ってよい" ) );
}

void ADebugTopAppearanceEntity::ApplyFontSize()
{
	if ( m_HUD == nullptr || m_FontSize == nullptr ) return;

	m_HUD->SetFontSize( m_FontSize->GetValue() );
}

void ADebugTopAppearanceEntity::ApplyIncludeCjk()
{
	if ( m_HUD == nullptr || m_IncludeCjk == nullptr ) return;

	m_HUD->SetFontIncludeCjk( m_IncludeCjk->GetValue() );
}

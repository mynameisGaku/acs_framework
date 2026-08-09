#include "DebugTopHUD.h"

#include "Debug/DebugTop/Input/DebugTopCursor.h"
#include "Debug/DebugTop/Service/DebugTopShortcuts.h"
#include "Debug/DebugTop/Widget/DebugTopFilterBox.h"
#include "Debug/DebugTop/Widget/DebugTopToastStack.h"

namespace
{
	/** 画面最上段へ出すタイトル。 */
	constexpr const char* kTitleText = "DebugTop";

	/** パンくずのページ名どうしを繋ぐ区切り。 */
	constexpr const char* kBreadcrumbSeparator = " - ";

	/** 描画の左端 X (ピクセル)。 */
	constexpr f32 kOriginX = 24.0f;

	/** タイトル行の上端 Y (ピクセル)。 */
	constexpr f32 kOriginY = 16.0f;

	/** タイトルの文字色。 */
	constexpr FVec4 kTitleColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	/** パンくずの文字色。 */
	constexpr FVec4 kBreadcrumbColor{ 0.60f, 0.75f, 0.95f, 1.0f };

	/** 説明文の既定の文字色。 */
	constexpr FVec4 kDescriptionColor{ 0.88f, 0.88f, 0.88f, 1.0f };

	/** 説明文の下敷きの色。 */
	constexpr FVec4 kDescriptionPanelColor{ 0.05f, 0.06f, 0.09f, 0.82f };

	/** 説明文の下敷きの縁の色。 */
	constexpr FVec4 kDescriptionBorderColor{ 0.45f, 0.55f, 0.70f, 0.85f };

	/** 説明文と画面端の間隔 (行の高さに対する倍率)。 */
	constexpr f32 kDescriptionMarginRatio = 0.8f;

	/** 説明文と下敷きの縁の間隔 (行の高さに対する倍率)。 */
	constexpr f32 kDescriptionPaddingRatio = 0.5f;

	/** 説明文の下敷きの縁の太さ (ピクセル)。 */
	constexpr f32 kDescriptionBorderWidth = 1.0f;

	/** 戻る操作を受け付けるゲームパッドのポート数。 */
	constexpr u32 kGamepadPlayerCount = 4;

	/** パンくずを辿る段数の上限 (親が輪になっていても止まるようにするため)。 */
	constexpr usize kMaxBreadcrumbDepth = 64;

	/** 一度に集める検索候補の上限 (多すぎると探すのが目的なのに探しにくくなる)。 */
	constexpr usize kMaxSearchHits = 8;

}


ADebugTopHUD::ADebugTopHUD()
{
	// どのページの行から頼まれても、この一覧が受ける。
	m_PathBrowser.MakeActive();

	// 検索欄は「何を探すか」を知らないので、集め方をここで差す。
	m_SearchBox.SetCollector( FDebugTopSearchCollector::CreateRaw<&ADebugTopHUD::CollectSearchHits>( this ) );
}

// TObjectPtr とフォントの解体をこの翻訳単位に閉じるためだけの定義 (中身は既定でよい)。
ADebugTopHUD::~ADebugTopHUD() noexcept
{
	// 解体後の自分へ頼みが来ないよう、受け皿から外す。
	m_PathBrowser.ClearActive();
}

ADebugTopEntity* ADebugTopHUD::AddEntity( const FString& Name )
{
	return AddEntity( NewObject<ADebugTopEntity>( Name ) );
}

ADebugTopEntity* ADebugTopHUD::AddEntity( TObjectPtr<ADebugTopEntity> Entity )
{
	if ( !Entity ) return nullptr;

	ADebugTopEntity* const Raw = Entity.Get();
	m_Entities.Add( Entity );

	if ( m_CurrentEntity == nullptr )
	{
		m_CurrentEntity = Raw;
	}

	// 木へ組み込まれた時点で中身を積ませる。
	Raw->Build();
	return Raw;
}

TObjectPtr<ADebugTopEntity> ADebugTopHUD::RemoveEntity( ADebugTopEntity* Entity ) noexcept
{
	if ( Entity == nullptr ) return TObjectPtr<ADebugTopEntity>();

	for ( usize Index = 0; Index < m_Entities.Num(); ++Index )
	{
		if ( m_Entities[Index].Get() != Entity ) continue;

		// 所有権を退避してから配列を縮める (ここで解放されないようにする)。
		TObjectPtr<ADebugTopEntity> Removed = m_Entities[Index];
		m_Entities.RemoveAt( Index );

		// 表示中のページが消える木の中にいたら、残っている先頭ページへ逃がす。
		for ( const ADebugTopEntity* Walk = m_CurrentEntity; Walk != nullptr; Walk = Walk->GetParent() )
		{
			if ( Walk != Entity ) continue;

			m_CurrentEntity = m_Entities.IsEmpty() ? nullptr : m_Entities[0].Get();
			break;
		}
		return Removed;
	}
	return TObjectPtr<ADebugTopEntity>();
}

void ADebugTopHUD::Build() noexcept
{
	if ( m_bBuilt ) return;

	// OnBuild の中から間接的に Build されても再入しないよう、呼ぶ前に立てる。
	m_bBuilt = true;
	OnBuild();
}

CDebugTopElement* ADebugTopHUD::GetCursorElement() const noexcept
{
	if ( m_CurrentEntity == nullptr ) return nullptr;
	return m_CurrentEntity->GetCursorElement();
}

void ADebugTopHUD::Update( f32 DeltaSeconds ) noexcept
{
	// フォントの焼き直しは手が止まってから行う。その待ち時間をここで進める。
	m_FontCache.Update( DeltaSeconds );

	// 吹き出しは前フレームの「指している行」で進める。ここで済ませておけば、この後どこで
	// 抜けても止まらない (1 フレーム遅れるが、出ているものは前フレームの絵なので見えない)。
	UpdateTooltip( DeltaSeconds );

	// 通知は画面の一番手前に出るので、先に見る。通知の上を押したぶんは後ろへ通さない
	// (閉じるボタンを押しただけで、裏のメニューの行まで実行されてしまわないように)。
	// HUD はワールドのノードではないので GetSubsystem を持たない。サブシステムが自分で
	// 登録している実体を引く (所有はあくまでサブシステムの側)。
	bool bToastConsumed = false;
	if ( CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive() )
	{
		bToastConsumed = Toasts->Update( DeltaSeconds );
	}

	// パスの一覧を開いている間は、そちらが入力を全て受け取る。
	if ( m_PathBrowser.Update( DeltaSeconds ) ) return;

	// ページを組み終え、保存してあった設定も戻し終えた状態を「元の値」として控える。
	if ( !m_bHistoryBegun )
	{
		m_History.Begin( *this );
		m_bHistoryBegun = true;
	}
	m_History.Update( DeltaSeconds );

	if ( m_CurrentEntity == nullptr || bToastConsumed ) return;

	// 打ち込み中の Ctrl+Z は文字の取り消しだと思われるので、そちらでは見ない。
	if ( !m_CurrentEntity->IsTyping() && !m_SearchBox.IsActive() && UpdateHistoryKeys() ) return;

	// 行へ割り当てたキーは、どのページを開いていても効く。値を打ち込んでいる間は文字が
	// 全て打ち込みへ入るので見ない (「r」がショートカットだと文字が打てなくなる)。
	if ( !m_CurrentEntity->IsTyping() && !m_SearchBox.IsActive() && DebugTopRunShortcuts( *this ) ) return;

	// ページ内の絞り込み。検索より先に見る (どちらも打った文字を食うので、順を決めておく)。
	if ( m_FilterBox.Update( DeltaSeconds, m_CurrentEntity->IsTyping() || m_SearchBox.IsActive() ) )
	{
		m_CurrentEntity->SetFilter( m_FilterBox.GetFilter() );
		return;
	}

	// 検索は HUD が持つので、どのページにいても同じ操作で開ける。打ち込み中は文字が
	// 全て検索語へ入るため、メニューの操作は止める。何を探すか・どこへ飛ぶかは
	// 検索欄ではなくこちらが決める (欄はメニューの構造を知らない)。
	if ( m_SearchBox.Update( DeltaSeconds, m_CurrentEntity->IsTyping() ) )
	{
		if ( const FDebugTopSearchHit* const Chosen = m_SearchBox.ConsumeChosen() ) JumpToHit( *Chosen );
		return;
	}

	m_CurrentEntity->Update( DeltaSeconds );

	if ( ADebugTopEntity* const Next = m_CurrentEntity->ConsumePendingTransition() )
	{
		// 合わせ先は遷移元が持っているので、切り替える前に取り出す。
		const CDebugTopElement* const Focus = m_CurrentEntity->ConsumePendingFocus();

		// 絞り込みはページごとの都合なので持ち越さない (移った先で何も出ないと迷う)。
		m_CurrentEntity->SetFilter( FString() );
		m_FilterBox.Clear();

		m_CurrentEntity = Next;

		// 一覧から選んだものが、飛んだ先でも選ばれた状態になるようにする。
		if ( Focus != nullptr ) m_CurrentEntity->FocusElement( *Focus );
		return;
	}

	// 値を打ち込んでいる間の Esc は打ち込みの取り消しなので、ページまで戻してしまわない。
	if ( m_CurrentEntity->IsTyping() ) return;

	// 検索を閉じた直後の Esc でページまで戻ってしまわないよう、ここでも見る。
	if ( m_SearchBox.IsActive() ) return;

	// 決定した同じフレームで戻ってしまわないよう、遷移が無かったときだけ復帰操作を見る。
	if ( WantsReturnToParent() )
	{
		if ( ADebugTopEntity* const Parent = m_CurrentEntity->GetParent() )
		{
			m_CurrentEntity = Parent;
		}
	}
}

bool ADebugTopHUD::UpdateHistoryKeys() noexcept
{
	const bool bCtrl = CInput::IsKeyDown( EKey::LeftCtrl ) || CInput::IsKeyDown( EKey::RightCtrl );
	if ( !bCtrl ) return false;

	const bool bShift = CInput::IsKeyDown( EKey::LeftShift ) || CInput::IsKeyDown( EKey::RightShift );

	// Ctrl+Y と Ctrl+Shift+Z のどちらでもやり直せるようにする (どちらの流儀で来ても済むように)。
	const bool bWantsRedo = CInput::IsKeyPressed( EKey::Y ) || ( bShift && CInput::IsKeyPressed( EKey::Z ) );
	const bool bWantsUndo = !bShift && CInput::IsKeyPressed( EKey::Z );

	if ( bWantsRedo )
	{
		FString Label;
		if ( m_History.Redo( Label ) ) DebugTopNotify( FString( "やり直しました" ), Label );
		else                           DebugTopNotify( FString( "やり直せる操作はありません" ) );
		return true;
	}

	if ( bWantsUndo )
	{
		FString Label;
		if ( m_History.Undo( Label ) ) DebugTopNotify( FString( "元に戻しました" ), Label );
		else                           DebugTopNotify( FString( "戻せる操作はありません" ) );
		return true;
	}

	return false;
}

void ADebugTopHUD::Draw( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept
{
	const CDebugTopText Text = m_FontCache.Resolve( RenderContext );
	if ( !Text.IsValid() ) return;

	const f32 OriginX = GetOriginX();
	const f32 OriginY = GetOriginY();
	f32 ListTop = OriginY + OnDrawTitle( RenderContext, Batch, Text, OriginX, OriginY );

	if ( m_CurrentEntity != nullptr )
	{
		// 絞り込み欄はパンくずの下。出ているぶんだけ一覧を押し下げる。
		const usize MatchCount = m_CurrentEntity->GetVisibleRowCount();
		ListTop += m_FilterBox.Draw( Batch, Text, OriginX, ListTop, MatchCount );

		m_CurrentEntity->Draw( RenderContext, Batch, Text, OriginX, ListTop );
	}

	// 説明文はページの行に重なることがあるので、一覧を描いた後に上から描く。
	// カーソル行 → ページ → メニュー共通、の順に、最初に見つかった説明文を出す。
	const FString* Description = &m_Description;
	FVec4 Color = kDescriptionColor;
	if ( m_CurrentEntity != nullptr )
	{
		if ( !m_CurrentEntity->GetDescription().IsEmpty() )
		{
			Description = &m_CurrentEntity->GetDescription();
		}

		const CDebugTopElement* const Cursor = m_CurrentEntity->GetCursorElement();
		if ( Cursor != nullptr && !Cursor->GetDescription().IsEmpty() )
		{
			Description = &Cursor->GetDescription();
		}

		const FDebugTopColor& EntityColor = m_CurrentEntity->GetDescriptionColor();
		if ( EntityColor.bSet ) Color = EntityColor.Color;
	}
	OnDrawDescription( RenderContext, Batch, Text, *Description, Color );

	// 吹き出しは説明文より手前。ポインタの脇に出るので、下の行に重なってよい。
	m_Tooltip.Draw( Batch, Text, static_cast<f32>( RenderContext.Width() ), static_cast<f32>( RenderContext.Height() ) );

	// パスの一覧は画面の中央に浮く。メニューより手前、通知より奥。
	m_PathBrowser.Draw( Batch, Text, static_cast<f32>( RenderContext.Width() ), static_cast<f32>( RenderContext.Height() ) );

	// 通知は一番手前。メニューにも説明文にも重なって出る。
	if ( CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive() )
	{
		Toasts->Draw( RenderContext, Batch, Text );
	}
}

f32 ADebugTopHUD::OnDrawTitle( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept
{
	const f32 LineHeight = Text.LineHeight();

	Text.Draw( Batch, kTitleText, OriginX, OriginY, kTitleColor );

	const FString Breadcrumb = BuildBreadcrumb();
	Text.Draw( Batch, Breadcrumb.Data(), OriginX, OriginY + LineHeight * 2.0f, kBreadcrumbColor );

	// 検索欄は見出しの右端へ寄せる。どのページにいても同じ場所にあるようにする。
	const f32 SearchX = static_cast<f32>( RenderContext.Width() ) - OriginX - m_SearchBox.MeasureWidth( Text );
	m_SearchBox.Draw( Batch, Text, SearchX, OriginY );

	// タイトル / 空行 / パンくず / 空行 の 4 行ぶん。
	return LineHeight * 4.0f;
}

void ADebugTopHUD::OnDrawDescription( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, const FString& Description, const FVec4& Color ) noexcept
{
	if ( Description.IsEmpty() ) return;

	const f32 LineHeight = Text.LineHeight();
	if ( LineHeight <= 0.0f ) return;

	const f32 TextWidth  = Text.MeasureWidth( Description.Data() );
	const f32 TextHeight = Text.MeasureHeight( Description.Data() );
	if ( TextWidth <= 0.0f || TextHeight <= 0.0f ) return;

	const f32 Margin  = LineHeight * kDescriptionMarginRatio;
	const f32 Padding = LineHeight * kDescriptionPaddingRatio;

	// 画面右下へ寄せる。文字は左揃えのまま、枠ごと右下に置く。
	const f32 PanelWidth  = TextWidth + Padding * 2.0f;
	const f32 PanelHeight = TextHeight + Padding * 2.0f;
	const f32 PanelX = static_cast<f32>( RenderContext.Width() ) - Margin - PanelWidth;

	// 通知も右下へ出る。積み上がっているぶんだけ上へ逃げて、重ならないようにする。
	f32 ToastHeight = 0.0f;
	if ( const CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive() ) ToastHeight = Toasts->GetOccupiedHeight();
	const f32 PanelY = static_cast<f32>( RenderContext.Height() ) - Margin - PanelHeight - ToastHeight;

	Batch.DrawRect( PanelX, PanelY, PanelWidth, PanelHeight, kDescriptionPanelColor );

	// 縁は 4 辺を細い矩形で描く (CSpriteBatch に枠線の描画が無いため)。
	const f32 Border = kDescriptionBorderWidth;
	Batch.DrawRect( PanelX, PanelY, PanelWidth, Border, kDescriptionBorderColor );
	Batch.DrawRect( PanelX, PanelY + PanelHeight - Border, PanelWidth, Border, kDescriptionBorderColor );
	Batch.DrawRect( PanelX, PanelY, Border, PanelHeight, kDescriptionBorderColor );
	Batch.DrawRect( PanelX + PanelWidth - Border, PanelY, Border, PanelHeight, kDescriptionBorderColor );

	Text.Draw( Batch, Description.Data(), PanelX + Padding, PanelY + Padding, Color );
}

void ADebugTopHUD::UpdateTooltip( f32 DeltaSeconds ) noexcept
{
	// 検索を開いている間は一覧を触っていないので出さない。
	const CDebugTopElement* Target = nullptr;
	if ( m_CurrentEntity != nullptr && !m_SearchBox.IsActive() )
	{
		// 右下のパネルはカーソル行の説明を出している。指した先がカーソル行と同じなら、
		// 吹き出しても同じ文が 2 つ並ぶだけなので出さない。
		const CDebugTopElement* const Hovered = m_CurrentEntity->GetHoverElement();
		if ( Hovered != m_CurrentEntity->GetCursorElement() ) Target = Hovered;
	}
	m_Tooltip.Update( Target, DeltaSeconds );
}

void ADebugTopHUD::CollectSearchHits( const FString& Query, TArray<FDebugTopSearchHit>& OutHits )
{
	DebugTopCollectMatches( *this, Query, kMaxSearchHits, OutHits );
}

void ADebugTopHUD::JumpToHit( const FDebugTopSearchHit& Hit ) noexcept
{
	if ( Hit.Page == nullptr ) return;

	m_CurrentEntity = Hit.Page;

	// 飛んだ先で目的の行にカーソルを合わせる。畳まれていて見えない行もあるので、
	// 見つからなければページを開くだけに留める。
	if ( Hit.Element != nullptr ) m_CurrentEntity->FocusElement( *Hit.Element );
}

FString ADebugTopHUD::BuildBreadcrumb() const
{
	FString Text( kTitleText );
	if ( m_CurrentEntity == nullptr ) return Text;

	// 親をたどると現在ページ→ルートの逆順で得られるので、いったん積んでから戻す。
	// 親が輪になっていると際限なく積んでしまうので段数で打ち切る (メニューの木がこれより
	// 深くなることは無い。輪はページの張り方を間違えたときにできる)。
	TArray<const ADebugTopEntity*> Path;
	for ( const ADebugTopEntity* Entity = m_CurrentEntity; Entity != nullptr && Path.Num() < kMaxBreadcrumbDepth; Entity = Entity->GetParent() )
	{
		Path.Add( Entity );
	}

	for ( usize i = Path.Num(); i > 0; --i )
	{
		Text.Append( kBreadcrumbSeparator );
		Text.Append( Path[i - 1]->GetName() );
	}
	return Text;
}

bool ADebugTopHUD::WantsReturnToParent() const noexcept
{
	if ( CInput::IsKeyPressed( EKey::Backspace ) ) return true;
	if ( CInput::IsKeyPressed( EKey::Escape ) ) return true;
	if ( CInput::IsMouseButtonPressed( EMouseButton::Right ) ) return true;

	// ゲームパッドは B ボタン (どのポートでも受ける)。
	for ( u32 Player = 0; Player < kGamepadPlayerCount; ++Player )
	{
		if ( CInput::IsGamepadButtonPressed( Player, EGamepadButton::B ) ) return true;
	}
	return false;
}

f32 ADebugTopHUD::GetOriginX() const noexcept
{
	return kOriginX;
}

f32 ADebugTopHUD::GetOriginY() const noexcept
{
	return kOriginY;
}

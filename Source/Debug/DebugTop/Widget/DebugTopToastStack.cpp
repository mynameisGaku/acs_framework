// SPDX-License-Identifier: Apache-2.0
#include "DebugTopToastStack.h"

namespace
{
	/** 通知の幅 (文字 1 行分の高さに対する倍率)。 */
	constexpr f32 kWidthRatio = 15.0f;

	/** 画面端との間隔 (行の高さに対する倍率)。 */
	constexpr f32 kScreenMarginRatio = 0.8f;

	/** 通知どうしの間隔 (行の高さに対する倍率)。 */
	constexpr f32 kGapRatio = 0.35f;

	/** 通知の内側の余白 (行の高さに対する倍率)。 */
	constexpr f32 kPaddingRatio = 0.5f;

	/** 左上のアイコンの一辺 (行の高さに対する倍率)。 */
	constexpr f32 kIconRatio = 0.9f;

	/** アイコンと見出しの間隔 (行の高さに対する倍率)。 */
	constexpr f32 kIconGapRatio = 0.4f;

	/** 閉じるボタンの一辺 (行の高さに対する倍率)。 */
	constexpr f32 kCloseRatio = 0.8f;

	/** ボタンの段の高さ (行の高さに対する倍率)。 */
	constexpr f32 kButtonHeightRatio = 1.15f;

	/** ボタンの左右の余白 (行の高さに対する倍率)。 */
	constexpr f32 kButtonPadRatio = 0.5f;

	/** ボタンどうしの間隔 (行の高さに対する倍率)。 */
	constexpr f32 kButtonGapRatio = 0.3f;

	/** 縁の太さ (ピクセル)。 */
	constexpr f32 kBorderWidth = 1.0f;

	/** 積み上がりが動く速さ (1 フレームで目標へ詰める割合)。 */
	constexpr f32 kSlideLerp = 0.23f;

	/** 背景の色。透かさない (裏の説明文が滲んで読みにくくなるため)。 */
	constexpr FVec4 kPanelColor{ 0.09f, 0.10f, 0.13f, 1.0f };

	/** 見出しの文字色。 */
	constexpr FVec4 kTitleColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	/** 本文の文字色。 */
	constexpr FVec4 kMessageColor{ 0.78f, 0.80f, 0.84f, 1.0f };

	/** ボタンの下敷きの色。 */
	constexpr FVec4 kButtonColor{ 0.22f, 0.28f, 0.38f, 1.0f };

	/** マウスを重ねているボタンの下敷きの色。 */
	constexpr FVec4 kButtonHoverColor{ 0.32f, 0.44f, 0.62f, 1.0f };

	/**
	 * 種類ごとの色を返す。
	 *
	 * @param Kind 通知の種類。
	 * @return 縁とアイコンに使う色。
	 */
	FVec4 KindColor( EDebugTopToastKind Kind ) noexcept
	{
		switch ( Kind )
		{
		case EDebugTopToastKind::Success: return FVec4{ 0.35f, 0.80f, 0.45f, 1.0f };
		case EDebugTopToastKind::Warning: return FVec4{ 0.95f, 0.75f, 0.25f, 1.0f };
		case EDebugTopToastKind::Error:   return FVec4{ 0.92f, 0.35f, 0.35f, 1.0f };
		default:                          return FVec4{ 0.35f, 0.62f, 0.95f, 1.0f };
		}
	}

	/**
	 * 本文の行数を数える。
	 *
	 * @param Message 本文 (空なら 0)。
	 * @return 行数。
	 */
	usize CountLines( const FString& Message ) noexcept
	{
		if ( Message.IsEmpty() ) return 0;

		usize Lines = 1;
		for ( usize Index = 0; Index < Message.Size(); ++Index )
		{
			if ( Message[Index] == '\n' ) ++Lines;
		}
		return Lines;
	}

	/**
	 * 幅に収まるところまで削った文字列を返す。
	 *
	 * @details
	 * 描画に切り抜きが無いので、長い文字列はそのままだと枠の外まで書き出してしまう。
	 * 収まらない場合は末尾を削って「...」を付ける。UTF-8 の途中で切らないよう、後続バイトを
	 * 跨いで 1 文字ずつ落とす。
	 * @param Text 幅を測るのに使うフォント。
	 * @param Source 元の文字列。
	 * @param MaxWidth 収めたい幅 (ピクセル)。
	 * @return 収まる文字列。
	 */
	FString FitText( const CDebugTopText& Text, const FString& Source, f32 MaxWidth )
	{
		if ( Source.IsEmpty() ) return Source;
		if ( Text.MeasureWidth( Source.Data() ) <= MaxWidth ) return Source;

		usize Size = Source.Size();
		while ( Size > 0 )
		{
			--Size;
			while ( Size > 0 && ( static_cast<u8>( Source[Size] ) & 0xC0u ) == 0x80u ) --Size;

			FString Candidate( FStringView( Source.Data(), Size ) );
			Candidate.Append( "..." );
			if ( Text.MeasureWidth( Candidate.Data() ) <= MaxWidth ) return Candidate;
		}
		return FString( "..." );
	}

	/**
	 * 本文から 1 行を切り出す。
	 *
	 * @param Message 本文。
	 * @param Begin 切り出しを始める位置 (次の行の先頭が書き戻される)。
	 * @return 切り出した 1 行。
	 */
	FString NextLine( const FString& Message, usize& Begin )
	{
		usize End = Begin;
		while ( End < Message.Size() && Message[End] != '\n' ) ++End;

		FString Line( FStringView( Message.Data() + Begin, End - Begin ) );
		Begin = ( End < Message.Size() ) ? End + 1 : Message.Size();
		return Line;
	}

	/**
	 * 色の濃さを掛ける。
	 *
	 * @param Color 元の色。
	 * @param Opacity 掛ける濃さ。
	 * @return 掛けた色。
	 */
	FVec4 Fade( const FVec4& Color, f32 Opacity ) noexcept
	{
		return FVec4{ Color.x, Color.y, Color.z, Color.w * Opacity };
	}

	/**
	 * 種類を表すアイコンを描く。
	 *
	 * @details
	 * 形そのもので種類が分かるようにする。お知らせと成功は丸みのある四角、注意と失敗は三角。
	 * 中の記号は文字ではなく図形で描く (フォントに無い字を避けるため)。
	 * @param Batch 描画コマンドを積む先。
	 * @param Kind 通知の種類。
	 * @param X 左端 X。
	 * @param Y 上端 Y。
	 * @param Size 一辺。
	 * @param Opacity 濃さ。
	 */
	void DrawIcon( CSpriteBatch& Batch, EDebugTopToastKind Kind, f32 X, f32 Y, f32 Size, f32 Opacity ) noexcept
	{
		const FVec4 Color = Fade( KindColor( Kind ), Opacity );
		const FVec4 Glyph = Fade( FVec4{ 0.06f, 0.07f, 0.09f, 1.0f }, Opacity );

		if ( Kind == EDebugTopToastKind::Warning || Kind == EDebugTopToastKind::Error )
		{
			// 注意・失敗は三角。危険の記号としてひと目で分かる。
			DebugTopDrawTriangle( Batch, EDebugTopTriangle::Up, X, Y, Size, Size, Color );

			// 中の「!」。棒と点を別々に置く。
			const f32 BarW = Size * 0.12f;
			Batch.DrawRect( X + Size * 0.5f - BarW * 0.5f, Y + Size * 0.34f, BarW, Size * 0.34f, Glyph );
			Batch.DrawRect( X + Size * 0.5f - BarW * 0.5f, Y + Size * 0.76f, BarW, BarW, Glyph );
			return;
		}

		// お知らせ・成功は四角 (角を落として丸く見せる)。
		const f32 Inset = Size * 0.12f;
		Batch.DrawRect( X + Inset, Y, Size - Inset * 2.0f, Size, Color );
		Batch.DrawRect( X, Y + Inset, Size, Size - Inset * 2.0f, Color );

		if ( Kind == EDebugTopToastKind::Success )
		{
			// レ点。太さを持たせた 2 本の線で描く。
			const f32 Thickness = Size * 0.13f;
			DebugTopDrawLine( Batch, X + Size * 0.26f, Y + Size * 0.52f, X + Size * 0.44f, Y + Size * 0.70f, Thickness, Glyph );
			DebugTopDrawLine( Batch, X + Size * 0.44f, Y + Size * 0.70f, X + Size * 0.74f, Y + Size * 0.30f, Thickness, Glyph );
			return;
		}

		// お知らせは「i」。点と棒を別々に置く。
		const f32 BarW = Size * 0.12f;
		Batch.DrawRect( X + Size * 0.5f - BarW * 0.5f, Y + Size * 0.22f, BarW, BarW, Glyph );
		Batch.DrawRect( X + Size * 0.5f - BarW * 0.5f, Y + Size * 0.42f, BarW, Size * 0.36f, Glyph );
	}

	/**
	 * 「×」を描く。
	 *
	 * @param Batch 描画コマンドを積む先。
	 * @param X 左端 X。
	 * @param Y 上端 Y。
	 * @param Size 一辺。
	 * @param Color 線の色。
	 */
	void DrawCross( CSpriteBatch& Batch, f32 X, f32 Y, f32 Size, const FVec4& Color ) noexcept
	{
		const f32 Inset = Size * 0.3f;
		const f32 Thickness = Size * 0.1f;
		DebugTopDrawLine( Batch, X + Inset, Y + Inset, X + Size - Inset, Y + Size - Inset, Thickness, Color );
		DebugTopDrawLine( Batch, X + Size - Inset, Y + Inset, X + Inset, Y + Size - Inset, Thickness, Color );
	}

	/**
	 * 枠を 4 辺の細い矩形で描く。
	 *
	 * @param Batch 描画コマンドを積む先。
	 * @param X 左端 X。
	 * @param Y 上端 Y。
	 * @param W 幅。
	 * @param H 高さ。
	 * @param Color 枠の色。
	 */
	void DrawFrame( CSpriteBatch& Batch, f32 X, f32 Y, f32 W, f32 H, const FVec4& Color ) noexcept
	{
		Batch.DrawRect( X, Y, W, kBorderWidth, Color );
		Batch.DrawRect( X, Y + H - kBorderWidth, W, kBorderWidth, Color );
		Batch.DrawRect( X, Y, kBorderWidth, H, Color );
		Batch.DrawRect( X + W - kBorderWidth, Y, kBorderWidth, H, Color );
	}

	/**
	 * 点が矩形の中にあるかを返す。
	 *
	 * @param Position 調べる点。
	 * @param X 矩形の左端。
	 * @param Y 矩形の上端。
	 * @param W 矩形の幅。
	 * @param H 矩形の高さ。
	 * @return 中にあれば true。
	 */
	bool Contains( FVec2 Position, f32 X, f32 Y, f32 W, f32 H ) noexcept
	{
		return Position.x >= X && Position.x <= X + W && Position.y >= Y && Position.y <= Y + H;
	}
}


CDebugTopToastSubsystem* CDebugTopToastSubsystem::s_Active = nullptr;

// GameInstance スコープへ登録する。シーンを切り替えても出した通知が消えないようにするため。
ACS_REGISTER_SUBSYSTEM( CDebugTopToastSubsystem, ESubsystemScope::GameInstance )


CDebugTopToastSubsystem::CDebugTopToastSubsystem()
{
	// 簡易関数 (DebugTopNotify) から引けるようにする。所有はサブシステムの側にある。
	s_Active = this;
}

CDebugTopToastSubsystem::~CDebugTopToastSubsystem()
{
	if ( s_Active == this ) s_Active = nullptr;
}

CDebugTopToast& CDebugTopToastSubsystem::Push( EDebugTopToastKind Kind, const FString& Title, const FString& Message )
{
	// 閉じ終わったものを先に片付けてから数える (画面から消えた枠で埋まらないように)。
	for ( usize Index = m_Toasts.Num(); Index > 0; --Index )
	{
		if ( m_Toasts[Index - 1] && !m_Toasts[Index - 1]->IsFinished() ) continue;

		m_Toasts.RemoveAt( Index - 1 );
	}

	// 溢れるぶんは古いものから閉じ始める (その場では消さず、薄くなって消えるのを見せる)。
	usize Alive = 0;
	for ( usize Index = 0; Index < m_Toasts.Num(); ++Index )
	{
		if ( m_Toasts[Index] ) ++Alive;
	}
	for ( usize Index = 0; Index < m_Toasts.Num() && Alive >= kDebugTopToastMax; ++Index )
	{
		if ( !m_Toasts[Index] ) continue;

		m_Toasts[Index]->Dismiss();
		--Alive;
	}

	TSharedPtr<CDebugTopToast> Toast = MakeShared<CDebugTopToast>( Kind, Title, Message );
	m_Toasts.Add( Toast );
	return *Toast;
}

void CDebugTopToastSubsystem::DismissAll() noexcept
{
	for ( usize Index = 0; Index < m_Toasts.Num(); ++Index )
	{
		if ( m_Toasts[Index] ) m_Toasts[Index]->Dismiss();
	}
}

bool CDebugTopToastSubsystem::Update( f32 DeltaSeconds )
{
	if ( m_Toasts.IsEmpty() ) return false;

	const FVec2 Mouse = CInput::MousePos();
	const bool bClicked = CInput::IsMouseButtonPressed( EMouseButton::Left );
	bool bConsumed = false;

	// 押されたボタンは、走らせる前に控えておく。走らせた先で通知を足されると
	// 配列が伸びて、この場で回している添字が壊れるため。
	FSimpleDelegate Pending;
	bool bHasPending = false;

	for ( usize Index = 0; Index < m_Toasts.Num(); ++Index )
	{
		if ( !m_Toasts[Index] ) continue;

		CDebugTopToast& Toast = *m_Toasts[Index];

		// 直前の描画が置いた矩形で当たりを見るので、通知は 1 フレーム遅れて反応する。
		const bool bHovered = Contains( Mouse, Toast.GetX(), Toast.GetY(), Toast.GetWidth(), Toast.GetHeight() );
		Toast.Update( DeltaSeconds, bHovered );

		if ( !bClicked || !bHovered ) continue;

		// 通知の上を押したぶんは、後ろのメニューへ通さない。
		bConsumed = true;

		if ( Contains( Mouse, Toast.GetCloseX(), Toast.GetCloseY(), Toast.GetCloseSize(), Toast.GetCloseSize() ) )
		{
			Toast.Dismiss();
			continue;
		}

		const TArray<FDebugTopToastButton>& Buttons = Toast.GetButtons();
		for ( usize ButtonIndex = 0; ButtonIndex < Buttons.Num(); ++ButtonIndex )
		{
			const FDebugTopToastButton& Button = Buttons[ButtonIndex];
			if ( Button.Width <= 0.0f ) continue;
			if ( !Contains( Mouse, Button.X, Toast.GetButtonY(), Button.Width, Toast.GetButtonHeight() ) ) continue;

			Pending = Button.OnPressed;
			bHasPending = true;
			Toast.Dismiss();
			break;
		}
	}

	// 閉じ終わったものを片付ける。
	for ( usize Index = m_Toasts.Num(); Index > 0; --Index )
	{
		if ( m_Toasts[Index - 1] && !m_Toasts[Index - 1]->IsFinished() ) continue;

		m_Toasts.RemoveAt( Index - 1 );
	}

	if ( bHasPending ) Pending.ExecuteIfBound();

	return bConsumed;
}

void CDebugTopToastSubsystem::Draw( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text )
{
	// 何も出していない間は場所も取らない。控えを残すと、説明文が上へ逃げたまま戻らない。
	m_OccupiedHeight = 0.0f;

	if ( m_Toasts.IsEmpty() || !Text.IsValid() ) return;

	const f32 LineHeight = Text.LineHeight();
	if ( LineHeight <= 0.0f ) return;

	const f32 Width = LineHeight * kWidthRatio;
	const f32 Margin = LineHeight * kScreenMarginRatio;
	const f32 Gap = LineHeight * kGapRatio;
	const f32 Padding = LineHeight * kPaddingRatio;
	const f32 IconSize = LineHeight * kIconRatio;
	const f32 CloseSize = LineHeight * kCloseRatio;
	const f32 ScreenW = static_cast<f32>( RenderContext.Width() );
	const f32 ScreenH = static_cast<f32>( RenderContext.Height() );

	// 一番新しいものが一番下。古いものほど上へ積み上がる。
	const f32 StackBottom = ScreenH - Margin;
	f32 Bottom = StackBottom;
	for ( usize Index = m_Toasts.Num(); Index > 0; --Index )
	{
		if ( !m_Toasts[Index - 1] ) continue;

		CDebugTopToast& Toast = *m_Toasts[Index - 1];

		const usize MessageLines = CountLines( Toast.GetMessage() );
		const bool bHasButtons = !Toast.GetButtons().IsEmpty();
		const f32 ButtonHeight = LineHeight * kButtonHeightRatio;
		const f32 Height = Padding * 2.0f
		                 + LineHeight
		                 + static_cast<f32>( MessageLines ) * LineHeight
		                 + ( bHasButtons ? ButtonHeight + Gap : 0.0f );

		// 積み上がりを滑らかに動かす。出た直後は所定の位置から始める (下から湧いて見えないように)。
		if ( !Toast.HasPlacement() )
		{
			Toast.SetAnimatedBottom( Bottom );
			Toast.MarkPlaced();
		}
		else
		{
			// 上へずれるのを一気にやらず、少しずつ詰める。フレーム時間で割らないのは、
			// 見た目の滑らかさだけの話で、正確な速度が要らないため。
			const f32 Current = Toast.GetAnimatedBottom();
			Toast.SetAnimatedBottom( Current + ( Bottom - Current ) * kSlideLerp );
		}

		const f32 Opacity = Toast.GetOpacity();
		const f32 Slide = Toast.GetSlideRatio();

		// 右の外から滑り込ませる。出きったところで画面端との間隔に収まる。
		const f32 X = ScreenW - Margin - Width + ( 1.0f - Slide ) * ( Width + Margin );
		const f32 Y = Toast.GetAnimatedBottom() - Height;
		Toast.SetRect( X, Y, Width, Height );

		const FVec4 Accent = KindColor( Toast.GetKind() );
		Batch.DrawRect( X, Y, Width, Height, Fade( kPanelColor, Opacity ) );
		DrawFrame( Batch, X, Y, Width, Height, Fade( Accent, Opacity * 0.85f ) );

		// 種類が一目で分かるよう、左端に色の帯も入れる。
		Batch.DrawRect( X, Y, LineHeight * 0.16f, Height, Fade( Accent, Opacity ) );

		DrawIcon( Batch, Toast.GetKind(), X + Padding, Y + Padding, IconSize, Opacity );

		const f32 TextX = X + Padding + IconSize + LineHeight * kIconGapRatio;

		// 見出しは閉じるボタンの手前まで、本文は枠の内側までに収める。
		const f32 TitleWidth = ( X + Width - Padding - CloseSize ) - TextX - Padding;
		const f32 MessageWidth = ( X + Width - Padding ) - TextX;

		Text.Draw( Batch, FitText( Text, Toast.GetTitle(), TitleWidth ).Data(), TextX, Y + Padding, Fade( kTitleColor, Opacity ) );

		// 行ごとに収めてから描く。長いパスをそのまま出すと枠の外まで書き出してしまう。
		usize LineBegin = 0;
		for ( usize Line = 0; Line < MessageLines; ++Line )
		{
			const FString Fitted = FitText( Text, NextLine( Toast.GetMessage(), LineBegin ), MessageWidth );
			Text.Draw( Batch, Fitted.Data(), TextX, Y + Padding + LineHeight * static_cast<f32>( Line + 1 ), Fade( kMessageColor, Opacity ) );
		}

		// 閉じるボタン。押せることが分かるよう枠を付ける。
		const f32 CloseX = X + Width - Padding - CloseSize;
		const f32 CloseY = Y + Padding;
		Toast.SetCloseRect( CloseX, CloseY, CloseSize );

		const bool bCloseHovered = Contains( CInput::MousePos(), CloseX, CloseY, CloseSize, CloseSize );
		if ( bCloseHovered )
		{
			Batch.DrawRect( CloseX, CloseY, CloseSize, CloseSize, Fade( kButtonHoverColor, Opacity ) );
		}
		DrawCross( Batch, CloseX, CloseY, CloseSize, Fade( kTitleColor, Opacity ) );

		if ( bHasButtons )
		{
			const f32 ButtonY = Y + Height - Padding - ButtonHeight;
			Toast.SetButtonRow( ButtonY, ButtonHeight );

			f32 ButtonX = TextX;
			TArray<FDebugTopToastButton>& Buttons = Toast.GetButtons();
			for ( usize ButtonIndex = 0; ButtonIndex < Buttons.Num(); ++ButtonIndex )
			{
				FDebugTopToastButton& Button = Buttons[ButtonIndex];

				// 枠から出てしまうぶんは文字を削る。押せる範囲と見た目を一致させたままにする。
				const f32 Available = ( X + Width - Padding ) - ButtonX - LineHeight * kButtonPadRatio * 2.0f;
				if ( Available <= 0.0f )
				{
					Button.Width = 0.0f;   // 置けないボタンは当たり判定からも外す
					continue;
				}

				const FString Label = FitText( Text, Button.Label, Available );
				const f32 ButtonW = Text.MeasureWidth( Label.Data() ) + LineHeight * kButtonPadRatio * 2.0f;

				// 次の描画の当たり判定で同じ矩形を使う。
				Button.X = ButtonX;
				Button.Width = ButtonW;

				const bool bHovered = Contains( CInput::MousePos(), ButtonX, ButtonY, ButtonW, ButtonHeight );
				Batch.DrawRect( ButtonX, ButtonY, ButtonW, ButtonHeight, Fade( bHovered ? kButtonHoverColor : kButtonColor, Opacity ) );
				DrawFrame( Batch, ButtonX, ButtonY, ButtonW, ButtonHeight, Fade( Accent, Opacity * 0.7f ) );
				Text.Draw( Batch, Label.Data(), ButtonX + LineHeight * kButtonPadRatio, ButtonY + ( ButtonHeight - LineHeight ) * 0.5f, Fade( kTitleColor, Opacity ) );

				ButtonX += ButtonW + LineHeight * kButtonGapRatio;
			}
		}

		Bottom = Y - Gap;
	}

	// 下から積み上げた高さを控える。右下へ出す他のもの (説明文) が、これを避けて上へ逃げる。
	m_OccupiedHeight = StackBottom - Bottom;
}


CDebugTopToast& DebugTopToastSink()
{
	// どこにも出ない捨て場。サブシステムが無い場面でも呼び方を変えずに済むようにする。
	static CDebugTopToast Sink( EDebugTopToastKind::Info, FString(), FString() );

	// 続けて AddButton を書かれても溜め込まないよう、渡す前に空にする。
	Sink.GetButtons().Reset();
	return Sink;
}

CDebugTopToast& DebugTopNotify( const FString& Title, const FString& Message )
{
	CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive();
	if ( Toasts == nullptr ) return DebugTopToastSink();

	return Toasts->Push( EDebugTopToastKind::Info, Title, Message );
}

CDebugTopToast& DebugTopNotifySuccess( const FString& Title, const FString& Message )
{
	CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive();
	if ( Toasts == nullptr ) return DebugTopToastSink();

	return Toasts->Push( EDebugTopToastKind::Success, Title, Message );
}

CDebugTopToast& DebugTopNotifyWarning( const FString& Title, const FString& Message )
{
	CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive();
	if ( Toasts == nullptr ) return DebugTopToastSink();

	return Toasts->Push( EDebugTopToastKind::Warning, Title, Message );
}

CDebugTopToast& DebugTopNotifyError( const FString& Title, const FString& Message )
{
	CDebugTopToastSubsystem* const Toasts = CDebugTopToastSubsystem::GetActive();
	if ( Toasts == nullptr ) return DebugTopToastSink();

	return Toasts->Push( EDebugTopToastKind::Error, Title, Message );
}

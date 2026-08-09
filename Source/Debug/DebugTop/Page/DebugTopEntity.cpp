#include "DebugTopEntity.h"

#include "Debug/DebugTop/Service/DebugTopSearchIndex.h"

namespace
{
	// 幅の類は全て「行の高さ (= 文字サイズ)」に対する倍率で持つ。こうしておくと
	// ADebugTopHUD::SetFontSize で文字を大きくしても、段差と桁位置がそのまま追従する。

	/** 深さ 1 段あたりのインデント幅 (行の高さに対する倍率)。 */
	constexpr f32 kIndentRatio = 1.0f;

	/** マーカー 1 個分として空ける幅 (行の高さに対する倍率)。子を持たない行はここが空になる。 */
	constexpr f32 kMarkerSlotRatio = 1.0f;

	/** 行の左端から右カラムまでの距離 (行の高さに対する倍率)。 */
	constexpr f32 kValueColumnRatio = 12.0f;

	/** 展開マーカーの一辺 (行の高さに対する倍率)。 */
	constexpr f32 kMarkerSizeRatio = 0.38f;

	/** 開閉できない行のマーカー (横棒) の太さ (行の高さに対する倍率)。 */
	constexpr f32 kLockedMarkerThicknessRatio = 0.09f;

	/** 開閉できない行のマーカーの濃さ (三角マーカーに対する不透明度の倍率)。 */
	constexpr f32 kLockedMarkerAlphaScale = 0.5f;

	/** 左右キーで値を変えられる行に出す矢印の一辺 (行の高さに対する倍率)。 */
	constexpr f32 kArrowSizeRatio = 0.38f;

	/** チェックボックスの一辺 (行の高さに対する倍率)。 */
	constexpr f32 kCheckBoxSizeRatio = 0.55f;

	/** 色見本の幅 (行の高さに対する倍率)。 */
	constexpr f32 kColorSwatchWidthRatio = 1.6f;

	/** スライダーの溝の幅 (行の高さに対する倍率)。 */
	constexpr f32 kSliderWidthRatio = 5.0f;

	/** スライダーの溝の高さ (行の高さに対する倍率)。 */
	constexpr f32 kSliderHeightRatio = 0.12f;

	/** 既定値から変わっている行の左に出す帯の幅 (行の高さに対する倍率)。 */
	constexpr f32 kModifiedMarkWidthRatio = 0.12f;

	/** 既定値から変わっている行の印の色。 */
	constexpr FVec4 kModifiedMarkColor{ 0.95f, 0.75f, 0.35f, 0.95f };

	/** 矢印と値の文字の間隔 (行の高さに対する倍率)。 */
	constexpr f32 kArrowGapRatio = 0.35f;

	/** ホイール 1 ノッチで動かす行数。 */
	constexpr i32 kWheelScrollRows = 3;

	/** カーソルの上下に残しておきたい行数 (この手前でスクロールを始める)。 */
	constexpr i32 kScrollMarginRows = 2;

	/** 続きがあることを示す三角の一辺 (行の高さに対する倍率)。 */
	constexpr f32 kMoreMarkerSizeRatio = 0.45f;

	/** 続きがあることを示す三角の色。 */
	constexpr FVec4 kMoreMarkerColor{ 0.70f, 0.80f, 0.95f, 0.90f };

	/** 続きがある側の端 1 行を薄くする濃さ (0..1)。 */
	constexpr f32 kEdgeFadeOpacity = 0.35f;

	/** カーソル行の下敷き。 */
	constexpr FVec4 kHighlightColor{ 0.20f, 0.35f, 0.60f, 0.85f };

	/** 打ち込みを始めた直後の全選択を示す下敷き。 */
	constexpr FVec4 kSelectionColor{ 0.25f, 0.50f, 0.85f, 0.75f };

	/** 入力欄の幅 (行の高さに対する倍率)。 */
	constexpr f32 kEditFieldWidthRatio = 6.5f;

	/** 入力欄の内側の余白 (行の高さに対する倍率)。 */
	constexpr f32 kEditFieldPadRatio = 0.25f;

	/** 入力欄の内側の色 (沈めて「ここへ入る」ことを示す)。 */
	constexpr FVec4 kEditFieldColor{ 0.04f, 0.05f, 0.07f, 1.0f };

	/** 打ち込んでいる入力欄の縁の色。 */
	constexpr FVec4 kEditBorderColor{ 0.45f, 0.70f, 0.98f, 1.0f };

	/** 打ち込んでいない入力欄の縁の色 (控えめに出して、欄であることだけ示す)。 */
	constexpr FVec4 kEditIdleBorderColor{ 0.30f, 0.34f, 0.42f, 1.0f };

	/** マウスを重ねている行の下敷き (押したらここが選ばれる、を示す)。 */
	constexpr FVec4 kHoverColor{ 0.30f, 0.45f, 0.70f, 0.35f };

	/** カーソル行の文字色。 */
	constexpr FVec4 kSelectedTextColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	/** 通常行の文字色。 */
	constexpr FVec4 kNormalTextColor{ 0.78f, 0.78f, 0.78f, 1.0f };

	/** 注意すべき範囲へ入った値の色 (SetWarnRange を設定した行だけ)。 */
	constexpr FVec4 kWarnColor{ 0.98f, 0.62f, 0.30f, 1.0f };

	/** ページ見出しの既定色。 */
	constexpr FVec4 kHeaderColor{ 0.95f, 0.90f, 0.55f, 1.0f };

	/** 折れ線の描画域の幅 (行の高さに対する倍率)。 */
	constexpr f32 kGraphWidthRatio = 10.0f;

	/** 折れ線の太さ (ピクセル)。 */
	constexpr f32 kGraphLineThickness = 1.5f;

	/** 折れ線の下敷きの濃さ (線に対する倍率)。 */
	constexpr f32 kGraphFrameAlphaScale = 0.12f;

	/** ピン留めの星ボタンへ空ける幅 (行の高さに対する倍率)。 */
	constexpr f32 kStarSlotRatio = 1.0f;

	/** ピン留めの星ボタンの一辺 (行の高さに対する倍率)。 */
	constexpr f32 kStarSizeRatio = 0.46f;

	/** 留めている星の色。 */
	constexpr FVec4 kPinColor{ 0.98f, 0.85f, 0.35f, 1.0f };

	/** 留めていない星の濃さ (留めている色に対する倍率)。押せることは分かる程度に薄く出す。 */
	constexpr f32 kStarIdleAlpha = 0.22f;

	/** マウスを重ねている星の濃さ (留めていないときの倍率)。 */
	constexpr f32 kStarHoverAlpha = 0.65f;

	/**
	 * 溜めた標本を折れ線で描く。
	 *
	 * @param Batch 描画コマンドを積む先。
	 * @param RowDraw この行の配置。
	 * @param Samples 標本の先頭 (古い順)。
	 * @param Count 標本の数。
	 * @param Min 縦軸の下端。
	 * @param Max 縦軸の上端。
	 * @param Color 線の色。
	 */
	void DrawGraph( CSpriteBatch& Batch, const ADebugTopEntity::FRowDraw& RowDraw, const f32* Samples, usize Count, f32 Min, f32 Max, const FVec4& Color ) noexcept
	{
		// 折れ線は文字の下の段、値の列に揃えて置く。ラベルの真下だと、下の行のラベルと
		// 縦に並んでしまって境目が分かりにくい。
		const f32 PlotX = RowDraw.ValueX;
		const f32 PlotY = RowDraw.Y + RowDraw.BaseHeight;
		const f32 PlotW = RowDraw.BaseHeight * kGraphWidthRatio;
		const f32 PlotH = RowDraw.Height - RowDraw.BaseHeight;
		if ( PlotW <= 0.0f || PlotH <= 0.0f ) return;

		FVec4 Frame = Color;
		Frame.w *= RowDraw.Opacity * kGraphFrameAlphaScale;
		Batch.DrawRect( PlotX, PlotY, PlotW, PlotH, Frame );

		if ( Samples == nullptr || Count < 1 ) return;
		if ( !( Max > Min ) ) return;

		/** 標本 1 つを画面座標へ移す。 */
		const auto ToScreen = [&]( usize Index, f32& OutX, f32& OutY ) noexcept
		{
			// 標本が 1 つだけのときは横位置を決められないので左端へ置く。
			const f32 Ratio = Count > 1 ? static_cast<f32>( Index ) / static_cast<f32>( Count - 1 ) : 0.0f;
			f32 Normalized = ( Samples[Index] - Min ) / ( Max - Min );
			if ( Normalized < 0.0f ) Normalized = 0.0f;
			if ( Normalized > 1.0f ) Normalized = 1.0f;

			OutX = PlotX + PlotW * Ratio;
			OutY = PlotY + PlotH * ( 1.0f - Normalized ); // 値が大きいほど上
		};

		FVec4 Line = Color;
		Line.w *= RowDraw.Opacity;

		f32 PrevX = 0.0f;
		f32 PrevY = 0.0f;
		ToScreen( 0, PrevX, PrevY );
		for ( usize Index = 1; Index < Count; ++Index )
		{
			f32 X = 0.0f;
			f32 Y = 0.0f;
			ToScreen( Index, X, Y );
			DebugTopDrawLine( Batch, PrevX, PrevY, X, Y, kGraphLineThickness, Line );
			PrevX = X;
			PrevY = Y;
		}
	}

	/**
	 * 指定 Entity を指す行を、Owner の子行の木から探して取り外す。
	 *
	 * @details
	 * Entity を指す行 (遷移行・インライン展開行) には降りない。インライン展開行の子行は
	 * 別の Entity が所有していて、Owner.RemoveChild では外せないため。
	 * @param Owner 探索の起点となる行。
	 * @param Target 取り外す対象の Entity。
	 * @return 取り外せたら true。
	 */
	bool RemoveLinkedRowFrom( CDebugTopElement& Owner, const ADebugTopEntity* Target )
	{
		const TArray<TSharedPtr<CDebugTopElement>>& Children = Owner.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			CDebugTopElement* const Child = Children[Index].Get();
			if ( Child == nullptr ) continue;

			ADebugTopEntity* const Linked = Child->GetLinkedEntity();
			if ( Linked == Target ) return Owner.RemoveChild( Child );
			if ( Linked != nullptr ) continue;

			if ( RemoveLinkedRowFrom( *Child, Target ) ) return true;
		}
		return false;
	}
}


CDebugTopElementEntityLink::CDebugTopElementEntityLink( const FString& Label, const FString& SubTitle, ADebugTopEntity& Owner, ADebugTopEntity& Target, const CDebugTopElement* Focus )
	: CDebugTopElement( Label, SubTitle )
	, m_Owner( &Owner )
	, m_Target( &Target )
	, m_Focus( Focus )
{
	// ここで戻り先を決めてはいけない。検索やお気に入りの結果行はルートページを指すので、
	// 自動で親にすると「ルートの親 = 結果ページ / 結果ページの親 = ルート」で輪ができる。
	// 戻り先が要る遷移は、張る側が SetParentIfUnset を明示的に呼ぶ。
}

void CDebugTopElementEntityLink::OnDecide()
{
	m_Owner->RequestTransition( m_Target, m_Focus );
}


CDebugTopElementEntityGroup::CDebugTopElementEntityGroup( const FString& Label, const FString& SubTitle, ADebugTopEntity& Target )
	: CDebugTopElement( Label, SubTitle )
	, m_Target( &Target )
{
}

const TArray<TSharedPtr<CDebugTopElement>>& CDebugTopElementEntityGroup::GetChildren() const noexcept
{
	return m_Target->GetElements();
}


ADebugTopEntity::ADebugTopEntity( const FString& Name )
	: m_Name( Name )
{
	// 振り分け先はページと同じだけ生きるので、ここで 1 度だけ繋いでおく。
	m_Mouse.Bind( m_VisibleRows, m_Scroller, m_ColorPicker, m_ValueEditor );
}

ADebugTopEntity::~ADebugTopEntity() noexcept = default;

void ADebugTopEntity::SetHeaderColor( const FVec4& Color ) noexcept
{
	m_HeaderColor.Color = Color;
	m_HeaderColor.bSet = true;
}

void ADebugTopEntity::SetDescriptionColor( const FVec4& Color ) noexcept
{
	m_DescriptionColor.Color = Color;
	m_DescriptionColor.bSet = true;
}

TSharedPtr<CDebugTopElement> ADebugTopEntity::AddElement( TSharedPtr<CDebugTopElement> Element )
{
	if ( Element )
	{
		m_Elements.Add( Element );
	}
	return Element;
}

bool ADebugTopEntity::RemoveElement( const CDebugTopElement* Element ) noexcept
{
	if ( Element == nullptr ) return false;

	for ( usize Index = 0; Index < m_Elements.Num(); ++Index )
	{
		if ( m_Elements[Index].Get() != Element ) continue;

		m_Elements.RemoveAt( Index );

		// 可視行が消えた行を指したままにならないよう、その場で組み直す。
		RebuildVisibleRows();
		return true;
	}
	return false;
}

void ADebugTopEntity::ClearElements() noexcept
{
	m_Elements.Reset();
	RebuildVisibleRows();
}

namespace
{
	/**
	 * 行とその子孫を既定値へ戻す。
	 *
	 * @param Element 対象の行。
	 */
	void ResetElementTree( CDebugTopElement& Element ) noexcept
	{
		// Entity を指す行の子は別ページの持ち物なので降りない。
		if ( Element.GetLinkedEntity() != nullptr ) return;

		Element.ResetToDefault();

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;

			ResetElementTree( *Children[Index] );
		}
	}
}

void ADebugTopEntity::ResetToDefaults() noexcept
{
	for ( usize Index = 0; Index < m_Elements.Num(); ++Index )
	{
		if ( !m_Elements[Index] ) continue;

		ResetElementTree( *m_Elements[Index] );
	}
}

ADebugTopEntity* ADebugTopEntity::AddChildEntity( const FString& Name, const FString& SubTitle, EDebugTopAttachMode Mode )
{
	return AddChildEntity( NewObject<ADebugTopEntity>( Name ), SubTitle, Mode );
}

ADebugTopEntity* ADebugTopEntity::AddChildEntity( TObjectPtr<ADebugTopEntity> Child, const FString& SubTitle, EDebugTopAttachMode Mode, CDebugTopElement* ParentElement )
{
	if ( !Child ) return nullptr;

	ADebugTopEntity* const Raw = Child.Get();
	Raw->m_Parent = this;
	m_ChildEntities.Add( Child );

	TSharedPtr<CDebugTopElement> Row;
	if ( Mode == EDebugTopAttachMode::Inline )
	{
		Row = MakeShared<CDebugTopElementEntityGroup>( Raw->GetName(), SubTitle, *Raw );
	}
	else
	{
		Row = MakeShared<CDebugTopElementEntityLink>( Raw->GetName(), SubTitle, *this, *Raw );
	}

	if ( ParentElement != nullptr )
	{
		ParentElement->AddChild( Row );
	}
	else
	{
		AddElement( Row );
	}

	// 木へ組み込まれた時点で中身を積ませる (親の OnBuild から呼ばれると深さ優先で降りていく)。
	Raw->Build();
	return Raw;
}

TObjectPtr<ADebugTopEntity> ADebugTopEntity::RemoveChildEntity( ADebugTopEntity* Child ) noexcept
{
	if ( Child == nullptr ) return TObjectPtr<ADebugTopEntity>();

	for ( usize Index = 0; Index < m_ChildEntities.Num(); ++Index )
	{
		if ( m_ChildEntities[Index].Get() != Child ) continue;

		// 所有権を退避してから配列を縮める (ここで解放されないようにする)。
		TObjectPtr<ADebugTopEntity> Removed = m_ChildEntities[Index];
		m_ChildEntities.RemoveAt( Index );

		// 対応する遷移行 / インライン展開行も落とす (カテゴリ行の下に置いた場合も探す)。
		bool bRowRemoved = false;
		for ( usize ElementIndex = 0; ElementIndex < m_Elements.Num(); ++ElementIndex )
		{
			if ( m_Elements[ElementIndex]->GetLinkedEntity() != Child ) continue;

			m_Elements.RemoveAt( ElementIndex );
			bRowRemoved = true;
			break;
		}
		for ( usize ElementIndex = 0; !bRowRemoved && ElementIndex < m_Elements.Num(); ++ElementIndex )
		{
			bRowRemoved = RemoveLinkedRowFrom( *m_Elements[ElementIndex], Child );
		}

		Removed->m_Parent = nullptr;
		RebuildVisibleRows();
		return Removed;
	}
	return TObjectPtr<ADebugTopEntity>();
}

void ADebugTopEntity::Build() noexcept
{
	if ( m_bBuilt ) return;

	// OnBuild の中から間接的に Build されても再入しないよう、呼ぶ前に立てる。
	m_bBuilt = true;
	OnBuild();
}

void ADebugTopEntity::Update( f32 DeltaSeconds ) noexcept
{
	RebuildVisibleRows();
	if ( m_VisibleRows.IsEmpty() ) return;

	// 値を追い続ける行 (グラフ) に標本を溜めさせる。畳まれている行は可視行に入らないので
	// 呼ばれない = 見ていない間は溜めない。
	for ( usize Index = 0; Index < m_VisibleRows.Num(); ++Index )
	{
		if ( m_VisibleRows[Index].Element == nullptr ) continue;

		m_VisibleRows[Index].Element->OnTick( DeltaSeconds );
	}

	// 打ち込み中は文字が全てバッファへ入るので、他の割り当ては止める
	// (数字キーでカーソルが動いたり、Enter で別の行が実行されたりしないように)。
	if ( m_ValueEditor.Update( DeltaSeconds, GetCursorElement() ) ) return;

	// 矢印キーは押しっぱなしで連射する (行が数十ある一覧を叩き続けずに辿れるように)。
	// テンキー側でも同じように効く。
	m_KeyNav.Update( DeltaSeconds );
	const i32 KeyVertical = m_KeyNav.GetVertical();
	if ( KeyVertical != 0 ) m_Scroller.MoveCursor( m_VisibleRows, KeyVertical );

	// カーソル移動後の行に対して操作を流す。
	CDebugTopElement* const Current = GetCursorElement();
	if ( Current != nullptr )
	{
		// F でピン留めの付け外し。留めた行はお気に入りのページへ集まる。
		if ( CInput::IsKeyPressed( EKey::F ) ) Current->SetFavorite( !Current->IsFavorite() );

		// 値も押しっぱなしで送り続ける (矢印をクリックし続けたときと同じ挙動)。
		const i32 KeyHorizontal = m_KeyNav.GetHorizontal();
		if ( KeyHorizontal != 0 ) Current->OnLeftRight( KeyHorizontal );
		if ( CInput::IsKeyPressed( EKey::Enter ) )
		{
			// 打ち込める行は決定で打ち込みを始める (左右キーで送るには遠い値のため)。
			// ただし決定に別の意味を持たせている行 (パスの行) はそちらを優先する。
			if ( Current->CanTypeValue() && !Current->PrefersDecide() ) m_ValueEditor.Begin( *Current );
			else Current->OnDecide();
		}
	}

	// ゲームパッドはキーボードと同じ扱い (どちらで触っても同じように動く)。
	UpdateGamepad( DeltaSeconds );

	// マウスはキーボードの後に見る (同じフレームで両方来ても、最後に触った方を優先する)。
	m_Mouse.Update( DeltaSeconds );
}

void ADebugTopEntity::UpdateGamepad( f32 DeltaSeconds ) noexcept
{
	m_Gamepad.Update( DeltaSeconds );
	if ( !m_Gamepad.IsConnected() ) return;

	const i32 Vertical = m_Gamepad.GetVertical();
	if ( Vertical != 0 ) m_Scroller.MoveCursor( m_VisibleRows, Vertical );

	if ( m_VisibleRows.IsEmpty() ) return;

	CDebugTopElement* const Current = GetCursorElement();
	if ( Current == nullptr ) return;

	const i32 Horizontal = m_Gamepad.GetHorizontal();
	if ( Horizontal != 0 ) Current->OnLeftRight( Horizontal );

	if ( m_Gamepad.IsDecidePressed() ) Current->OnDecide();
}

void ADebugTopEntity::Draw( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept
{
	RebuildVisibleRows();

	// 描き直すまでの間にマウス判定が古い矩形を拾わないよう、先に捨てる。
	m_Mouse.BeginFrame( 0 );
	m_Scroller.RememberViewport( 0.0f, 0.0f, 0 );
	if ( !Text.IsValid() ) return;

	const f32 RowHeight = Text.LineHeight();
	if ( RowHeight <= 0.0f ) return;

	const f32 ListTop = OriginY + OnDrawHeader( RenderContext, Batch, Text, OriginX, OriginY );
	if ( m_VisibleRows.IsEmpty() ) return;

	// 画面下端まで何行入るかはここでしか分からないので、スクロール追従も併せて行う。
	// 一番下には「まだ続きがある」印を出す余地を残しておく。
	const f32 MoreMarkerSize = RowHeight * kMoreMarkerSizeRatio;
	const f32 ListBottom = static_cast<f32>( RenderContext.Height() ) - MoreMarkerSize;
	// 収まる行数は高さを積んで数える (行ごとに高さが違うため、割り算では出ない)。
	const f32 Available = ListBottom - ListTop;
	m_Scroller.ScrollToCursor( m_VisibleRows, Available, RowHeight );

	const i32 VisibleRowCount = m_Scroller.CountRowsThatFit( m_VisibleRows, m_Scroller.GetScrollTop(), Available, RowHeight );

	// ホイールは描画の外で来るので、数え直せるよう寸法を控えておく。
	m_Scroller.RememberViewport( Available, RowHeight, VisibleRowCount );

	const f32 RowWidth = static_cast<f32>( RenderContext.Width() ) - OriginX * 2.0f;
	const usize RowCount = m_VisibleRows.Num();
	usize RowIndex = static_cast<usize>( m_Scroller.GetScrollTop() );
	f32 Y = ListTop;

	// 続きがある側は端の 1 行だけ薄く描いて、まだ先があることを見た目でも示す。
	const bool bMoreAbove = m_Scroller.GetScrollTop() > 0;
	const bool bMoreBelow = static_cast<usize>( m_Scroller.GetScrollTop() ) + static_cast<usize>( VisibleRowCount ) < RowCount;

	m_Mouse.BeginFrame( static_cast<usize>( VisibleRowCount ) );
	for ( i32 Drawn = 0; Drawn < VisibleRowCount && RowIndex < RowCount; ++Drawn, ++RowIndex )
	{
		const FDebugTopVisibleRow& Row = m_VisibleRows[RowIndex];

		// 背の高い行 (色を選ぶ面など) があるので、行ごとに高さを引く。
		const f32 ThisRowHeight = Row.Element != nullptr
			? RowHeight * Row.Element->GetHeightRatio()
			: RowHeight;

		if ( Row.Element != nullptr )
		{
			FRowDraw RowDraw = BuildRowDraw( Text, Row, OriginX, Y, RowWidth, ThisRowHeight, RowHeight );
			if ( ( bMoreAbove && Drawn == 0 ) || ( bMoreBelow && Drawn == VisibleRowCount - 1 ) )
			{
				RowDraw.Opacity = kEdgeFadeOpacity;
			}

			// 打ち込み中の行は、確定前の文字をそのまま出す。矢印とスライダーは確定した値を
			// 指すものなので、この間は出さない (打っている数字と食い違って見えるため)。
			if ( m_ValueEditor.IsEditing( Row.Element ) )
			{
				RowDraw.ValueText = m_ValueEditor.MakeDisplayText();
				RowDraw.ArrowSize = 0.0f;
				RowDraw.SliderWidth = 0.0f;

				// 入力欄の枠を出し、文字はその内側へ寄せる。枠が無いと、値を出しているだけ
				// なのか打ち込めるのかが見分けられない。
				RowDraw.EditWidth = RowHeight * kEditFieldWidthRatio;
				RowDraw.ValueTextX = RowDraw.ValueX + RowHeight * kEditFieldPadRatio;

				// 打ち始めた直後は全選択。次の 1 文字で丸ごと置き換わることを下敷きで示す。
				if ( m_ValueEditor.IsSelectingAll() )
				{
					RowDraw.SelectionWidth = Text.MeasureWidth( RowDraw.ValueText.Data() );
				}
			}
			ApplyIdleFieldLayout( RowDraw, Row );

			// 打ち込み中の欄は、外を押したかの判定に使うので矩形を控える。
			if ( m_ValueEditor.IsEditing( Row.Element ) )
			{
				// 欄の外を押したときに確定できるよう、いま置いた矩形を教える。
				m_ValueEditor.SetFieldRect( RowDraw.ValueX, RowDraw.Y, RowDraw.EditWidth, RowDraw.BaseHeight );
			}
			OnDrawRow( RenderContext, Batch, Text, Row, RowDraw, static_cast<i32>( RowIndex ) == m_Scroller.GetCursorRow() );

			// 次フレームのマウス判定へ、いま描いた矩形をそのまま渡す。
			FDebugTopDrawnRow Hit;
			Hit.RowIndex = static_cast<i32>( RowIndex );
			Hit.Element = Row.Element;
			Hit.X = RowDraw.OriginX;
			Hit.ContentX = RowDraw.LabelX;
			Hit.Y = RowDraw.Y;
			Hit.Width = RowDraw.Width;
			Hit.Height = RowDraw.Height;
			Hit.ArrowSize = RowDraw.ArrowSize;
			Hit.LeftArrowX = RowDraw.LeftArrowX;
			Hit.RightArrowX = RowDraw.RightArrowX;
			Hit.SliderX = RowDraw.SliderX;
			Hit.SliderWidth = RowDraw.SliderWidth;
			Hit.StarX = RowDraw.StarX;
			Hit.StarSize = RowDraw.StarSize;
			Hit.EditX = RowDraw.ValueX;
			Hit.EditWidth = RowDraw.EditWidth;
			Hit.SwatchX = RowDraw.SwatchX;
			Hit.SwatchY = RowDraw.SwatchY;
			Hit.SwatchWidth = RowDraw.SwatchWidth;
			Hit.SwatchHeight= RowDraw.SwatchHeight;
			m_Mouse.AddDrawnRow( Hit );
		}
		Y += ThisRowHeight;
	}

	// 色の面は行より手前に浮かせる。行を全部描き終えてから重ねる。
	m_ColorPicker.Draw( Batch, RenderContext, RowHeight );

	// 画面に入り切らなかったぶんがあることを、一覧の幅の中央へ三角で示す。
	const f32 MarkerX = OriginX + RowWidth * 0.5f - MoreMarkerSize * 0.5f;
	if ( m_Scroller.GetScrollTop() > 0 )
	{
		DebugTopDrawTriangle( Batch, EDebugTopTriangle::Up, MarkerX, ListTop - MoreMarkerSize, MoreMarkerSize, MoreMarkerSize, kMoreMarkerColor );
	}
	if ( RowIndex < RowCount )
	{
		DebugTopDrawTriangle( Batch, EDebugTopTriangle::Down, MarkerX, Y, MoreMarkerSize, MoreMarkerSize, kMoreMarkerColor );
	}
}

ADebugTopEntity::FRowDraw ADebugTopEntity::BuildRowDraw( const CDebugTopText& Text, const FDebugTopVisibleRow& Row, f32 OriginX, f32 Y, f32 Width, f32 Height, f32 BaseHeight ) const
{
	FRowDraw RowDraw;
	RowDraw.OriginX = OriginX;
	RowDraw.Y = Y;
	RowDraw.Width = Width;
	RowDraw.Height = Height;
	RowDraw.BaseHeight = BaseHeight;

	// 横方向は全て文字 1 行分の高さを基準にする。行そのものの高さを使うと、背の高い行
	// (色を選ぶ面など) だけ段差と桁位置が数倍に膨らんで、列が揃わなくなる。
	// 星は階層に関わらず一番左へ揃える。段差に混ざると押す場所が行ごとに変わって狙いにくい。
	RowDraw.StarSize = BaseHeight * kStarSizeRatio;
	RowDraw.StarX = OriginX + ( BaseHeight * kStarSlotRatio - RowDraw.StarSize ) * 0.5f;

	// インデントとマーカー枠は空白文字ではなく X 座標で作る (フォントのグリフ幅に左右されないため)。
	const f32 IndentBase = OriginX + BaseHeight * kStarSlotRatio;
	RowDraw.MarkerX = IndentBase + static_cast<f32>( Row.Depth ) * BaseHeight * kIndentRatio;
	RowDraw.LabelX = RowDraw.MarkerX + BaseHeight * kMarkerSlotRatio;
	RowDraw.ValueX = OriginX + BaseHeight * kValueColumnRatio;
	RowDraw.ValueTextX = RowDraw.ValueX;

	if ( Row.Element == nullptr ) return RowDraw;

	RowDraw.ValueText = BuildValueText( *Row.Element );

	// 色見本は押せる的なので、描画と当たり判定で同じ矩形を使う。
	FVec4 Swatch{};
	if ( Row.Element->TryGetColorSwatch( Swatch ) )
	{
		RowDraw.SwatchWidth = BaseHeight * kColorSwatchWidthRatio;
		RowDraw.SwatchHeight = BaseHeight * kCheckBoxSizeRatio;
		RowDraw.SwatchX = RowDraw.ValueTextX;
		RowDraw.SwatchY = Y + ( BaseHeight - RowDraw.SwatchHeight ) * 0.5f;
	}

	if ( !Row.Element->IsLeftRightAdjustable() ) return RowDraw;

	// 「◀ 値 ▶」の形に置く。右矢印は値の文字の右端に付くので、幅を測ってから位置を決める。
	const f32 Gap = BaseHeight * kArrowGapRatio;
	RowDraw.ArrowSize = BaseHeight * kArrowSizeRatio;
	RowDraw.LeftArrowX = RowDraw.ValueX;
	RowDraw.ValueTextX = RowDraw.LeftArrowX + RowDraw.ArrowSize + Gap;
	/** 右カラムの中身の幅 (チェックボックスの行は箱のぶん)。 */
	const f32 ValueWidth = Row.Element->GetValueKind() == EDebugTopValueKind::Bool
		? BaseHeight * kCheckBoxSizeRatio
		: Text.MeasureWidth( RowDraw.ValueText.Data() );
	RowDraw.RightArrowX = RowDraw.ValueTextX + ValueWidth + Gap;

	// 範囲を持つ行はスライダーを置く。ドラッグ判定でも同じ矩形を使う。
	f32 Ratio = 0.0f;
	if ( Row.Element->TryGetRatio( Ratio ) )
	{
		RowDraw.SliderX = RowDraw.RightArrowX + RowDraw.ArrowSize + Gap;
		RowDraw.SliderWidth = BaseHeight * kSliderWidthRatio;
	}
	return RowDraw;
}

void ADebugTopEntity::ApplyIdleFieldLayout( FRowDraw& RowDraw, const FDebugTopVisibleRow& Row ) const
{
	// 打ち込める行は、打っていない間も欄として見せる。値を出しているだけの行と見分けが
	// 付かないと、そこへ文字を入れられることに気付けない。
	if ( Row.Element == nullptr || !Row.Element->CanTypeValue() ) return;
	if ( RowDraw.EditWidth > 0.0f ) return; // 打ち込み中は明るい枠の方を出す

	RowDraw.EditWidth = RowDraw.BaseHeight * kEditFieldWidthRatio;
	RowDraw.ValueTextX = RowDraw.ValueX + RowDraw.BaseHeight * kEditFieldPadRatio;
	RowDraw.bIdleField = true;

	// 左右キーで送れる行 (数値) は矢印とスライダーを欄の右へ逃がす。
	if ( RowDraw.ArrowSize <= 0.0f ) return;

	const f32 Gap = RowDraw.BaseHeight * kArrowGapRatio;
	RowDraw.LeftArrowX = RowDraw.ValueX + RowDraw.EditWidth + Gap;
	RowDraw.RightArrowX = RowDraw.LeftArrowX + RowDraw.ArrowSize + Gap;
	if ( RowDraw.SliderWidth > 0.0f )
	{
		RowDraw.SliderX = RowDraw.RightArrowX + RowDraw.ArrowSize + Gap;
	}
}

f32 ADebugTopEntity::OnDrawHeader( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept
{
	(void)RenderContext;

	if ( m_Header.IsEmpty() ) return 0.0f;

	Text.Draw( Batch, m_Header.Data(), OriginX, OriginY, m_HeaderColor.bSet ? m_HeaderColor.Color : kHeaderColor );

	// 見出しと一覧の間を 1 行ぶん空ける。
	return Text.LineHeight() * 2.0f;
}

void ADebugTopEntity::OnDrawRow( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, const FDebugTopVisibleRow& Row, const FRowDraw& RowDraw, bool bSelected ) noexcept
{
	(void)RenderContext;

	/** 行の濃さを掛けた色を返す (端の行を薄く見せるため)。 */
	const auto Fade = [&RowDraw]( const FVec4& Color ) noexcept
	{
		return FVec4{ Color.x, Color.y, Color.z, Color.w * RowDraw.Opacity };
	};

	if ( bSelected )
	{
		Batch.DrawRect( RowDraw.OriginX, RowDraw.Y, RowDraw.Width, RowDraw.Height, Fade( kHighlightColor ) );
	}
	else if ( Row.Element == GetHoverElement() )
	{
		// 押したらここが選ばれる、という予告。選択そのものはクリックまで動かさない。
		Batch.DrawRect( RowDraw.OriginX, RowDraw.Y, RowDraw.Width, RowDraw.Height, Fade( kHoverColor ) );
	}

	// 既定値から動かした行は、左端の帯で示す (何を触ったか後から追える)。
	if ( Row.Element->IsModified() )
	{
		const f32 MarkWidth = RowDraw.Height * kModifiedMarkWidthRatio;
		Batch.DrawRect( RowDraw.OriginX - MarkWidth * 2.0f, RowDraw.Y, MarkWidth, RowDraw.Height, Fade( kModifiedMarkColor ) );
	}

	// 色を明示指定した行はその色を優先し、未指定の行だけ選択状態で色を変える。
	const FDebugTopColor& TextColor = Row.Element->GetTextColor();
	const FVec4 LabelColor = Fade( TextColor.bSet ? TextColor.Color : ( bSelected ? kSelectedTextColor : kNormalTextColor ) );

	if ( Row.Element->ShouldShowMarker() )
	{
		const f32 MarkerSize = RowDraw.BaseHeight * kMarkerSizeRatio;
		const f32 MarkerY = RowDraw.Y + ( RowDraw.BaseHeight - MarkerSize ) * 0.5f;

		// 「押せば開く」行だけを三角形にする。開閉を禁じた行と、マーカーを出しているが
		// 子行を持たない行は、押しても何も起きないので見た目を分ける。
		if ( Row.Element->IsExpandable() && Row.Element->CanCollapse() )
		{
			// 開いていれば下向き、閉じていれば右向き。文字では出ないので図形で描く。
			DebugTopDrawTriangle( Batch, Row.Element->IsExpanded() ? EDebugTopTriangle::Down : EDebugTopTriangle::Right, RowDraw.MarkerX, MarkerY, MarkerSize, MarkerSize, LabelColor );
		}
		else
		{
			// 押しても何も起きない行を「押せそう」に見せないため、薄い横棒にする。
			const f32 Thickness = RowDraw.BaseHeight * kLockedMarkerThicknessRatio;
			const FVec4 LockedColor{ LabelColor.x, LabelColor.y, LabelColor.z, LabelColor.w * kLockedMarkerAlphaScale };
			Batch.DrawRect( RowDraw.MarkerX, RowDraw.Y + ( RowDraw.BaseHeight - Thickness ) * 0.5f, MarkerSize, Thickness, LockedColor );
		}
	}
	// 表示名はプロバイダ付きの行だと毎フレーム変わるので、そのつど組み立てたものを描く。
	const FString Label = Row.Element->GetDisplayLabel();
	Text.Draw( Batch, Label.Data(), RowDraw.LabelX, RowDraw.Y, LabelColor );

	// 行の左端に星ボタンを置く。留めていない行にも薄く出しておかないと、押せることに
	// 気付けない (F キーだけだと見た目に手がかりが無い)。
	if ( RowDraw.StarSize > 0.0f )
	{
		const bool bStarHovered = Row.Element == GetHoverElement() && CInput::MousePos().x <= RowDraw.StarX + RowDraw.StarSize * 2.0f;

		f32 Alpha = kStarIdleAlpha;
		if ( Row.Element->IsFavorite() ) Alpha = 1.0f;
		else if ( bStarHovered ) Alpha = kStarHoverAlpha;

		const FVec4 StarColor{ kPinColor.x, kPinColor.y, kPinColor.z, kPinColor.w * Alpha * RowDraw.Opacity };

		// 上下向きの三角を重ねて六芒星にする (文字では出ないため)。
		const f32 StarY = RowDraw.Y + ( RowDraw.BaseHeight - RowDraw.StarSize ) * 0.5f;
		DebugTopDrawTriangle( Batch, EDebugTopTriangle::Up, RowDraw.StarX, StarY, RowDraw.StarSize, RowDraw.StarSize * 0.75f, StarColor );
		DebugTopDrawTriangle( Batch, EDebugTopTriangle::Down, RowDraw.StarX, StarY + RowDraw.StarSize * 0.25f, RowDraw.StarSize, RowDraw.StarSize * 0.75f, StarColor );
	}

	const FDebugTopColor& ValueColor = Row.Element->GetValueColor();
	FVec4 RightColor = ValueColor.bSet ? Fade( ValueColor.Color ) : LabelColor;

	// 注意すべき範囲へ入った値は色で知らせる。行の色指定より優先する (知らせるのが目的なので)。
	// 右カラムの描画は全てこの色を使うので、文字もスライダーも矢印もまとめて変わる。
	if ( Row.Element->IsValueWarned() ) RightColor = Fade( kWarnColor );

	// 値を追い続ける行は、溜めた標本を折れ線にする。
	const f32* Samples = nullptr;
	usize SampleCount = 0;
	f32 GraphMin = 0.0f;
	f32 GraphMax = 0.0f;
	if ( Row.Element->TryGetGraph( Samples, SampleCount, GraphMin, GraphMax ) )
	{
		DrawGraph( Batch, RowDraw, Samples, SampleCount, GraphMin, GraphMax, RightColor );
	}

	// 色を選ぶ面を持つ行は、面と帯をその場で描く。
	f32 FieldHue = 0.0f;
	f32 FieldSaturation = 0.0f;
	f32 FieldValue = 0.0f;
	if ( Row.Element->TryGetColorField( FieldHue, FieldSaturation, FieldValue ) )
	{
		DebugTopDrawColorField( Batch, RowDraw.LabelX, RowDraw.Y, RowDraw.Height, FieldHue, FieldSaturation, FieldValue, RowDraw.Opacity );
	}

	// 色の行は右カラムへ見本を出す (数値の羅列より、いまの色が一目で分かる)。
	FVec4 Swatch{};
	if ( Row.Element->TryGetColorSwatch( Swatch ) )
	{
		// 枠を先に敷いてから中身を塗る (暗い色でも位置が分かるように)。
		Batch.DrawRect( RowDraw.SwatchX, RowDraw.SwatchY, RowDraw.SwatchWidth, RowDraw.SwatchHeight, Fade( RightColor ) );

		const f32 Border = 1.0f;
		Swatch.w *= RowDraw.Opacity;
		Batch.DrawRect( RowDraw.SwatchX + Border, RowDraw.SwatchY + Border, RowDraw.SwatchWidth - Border * 2.0f, RowDraw.SwatchHeight - Border * 2.0f, Swatch );
	}

	// ON / OFF の行は文字ではなく四角で出す (並んだときに状態を追いやすい)。
	bool bBoolValue = false;
	if ( Row.Element->GetValueKind() == EDebugTopValueKind::Bool && Row.Element->TryGetBool( bBoolValue ) )
	{
		const f32 BoxSize = RowDraw.BaseHeight * kCheckBoxSizeRatio;
		DebugTopDrawCheckBox( Batch, RowDraw.ValueTextX, RowDraw.Y + ( RowDraw.BaseHeight - BoxSize ) * 0.5f, BoxSize, bBoolValue, RightColor );
	}
	else if ( !RowDraw.ValueText.IsEmpty() )
	{
		// 打ち込み中は入力欄として見せる。内側を暗く沈めて、縁を明るくする。
		if ( RowDraw.EditWidth > 0.0f )
		{
			const f32 FieldH = RowDraw.BaseHeight;
			Batch.DrawRect( RowDraw.ValueX, RowDraw.Y, RowDraw.EditWidth, FieldH, Fade( kEditFieldColor ) );

			const f32 Border = 1.0f;
			const FVec4 Edge = Fade( RowDraw.bIdleField ? kEditIdleBorderColor : kEditBorderColor );
			Batch.DrawRect( RowDraw.ValueX, RowDraw.Y, RowDraw.EditWidth, Border, Edge );
			Batch.DrawRect( RowDraw.ValueX, RowDraw.Y + FieldH - Border, RowDraw.EditWidth, Border, Edge );
			Batch.DrawRect( RowDraw.ValueX, RowDraw.Y, Border, FieldH, Edge );
			Batch.DrawRect( RowDraw.ValueX + RowDraw.EditWidth - Border, RowDraw.Y, Border, FieldH, Edge );
		}

		if ( RowDraw.SelectionWidth > 0.0f )
		{
			Batch.DrawRect( RowDraw.ValueTextX, RowDraw.Y, RowDraw.SelectionWidth, RowDraw.BaseHeight, Fade( kSelectionColor ) );
		}
		Text.Draw( Batch, RowDraw.ValueText.Data(), RowDraw.ValueTextX, RowDraw.Y, RightColor );
	}

	// 範囲を持つ行は、値の右にバーを出して増減を掴みやすくする (ドラッグでも動かせる)。
	f32 Ratio = 0.0f;
	if ( RowDraw.SliderWidth > 0.0f && Row.Element->TryGetRatio( Ratio ) )
	{
		const f32 SliderHeight = RowDraw.BaseHeight * kSliderHeightRatio;
		DebugTopDrawSlider( Batch, RowDraw.SliderX, RowDraw.Y + ( RowDraw.BaseHeight - SliderHeight ) * 0.5f, RowDraw.SliderWidth, SliderHeight, Ratio, RightColor );
	}

	if ( RowDraw.ArrowSize > 0.0f )
	{
		// 左右キーで変えられること、クリックできることを矢印で示す。
		const f32 ArrowY = RowDraw.Y + ( RowDraw.BaseHeight - RowDraw.ArrowSize ) * 0.5f;
		DebugTopDrawTriangle( Batch, EDebugTopTriangle::Left, RowDraw.LeftArrowX, ArrowY, RowDraw.ArrowSize, RowDraw.ArrowSize, RightColor );
		DebugTopDrawTriangle( Batch, EDebugTopTriangle::Right, RowDraw.RightArrowX, ArrowY, RowDraw.ArrowSize, RowDraw.ArrowSize, RightColor );
	}
}

FString ADebugTopEntity::BuildValueText( const CDebugTopElement& Element ) const
{
	FString Text = Element.GetValueText();

	// 単位は表示にだけ足す。打ち込みの中身 (GetEditText) は素の値のままなので、単位を
	// 付けても数値をそのまま打ち込める。値の文字を持たない行 (色など) には添えない。
	if ( !Text.IsEmpty() && !Element.GetUnit().IsEmpty() )
	{
		Text.Append( " " );
		Text.Append( Element.GetUnit().View() );
	}

	if ( !Element.IsLeftRightAdjustable() ) return Text;

	// 候補列を持つ行は「値 n/n」にして、選択肢のどこにいるかを添える。
	i32 SelectionIndex = 0;
	i32 SelectionCount = 0;
	if ( Element.TryGetSelection( SelectionIndex, SelectionCount ) && SelectionCount > 0 )
	{
		Text.AppendFormat( " %d/%d", SelectionIndex + 1, SelectionCount );
	}
	return Text;
}

CDebugTopElement* ADebugTopEntity::GetCursorElement() const noexcept
{
	if ( m_VisibleRows.IsEmpty() ) return nullptr;
	const i32 CursorRow = m_Scroller.GetCursorRow();
	if ( CursorRow < 0 || static_cast<usize>( CursorRow ) >= m_VisibleRows.Num() ) return nullptr;

	return m_VisibleRows[static_cast<usize>( CursorRow )].Element;
}

CDebugTopElement* ADebugTopEntity::GetHoverElement() const noexcept
{
	const i32 HoverRow = m_Mouse.GetHoverRow();
	if ( HoverRow < 0 || static_cast<usize>( HoverRow ) >= m_VisibleRows.Num() ) return nullptr;

	return m_VisibleRows[static_cast<usize>( HoverRow )].Element;
}

namespace
{
	/**
	 * 目的の行までの経路にある親を全て開く。
	 *
	 * @param Element 探す起点の行。
	 * @param Target 開きたい行。
	 * @return 経路上にあれば true。
	 */
	bool ExpandTowards( CDebugTopElement& Element, const CDebugTopElement& Target ) noexcept
	{
		if ( &Element == &Target ) return true;

		// Entity を指す行の子は別ページの持ち物なので降りない。
		if ( Element.GetLinkedEntity() != nullptr ) return false;

		const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
		for ( usize Index = 0; Index < Children.Num(); ++Index )
		{
			if ( !Children[Index] ) continue;
			if ( !ExpandTowards( *Children[Index], Target ) ) continue;

			Element.SetExpanded( true );
			return true;
		}
		return false;
	}
}

bool ADebugTopEntity::FocusElement( const CDebugTopElement& Element ) noexcept
{
	// 畳まれている行は可視行に入らないので、先に経路を開いてから探す。
	for ( usize Index = 0; Index < m_Elements.Num(); ++Index )
	{
		if ( !m_Elements[Index] ) continue;

		ExpandTowards( *m_Elements[Index], Element );
	}
	RebuildVisibleRows();

	for ( usize Index = 0; Index < m_VisibleRows.Num(); ++Index )
	{
		if ( m_VisibleRows[Index].Element != &Element ) continue;

		m_Scroller.SetCursorRow( static_cast<i32>( Index ) );
		return true;
	}
	return false;
}

void ADebugTopEntity::SetCursorRow( i32 CursorRow ) noexcept
{
	if ( CursorRow < 0 || CursorRow >= static_cast<i32>( m_VisibleRows.Num() ) ) return;
	m_Scroller.SetCursorRow( CursorRow );
}

ADebugTopEntity* ADebugTopEntity::ConsumePendingTransition() noexcept
{
	ADebugTopEntity* const Target = m_PendingTransition;
	m_PendingTransition = nullptr;
	return Target;
}

const CDebugTopElement* ADebugTopEntity::ConsumePendingFocus() noexcept
{
	const CDebugTopElement* const Focus = m_PendingFocus;
	m_PendingFocus = nullptr;
	return Focus;
}

void ADebugTopEntity::RebuildVisibleRows() noexcept
{
	m_VisibleRows.Reset();
	for ( usize i = 0; i < m_Elements.Num(); ++i )
	{
		FlattenElement( m_Elements[i], 0 );
	}

	// 折り畳みで行数が減ってもカーソルが範囲外に残らないようにする。
	m_Scroller.ClampCursor( m_VisibleRows );
}

void ADebugTopEntity::FlattenElement( const TSharedPtr<CDebugTopElement>& Element, i32 Depth ) noexcept
{
	if ( !Element ) return;

	// 絞り込み中は、自分か子孫が引っかかる行だけを残す。引っかからない枝は丸ごと出さない。
	const bool bFiltering = !m_Filter.IsEmpty();
	if ( bFiltering && !MatchesFilterDeep( *Element ) ) return;

	FDebugTopVisibleRow Row;
	Row.Element = Element.Get();
	Row.Depth = Depth;
	m_VisibleRows.Add( Row );

	// 絞り込み中は畳んでいても降りる。畳んだ奥にある行を探しているのに出てこないと困る。
	if ( !bFiltering && !Element->IsExpanded() ) return;

	const TArray<TSharedPtr<CDebugTopElement>>& Children = Element->GetChildren();
	for ( usize i = 0; i < Children.Num(); ++i )
	{
		FlattenElement( Children[i], Depth + 1 );
	}
}

bool ADebugTopEntity::MatchesFilterDeep( const CDebugTopElement& Element ) const noexcept
{
	if ( DebugTopContainsIgnoreCase( Element.GetDisplayLabel(), m_Filter ) ) return true;

	const TArray<TSharedPtr<CDebugTopElement>>& Children = Element.GetChildren();
	for ( usize i = 0; i < Children.Num(); ++i )
	{
		if ( Children[i] && MatchesFilterDeep( *Children[i] ) ) return true;
	}
	return false;
}


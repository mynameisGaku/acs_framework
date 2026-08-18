// SPDX-License-Identifier: Apache-2.0
#include "DebugTopRowMouse.h"

#include "Debug/DebugTop/Input/DebugTopCursor.h"

namespace
{
	/** 2 回目のクリックをダブルクリックとみなす間隔 (秒。Windows の既定に合わせる)。 */
	constexpr f32 kDoubleClickSeconds = 0.5f;

	/** 一覧の当たり範囲を外側へ広げる量 (行の高さに対する倍率)。 */
	constexpr f32 kHitMarginRatio = 1.0f;

	/** 矢印のクリック判定を上下左右へ広げる量 (行の高さに対する倍率)。 */
	constexpr f32 kArrowHitPadRatio = 0.30f;

	/** 星の当たり判定を左右へ広げる量 (星の一辺に対する倍率)。小さすぎると押しにくい。 */
	constexpr f32 kStarHitPadRatio = 0.5f;

	/**
	 * 値の列の左端から色のパネルまでの距離 (行の高さに対する倍率)。
	 *
	 * @details 入力欄・矢印・スライダーを並べた幅より広く取って、操作するものに被らせない。
	 */
	constexpr f32 kPickerColumnRatio = 14.0f;
}


void CDebugTopRowMouse::Bind( const FDebugTopVisibleRows& Rows, CDebugTopRowScroller& Scroller, CDebugTopColorPicker& ColorPicker, CDebugTopValueEditor& ValueEditor ) noexcept
{
	m_Rows = &Rows;
	m_Scroller = &Scroller;
	m_ColorPicker = &ColorPicker;
	m_ValueEditor = &ValueEditor;
}

void CDebugTopRowMouse::BeginFrame( usize ExpectedRowCount ) noexcept
{
	// 描き直すまでの間に古い矩形を拾わないよう、先に捨てる。
	m_DrawnRows.Reset();
	m_DrawnRows.Reserve( ExpectedRowCount );
}

void CDebugTopRowMouse::ResetClickTracking() noexcept
{
	m_ClickElapsed = 0.0f;
	m_LastClickRow = -1;
}

void CDebugTopRowMouse::Update( f32 DeltaSeconds ) noexcept
{
	if ( m_Rows == nullptr ) return; // 未配線

	const FDebugTopVisibleRows& Rows = *m_Rows;

	// ダブルクリックの判定に使うので、押されていない間も時間は進める。
	if ( m_ClickElapsed < kDoubleClickSeconds ) m_ClickElapsed += DeltaSeconds;

	m_HoverRow = -1;

	const bool bHeld = CInput::IsMouseButtonDown( EMouseButton::Left );
	const FVec2 MousePosition = CInput::MousePos();

	// --- 掴んでいる間は行の当たり判定より優先する (スライダーと、面を持つ行) ---
	if ( m_DragElement != nullptr )
	{
		if ( !bHeld )
		{
			m_DragElement = nullptr;
		}
		else
		{
			for ( usize Index = 0; Index < Rows.Num(); ++Index )
			{
				if ( Rows[Index].Element != m_DragElement ) continue;

				if ( m_DragSliderWidth > 0.0f )
				{
					const f32 Ratio = ( MousePosition.x - m_DragSliderX ) / m_DragSliderWidth;
					Rows[Index].Element->TrySetRatio( Ratio );
				}
				else
				{
					Rows[Index].Element->OnPointer( MousePosition.x - m_DragRowX, MousePosition.y - m_DragRowY, m_DragRowWidth, m_DragRowHeight );
				}
				break;
			}
			return;
		}
	}

	// --- 矢印の押しっぱなし (掴んだ矢印から離れても、ボタンを離すまで続ける) ---
	if ( m_HoldDirection != 0 )
	{
		if ( !bHeld )
		{
			m_HoldDirection = 0;
			m_HoldElement = nullptr;
		}
		else
		{
			if ( m_HoldRepeat.Step( m_HoldDirection, DeltaSeconds ) != 0 )
			{
				for ( usize Index = 0; Index < Rows.Num(); ++Index )
				{
					if ( Rows[Index].Element != m_HoldElement ) continue;

					Rows[Index].Element->OnLeftRight( m_HoldDirection );
					break;
				}
			}
			return;
		}
	}

	// 直前の描画が行を 1 つも置いていなければ、当てる先が無い。
	if ( m_DrawnRows.IsEmpty() ) return;

	// 開いている色の面は行より手前にあるので、先に相手をする。
	if ( m_ColorPicker != nullptr && m_ColorPicker->Update() ) return;

	// ホイールはカーソルを画面内へ詰め直すので先に処理し、同じフレームに来た選択で上書きさせる。
	const f32 Wheel = CInput::MouseWheel();
	if ( Wheel != 0.0f && m_Scroller != nullptr ) m_Scroller->ScrollByWheel( Rows, Wheel );

	const i32 DrawnIndex = FindNearestDrawnRow( MousePosition );
	if ( DrawnIndex < 0 ) return;

	const FDebugTopDrawnRow& Hit = m_DrawnRows[static_cast<usize>( DrawnIndex )];

	// 記録したのは前フレームの状態なので、行が入れ替わっていたら何もしない。
	if ( Hit.RowIndex < 0 || static_cast<usize>( Hit.RowIndex ) >= Rows.Num() ) return;

	CDebugTopElement* const Element = Rows[static_cast<usize>( Hit.RowIndex )].Element;
	if ( Element == nullptr || Element != Hit.Element ) return;

	// 押したらどれが選ばれるかを先に見せる (選択そのものは動かさない)。
	m_HoverRow = Hit.RowIndex;

	// 入力欄の上ではカーソルを縦棒にする。打てる場所であることが指した時点で分かる。
	const bool bOverField = Hit.EditWidth > 0.0f && MousePosition.x >= Hit.EditX && MousePosition.x <= Hit.EditX + Hit.EditWidth;
	DebugTopSetCursor( bOverField ? EDebugTopCursor::Text : EDebugTopCursor::Arrow );

	if ( !CInput::IsMouseButtonPressed( EMouseButton::Left ) ) return;

	if ( m_Scroller != nullptr ) m_Scroller->SetCursorRow( Hit.RowIndex );

	// 色見本を押したら面を出す。行として常に置くと縦に嵩むので、要るときだけ浮かせる。
	if ( m_ColorPicker != nullptr && Hit.SwatchWidth > 0.0f && MousePosition.x >= Hit.SwatchX && MousePosition.x <= Hit.SwatchX + Hit.SwatchWidth && MousePosition.y >= Hit.SwatchY && MousePosition.y <= Hit.SwatchY + Hit.SwatchHeight )
	{
		CDebugTopElementColor* const Color = Element->AsColor();
		if ( Color != nullptr )
		{
			// 同じ見本を押したら閉じる (トグル)。
			if ( m_ColorPicker->IsOpenFor( Color ) )
			{
				m_ColorPicker->Close();
			}
			else
			{
				// 値の列の右外へ出す。見本のすぐ隣だと、下に並ぶ成分の入力欄やスライダーに
				// 被って数値を追えない。起点の縦位置は見本の中心。
				m_ColorPicker->Open( *Color, Hit.EditX + Hit.Height * kPickerColumnRatio, Hit.SwatchY + Hit.SwatchHeight * 0.5f );
			}
			return;
		}
	}

	// 入力欄は 1 クリックで打ち込みを始める。値を入れるだけの操作に 2 度押させない。
	if ( bOverField && Element->CanTypeValue() && m_ValueEditor != nullptr )
	{
		m_ValueEditor->Begin( *Element );
		return;
	}

	// 左端の星はピン留めの付け外し。行を開いたり実行したりはしない。
	if ( Hit.StarSize > 0.0f )
	{
		const f32 Pad = Hit.StarSize * kStarHitPadRatio;
		if ( MousePosition.x >= Hit.StarX - Pad && MousePosition.x <= Hit.StarX + Hit.StarSize + Pad )
		{
			Element->SetFavorite( !Element->IsFavorite() );
			return;
		}
	}

	// --- 値を直接いじる操作は、ダブルクリック判定を通さず即座に効かせる ---
	// (通すと連打したときに 1 回おきしか効かず、反応が鈍く感じる)

	// 面や帯を持つ行 (色を選ぶ行など) は、押した位置をそのまま渡してドラッグを続ける。
	if ( Element->OnPointer( MousePosition.x - Hit.ContentX, MousePosition.y - Hit.Y, Hit.Width, Hit.Height ) )
	{
		m_DragElement = Element;
		m_DragSliderWidth = 0.0f; // スライダーではなく行そのものへ渡す
		m_DragRowX = Hit.ContentX;
		m_DragRowY = Hit.Y;
		m_DragRowWidth = Hit.Width;
		m_DragRowHeight = Hit.Height;
		ResetClickTracking();
		return;
	}

	// スライダーを掴んだ: その場で位置を反映し、以後ドラッグで追従する。
	if ( Hit.SliderWidth > 0.0f && MousePosition.x >= Hit.SliderX && MousePosition.x <= Hit.SliderX + Hit.SliderWidth )
	{
		m_DragElement = Element;
		m_DragSliderX = Hit.SliderX;
		m_DragSliderWidth = Hit.SliderWidth;
		Element->TrySetRatio( ( MousePosition.x - Hit.SliderX ) / Hit.SliderWidth );
		ResetClickTracking();
		return;
	}

	// 矢印を踏んだ: 1 回効かせてから、押しっぱなしなら連打へ移る。
	if ( Hit.ArrowSize > 0.0f )
	{
		const f32 HitPad = Hit.Height * kArrowHitPadRatio;
		i32 Direction = 0;
		if ( MousePosition.x >= Hit.LeftArrowX - HitPad && MousePosition.x <= Hit.LeftArrowX + Hit.ArrowSize + HitPad )
		{
			Direction = -1;
		}
		else if ( MousePosition.x >= Hit.RightArrowX - HitPad && MousePosition.x <= Hit.RightArrowX + Hit.ArrowSize + HitPad )
		{
			Direction = 1;
		}

		if ( Direction != 0 )
		{
			Element->OnLeftRight( Direction );
			m_HoldDirection = Direction;
			m_HoldElement = Element;

			// 押した 1 回は上で送ったので、連射側の初回ぶんは捨てて待ち時間から始めさせる。
			m_HoldRepeat.Reset();
			m_HoldRepeat.Step( Direction, 0.0f );
			ResetClickTracking();
			return;
		}
	}

	// ON / OFF の行は 1 クリックで切り替える (チェックボックスは押した瞬間に反応するのが自然)。
	bool bBoolValue = false;
	if ( Element->GetValueKind() == EDebugTopValueKind::Bool && Element->TryGetBool( bBoolValue ) )
	{
		Element->TrySetBool( !bBoolValue );
		ResetClickTracking();
		return;
	}

	// --- ここから先は「選ぶ / 開く / 決定する」操作 ---
	const bool bDoubleClick = ( m_LastClickRow == Hit.RowIndex ) && ( m_ClickElapsed <= kDoubleClickSeconds );
	m_ClickElapsed = 0.0f;
	m_LastClickRow = bDoubleClick ? -1 : Hit.RowIndex; // 3 回目を続けて拾わないよう区切る

	if ( bDoubleClick )
	{
		// 開閉できる行は 1 回目のクリックで既に開閉しているため、二度手間にしない。
		const bool bToggleable = Element->IsExpandable() && Element->CanCollapse();
		if ( bToggleable ) return;

		// 打ち込みは欄への 1 クリックで始まるので、ここでは決定だけを扱う。
		Element->OnDecide();
		return;
	}

	// 開閉できる行は 1 クリックで開閉する (決定はダブルクリックに割り当てている)。
	if ( Element->IsExpandable() && Element->CanCollapse() )
	{
		Element->SetExpanded( !Element->IsExpanded() );
	}
}

i32 CDebugTopRowMouse::FindNearestDrawnRow( FVec2 Position ) const noexcept
{
	if ( m_DrawnRows.IsEmpty() ) return -1;

	const FDebugTopDrawnRow& First = m_DrawnRows[0];
	const FDebugTopDrawnRow& Last = m_DrawnRows[m_DrawnRows.Num() - 1];

	// 一覧から離れた場所のクリックまで拾うと、関係ない行が選ばれて驚くので範囲を切る。
	const f32 Margin = First.Height * kHitMarginRatio;
	if ( Position.x < First.X - Margin || Position.x > First.X + First.Width + Margin ) return -1;
	if ( Position.y < First.Y - Margin || Position.y > Last.Y + Last.Height + Margin ) return -1;

	i32 NearestIndex = 0;
	f32 NearestDistance = -1.0f;
	for ( usize Index = 0; Index < m_DrawnRows.Num(); ++Index )
	{
		const FDebugTopDrawnRow& Row = m_DrawnRows[Index];

		// 行の矩形に入っていればそれで確定。
		if ( Position.y >= Row.Y && Position.y <= Row.Y + Row.Height ) return static_cast<i32>( Index );

		// 外れていれば行の中心までの距離で競わせる。
		const f32 Center = Row.Y + Row.Height * 0.5f;
		const f32 Distance = Position.y < Center ? Center - Position.y : Position.y - Center;
		if ( NearestDistance < 0.0f || Distance < NearestDistance )
		{
			NearestDistance = Distance;
			NearestIndex = static_cast<i32>( Index );
		}
	}
	return NearestIndex;
}

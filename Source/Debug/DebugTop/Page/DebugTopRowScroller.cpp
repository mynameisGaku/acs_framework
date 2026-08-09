#include "DebugTopRowScroller.h"

namespace
{
	/** ホイール 1 ノッチで動かす行数。 */
	constexpr i32 kWheelScrollRows = 3;

	/** カーソルの上下に残しておきたい行数 (この手前でスクロールを始める)。 */
	constexpr i32 kScrollMarginRows = 2;
}


void CDebugTopRowScroller::MoveCursor( const FDebugTopVisibleRows& Rows, i32 Delta ) noexcept
{
	const i32 RowCount = static_cast<i32>( Rows.Num() );
	if ( RowCount <= 0 ) return;

	m_CursorRow += Delta;
	if ( m_CursorRow < 0 )         m_CursorRow = RowCount - 1;
	if ( m_CursorRow >= RowCount ) m_CursorRow = 0;
}

void CDebugTopRowScroller::ClampCursor( const FDebugTopVisibleRows& Rows ) noexcept
{
	// 折り畳みで行数が減ってもカーソルが範囲外に残らないようにする。
	const i32 RowCount = static_cast<i32>( Rows.Num() );
	if ( m_CursorRow >= RowCount ) m_CursorRow = RowCount - 1;
	if ( m_CursorRow < 0 )         m_CursorRow = 0;
}

f32 CDebugTopRowScroller::GetRowHeight( const FDebugTopVisibleRows& Rows, i32 RowIndex, f32 BaseHeight ) const noexcept
{
	if ( RowIndex < 0 || static_cast<usize>( RowIndex ) >= Rows.Num() ) return BaseHeight;

	const CDebugTopElement* const Element = Rows[static_cast<usize>( RowIndex )].Element;
	return Element != nullptr ? BaseHeight * Element->GetHeightRatio() : BaseHeight;
}

i32 CDebugTopRowScroller::CountRowsThatFit( const FDebugTopVisibleRows& Rows, i32 From, f32 Available, f32 BaseHeight ) const noexcept
{
	const i32 RowCount = static_cast<i32>( Rows.Num() );
	if ( From < 0 || From >= RowCount ) return 0;

	i32 Fit = 0;
	f32 Used = 0.0f;
	for ( i32 Index = From; Index < RowCount; ++Index )
	{
		const f32 Height = GetRowHeight( Rows, Index, BaseHeight );
		if ( Used + Height > Available ) break;

		Used += Height;
		++Fit;
	}

	// 1 行も入らないほど背の高い行でも、その行だけは描く (何も出ないよりましなため)。
	return Fit > 0 ? Fit : 1;
}

void CDebugTopRowScroller::ScrollToCursor( const FDebugTopVisibleRows& Rows, f32 Available, f32 BaseHeight ) noexcept
{
	const i32 RowCount = static_cast<i32>( Rows.Num() );
	if ( RowCount <= 0 || Available <= 0.0f || BaseHeight <= 0.0f ) return;

	// 最後の行まで見せられる先頭位置を、実際の高さを積んで求める。行の高さが一定でない
	// (色の面や折れ線は背が高い) ので、行数だけで割ると送り足りずに下の方の行へ辿り着けなくなる。
	i32 MaxScrollTop = RowCount - 1;
	f32 Used = 0.0f;
	for ( i32 Index = RowCount - 1; Index >= 0; --Index )
	{
		const f32 Height = GetRowHeight( Rows, Index, BaseHeight );
		if ( Used + Height > Available ) break;

		Used += Height;
		MaxScrollTop = Index;
	}

	// 端に貼り付いてからスクロールし始めると先が見えないので、数行手前で動かす。
	// 画面に少ししか入らない場合は余白を取らない (取ると常にスクロールしてしまうため)。
	i32 Margin = kScrollMarginRows;
	if ( Margin * 2 >= CountRowsThatFit( Rows, m_ScrollTop, Available, BaseHeight ) ) Margin = 0;

	if ( m_CursorRow - Margin < m_ScrollTop ) m_ScrollTop = m_CursorRow - Margin;
	if ( m_ScrollTop < 0 ) m_ScrollTop = 0;

	// カーソル (と余白ぶん) が下に隠れている間、1 行ずつ送る。
	i32 Need = m_CursorRow + Margin;
	if ( Need > RowCount - 1 ) Need = RowCount - 1;
	while ( m_ScrollTop < Need && m_ScrollTop + CountRowsThatFit( Rows, m_ScrollTop, Available, BaseHeight ) - 1 < Need )
	{
		++m_ScrollTop;
	}

	if ( m_ScrollTop > MaxScrollTop ) m_ScrollTop = MaxScrollTop;
	if ( m_ScrollTop < 0 )            m_ScrollTop = 0;

	m_MaxScrollTop = MaxScrollTop;
}

void CDebugTopRowScroller::ScrollByWheel( const FDebugTopVisibleRows& Rows, f32 Wheel ) noexcept
{
	const i32 RowCount = static_cast<i32>( Rows.Num() );
	if ( RowCount <= 0 || m_DrawnRowCapacity <= 0 ) return;

	// 奥へ回すと前の行が出るように符号を反転する。1 ノッチ未満でも 1 行は動かす。
	i32 Delta = -static_cast<i32>( Wheel ) * kWheelScrollRows;
	if ( Delta == 0 ) Delta = Wheel > 0.0f ? -1 : 1;

	m_ScrollTop += Delta;

	// 上限は直前の描画が求めたもの (行ごとに高さが違うので行数からは出ない)。
	if ( m_ScrollTop > m_MaxScrollTop ) m_ScrollTop = m_MaxScrollTop;
	if ( m_ScrollTop < 0 )              m_ScrollTop = 0;

	// カーソルを画面外に残すと、次の Draw のスクロール追従が引き戻してしまう。収まる行数は
	// 送った先で数え直す (元の位置の行数を使うと、背の高い行を跨いだときにずれる)。
	const i32 Fit = CountRowsThatFit( Rows, m_ScrollTop, m_LastAvailable, m_LastBaseHeight );

	// 追従の余白ぶんも内側へ入れておく。端に貼り付けたままだと、次の Draw が余白を作ろうと
	// してさらに送り、ホイールを回すたびに行き過ぎてしまう。
	i32 Margin = kScrollMarginRows;
	if ( Margin * 2 >= Fit ) Margin = 0;

	const i32 TopRow = m_ScrollTop + Margin;
	const i32 BottomRow = m_ScrollTop + Fit - 1 - Margin;
	if ( m_CursorRow < TopRow )    m_CursorRow = TopRow;
	if ( m_CursorRow > BottomRow ) m_CursorRow = BottomRow;
	if ( m_CursorRow >= RowCount ) m_CursorRow = RowCount - 1;
	if ( m_CursorRow < 0 )         m_CursorRow = 0;
}

void CDebugTopRowScroller::RememberViewport( f32 Available, f32 BaseHeight, i32 DrawnRowCapacity ) noexcept
{
	m_LastAvailable = Available;
	m_LastBaseHeight = BaseHeight;
	m_DrawnRowCapacity = DrawnRowCapacity;
}

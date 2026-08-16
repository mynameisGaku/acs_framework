// SPDX-License-Identifier: Apache-2.0
#include "Debug/HotReload/View/HotReloadPage.h"

#include "Debug/DebugTop/Element/DebugTopElementWatch.h"
#include "Debug/HotReload/HotReloadSubsystem.h"

namespace
{
	/** 見出しの色。 */
	constexpr FVec4 kHeaderColor{ 0.86f, 0.68f, 0.42f, 1.0f };
}


AHotReloadPage::AHotReloadPage( const FString& Name, CHotReloadSubsystem& HotReload )
	: ADebugTopEntity( Name )
	, m_HotReload( &HotReload )
{
	SetHeader( FString( "Hot Reload" ) );
	SetHeaderColor( kHeaderColor );
	SetDescription( FString( "差し替えが反映されないときの切り分け用\n" "見張れているか → 拾えているか → 渡っているか の順に見る" ) );
}


void AHotReloadPage::OnBuild() noexcept
{
	Add<CDebugTopElementWatch>( FString( "Watching" ), FDebugTopTextDelegate::CreateRaw<&AHotReloadPage::MakeStateText>( this ) )
		->SetDescription( FString( "見張っているかと、見張っている場所の数" ) );

	Add<CDebugTopElementWatch>( FString( "Pending" ), FDebugTopTextDelegate::CreateRaw<&AHotReloadPage::MakePendingText>( this ) )
		->SetDescription( FString( "拾ったがまだ配っていない件数" ) );

	Add<CDebugTopElementWatch>( FString( "Dispatched" ), FDebugTopTextDelegate::CreateRaw<&AHotReloadPage::MakeDispatchedText>( this ) )
		->SetDescription( FString( "配った件数 / 引き受け手が居らず捨てた件数" ) );
}


FString AHotReloadPage::MakeStateText() const
{
	FString Text;
	if ( m_HotReload == nullptr ) return Text;

	Text.AppendFormat( "%s (%u 箇所)", m_HotReload->IsWatching() ? "見張り中" : "停止", m_HotReload->GetWatchedCount() );

	return Text;
}


FString AHotReloadPage::MakePendingText() const
{
	FString Text;
	if ( m_HotReload == nullptr ) return Text;

	Text.AppendFormat( "%u", m_HotReload->GetPendingCount() );

	return Text;
}


FString AHotReloadPage::MakeDispatchedText() const
{
	FString Text;
	if ( m_HotReload == nullptr ) return Text;

	Text.AppendFormat( "%llu / %llu",
		static_cast<unsigned long long>( m_HotReload->GetDispatchedCount() ),
		static_cast<unsigned long long>( m_HotReload->GetUnhandledCount() ) );

	return Text;
}

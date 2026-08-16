// SPDX-License-Identifier: Apache-2.0
#include "Common/Test/TestHarness.h"
#include "Debug/HotReload/HotReloadDispatcher.h"
#include "Debug/HotReload/HotReloadWatchPlan.h"

namespace
{
	/** 決めた順に «変わった» を返す、見張りの代わり。 */
	class CFakeEventSource final : public IHotReloadEventSource
	{
	public:
		void Queue( const char* Path, bool bRemoved = false ) noexcept
		{
			FHotReloadEvent Event;
			Event.file_path = Path;
			Event.removed = bRemoved;
			m_Events.TryAdd( Event );
		}

		bool ConsumeNextEvent( FHotReloadEvent& OutEvent ) noexcept override
		{
			if ( m_Cursor >= m_Events.Num() ) return false;

			OutEvent = m_Events[m_Cursor++];
			return true;
		}

		usize GetRemainingCount() const noexcept { return m_Events.Num() - m_Cursor; }

	private:
		TArray<FHotReloadEvent> m_Events;
		usize m_Cursor = 0u;
	};

	/** 拡張子で受け取るかを決め、受け取った数を数える引き受け手。 */
	class CCountingHandler final : public IHotReloadHandler
	{
	public:
		explicit CCountingHandler( const char* Extension ) noexcept
			: m_Extension( Extension )
		{
		}

		bool CanHandle( const FHotReloadEvent& Event ) const noexcept override
		{
			if ( Event.file_path == nullptr ) return false;
			if ( m_Extension == nullptr ) return true;

			return FStringView( Event.file_path ).EndsWith( FStringView( m_Extension ) );
		}

		void OnFileChanged( const FHotReloadEvent& Event ) noexcept override
		{
			(void)Event;
			++m_HandledCount;
		}

		u32 GetHandledCount() const noexcept { return m_HandledCount; }

	private:
		const char* m_Extension = nullptr;
		u32 m_HandledCount = 0u;
	};
}


void RunHotReloadDispatcherTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CHotReloadDispatcher / 担当だけへ配る" );

	{
		CFakeEventSource Source;
		Source.Queue( "Assets/Hero.png" );
		Source.Queue( "Assets/Config.json" );
		Source.Queue( "Assets/Tree.png" );

		CCountingHandler Images( ".png" );
		CCountingHandler Configs( ".json" );

		CHotReloadDispatcher Dispatcher;
		Harness.Check( Dispatcher.AddHandler( Images ), "引き受け手を足せる" );
		Harness.Check( Dispatcher.AddHandler( Configs ), "2 つ目も足せる" );
		Harness.CheckEqualU64( Dispatcher.GetHandlerCount(), 2u, "引き受け手の数" );

		const usize Delivered = Dispatcher.DispatchPending( Source, 16u );

		Harness.CheckEqualU64( Delivered, 3u, "3 件とも取り出す" );
		Harness.CheckEqualU64( Images.GetHandledCount(), 2u, "png は画像側だけへ" );
		Harness.CheckEqualU64( Configs.GetHandledCount(), 1u, "json は設定側だけへ" );
		Harness.CheckEqualU64( Dispatcher.GetDispatchedCount(), 3u, "配った数" );
		Harness.CheckEqualU64( Dispatcher.GetUnhandledCount(), 0u, "捨てた数は 0" );
	}

	Harness.BeginSuite( "CHotReloadDispatcher / 担当が居なければ数える" );

	{
		// 黙って捨てると «反映されない» 原因が分からなくなる。数だけは残す。
		CFakeEventSource Source;
		Source.Queue( "Assets/Sound.wav" );
		Source.Queue( "Assets/Sound2.wav" );

		CCountingHandler Images( ".png" );

		CHotReloadDispatcher Dispatcher;
		Dispatcher.AddHandler( Images );

		const usize Delivered = Dispatcher.DispatchPending( Source, 16u );

		Harness.CheckEqualU64( Delivered, 2u, "取り出しはする" );
		Harness.CheckEqualU64( Images.GetHandledCount(), 0u, "担当外へは渡さない" );
		Harness.CheckEqualU64( Dispatcher.GetUnhandledCount(), 2u, "捨てた数を数えている" );
		Harness.CheckEqualU64( Dispatcher.GetDispatchedCount(), 0u, "配った数は 0" );
	}

	Harness.BeginSuite( "CHotReloadDispatcher / 1 回の上限で止まる" );

	{
		// 大量に差し替えても、そのフレームが伸び切らないようにする。残りは次へ回る。
		CFakeEventSource Source;
		for ( u32 Index = 0u; Index < 10u; ++Index ) Source.Queue( "Assets/A.png" );

		CCountingHandler Images( ".png" );

		CHotReloadDispatcher Dispatcher;
		Dispatcher.AddHandler( Images );

		Harness.CheckEqualU64( Dispatcher.DispatchPending( Source, 4u ), 4u, "上限まで" );
		Harness.CheckEqualU64( Source.GetRemainingCount(), 6u, "残りは取り出さない" );

		Harness.CheckEqualU64( Dispatcher.DispatchPending( Source, 4u ), 4u, "次の回で続き" );
		Harness.CheckEqualU64( Dispatcher.DispatchPending( Source, 4u ), 2u, "最後は残りぶんだけ" );
		Harness.CheckEqualU64( Dispatcher.DispatchPending( Source, 4u ), 0u, "空なら 0" );

		Harness.CheckEqualU64( Images.GetHandledCount(), 10u, "全部届く" );
	}

	Harness.BeginSuite( "CHotReloadDispatcher / 複数が同じものを引き受ける" );

	{
		CFakeEventSource Source;
		Source.Queue( "Assets/Shared.png" );

		CCountingHandler First( ".png" );
		CCountingHandler Second( nullptr );   // 何でも引き受ける

		CHotReloadDispatcher Dispatcher;
		Dispatcher.AddHandler( First );
		Dispatcher.AddHandler( Second );

		Dispatcher.DispatchPending( Source, 8u );

		Harness.CheckEqualU64( First.GetHandledCount(), 1u, "片方へ届く" );
		Harness.CheckEqualU64( Second.GetHandledCount(), 1u, "もう片方へも届く" );
		Harness.CheckEqualU64( Dispatcher.GetDispatchedCount(), 1u, "件数としては 1 件" );
	}

	Harness.BeginSuite( "CHotReloadWatchPlan / 見る場所を溜める" );

	{
		CHotReloadWatchPlan Plan;

		Harness.Check( Plan.AddDirectory( FString( "Assets" ), true ), "フォルダを足せる" );
		Harness.Check( Plan.AddFile( FString( "Config.json" ) ), "ファイルも足せる" );
		Harness.Check( !Plan.AddDirectory( FString(), true ), "空のパスは足さない" );
		Harness.Check( !Plan.AddFile( FString() ), "空のファイルも足さない" );

		Harness.CheckEqualU64( Plan.Num(), 2u, "件数" );
		Harness.Check( Plan.Get( 0u ).bDirectory, "1 つ目はフォルダ" );
		Harness.Check( Plan.Get( 0u ).bRecursive, "下まで見る" );
		Harness.Check( !Plan.Get( 1u ).bDirectory, "2 つ目はファイル" );
		Harness.Check( Plan.Get( 1u ).IsValid(), "有効" );

		CHotReloadWatchPlan Defaults;
		Defaults.AddFrameworkDefaults();
		Harness.Check( Defaults.Num() != 0u, "既定の場所が入る" );
	}
}

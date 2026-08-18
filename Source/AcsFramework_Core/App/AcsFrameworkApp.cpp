// SPDX-License-Identifier: Apache-2.0
#include "AcsFrameworkApp.h"

#include "AcsFramework_Core/App/AppSubsystem.h"
#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Audio/AudioSubsystem.h"
#include "AcsFramework_Core/Audio/Music/MusicSubsystem.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialAudioSubsystem.h"
#include "AcsFramework_Core/Event/EventSubsystem.h"
#include "AcsFramework_Core/Boot/BootScene.h"
#include "AcsFramework_Core/Fade/FadeSubsystem.h"
#include "AcsFramework_Core/Loading/LoadingScreenSubsystem.h"
#include "AcsFramework_Core/Pause/PauseScreenSubsystem.h"
#include "AcsFramework_Core/Save/SaveSubsystem.h"
#include "AcsFramework_Core/Scene/SceneTravelSubsystem.h"
#include "AcsFramework_Sample/Scene/Demo3DScene.h"
#include "AcsFramework_Sample/Scene/MinimalScene.h"
#include "AcsFramework_Core/Settings/GameSettingsSubsystem.h"
#include "AcsFramework_Core/Simulation/SimulationSubsystem.h"
#include "AcsFramework_Core/State/AppStateSubsystem.h"
#include "AcsFramework_Core/Screen/ScreenSubsystem.h"
#include "AcsFramework_Core/Text/UiFontSubsystem.h"
#include "AcsFramework_Core/Time/TimeSubsystem.h"
#include "AcsFramework_Core/Timer/TimerSubsystem.h"

#include "Debug/Perf/PerfBudgetSubsystem.h"
#include "Debug/Perf/ScopedPerfSample.h"

namespace
{
	/** 1 フレームの目標時間 (ms)。60fps を基準にする。 */
	constexpr f32 kFrameBudgetMilliseconds = 16.6f;
}

#if _DEBUG
#include "Debug/DebugTop/DebugTopOverlaySubsystem.h"
#include "Debug/DevConsole/Builtin/ConsoleCommandsApp.h"
#include "Debug/DevConsole/Builtin/ConsoleCommandsAudio.h"
#include "Debug/DevConsole/Builtin/ConsoleCommandsPerf.h"
#include "Debug/DevConsole/DevConsoleSubsystem.h"
#include "Debug/DevConsole/View/DevConsolePage.h"
#include "Debug/HotReload/Builtin/HotReloadLogHandler.h"
#include "Debug/HotReload/HotReloadSubsystem.h"
#include "Debug/HotReload/View/HotReloadPage.h"
#include "Debug/Perf/View/PerfBudgetPage.h"
#include "Debug/Simulation/View/SimulationPage.h"

namespace
{
	/** デバッグメニューが掴んでいる間、時間を止めておくための理由。 */
	constexpr const char* kDebugMenuPauseReason = "DebugTop";
}
#endif

CAcsFrameworkApp::CAcsFrameworkApp()
{

}

CAcsFrameworkApp::~CAcsFrameworkApp()
{

}

TUniquePtr<AScene> CAcsFrameworkApp::InitialScene() noexcept
{
	// フェードの幕はエンジン (CGame) が持っている。ゲームコードからは CGame を辿れないので、
	// ここで渡して GetSubsystem<CFadeSubsystem>() から使えるようにする。
	if ( CFadeSubsystem* const Fade = GetSubsystem<CFadeSubsystem>() )
	{
		Fade->Bind( *this );
	}

	// シーンの切り替えも CGame が持っている。同じ理由でここから渡す。
	if ( CSceneTravelSubsystem* const Travel = GetSubsystem<CSceneTravelSubsystem>() )
	{
		Travel->Bind( *this );
	}

	// 窓もアプリ本体も、辿れるのはここだけ。頼み事を受ける係へ渡しておく。
	if ( CScreenSubsystem* const Screen = GetSubsystem<CScreenSubsystem>() )
	{
		Screen->Bind( *this );
	}

	if ( CAppSubsystem* const App = GetSubsystem<CAppSubsystem>() )
	{
		App->Bind( *this );
	}

	if ( CEventSubsystem* const Events = GetSubsystem<CEventSubsystem>() )
	{
		Events->Bind( *this );
	}

	if ( CAppStateSubsystem* const State = GetSubsystem<CAppStateSubsystem>() )
	{
		State->Bind( *this );
	}

	// セーブの置き場所。ゲームごとに変えたいので、ここが唯一の決め所になる。
	if ( CSaveSubsystem* const Save = GetSubsystem<CSaveSubsystem>() )
	{
		Save->Configure( FString( "Saved/Save" ), FString( "Slot" ), 3 );
	}

	// プレイヤーが決める設定。あれば読み込まれる (初回は何も無いので既定のまま)。
	CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>();
	if ( Settings != nullptr )
	{
		Settings->Configure( FString( "Saved/GameSettings.acscfg" ) );
	}

	// 音の一式を組み立てる。音量は «残す側» と «鳴らす側» のどちらも相手を知らないので、
	// 決め所であるここで繋ぐ。
	CAudioSubsystem* const Audio = GetSubsystem<CAudioSubsystem>();
	if ( Audio != nullptr )
	{
		Audio->Bind( *this );
		if ( Settings != nullptr )
		{
			Audio->SetMasterVolume( Settings->GetFloat( FString( "Audio/Master" ), 1.0f ) );
			Audio->SetBgmVolume( Settings->GetFloat( FString( "Audio/Bgm" ), 1.0f ) );
			Audio->SetSfxVolume( Settings->GetFloat( FString( "Audio/Sfx" ), 1.0f ) );
		}
	}

	// 曲を «決める側» と 場所のある音は、どちらも «鳴らす側» を知らない。ここで繋ぐ
	// (曲の登録と聴く位置の指定はゲーム側が行う)。
	if ( Audio != nullptr )
	{
		if ( CMusicSubsystem* const Music = GetSubsystem<CMusicSubsystem>() ) Music->Bind( *Audio );

		if ( CSpatialAudioSubsystem* const Spatial = GetSubsystem<CSpatialAudioSubsystem>() ) Spatial->Bind( *Audio );
	}

	// 時間の倍率もアプリ (CGame) が持っている。同じ理由でここから渡す。
	if ( CTimeSubsystem* const Time = GetSubsystem<CTimeSubsystem>() )
	{
		Time->Bind( *this );
	}

	// アセットのレジストリもアプリが持っている。読み込み役へ渡す (ロード画面は関与しない)。
	if ( CAssetLoaderSubsystem* const Loader = GetSubsystem<CAssetLoaderSubsystem>() )
	{
		Loader->Bind( GetAssets() );
	}

	// フレームの予算。枠組みが測る場所は既定で入る。ゲーム側の区分は DefineCategory で足す。
	CPerfBudgetSubsystem* const Perf = GetSubsystem<CPerfBudgetSubsystem>();
	if ( Perf != nullptr )
	{
		Perf->Configure( kFrameBudgetMilliseconds );
	}

	// 開発中の道具はここで組む。どのコマンドを積むかは «決め所» であるアプリが決め、
	// コンソール側は積まれたものを知らないままにしておく。
#if _DEBUG
	CDebugTopOverlaySubsystem* const Overlay = GetSubsystem<CDebugTopOverlaySubsystem>();

	if ( Perf != nullptr && Overlay != nullptr )
	{
		Overlay->GetHUD().AddEntity( NewObject<APerfBudgetPage>( FString( "Perf" ), *Perf ) );
	}

	if ( CDevConsoleSubsystem* const Console = GetSubsystem<CDevConsoleSubsystem>() )
	{
		if ( CAppSubsystem* const ConsoleApp = GetSubsystem<CAppSubsystem>() )
		{
			Console->AddProvider( MakeUnique<CConsoleCommandsApp>( *Console, *ConsoleApp ) );
		}

		if ( Audio != nullptr )
		{
			Console->AddProvider( MakeUnique<CConsoleCommandsAudio>( *Console, *Audio ) );
		}

		if ( Perf != nullptr )
		{
			Console->AddProvider( MakeUnique<CConsoleCommandsPerf>( *Console, *Perf ) );
		}

		if ( Overlay != nullptr )
		{
			Overlay->GetHUD().AddEntity( NewObject<ADevConsolePage>( FString( "Console" ), *Console ) );
		}
	}

	// 記録と再生の操作盤。バグが出た瞬間に画面からテープを保存できるようにしておく。
	if ( CSimulationSubsystem* const Simulation = GetSubsystem<CSimulationSubsystem>() )
	{
		if ( Overlay != nullptr )
		{
			Overlay->GetHUD().AddEntity( NewObject<ASimulationPage>( FString( "Simulation" ), *Simulation ) );
		}
	}

	// 差し替えの見張り。何を作り直すかは引き受け手が決めるので、既定では記録に残すだけ。
	if ( CHotReloadSubsystem* const HotReload = GetSubsystem<CHotReloadSubsystem>() )
	{
		if ( HotReload->StartWatchingDefaults() )
		{
			HotReload->AddHandler( MakeUnique<CHotReloadLogHandler>() );
		}

		if ( Overlay != nullptr )
		{
			Overlay->GetHUD().AddEntity( NewObject<AHotReloadPage>( FString( "HotReload" ), *HotReload ) );
		}
	}
#endif

	return CreateInitialScene();
}

TUniquePtr<AScene> CAcsFrameworkApp::CreateInitialScene() noexcept
{
	// 既定は «何も映らない» ではなく «3D が映る» にしてある。
	// 最小の書き方を見たいなら AMinimalScene へ差し替える (14 行で床と物と影が出る)。この枠組みの目当てが 3D なので、
	// 起動していきなり黒画面だと、動いているのかどうかも分からない。
	// 自分のゲームを作るときは、この関数を override して差し替える (ABootScene は空の起動場面)。
	return MakeUnique<ADemo3DScene>();
}

void CAcsFrameworkApp::OnStart() noexcept
{
	// GameInstance service の配線は InitialScene() 内で OnEnter より前に完了している。
	CGame::OnStart();
}

void CAcsFrameworkApp::OnUpdate( f32 DeltaSeconds ) noexcept
{
	// 予算の計測はこのフレームの全てを含めたいので、何よりも先に開ける。
	CPerfBudgetSubsystem* const Perf = GetSubsystem<CPerfBudgetSubsystem>();
	if ( Perf != nullptr ) Perf->BeginFrame();

	CTimeSubsystem* const Time = GetSubsystem<CTimeSubsystem>();

	// 重ねているデバッグメニューはシーンより先に見る。出ている間はゲームを止めたいので、
	// シーンを進める前に決める必要がある。止め方は時間の担当へ任せ、ここでは
	// 「メニューが掴んでいる」ことだけを伝える (ポーズ画面など他の止め手と重なっても、
	// 片方を閉じただけで動き出さないようにするため)。
#if _DEBUG
	if ( CDebugTopOverlaySubsystem* const Overlay = GetSubsystem<CDebugTopOverlaySubsystem>() )
	{
		const FScopedPerfSample Sample( Perf, "Debug/Overlay" );

		const bool bCaptured = Overlay->Update( DeltaSeconds );
		if ( Time != nullptr )
		{
			if ( bCaptured ) Time->Pause( FString( kDebugMenuPauseReason ) );
			else             Time->Resume( FString( kDebugMenuPauseReason ) );
		}
	}
#endif

	// 決めた速さを倍率へ反映する。シーンを進める前に呼ぶこと。
	if ( Time != nullptr ) Time->Update();

	// 現 top シーンの駆動は基底が行う。止まっている間は呼ばない (倍率 0 だけだとシーンは
	// 呼ばれ続けるので、止めているつもりでもキー入力が奥まで届いてしまう)。
	if ( Time == nullptr || Time->ShouldTickScenes() )
	{
		const FScopedPerfSample Sample( Perf, "Scene/Update" );

		CGame::OnUpdate( DeltaSeconds );
	}

	// ゲームロジックは固定ステップで進める。渡すのは倍率を掛けた «ゲームの時間» なので、
	// 止めれば盤面も止まり、遅くすれば盤面も遅くなる。規則が差さっていなければ回さない。
	if ( CSimulationSubsystem* const Simulation = GetSubsystem<CSimulationSubsystem>() )
	{
		if ( Simulation->HasRule() && ( Time == nullptr || Time->ShouldTickScenes() ) )
		{
			const FScopedPerfSample Sample( Perf, "Sim/Update" );

			const f32 Scale = ( Time != nullptr ) ? Time->GetEffectiveScale() : 1.0f;
			Simulation->Update( static_cast<f64>( DeltaSeconds * Scale ) );
		}
	}

	// 読み込みはシーンに属さない (遷移を跨いで続く) ので、アプリの側で進める。
	// ロード画面より先に進めること。同じフレームの進み具合が画面へ乗る。
	if ( CAssetLoaderSubsystem* const Loader = GetSubsystem<CAssetLoaderSubsystem>() )
	{
		const FScopedPerfSample Sample( Perf, "Assets/Load" );

		Loader->Update();
	}

	// ロード画面もシーンに属さないので、アプリの側で進める。
	if ( CLoadingScreenSubsystem* const Loading = GetSubsystem<CLoadingScreenSubsystem>() )
	{
		Loading->Update( DeltaSeconds );
	}

	// 幕を張った積み下ろしは、暗転しきってから実際に行う。その続きをここで進める。
	if ( CSceneTravelSubsystem* const Travel = GetSubsystem<CSceneTravelSubsystem>() )
	{
		Travel->Update();
	}

	// タイマーはシーンに属さない (遷移を跨いで仕掛かったままになる) ので、アプリの側で進める。
	// ゲーム時間の時計へ掛ける倍率だけをここで渡す。時計の側は止め方を知らない。
	if ( CTimerSubsystem* const Timers = GetSubsystem<CTimerSubsystem>() )
	{
		Timers->Update( DeltaSeconds, Time != nullptr ? Time->GetEffectiveScale() : 1.0f );
	}

	// 音は実時間で進める。ゲームを止めても曲は流れ続ける。
	if ( CAudioSubsystem* const Audio = GetSubsystem<CAudioSubsystem>() )
	{
		const FScopedPerfSample Sample( Perf, "Audio/Update" );

		Audio->Update( DeltaSeconds );

		// 曲の切り替えは、鳴らす側を進めた直後に見る。集まった申告はここで 1 つに決まる。
		if ( CMusicSubsystem* const Music = GetSubsystem<CMusicSubsystem>() ) Music->Update( DeltaSeconds );

		// 聴く位置はシーンが動いた後の位置を使いたいので、曲の後に更新する。
		if ( CSpatialAudioSubsystem* const Spatial = GetSubsystem<CSpatialAudioSubsystem>() ) Spatial->Update( DeltaSeconds );
	}

	// 設定は書き換えられてから手が止まったところで書く。ここで測る。
	if ( CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() )
	{
		Settings->Update( DeltaSeconds );
	}

	// ポーズの幕は止まっている間も進める (止まっていることを見せる幕なので、
	// 止まると出入りしなくなっては困る)。
	if ( CPauseScreenSubsystem* const Pause = GetSubsystem<CPauseScreenSubsystem>() )
	{
		Pause->Update( DeltaSeconds );
	}

	// 差し替えの見張りも実時間で進める。止めて眺めながら絵を差し替えることがあるため。
#if _DEBUG
	if ( CHotReloadSubsystem* const HotReload = GetSubsystem<CHotReloadSubsystem>() )
	{
		HotReload->Update( DeltaSeconds );
	}
#endif

	// 予算の計測はここで閉じる。以降このフレームでは積まない。
	if ( Perf != nullptr ) Perf->EndFrame();
}

void CAcsFrameworkApp::OnRender() noexcept
{
	// 文字はこの層で焼いたものを使う。エンジン共有の UI フォントは漢字を焼いておらず
	// (「重」等が無言で消える)、そのうえシーンを描き終えると FRenderContext から取れなくなる。
	// ここで用意しておけば、シーンも幕も同じものを使える。
	if ( CUiFontSubsystem* const UiFont = GetSubsystem<CUiFontSubsystem>() ) m_UiFont = UiFont->Acquire( GetRenderer() );

	// 現 top シーンの描画は基底が行う。
	{
		const FScopedPerfSample Sample( GetSubsystem<CPerfBudgetSubsystem>(), "Scene/Render" );

		CGame::OnRender();
	}

	// ポーズの幕はゲームのすぐ上。開発用の道具 (デバッグメニュー) はその更に上に出したいので、
	// ここで先に重ねておく。
	if ( CPauseScreenSubsystem* const Pause = GetSubsystem<CPauseScreenSubsystem>() )
	{
		Pause->Draw( GetRenderer(), m_UiFont );
	}

	// 重ねているデバッグメニューはシーンの上。下のゲームは動いたまま見える。
#if _DEBUG
	if ( CDebugTopOverlaySubsystem* const Overlay = GetSubsystem<CDebugTopOverlaySubsystem>() )
	{
		Overlay->Draw( GetRenderer(), m_UiFont );
	}
#endif

	// ロード画面はさらに手前。基底がフェードの幕まで重ね終えた後に出す
	// (待たせている最中でも、暗転はロード画面の上に乗るのが自然なため)。
	if ( CLoadingScreenSubsystem* const Loading = GetSubsystem<CLoadingScreenSubsystem>() )
	{
		Loading->Draw( GetRenderer(), m_UiFont );
	}
}

void CAcsFrameworkApp::OnShutdown() noexcept
{
	// 書き換えた設定を取りこぼさない。手が止まる前に落とされても残るようにする。
	if ( CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() )
	{
		if ( Settings->IsDirty() ) Settings->Save();
	}

	// 残ったシーンへの OnExit は基底が行う。
	CGame::OnShutdown();
}

void CAcsFrameworkApp::OnEvent( const FEvent& Event ) noexcept
{
	// 現 top シーンへの配送は基底が行う。
	CGame::OnEvent( Event );
}

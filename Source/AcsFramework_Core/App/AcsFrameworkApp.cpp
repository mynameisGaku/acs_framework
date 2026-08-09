#include "AcsFrameworkApp.h"

#include "AcsFramework_Core/App/AppSubsystem.h"
#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Audio/AudioSubsystem.h"
#include "AcsFramework_Core/Event/EventSubsystem.h"
#include "AcsFramework_Core/Boot/BootScene.h"
#include "AcsFramework_Core/Fade/FadeSubsystem.h"
#include "AcsFramework_Core/Loading/LoadingScreenSubsystem.h"
#include "AcsFramework_Core/Pause/PauseScreenSubsystem.h"
#include "AcsFramework_Core/Save/SaveSubsystem.h"
#include "AcsFramework_Core/Scene/SceneTravelSubsystem.h"
#include "AcsFramework_Core/Settings/GameSettingsSubsystem.h"
#include "AcsFramework_Core/State/AppStateSubsystem.h"
#include "AcsFramework_Core/Screen/ScreenSubsystem.h"
#include "AcsFramework_Core/Text/UiFontSubsystem.h"
#include "AcsFramework_Core/Time/TimeSubsystem.h"
#include "AcsFramework_Core/Timer/TimerSubsystem.h"

#if _DEBUG
#include "Debug/DebugTop/DebugTopOverlaySubsystem.h"

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
	if ( CAudioSubsystem* const Audio = GetSubsystem<CAudioSubsystem>() )
	{
		Audio->Bind( *this );
		if ( Settings != nullptr )
		{
			Audio->SetMasterVolume( Settings->GetFloat( FString( "Audio/Master" ), 1.0f ) );
			Audio->SetBgmVolume( Settings->GetFloat( FString( "Audio/Bgm" ), 1.0f ) );
			Audio->SetSfxVolume( Settings->GetFloat( FString( "Audio/Sfx" ), 1.0f ) );
		}
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

	return MakeUnique<ABootScene>();
}

void CAcsFrameworkApp::OnStart() noexcept
{
	// GameInstance service の配線は InitialScene() 内で OnEnter より前に完了している。
	CGame::OnStart();
}

void CAcsFrameworkApp::OnUpdate( f32 DeltaSeconds ) noexcept
{
	CTimeSubsystem* const Time = GetSubsystem<CTimeSubsystem>();

	// 重ねているデバッグメニューはシーンより先に見る。出ている間はゲームを止めたいので、
	// シーンを進める前に決める必要がある。止め方は時間の担当へ任せ、ここでは
	// 「メニューが掴んでいる」ことだけを伝える (ポーズ画面など他の止め手と重なっても、
	// 片方を閉じただけで動き出さないようにするため)。
#if _DEBUG
	if ( CDebugTopOverlaySubsystem* const Overlay = GetSubsystem<CDebugTopOverlaySubsystem>() )
	{
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
	if ( Time == nullptr || Time->ShouldTickScenes() ) CGame::OnUpdate( DeltaSeconds );

	// 読み込みはシーンに属さない (遷移を跨いで続く) ので、アプリの側で進める。
	// ロード画面より先に進めること。同じフレームの進み具合が画面へ乗る。
	if ( CAssetLoaderSubsystem* const Loader = GetSubsystem<CAssetLoaderSubsystem>() )
	{
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
		Audio->Update( DeltaSeconds );
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
}

void CAcsFrameworkApp::OnRender() noexcept
{
	// 文字はこの層で焼いたものを使う。エンジン共有の UI フォントは漢字を焼いておらず
	// (「重」等が無言で消える)、そのうえシーンを描き終えると FRenderContext から取れなくなる。
	// ここで用意しておけば、シーンも幕も同じものを使える。
	if ( CUiFontSubsystem* const UiFont = GetSubsystem<CUiFontSubsystem>() ) m_UiFont = UiFont->Acquire( GetRenderer() );

	// 現 top シーンの描画は基底が行う。
	CGame::OnRender();

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

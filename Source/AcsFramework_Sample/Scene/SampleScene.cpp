#include "SampleScene.h"

#include "AcsFramework_Core/AcsFramework.h"
#include "AcsFramework_Sample/Scene/SampleModalScene.h"

#if _DEBUG
#include "AcsFramework_Sample/Scene/SampleDebugPage.h"
#include "Debug/DebugTop/DebugTopOverlaySubsystem.h"
#include "Debug/DebugTop/Settings/DebugTopSettings.h"
#endif

void ASampleScene::OnEnter() noexcept
{
	// シーンを跨いで残る状態。作り直しても消えないので、入った回数がそのまま積み上がる。
	if ( CAppStateSubsystem* const State = GetSubsystem<CAppStateSubsystem>() )
	{
		if ( FSampleAppState* const Shared = State->GetOrCreate<FSampleAppState>() )
		{
			++Shared->EnterCount;
			m_EnterCount = Shared->EnterCount;
		}
	}

	// 時計の知らせを受け取る。控えをメンバに持つので、このシーンが畳まれれば購読も外れる。
	if ( CEventSubsystem* const Events = GetSubsystem<CEventSubsystem>() )
	{
		m_TickSubscription = Events->Subscribe<FSampleTick, &ASampleScene::OnTickEvent>( this );
	}

	// 2 つの時計へ 1 秒ごとに仕掛ける。止めると片方だけが止まるので、違いが目で分かる。
	if ( CTimerSubsystem* const Timers = GetSubsystem<CTimerSubsystem>() )
	{
		Timers->CancelAll();
		Timers->Every( 1.0f, FSimpleDelegate::CreateRaw<&ASampleScene::OnGameTick>( this ) );
		Timers->EveryUnscaled( 1.0f, FSimpleDelegate::CreateRaw<&ASampleScene::OnRealTick>( this ) );
	}

	// 「この理由で止まっているときだけ幕を出す」と決めておく。止める側は幕を知らない。
	CPauseScreenSubsystem* const Pause = GetSubsystem<CPauseScreenSubsystem>();
	CTimeSubsystem* const Time = GetSubsystem<CTimeSubsystem>();
	if ( Pause != nullptr && Time != nullptr )
	{
		Pause->Follow( *Time, FString( kGamePauseReason ), FString( "PAUSED" ) );
	}

#if _DEBUG
	// デバッグメニューで設定した値をここで受け取る。保管庫はシーンを跨いで残るので、
	// 遷移してきた直後でも同じ値が読める。キーは行に付けた SetSaveKey と同じもの。
	// メニューを通らずに起動した場合は保管庫が空なので、既定値がそのまま使われる。
	const CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>();
	if ( Settings != nullptr )
	{
		m_MoveSpeed  = Settings->GetFloat( FString( "Gameplay/MoveSpeed" ), 1.0f );
		m_StartLevel = Settings->GetInt( FString( "Gameplay/StartLevel" ), 0 );

		ACS_LOG_INFO( "Sample: MoveSpeed=%.3f StartLevel=%d をデバッグメニューから受け取った", static_cast<double>( m_MoveSpeed ), m_StartLevel );
	}

	// 動かしたまま触れるメニューを重ねる。シーンとしてのメニュー (ADebugTopScene) と違い、
	// このシーンは生きたままなので、値をいじった結果がその場で見える。
	CDebugTopOverlaySubsystem* const Overlay = GetSubsystem<CDebugTopOverlaySubsystem>();
	if ( Overlay == nullptr ) return;

	ADebugTopHUD& HUD = Overlay->GetHUD();
	HUD.SetFontSize( 22.0f );
	HUD.SetDescription( FString( "F1 : 閉じる   上下キー : 選択   左右キー : 値を変更" ) );

	// 既に作ってあれば作り直さない (このシーンへ何度戻ってきても 1 つだけ)。
	if ( m_DebugPage == nullptr )
	{
		m_DebugPage = static_cast<ASampleDebugPage*>( HUD.AddEntity( NewObject<ASampleDebugPage>( "Sample", FSimpleDelegate::CreateRaw<&ASampleScene::PullDebugValues>( this ), GetSubsystem<CTimeSubsystem>(), GetSubsystem<CScreenSubsystem>(), FSimpleDelegate::CreateRaw<&ASampleScene::ReloadScene>( this ), GetSubsystem<CSceneTravelSubsystem>(), FSimpleDelegate::CreateRaw<&ASampleScene::WriteSave>( this ), FSimpleDelegate::CreateRaw<&ASampleScene::ReadSave>( this ), FSimpleDelegate::CreateRaw<&ASampleScene::EraseSave>( this ), GetSubsystem<CGameSettingsSubsystem>(), GetSubsystem<CAudioSubsystem>() ) ) );
	}
	PullDebugValues();
	RefreshSaveSummary();
#endif
}

#if _DEBUG
void ASampleScene::PullDebugValues() noexcept
{
	if ( m_DebugPage == nullptr ) return;

	m_MoveSpeed  = m_DebugPage->GetMoveSpeed();
	m_StartLevel = m_DebugPage->GetStartLevel();
	m_TextColor  = m_DebugPage->GetTextColor();
}

void ASampleScene::ReloadScene() noexcept
{
	// このシーンは畳まれ、購読も時計も一緒に落ちる。積み直した先で数え直しになる。
	if ( CSceneTravelSubsystem* const Travel = GetSubsystem<CSceneTravelSubsystem>() )
	{
		Travel->TravelTo( MakeUnique<ASampleScene>(), ESceneTransition::Cut );
	}
}

#endif

#if _DEBUG
void ASampleScene::WriteSave() noexcept
{
	CSaveSubsystem* const Save = GetSubsystem<CSaveSubsystem>();
	if ( Save == nullptr ) return;

	FSampleSave Data;
	Data.Stage = m_StartLevel;
	Data.Score = static_cast<i32>( m_SceneSeconds * 100.0f );
	Data.EnterCount = m_EnterCount;
	Save->Write( 0, Data );

	RefreshSaveSummary();
}

void ASampleScene::ReadSave() noexcept
{
	CSaveSubsystem* const Save = GetSubsystem<CSaveSubsystem>();
	if ( Save == nullptr ) return;

	FSampleSave Data;
	if ( !Save->Read( 0, Data ) )
	{
		m_SaveSummary = FString( "read failed (no data or version mismatch)" );
		return;
	}

	m_SaveSummary = FString();
	m_SaveSummary.AppendFormat( "read: Stage=%d Score=%d EnterCount=%d", Data.Stage, Data.Score, Data.EnterCount );
}

void ASampleScene::EraseSave() noexcept
{
	CSaveSubsystem* const Save = GetSubsystem<CSaveSubsystem>();
	if ( Save == nullptr ) return;

	Save->Erase( 0 );
	RefreshSaveSummary();
}

void ASampleScene::RefreshSaveSummary() noexcept
{
	CSaveSubsystem* const Save = GetSubsystem<CSaveSubsystem>();
	if ( Save == nullptr ) return;

	// 中身は読まずに見出しだけで一覧を組み立てる (タイトル画面がこの形で使う)。
	m_SaveSummary = FString();
	for ( i32 Index = 0; Index < Save->GetSlotCount(); ++Index )
	{
		const FSaveSlotInfo Info = Save->GetSlotInfo( Index );
		if ( Index > 0 ) m_SaveSummary.Append( "  " );

		if ( Info.bExists ) m_SaveSummary.AppendFormat( "[%d] v%u %llu B", Info.Index, Info.Version, static_cast<unsigned long long>( Info.SizeBytes ) );
		else                m_SaveSummary.AppendFormat( "[%d] empty", Info.Index );
	}
}

#endif

void ASampleScene::OnResume() noexcept
{
	// 重ねていた画面が返した答えを受け取る。誰が返したかは知らず、型だけで受ける。
	const CSampleModalResult* const Result = TravelContext<CSampleModalResult>();
	if ( Result == nullptr ) return;

	m_ModalResult = Result->bAccepted ? FString( "accepted" ) : FString( "cancelled" );
	m_ModalResult.AppendFormat( " (open #%d)", Result->OpenCount );
}

void ASampleScene::OnGameTick() noexcept
{
	// 数えるのは自分ではなく、知らせを受け取った側。出す側は誰が聞いているかを知らない。
	if ( CEventSubsystem* const Events = GetSubsystem<CEventSubsystem>() ) Events->Publish( FSampleTick{ true } );
}

void ASampleScene::OnRealTick() noexcept
{
	if ( CEventSubsystem* const Events = GetSubsystem<CEventSubsystem>() ) Events->Publish( FSampleTick{ false } );
}

void ASampleScene::OnTickEvent( const FSampleTick& Event ) noexcept
{
	if ( Event.bGameTime ) ++m_GameTicks;
	else                   ++m_RealTicks;
}

void ASampleScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	AScene::OnUpdate( DeltaSeconds );

	// 倍率が乗った経過秒をそのまま足す。止まっていればここへ来ないので増えない。
	m_SceneSeconds += DeltaSeconds;
}

void ASampleScene::OnDrawHud( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept
{
#if _DEBUG
	if ( !RenderContext.HasFont() ) return;

	// 受け取った値をそのまま出して、渡ってきていることを目で確かめられるようにする。
	FString Text( "Received from DebugTop:" );
	Text.AppendFormat( "\n  Gameplay/MoveSpeed  = %.3f", static_cast<double>( m_MoveSpeed ) );
	Text.AppendFormat( "\n  Gameplay/StartLevel = %d", m_StartLevel );

	// 時間の倍率の効き目を、目で追える形で出す。止めている間は増えない。
	Text.AppendFormat( "\n\nSceneTime = %.2f s", static_cast<double>( m_SceneSeconds ) );
	if ( const CTimeSubsystem* const Time = GetSubsystem<CTimeSubsystem>() )
	{
		Text.AppendFormat( "\n  Speed  = %.2f", static_cast<double>( Time->GetSpeed() ) );
		Text.Append( Time->IsPaused() ? "\n  Paused = yes" : "\n  Paused = no" );
		for ( usize Index = 0; Index < Time->GetPauseReasonCount(); ++Index )
		{
			Text.AppendFormat( "\n    by %s", Time->GetPauseReason( Index ).Data() );
		}
	}

	// 2 つの時計。数はイベントを経由して届いたもの (出す側と数える側は互いを知らない)。
	Text.AppendFormat( "\n\nTimer  game = %d   real = %d", m_GameTicks, m_RealTicks );
	if ( CEventSubsystem* const Events = GetSubsystem<CEventSubsystem>() )
	{
		Text.AppendFormat( "\n  Subscribers = %u", Events->GetSubscriberCount<FSampleTick>() );
	}

	// 窓とアプリの様子も出す。全画面へ切り替えると大きさが変わることが目で分かる。
	if ( const CScreenSubsystem* const Screen = GetSubsystem<CScreenSubsystem>() )
	{
		Text.AppendFormat( "\n\nScreen = %u x %u", Screen->GetWidth(), Screen->GetHeight() );
		Text.Append( Screen->IsFullscreen() ? "  (全画面)" : "  (窓)" );
	}
	if ( const CAppSubsystem* const App = GetSubsystem<CAppSubsystem>() )
	{
		Text.AppendFormat( "\n  FPS = %.0f", static_cast<double>( App->GetFps() ) );
	}

	// シーンを跨いで残っているもの。作り直すと SceneTime は 0 に戻るが、これは戻らない。
	Text.AppendFormat( "\n\nAppState  EnterCount = %d", m_EnterCount );
	if ( const CTimeSubsystem* const Time = GetSubsystem<CTimeSubsystem>() )
	{
		Text.AppendFormat( "   FixedStep = %.4f s", static_cast<double>( Time->GetFixedTimestep() ) );
	}

	// 音の一式。鳴らす素材が無くても、繋がっていることと音量は見える。
	if ( const CAudioSubsystem* const Audio = GetSubsystem<CAudioSubsystem>() )
	{
		Text.AppendFormat( "\nAudio  %s  master=%.2f bgm=%.2f sfx=%.2f", Audio->IsAudible() ? "ready" : "silent", static_cast<double>( Audio->GetMasterVolume() ), static_cast<double>( Audio->GetBgmVolume() ), static_cast<double>( Audio->GetSfxVolume() ) );
	}

	// 設定は次の起動でも残る。起動直後にこの値が既定でなければ、読み込めている証拠。
	if ( const CGameSettingsSubsystem* const Settings = GetSubsystem<CGameSettingsSubsystem>() )
	{
		Text.AppendFormat( "\nSettings  Audio/Bgm = %.2f%s", static_cast<double>( Settings->GetFloat( FString( "Audio/Bgm" ), 1.0f ) ), Settings->IsDirty() ? "  (未保存)" : "" );
	}

	// セーブの様子。中身を読まずに見出しだけで並べている。
	if ( !m_SaveSummary.IsEmpty() ) Text.AppendFormat( "\nSave  %s", m_SaveSummary.Data() );

	// 重ねた画面が返した答え。誰が返したかは知らず、型だけで受け取っている。
	if ( !m_ModalResult.IsEmpty() ) Text.AppendFormat( "\n\nModal result = %s", m_ModalResult.Data() );

	Text.Append( "\n\n  F1 : メニューを重ねる" );

	// 漢字を出したいので、この層で焼いたフォントを使う。エンジン共有のものは仮名までしか
	// 焼いておらず、漢字が無言で消える (「重ねる」が「ねる」になる)。
	CUiFontSubsystem* const UiFont = GetSubsystem<CUiFontSubsystem>();
	FFont* const Font = UiFont != nullptr ? UiFont->Peek() : nullptr;
	Batch.DrawString( Font != nullptr ? *Font : RenderContext.GetFont(), Text.Data(), 24.0f, 24.0f, m_TextColor );
#else
	(void)RenderContext;
	(void)Batch;
#endif
}

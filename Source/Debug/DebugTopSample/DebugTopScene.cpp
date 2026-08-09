#include "DebugTopScene.h"

#include <cstdlib>

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Fade/FadeSubsystem.h"
#include "AcsFramework_Core/Loading/LoadingScreenSubsystem.h"
#include "AcsFramework_Sample/Scene/SampleScene.h"
#include "Debug/DebugTop/Builtin/DebugTopAppearance.h"
#include "Debug/DebugTop/Builtin/DebugTopFavorites.h"
#include "Debug/DebugTopSample/Pages/RootEntity.h"
#include "Debug/DebugTop/Service/DebugTopSnapshot.h"
#include "Debug/DebugTop/Settings/DebugTopSettings.h"

namespace
{
	/** ロード画面の見本を出しておく秒数。 */
	constexpr f32 kLoadingDemoSeconds = 3.0f;
}


void ADebugTopScene::OnEnter() noexcept
{
	m_HUD = NewObject<ADebugTopHUD>();
	if ( !m_HUD ) return;

	// 文字サイズを指定すると、そのサイズ専用のアトラスを HUD が焼いて使う (漢字も出せる)。
	// 0 を指定するとエンジン共有フォント (18px・漢字なし) をそのまま使い、焼く時間もかからない。
	m_HUD->SetFontSize( 25.0f );

	// ページ側が説明文を持たないときの、メニュー共通のボタンヒント。
	m_HUD->SetDescription( FString( "上下キー / マウス移動 : 選択\n" "Enter / 左クリック : 決定\n" "Esc / 右クリック : 戻る\n" "/ : どこからでも検索 F / 左端の星 : お気に入り" ) );

	// 保存先 (置き場所・ファイル名・形式) を決める。
	if ( CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>() )
	{
		CDebugTopSettingsPath& Path = Settings->MutablePath();
		Path.SetDirectory( FString( kSampleSettingsDirectory ) );
		Path.SetFileName( FString( kSampleSettingsFileName ) );
		Path.SetFormat( kSampleSettingsFormat );
	}

	m_HUD->Build();

	// ルートは最初に置く (先頭のページが初期表示になる)。
	ADebugTopEntity* const Root = m_HUD->AddEntity( NewObject<ARootEntity>( "Entity1", FSimpleDelegate::CreateRaw<&ADebugTopScene::TravelToSampleWithFade>( this ), FSimpleDelegate::CreateRaw<&ADebugTopScene::TravelToSampleWithCut>( this ), FSimpleDelegate::CreateRaw<&ADebugTopScene::SaveSettings>( this ), FSimpleDelegate::CreateRaw<&ADebugTopScene::LoadSettings>( this ), FSimpleDelegate::CreateRaw<&ADebugTopScene::CopySnapshot>( this ), GetSubsystem<CDebugTopSettings>() ) );

	// メニュー全体を対象にするページは、ルートを積んだ後に足す (全ページが対象になる)。
	// 検索はページを持たず HUD に常設してあるので、ここには要らない。
	ADebugTopEntity* const Favorites = m_HUD->AddEntity( NewObject<ADebugTopFavoritesEntity>( "Favorites", *m_HUD ) );

	// ルートへ入口の行を足す。これが無いとページはあっても辿り着けない
	// (AddEntity は兄弟のページを増やすだけで、行としては現れないため)。
	if ( Root != nullptr && Favorites != nullptr )
	{
		Root->Add<CDebugTopElementEntityLink>( FString( "Favorites" ), FString( "F で留めた行を集める" ), *Root, *Favorites );
		Favorites->SetParentIfUnset( Root );
	}

	// メニュー自身の見た目を変えるページ。同梱の既製ページで、HUD へその場で効く。
	ADebugTopEntity* const Appearance = m_HUD->AddEntity( NewObject<ADebugTopAppearanceEntity>( "Appearance", *m_HUD ) );
	if ( Root != nullptr && Appearance != nullptr )
	{
		Root->Add<CDebugTopElementEntityLink>( FString( "Appearance" ), FString( "メニュー自身の見た目" ), *Root, *Appearance );
		Appearance->SetParentIfUnset( Root );
	}

	// 土台として用意した機能の見本。シーンを差し替えずに被せられることを確かめられる。
	if ( Root != nullptr )
	{
		CDebugTopElement* const Framework = Root->Add<CDebugTopElement>( "Framework", "土台の機能" );
		Framework->SetDescription( FString( "どのシーンからでも使えるものを並べています" ) );

		Framework->Add<CDebugTopElementAction>( "Loading", "ロード画面 (進捗なし)", FSimpleDelegate::CreateRaw<&ADebugTopScene::ShowLoadingDemo>( this ) )
			->SetDescription( FString( "スピナーだけを出します\nどれだけ掛かるか分からない待ちに使います" ) );

		Framework->Add<CDebugTopElementAction>( "LoadingProgress", "ロード画面 (進捗つき)", FSimpleDelegate::CreateRaw<&ADebugTopScene::ShowLoadingProgressDemo>( this ) )
			->SetDescription( FString( "割合が分かる待ちにはバーを出します" ) );

		Framework->Add<CDebugTopElementAction>( "Fade", "暗転して明転", FSimpleDelegate::CreateRaw<&ADebugTopScene::ShowFadeDemo>( this ) )
			->SetDescription( FString( "暗転しきったところで少し留まります\n実際はその間に重い切り替えを済ませます" ) );

		Framework->Add<CDebugTopElementAction>( "LoadingToggle", "ロード画面を出す / 消す", FSimpleDelegate::CreateRaw<&ADebugTopScene::ToggleLoading>( this ) )
			->SetDescription( FString( "決定するたびに出し入れが入れ替わります\n" "出しっぱなしにして見た目を確かめるとき用です" ) );

		Framework->Add<CDebugTopElementAction>( "LoadAssets", "アセットを非同期で読む", FSimpleDelegate::CreateRaw<&ADebugTopScene::LoadAssetsDemo>( this ) )
			->SetDescription( FString( "アセットの一覧を渡すだけで、裏で読みながらバーが進みます\n" "読み終わると自分で消えて、完了の通知を出します" ) );
	}

	// 前回の設定を復元する (初回はファイルが無いので何もしない)。
	LoadSettings();
}

void ADebugTopScene::SaveSettings() noexcept
{
	if ( !m_HUD ) return;

	// メニューの現在値を保管庫へ吸い出してからファイルへ落とす (保存先はログに出る)。
	CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>();
	if ( Settings == nullptr )
	{
		DebugTopNotifyError( FString( "保存できませんでした" ), FString( "設定の保管庫が見つかりません" ) );
		return;
	}

	Settings->CaptureFrom( *m_HUD );
	if ( !Settings->Save() )
	{
		DebugTopNotifyError( FString( "保存に失敗しました" ), Settings->GetPath().BuildAbsolute() )
			.AddButton( FString( "もう一度試す" ), FSimpleDelegate::CreateRaw<&ADebugTopScene::SaveSettings>( this ) )
			.AddButton( FString( "保存先を開く" ), FSimpleDelegate::CreateRaw<&ADebugTopScene::OpenSaveFolder>( this ) );
		return;
	}

	// 保存できたら、そのまま次の行動 (置き場所を見に行く) へつなげる。
	FString Count;
	Count.AppendFormat( "%zu 件 : ", Settings->GetAll().Num() );
	Count.Append( Settings->GetPath().BuildAbsolute().View() );

	DebugTopNotifySuccess( FString( "設定を保存しました" ), Count )
		.AddButton( FString( "フォルダを開く" ), FSimpleDelegate::CreateRaw<&ADebugTopScene::OpenSaveFolder>( this ) );
}

void ADebugTopScene::SyncSettingsIfChanged() noexcept
{
	if ( !m_HUD ) return;

	const u32 Version = CDebugTopElement::GetValueVersion();
	if ( Version == m_SyncedValueVersion ) return;

	m_SyncedValueVersion = Version;

	CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>();
	if ( Settings == nullptr ) return;

	Settings->CaptureFrom( *m_HUD );
}

void ADebugTopScene::CopySnapshot() noexcept
{
	if ( !m_HUD ) return;

	const FString Text = DebugTopMakeSnapshotText( *m_HUD );
	if ( !DebugTopCopyToClipboard( Text ) )
	{
		DebugTopNotifyError( FString( "クリップボードへ写せませんでした" ), FString( "他のアプリが掴んでいるかもしれません" ) );
		return;
	}

	// 何行ぶん写したかを添える。空でないことがその場で分かる。
	usize LineCount = 0;
	for ( usize Index = 0; Index < Text.Size(); ++Index )
	{
		if ( Text[Index] == '\n' ) ++LineCount;
	}

	FString Detail;
	Detail.AppendFormat( "%zu 行", LineCount );
	DebugTopNotifySuccess( FString( "現在値をクリップボードへ写しました" ), Detail );
}

void ADebugTopScene::OpenSaveFolder() noexcept
{
	CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>();
	if ( Settings == nullptr ) return;

	// 区切りは Windows 綴りにする。/ のままだと選択状態で開いてくれない。
	// 置換はパスだけに施す (コマンドごと舐めると /select, の / まで潰れる)。
	FString Path = Settings->GetPath().BuildAbsolute();
	for ( usize Index = 0; Index < Path.Size(); ++Index )
	{
		if ( Path[Index] == '/' ) Path[Index] = '\\';
	}

	// 保存したファイルを選んだ状態でエクスプローラーを開く。
	FString Command( "explorer.exe /select,\"" );
	Command.Append( Path.View() );
	Command.Append( "\"" );
	std::system( Command.Data() );
}

void ADebugTopScene::LoadSettings() noexcept
{
	if ( !m_HUD ) return;

	CDebugTopSettings* const Settings = GetSubsystem<CDebugTopSettings>();
	if ( Settings == nullptr ) return;

	if ( !Settings->Load() )
	{
		// まだ保存していないだけのこともあるので、失敗ではなく注意として出す。
		DebugTopNotifyWarning( FString( "読み込めるファイルがありません" ), Settings->GetPath().BuildAbsolute() );
		return;
	}

	Settings->ApplyTo( *m_HUD );

	FString Count;
	Count.AppendFormat( "%zu 件を読み込みました", Settings->GetAll().Num() );
	DebugTopNotify( FString( "設定を復元しました" ), Count );
}

void ADebugTopScene::OnTick( f32 DeltaSeconds ) noexcept
{
	if ( m_HUD )
	{
		m_HUD->Update( DeltaSeconds );
	}

	// 値が変わったフレームだけ、メニューの現在値を保管庫へ吸い上げる。こうしておくと
	// 保存を押さなくても、生きているサブシステムやシーンが最新の値を読める。
	// 版を見るので、変わっていないフレームはメニュー全体を走査しない。
	SyncSettingsIfChanged();

	// ロード画面の見本。実際に待つものが無いので、時間で進捗を作って自分で閉じる。
	if ( m_LoadingDemoLeft <= 0.0f ) return;

	m_LoadingDemoLeft -= DeltaSeconds;

	CLoadingScreenSubsystem* const Loading = GetSubsystem<CLoadingScreenSubsystem>();
	if ( Loading == nullptr ) return;

	if ( m_LoadingDemoLeft <= 0.0f )
	{
		m_LoadingDemoLeft = 0.0f;
		Loading->Disable();
		DebugTopNotifySuccess( FString( "読み込みが終わりました" ) );
		return;
	}

	// 進捗を出す指定のときだけ、残り時間から割合を作って渡す。
	if ( m_bLoadingDemoProgress )
	{
		Loading->SetProgress( 1.0f - m_LoadingDemoLeft / kLoadingDemoSeconds );
	}
}

void ADebugTopScene::ShowLoadingDemo() noexcept
{
	StartLoadingDemo( false );
}

void ADebugTopScene::ShowLoadingProgressDemo() noexcept
{
	StartLoadingDemo( true );
}

void ADebugTopScene::StartLoadingDemo( bool bWithProgress ) noexcept
{
	CLoadingScreenSubsystem* const Loading = GetSubsystem<CLoadingScreenSubsystem>();
	if ( Loading == nullptr ) return;

	m_LoadingDemoLeft = kLoadingDemoSeconds;
	m_bLoadingDemoProgress = bWithProgress;

	Loading->SetProgress( bWithProgress ? 0.0f : -1.0f );
	Loading->Show( bWithProgress ? FString( "読み込み中..." ) : FString( "しばらくお待ちください" ) );
}

void ADebugTopScene::ShowFadeDemo() noexcept
{
	CFadeSubsystem* const Fade = GetSubsystem<CFadeSubsystem>();
	if ( Fade == nullptr ) return;

	// 暗転しきったところで少し留める。実際はここで重い切り替えを済ませる。
	Fade->FadeOutIn( 0.35f, 0.35f, 0.25f );
}

void ADebugTopScene::ToggleLoading() noexcept
{
	CLoadingScreenSubsystem* const Loading = GetSubsystem<CLoadingScreenSubsystem>();
	if ( Loading == nullptr ) return;

	// 時間で閉じる見本が走っていると勝手に消えてしまうので、こちらへ切り替える。
	m_LoadingDemoLeft = 0.0f;

	Loading->SetMessage( FString( "出しっぱなしにしています" ) );
	Loading->SetProgress( -1.0f );
	Loading->Toggle();
}

void ADebugTopScene::LoadAssetsDemo() noexcept
{
	CAssetLoaderSubsystem* const Loader = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Loader == nullptr ) return;

	m_LoadingDemoLeft = 0.0f;

	// 読ませたいものを並べて渡すだけ。非同期に読むのは読み込み役の仕事。
	TArray<FString> Assets;
	Assets.Add( FString( "Assets/card.jpg" ) );
	Assets.Add( FString( "Assets/circle.png" ) );
	Assets.Add( FString( "Assets/loader.png" ) );
	Assets.Add( FString( "Assets/Suzanne.obj" ) );

	Loader->Begin( Assets, FSimpleDelegate::CreateRaw<&ADebugTopScene::OnAssetsLoaded>( this ) );

	// 待っている間に何を見せるかは、こちらが決める。読み込み側はこの行を知らない。
	if ( CLoadingScreenSubsystem* const Loading = GetSubsystem<CLoadingScreenSubsystem>() )
	{
		Loading->Follow( *Loader, FString( "アセットを読み込んでいます" ) );
	}
}

void ADebugTopScene::OnAssetsLoaded() noexcept
{
	const CAssetLoaderSubsystem* const Loader = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Loader != nullptr && Loader->HasFailed() )
	{
		DebugTopNotifyError( FString( "読み込めなかったアセットがあります" ), FString( "詳しくはログを見てください" ) );
		return;
	}

	DebugTopNotifySuccess( FString( "アセットを読み込みました" ) );
}

void ADebugTopScene::OnDrawHud( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept
{
	if ( m_HUD )
	{
		m_HUD->Draw( RenderContext, Batch );
	}
}

void ADebugTopScene::TravelToSampleWithFade() noexcept
{
	TravelToSample( ESceneTransition::Fade );
}

void ADebugTopScene::TravelToSampleWithCut() noexcept
{
	TravelToSample( ESceneTransition::Cut );
}

void ADebugTopScene::TravelToSample( ESceneTransition Transition ) noexcept
{
	// メニューで設定した値を保管庫へ移してから遷移する。保管庫はシーンを跨いで残るので、
	// 遷移先は GetSubsystem<CDebugTopSettings>() から同じ値を受け取れる。ファイルにも残すため、
	// 次回の起動時やビルドの外 (ツール等) からも同じ値を読める。
	SaveSettings();

	ACS_LOG_INFO( "Travel to ASampleScene (%s)", Transition == ESceneTransition::Fade ? "fade" : "cut" );

	CSceneTravelSubsystem* const Travel = GetSubsystem<CSceneTravelSubsystem>();
	if ( Travel == nullptr )
	{
		Scenes().ChangeScene( MakeUnique<ASampleScene>() );
		return;
	}

	// 幕を張るかどうかはここで決まる。Fade なら遷移先は明転のことを考えなくてよい。
	Travel->TravelTo( MakeUnique<ASampleScene>(), Transition );
}

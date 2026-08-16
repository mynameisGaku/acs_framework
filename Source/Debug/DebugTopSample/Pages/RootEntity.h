#pragma once

#include <acs.h>

#include <cmath>

#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"
#include "Debug/DebugTop/Settings/DebugTopSettings.h"
#include "Debug/DebugTop/Widget/DebugTopToastStack.h"
#include "Debug/DebugTopSample/Pages/DisplaySettingsEntity.h"
#include "Debug/DebugTopSample/Pages/LongListEntity.h"
#include "Debug/DebugTopSample/Pages/SampleProfile.h"
#include "Debug/DebugTopSample/Pages/SampleSettingsPath.h"
#include "Debug/DebugTopSample/Pages/TravelEntity.h"

using namespace acs;

// デバッグメニューのルートページ。行の型を一通り並べた見本になっている。

/**
 * デバッグメニューのルートページ。
 */
class ARootEntity : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name ページ名。
	 * @param OnTravelWithFade Travel ページへ渡す「暗転して切り替える」デリゲート。
	 * @param OnTravelWithCut Travel ページへ渡す「幕なしで切り替える」デリゲート。
	 * @param OnSaveSettings 設定を保存するデリゲート。
	 * @param OnLoadSettings 設定を読み直すデリゲート。
	 * @param OnCopySnapshot 現在値をクリップボードへ写すデリゲート。
	 */
	ARootEntity( const FString& Name, FSimpleDelegate OnTravelWithFade, FSimpleDelegate OnTravelWithCut, FSimpleDelegate OnSaveSettings, FSimpleDelegate OnLoadSettings, FSimpleDelegate OnCopySnapshot, CDebugTopSettings* Settings )
		: ADebugTopEntity( Name )
		, m_OnTravelWithFade( OnTravelWithFade )
		, m_OnTravelWithCut( OnTravelWithCut )
		, m_OnSaveSettings( OnSaveSettings )
		, m_OnLoadSettings( OnLoadSettings )
		, m_OnCopySnapshot( OnCopySnapshot )
		, m_Settings( Settings )
	{
	}

	/**
	 * 1 フレーム進める。
	 *
	 * @details グラフへ出すフレーム時間をここで控える (行の側からは経過秒が取れないため)。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept override
	{
		m_FrameSeconds = DeltaSeconds;
		ADebugTopEntity::Update( DeltaSeconds );
	}

protected:
	void OnBuild() noexcept override
	{
		// ページ固有の説明文。空にすると HUD 側に設定した共通の説明文が出る。
		SetDescription( FString( "ルートページ\n" "ホイール : スクロール\n" "左右キー / 矢印クリック : 値を変更" ) );

		CDebugTopElement* const Menu1 = Add<CDebugTopElement>( "Menu1", "SubTitle1" );
		Menu1->SetExpanded( true );

		CDebugTopElement* const ChildMenu1 = Menu1->Add<CDebugTopElement>( "ChildMenu1", "SubTitle2" );
		ChildMenu1->SetExpanded( true );

		// 保存キーを付けた行は、メニューのどこに置いても同じキーでゲーム側から受け取れる。
		ChildMenu1->Add<CDebugTopElementInt>( "IntValue", 0, -10, 10 )
			->SetSaveKey( FString( "Gameplay/StartLevel" ) );

		m_FloatValue = ChildMenu1->Add<CDebugTopElementFloat>( "FloatValue", 1.0f, 0.0f, 4.0f, 0.25f );
		m_FloatValue->SetSaveKey( FString( "Gameplay/MoveSpeed" ) );
		m_FloatValue->SetUnit( FString( "m/s" ) );
		m_FloatValue->SetDescription( FString( "左右キー / 矢印クリックで 0.25 ずつ\n" "欄をクリックすると数値を直接打ち込めます" ) );

		// 文字列の行。決定すると打ち込みが始まり、Enter で確定・Esc で取り消す。
		ChildMenu1->Add<CDebugTopElementString>( "PlayerName", FString( "Guest" ) )
			->SetDescription( FString( "クリックで打ち込み開始\n" "Enter か欄の外をクリック : 確定 / Esc : 取り消し\n" "IME 確定後の文字を受けるので日本語も打てます" ) );

		// パスの行。決定するとフォルダの選択ダイアログが開く。欄をクリックすれば手打ちもできる。
		ChildMenu1->Add<CDebugTopElementPath>( "ExportDir", FString( "Saved" ), EDebugTopPickKind::Folder )
			->SetDescription( FString( "Enter かダブルクリック : フォルダを選ぶ\n" "欄をクリックすると手で打ち込めます" ) );

		// 表示名をデリゲートで作る行。値をいじると左カラムの文字がその場で変わる。
		CDebugTopElement* const Status = ChildMenu1->Add<CDebugTopElement>( "Status" );
		Status->SetLabelProvider( FDebugTopTextDelegate::CreateRaw<&ARootEntity::MakeStatusLabel>( this ) );
		Status->SetTextColor( FVec4{ 0.55f, 0.85f, 0.95f, 1.0f } );

		// 候補を登録すると左右キーが候補の切り替えになり、右カラムとサブタイトルが Title になる。
		CDebugTopElementInt* const DataValue = ChildMenu1->Add<CDebugTopElementInt>( "DataValue", 0, 0, 0 );
		DataValue->AddData( "Title", 10 );
		DataValue->AddData( "Alpha", 20 );
		DataValue->AddData( "Beta", 30 );

		// 配列は 1 行にまとまり、展開すると [0] [1] [2] を個別に編集できる。
		TArray<i32> Offsets;
		Offsets.Add( 0 );
		Offsets.Add( 4 );
		Offsets.Add( 8 );
		ChildMenu1->Add<CDebugTopElementIntArray>( "IntArray", Move( Offsets ), -32, 32, 1 );

		// ベクトルは 1 行にまとまり、展開すると X / Y / Z を個別に詰められる。
		// 位置や速度のように「まとめて見たいが個別に触りたい」値のための行。
		m_SpawnPoint = ChildMenu1->Add<CDebugTopElementVec3>( "SpawnPoint", FVec3{ 0.0f, 1.5f, 0.0f }, -50.0f, 50.0f, 0.5f );
		m_SpawnPoint->SetUnit( FString( "m" ) );
		m_SpawnPoint->SetDescription( FString( "展開すると X / Y / Z を個別に編集できます\n" "畳んでいる間も右カラムに現在値が並びます" ) );

		// 値を見るだけの行。触れないので、いじってはいけないものを安心して並べられる。
		CDebugTopElementWatch* const SpawnDistance = ChildMenu1->Add<CDebugTopElementWatch>( "SpawnDistance", FDebugTopTextDelegate::CreateRaw<&ARootEntity::MakeSpawnDistanceText>( this ) );
		SpawnDistance->SetUnit( FString( "m" ) );
		SpawnDistance->SetWarnRange( 0.0f, 10.0f );
		SpawnDistance->SetDescription( FString( "SpawnPoint の原点からの距離\n" "毎フレーム取り直すので、上の値を動かすと即座に追従します" ) );

		// 開いた状態で始まり、決定キーでは畳めない子メニュー。
		CDebugTopElement* const ChildMenu2 = Menu1->Add<CDebugTopElement>( "ChildMenu2", "SubTitle3" );
		ChildMenu2->SetExpanded( true );
		ChildMenu2->SetExpandable( false );

		// 子を持たないがマーカーだけ出す行。
		ChildMenu2->Add<CDebugTopElement>( "Item1", "SubTitle4" )
			->SetMarkerVisibility( EDebugTopMarkerVisibility::Always );
		ChildMenu2->Add<CDebugTopElement>( "Item2", "SubTitle5" );

		// Entity を画面遷移させずに Menu1 の下へその場で展開する。
		AddChildEntity( NewObject<ADisplaySettingsEntity>( "ChildMenu3" ), "SubTitle6", EDebugTopAttachMode::Inline, Menu1 );

		// 色は展開すると彩度明度の面と色相帯が出る。閉じていても右カラムの見本で分かる。
		CDebugTopElementColor* const TintColor = ChildMenu1->Add<CDebugTopElementColor>( "TintColor", FVec4{ 0.9f, 0.4f, 0.2f, 1.0f } );
		TintColor->SetExpanded( true );
		TintColor->SetDescription( FString( "面をクリック / ドラッグで彩度と明度\n" "帯をクリック / ドラッグで色相\n" "R/G/B/A のスライダーで数値を詰められます" ) );

		// 値の移り変わりを折れ線で見る行。数字だけでは分からない揺れが見える。
		CDebugTopElementGraph* const FrameMs = ChildMenu1->Add<CDebugTopElementGraph>( "FrameMs", FDebugTopValueDelegate::CreateRaw<&ARootEntity::SampleFrameMs>( this ) );
		FrameMs->SetUnit( FString( "ms" ) );
		FrameMs->SetWarnRange( 0.0f, 16.6f );   // 60fps の予算を超えたら色で知らせる
		FrameMs->SetDescription( FString( "1 フレームにかかった時間 (ミリ秒)\n" "縦軸は溜まっている範囲へ自動で合わせます" ) );

		CDebugTopElement* const Menu2 = Add<CDebugTopElement>( "Menu2", "SubTitle7" );
		Menu2->SetTextColor( FVec4{ 0.95f, 0.60f, 0.55f, 1.0f } );
		Menu2->Add<CDebugTopElement>( "Item3", "SubTitle8" );

		// 設定の保存・読み直し。Travel 時にも自動で保存されるが、単体でも確かめられるようにする。
		CDebugTopElement* const Settings = Add<CDebugTopElement>( "Settings" );
		Settings->SetExpanded( true );
		Settings->SetDescription( FString( "メニューの値をファイルへ出し入れします\n" "保存先は Save を実行するとログに出ます" ) );

		// 出力形式は列挙型から選択肢を作る (列挙子を足せば選択肢も自動で増える)。
		const EDebugTopSettingsFormat CurrentFormat = m_Settings != nullptr
			? m_Settings->GetPath().GetFormat()
			: EDebugTopSettingsFormat::Text;
		m_Format = DebugTopAddEnumRow<EDebugTopSettingsFormat>( *Settings, "Format", CurrentFormat );
		m_Format->SetOnChanged( FSimpleDelegate::CreateRaw<&ARootEntity::ApplyFormat>( this ) );
		m_Format->SetDescription( FString( "保存する形式\n" "Text : 1 行 1 設定のテキスト (手で直せる)\n" "Json : 外部ツールから読ませる\n" "Binary : acs 独自形式 (.acsset)" ) );

		// プロファイル。切り替えると保存先のファイル名が変わるので、セットごとに独立する。
		m_Profile = DebugTopAddEnumRow<EDebugTopProfile>( *Settings, "Profile", EDebugTopProfile::Default );
		m_Profile->SetOnChanged( FSimpleDelegate::CreateRaw<&ARootEntity::ApplyProfile>( this ) );
		m_Profile->SetDescription( FString( "設定のセットを切り替えます\n" "切り替えると保存先のファイル名が変わるので\n" "セットごとに別々の値を持てます" ) );

		Settings->Add<CDebugTopElementAction>( "Save", "現在値をファイルへ書き出す", m_OnSaveSettings )
			->SetDescription( FString( "Enter かダブルクリックで実行\n" "いまのメニューの値を保存します\n" "書き出した絶対パスはログに出ます" ) );
		Settings->Add<CDebugTopElementAction>( "Reset", "このページの値を既定へ戻す", FSimpleDelegate::CreateRaw<&ARootEntity::ResetPage>( this ) )
			->SetDescription( FString( "Enter かダブルクリックで実行\n" "このページの値を構築時の状態へ戻します\n" "左端に帯が出ている行が既定値から変わっている行です" ) );
		CDebugTopElementAction* const CopyRow = Settings->Add<CDebugTopElementAction>( "Copy", "現在値をクリップボードへ写す", m_OnCopySnapshot );
		CopyRow->SetShortcut( EKey::C );
		CopyRow->SetDescription( FString( "不具合の報告へ貼るためのもの\n" "C キーでどのページからでも写せます\n" "畳んでいる行の値も全部入ります" ) );

		Settings->Add<CDebugTopElementAction>( "Load", "ファイルの値をメニューへ戻す", m_OnLoadSettings )
			->SetDescription( FString( "Enter かダブルクリックで実行\n" "保存済みのファイルを読み直します\n" "形式は中身から自動で見分けます" ) );

		// 通知の見本。種類ごとの見た目と、ボタンから次の行動へつなぐ形を確かめられる。
		CDebugTopElement* const Notify = Add<CDebugTopElement>( "Notify", "画面右下へ出す通知" );
		Notify->SetDescription( FString( "決定すると右下へ通知が出ます\n" "5 秒で消え、× でいつでも閉じられます\n" "同時に 3 件まで。溢れると古いものから消えます" ) );
		CDebugTopElementAction* const NotifyInfoRow = Notify->Add<CDebugTopElementAction>( "Info", "お知らせ", FSimpleDelegate::CreateRaw<&ARootEntity::NotifyInfo>( this ) );

		// キーを割り当てると、行まで辿らずどのページからでも叩ける。
		NotifyInfoRow->SetShortcut( EKey::N );
		NotifyInfoRow->SetDescription( FString( "N キーでどのページからでも実行できます\n" "行まで辿らずに叩きたい操作へ割り当てます" ) );
		Notify->Add<CDebugTopElementAction>( "Warning", "注意", FSimpleDelegate::CreateRaw<&ARootEntity::NotifyWarning>( this ) );
		Notify->Add<CDebugTopElementAction>( "Error", "失敗 (ボタン付き)", FSimpleDelegate::CreateRaw<&ARootEntity::NotifyError>( this ) );

		// 行数の多いページ (スクロールと端のフェード、続きの印を確認する)。
		AddChildEntity( NewObject<ALongListEntity>( "LongList" ), "40 行" );

		// こちらは画面ごと切り替える。行自体は子を持たないのでマーカーは付かない。
		AddChildEntity( NewObject<ATravelEntity>( "Travel", m_OnTravelWithFade, m_OnTravelWithCut ), "TravelToAnyScene" );
	}

private:
	/** お知らせの通知を出す (見本)。 */
	void NotifyInfo()
	{
		DebugTopNotify( FString( "お知らせ" ), FString( "これはただの通知です" ) );
	}

	/** 注意の通知を出す (見本)。 */
	void NotifyWarning()
	{
		DebugTopNotifyWarning( FString( "注意" ), FString( "気に留めてほしいことがあります" ) );
	}

	/**
	 * 失敗の通知を出す (見本)。
	 *
	 * @details ボタンから次の行動へつなげられることを、続けて通知を出して示す。
	 */
	void NotifyError()
	{
		DebugTopNotifyError( FString( "失敗しました" ), FString( "ボタンから次の行動へつなげます" ) )
			.AddButton( FString( "詳しく" ), FSimpleDelegate::CreateRaw<&ARootEntity::NotifyDetail>( this ) );
	}

	/** 失敗の通知のボタンから呼ばれる (ボタンが効いていることの確認用)。 */
	void NotifyDetail()
	{
		DebugTopNotify( FString( "ボタンが押されました" ), FString( "任意の処理をここへ結び付けられます" ) );
	}

	/**
	 * 折れ線へ出す値を返す (グラフの行から毎フレーム呼ばれる)。
	 *
	 * @return 直前のフレームにかかった時間 (ミリ秒)。
	 */
	f32 SampleFrameMs() const
	{
		return m_FrameSeconds * 1000.0f;
	}

	/**
	 * 表示名をその場で作る (SetLabelProvider から毎フレーム呼ばれる)。
	 *
	 * @return 監視中の行の現在値を埋め込んだ表示名。
	 */
	FString MakeStatusLabel() const
	{
		FString Text( "MoveSpeed is " );
		if ( m_FloatValue != nullptr )
		{
			const FString Value = DebugTopFormatValue( m_FloatValue->GetValue() );
			Text.Append( Value.View() );
		}
		return Text;
	}

	/**
	 * 監視行へ出す文字列を作る (毎フレーム呼ばれる)。
	 *
	 * @details ベクトル行の値をそのまま読んでいるので、上の行を動かすと即座に追従する。
	 * @return SpawnPoint の原点からの距離。
	 */
	FString MakeSpawnDistanceText() const
	{
		FString Text;
		if ( m_SpawnPoint == nullptr ) return Text;

		const FVec3 Point = m_SpawnPoint->GetValue();
		const f32 Distance = std::sqrt( Point.x * Point.x + Point.y * Point.y + Point.z * Point.z );
		Text.AppendFormat( "%.2f", static_cast<double>( Distance ) );
		return Text;
	}

	/** このページの値を既定へ戻す。 */
	void ResetPage() { ResetToDefaults(); }

	/** Profile 行で選ばれたセットを保存先のファイル名へ反映する。 */
	void ApplyProfile()
	{
		if ( m_Profile == nullptr || m_Settings == nullptr ) return;

		const EDebugTopProfile Profile = DebugTopGetEnumValue<EDebugTopProfile>( *m_Profile );

		// 既定のセットだけは元のファイル名のまま (今まで保存したものを読めるように)。
		FString FileName( kSampleSettingsFileName );
		if ( Profile != EDebugTopProfile::Default )
		{
			FileName.Append( "_" );
			FileName.Append( DebugTopToString( Profile ).ToString().View() );
		}
		m_Settings->MutablePath().SetFileName( FileName );
	}

	/** Format 行で選ばれた形式を保存先の設定へ反映する。 */
	void ApplyFormat()
	{
		if ( m_Format == nullptr || m_Settings == nullptr ) return;

		m_Settings->MutablePath().SetFormat( DebugTopGetEnumValue<EDebugTopSettingsFormat>( *m_Format ) );
	}

	/** Travel ページへ渡す「暗転して切り替える」デリゲート。 */
	FSimpleDelegate m_OnTravelWithFade;

	/** Travel ページへ渡す「幕なしで切り替える」デリゲート。 */
	FSimpleDelegate m_OnTravelWithCut;

	/** 設定を保存するデリゲート。 */
	FSimpleDelegate m_OnSaveSettings;

	/** 設定を読み直すデリゲート。 */
	FSimpleDelegate m_OnLoadSettings;

	/** 現在値をクリップボードへ写すデリゲート。 */
	FSimpleDelegate m_OnCopySnapshot;

	/** 表示名の材料にする行。所有はしない (このページが所有している)。 */
	CDebugTopElementFloat* m_FloatValue = nullptr;

	/** 監視行の材料にするベクトルの行。所有はしない (このページが所有している)。 */
	CDebugTopElementVec3* m_SpawnPoint = nullptr;

	/** 出力形式を選ぶ行。所有はしない (このページが所有している)。 */
	CDebugTopElementEnum* m_Format = nullptr;

	/** 直前のフレームにかかった秒 (グラフへ出すために控える)。 */
	f32 m_FrameSeconds = 0.0f;

	/** 設定のセットを選ぶ行。所有はしない (このページが所有している)。 */
	CDebugTopElementEnum* m_Profile = nullptr;

	/** 設定の保管庫。所有はしない (GameInstance スコープのサブシステム)。 */
	CDebugTopSettings* m_Settings = nullptr;
};

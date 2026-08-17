#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"
#include "AcsFramework_Core/Scene/SceneTravelSubsystem.h"
#include "AcsFramework_Core/Screen/ScreenSubsystem.h"
#include "AcsFramework_Core/Settings/GameSettingsSubsystem.h"
#include "AcsFramework_Core/Time/TimeSubsystem.h"
#include "AcsFramework_Sample/Scene/SampleModalScene.h"
#include "AcsFramework_Sample/Scene/SampleScene.h"

using namespace acs;

// ゲームを動かしたまま重ねて触るページ。値をいじった結果がその場で画面に出る。

/**
 * サンプルシーンの値を、重ねたメニューから触るページ。
 *
 * @details
 * 行はゲーム側の変数を直接持たず、値が変わったら知らせてもらってシーンへ押し込む
 * (行が生ポインタを握ると、シーンが先に死んだときに壊れるため)。
 */
class ASampleDebugPage : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param OnChanged 値が変わったときに呼ぶもの (シーンが現在値を取りに来る)。
	 * @param Time 時間の担当 (ページはワールドのノードではなく GetSubsystem を持たないため、
	 *             引ける側から渡してもらう)。
	 * @param Screen 窓の担当 (同じ理由で渡してもらう)。
	 * @param OnReload シーンを作り直すときに呼ぶもの。
	 * @param Travel 遷移の担当 (同じ理由で渡してもらう)。
	 * @param OnSaveWrite セーブへ書くときに呼ぶもの。
	 * @param OnSaveRead セーブから読むときに呼ぶもの。
	 * @param OnSaveErase セーブを消すときに呼ぶもの。
	 * @param Settings 設定の担当 (同じ理由で渡してもらう)。
	 * @param Audio 音の担当 (同じ理由で渡してもらう)。
	 */
	ASampleDebugPage( const FString& Name, FSimpleDelegate OnChanged, CTimeSubsystem* Time, CScreenSubsystem* Screen, FSimpleDelegate OnReload, CSceneTravelSubsystem* Travel, FSimpleDelegate OnSaveWrite, FSimpleDelegate OnSaveRead, FSimpleDelegate OnSaveErase, CGameSettingsSubsystem* Settings, CAudioSubsystem* Audio )
		: ADebugTopEntity( Name )
		, m_OnChanged( OnChanged )
		, m_OnReload( OnReload )
		, m_OnSaveWrite( OnSaveWrite )
		, m_OnSaveRead( OnSaveRead )
		, m_OnSaveErase( OnSaveErase )
		, m_Time( Time )
		, m_Screen( Screen )
		, m_Travel( Travel )
		, m_Settings( Settings )
		, m_Audio( Audio )
	{
	}

	/** 移動速度の現在値を返す。 */
	f32 GetMoveSpeed() const noexcept { return m_MoveSpeed != nullptr ? m_MoveSpeed->GetValue() : 1.0f; }

	/** 開始レベルの現在値を返す。 */
	i32 GetStartLevel() const noexcept { return m_StartLevel != nullptr ? m_StartLevel->GetValue() : 0; }

	/** 文字色の現在値を返す。 */
	FVec4 GetTextColor() const noexcept { return m_TextColor != nullptr ? m_TextColor->GetValue() : FVec4{ 1.0f, 1.0f, 1.0f, 1.0f }; }

protected:
	void OnBuild() noexcept override
	{
		SetHeader( "Sample (overlay)" );
		SetDescription( FString( "F1 で出し入れします\n" "出している間ゲームは止まります\n" "値を変えると下の表示がその場で変わります" ) );

		m_MoveSpeed = Add<CDebugTopElementFloat>( "MoveSpeed", 1.0f, 0.0f, 8.0f, 0.25f );
		m_MoveSpeed->SetUnit( FString( "m/s" ) );
		m_MoveSpeed->SetSaveKey( FString( "Gameplay/MoveSpeed" ) );
		m_MoveSpeed->SetWarnRange( 0.0f, 4.0f );
		m_MoveSpeed->SetOnChanged( m_OnChanged );

		m_StartLevel = Add<CDebugTopElementInt>( "StartLevel", 0, -10, 10 );
		m_StartLevel->SetSaveKey( FString( "Gameplay/StartLevel" ) );
		m_StartLevel->SetOnChanged( m_OnChanged );

		m_TextColor = Add<CDebugTopElementColor>( "TextColor", FVec4{ 0.95f, 0.95f, 0.75f, 1.0f } );
		m_TextColor->SetDescription( FString( "見本の文字色\n" "見本を押すと色を選ぶ面が出ます" ) );

		BuildTimeRows();
		BuildScreenRows();
		BuildStackRows();
		BuildSaveRows();
		BuildSettingsRows();
		BuildAudioRows();
	}

private:
	/**
	 * 窓を触る行を並べる。
	 *
	 * @details 窓の状態は CScreenSubsystem が持っている。ここは出し入れするだけ。
	 */
	void BuildScreenRows() noexcept
	{
		CDebugTopElement* const Group = Add<CDebugTopElement>( "Screen", "CScreenSubsystem" );
		Group->SetDescription( FString( "窓の見え方\n" "全画面にすると画面の大きさが変わります" ) );

		CDebugTopElementBool* const Full = Group->Add<CDebugTopElementBool>( "Fullscreen", false );
		Full->SetDescription( FString( "全画面と窓を入れ替えます" ) );
		Full->SetOnChanged( FSimpleDelegate::CreateRaw<&ASampleDebugPage::ApplyFullscreen>( this ) );
		m_Fullscreen = Full;

		CDebugTopElementAction* const Title = Group->Add<CDebugTopElementAction>( "SetTitle", "決定で見出しを変える", FSimpleDelegate::CreateRaw<&ASampleDebugPage::ApplyTitle>( this ) );
		Title->SetDescription( FString( "窓の見出しを差し替えます" ) );

		CDebugTopElementAction* const Reload = Add<CDebugTopElementAction>( "ReloadScene", "決定でシーンを作り直す", m_OnReload );
		Reload->SetDescription( FString( "シーンを畳んで積み直します\n" "購読が自動で外れていれば Subscribers は 1 のままです" ) );
	}

	/**
	 * 重ねる遷移を触る行を並べる。
	 *
	 * @details 積み下ろしは CSceneTravelSubsystem が持っている。ここは頼むだけ。
	 */
	void BuildStackRows() noexcept
	{
		CDebugTopElement* const Group = Add<CDebugTopElement>( "SceneStack", "CSceneTravelSubsystem" );
		Group->SetDescription( FString( "重ねる遷移\n" "下のシーンは畳まれず、止まって残ります" ) );

		CDebugTopElementAction* const PushCut = Group->Add<CDebugTopElementAction>( "PushCut", "幕なしで重ねる", FSimpleDelegate::CreateRaw<&ASampleDebugPage::PushModalCut>( this ) );
		PushCut->SetDescription( FString( "その場で重ねます" ) );

		CDebugTopElementAction* const PushFade = Group->Add<CDebugTopElementAction>( "PushFade", "暗転して重ねる", FSimpleDelegate::CreateRaw<&ASampleDebugPage::PushModalFade>( this ) );
		PushFade->SetDescription( FString( "暗転しきってから重ね、明転して戻します" ) );

		CDebugTopElementAction* const Pop = Group->Add<CDebugTopElementAction>( "Pop", "下ろす", FSimpleDelegate::CreateRaw<&ASampleDebugPage::PopModal>( this ) );
		Pop->SetDescription( FString( "1 枚下ろします\n" "1 枚しか積んでいなければ何も起きません" ) );
	}

	/**
	 * プレイヤーが決める設定を触る行を並べる。
	 *
	 * @details
	 * 値を持つのは CGameSettingsSubsystem。ここは出し入れするだけで、いつファイルへ書くかも
	 * 知らない (手が止まったら向こうが書く)。
	 */
	void BuildSettingsRows() noexcept
	{
		CDebugTopElement* const Group = Add<CDebugTopElement>( "GameSettings", "CGameSettingsSubsystem" );
		Group->SetDescription( FString( "プレイヤーが決める設定\n" "手が止まってから書かれ、次の起動でも残ります" ) );

		CDebugTopElementFloat* const Bgm = Group->Add<CDebugTopElementFloat>( "Audio/Bgm", 1.0f, 0.0f, 1.0f, 0.1f );
		Bgm->SetDescription( FString( "音量の例。動かすと 1 秒後に書かれます" ) );
		Bgm->SetOnChanged( FSimpleDelegate::CreateRaw<&ASampleDebugPage::ApplyBgmVolume>( this ) );
		m_BgmVolume = Bgm;
	}

	/**
	 * 音を触る行を並べる。
	 *
	 * @details 鳴らす素材はサンプルに置いていないので、名前と音量の «状態» だけが動く。
	 */
	void BuildAudioRows() noexcept
	{
		CDebugTopElement* const Group = Add<CDebugTopElement>( "Audio", "CAudioSubsystem" );
		Group->SetDescription( FString( "音の一式" ) );

		Group->Add<CDebugTopElementAction>( "PlayBgm", "決定で鳴らす", FSimpleDelegate::CreateRaw<&ASampleDebugPage::PlayBgm>( this ) );
		Group->Add<CDebugTopElementAction>( "StopBgm", "決定で止める", FSimpleDelegate::CreateRaw<&ASampleDebugPage::StopBgm>( this ) );
	}

	/** BGM を鳴らすよう頼む。 */
	void PlayBgm() noexcept
	{
		if ( m_Audio == nullptr ) return;

		m_Audio->PlayBgm( FString( "Assets/Bgm/Field.wav" ), 0.5f, true );
	}

	/** BGM を止めるよう頼む。 */
	void StopBgm() noexcept
	{
		if ( m_Audio == nullptr ) return;

		m_Audio->StopBgm( 0.3f );
	}

	/** 行の値を設定へ渡す。 */
	void ApplyBgmVolume() noexcept
	{
		if ( m_Settings == nullptr || m_BgmVolume == nullptr ) return;

		m_Settings->SetFloat( FString( "Audio/Bgm" ), m_BgmVolume->GetValue() );
	}

	/**
	 * セーブを触る行を並べる。
	 *
	 * @details 読み書きはシーンが行う (何を保存するかはシーンの都合なので)。
	 */
	void BuildSaveRows() noexcept
	{
		CDebugTopElement* const Group = Add<CDebugTopElement>( "Save", "CSaveSubsystem" );
		Group->SetDescription( FString( "枠ごとのセーブ" ) );

		Group->Add<CDebugTopElementAction>( "Write", "0 番へ書く", m_OnSaveWrite );
		Group->Add<CDebugTopElementAction>( "Read", "0 番から読む", m_OnSaveRead );
		Group->Add<CDebugTopElementAction>( "Erase", "0 番を消す", m_OnSaveErase );
	}

	/** 幕なしで重ねる。 */
	void PushModalCut() noexcept { PushModal( ESceneTransition::Cut ); }

	/** 暗転して重ねる。 */
	void PushModalFade() noexcept { PushModal( ESceneTransition::Fade ); }

	/**
	 * 何回目に開いたかを持たせて重ねる。
	 *
	 * @param Transition 見せ方。
	 */
	void PushModal( ESceneTransition Transition ) noexcept
	{
		if ( m_Travel == nullptr ) return;

		++m_ModalOpenCount;

		TUniquePtr<CSampleModalOpen> Open = MakeUnique<CSampleModalOpen>();
		Open->OpenCount = m_ModalOpenCount;
		m_Travel->PushScene( MakeUnique<ASampleModalScene>(), Move( Open ), Transition );
	}

	/** 重ねたものを下ろす。 */
	void PopModal() noexcept
	{
		if ( m_Travel == nullptr ) return;

		m_Travel->PopScene( ESceneTransition::Fade );
	}

	/** 行の値を窓へ渡す。 */
	void ApplyFullscreen() noexcept
	{
		if ( m_Screen == nullptr || m_Fullscreen == nullptr ) return;

		m_Screen->SetFullscreen( m_Fullscreen->GetValue() );
	}

	/** 窓の見出しを差し替える。 */
	void ApplyTitle() noexcept
	{
		if ( m_Screen == nullptr ) return;

		m_Screen->SetTitle( FString( "acs_framework - 見出しを変えた" ) );
	}

	/**
	 * 時間の進み方を触る行を並べる。
	 *
	 * @details
	 * 倍率も止め方も CTimeSubsystem が持っている。ここはその値を出し入れするだけで、
	 * 自分では時間を持たない。
	 */
	void BuildTimeRows() noexcept
	{
		CDebugTopElement* const Group = Add<CDebugTopElement>( "Time", "CTimeSubsystem" );
		Group->SetDescription( FString( "ゲームの時間の進み方\n" "メニューを出している間は DebugTop が止めているので、\n" "速さの効き目は閉じてから見えます" ) );

		CDebugTopElementFloat* const Speed = Group->Add<CDebugTopElementFloat>( "Speed", 1.0f, 0.0f, 4.0f, 0.25f );
		Speed->SetUnit( FString( "x" ) );
		Speed->SetDescription( FString( "1 で等速、0.25 で 1/4 の速さ、2 で倍速" ) );
		Speed->SetOnChanged( FSimpleDelegate::CreateRaw<&ASampleDebugPage::ApplySpeed>( this ) );
		m_Speed = Speed;

		CDebugTopElementAction* const Step = Group->Add<CDebugTopElementAction>( "StepOnce", "決定で 1 フレーム", FSimpleDelegate::CreateRaw<&ASampleDebugPage::RequestStep>( this ) );
		Step->SetDescription( FString( "止めたまま 1 フレームだけ進めます\n" "SceneTime が 1 回ぶんだけ増えます" ) );

		CDebugTopElementBool* const Paused = Group->Add<CDebugTopElementBool>( "PauseMenu", false );
		Paused->SetDescription( FString( "ゲーム側の理由で止めます\n" "メニューを閉じるとポーズの幕が出ます\n" "(デバッグメニューの理由では幕は出ません)" ) );
		Paused->SetOnChanged( FSimpleDelegate::CreateRaw<&ASampleDebugPage::ApplyGamePause>( this ) );
		m_GamePause = Paused;
	}

	/** ゲーム側の理由での停止を、時間の担当へ渡す。 */
	void ApplyGamePause() noexcept
	{
		if ( m_Time == nullptr || m_GamePause == nullptr ) return;

		if ( m_GamePause->GetValue() ) m_Time->Pause( FString( kGamePauseReason ) );
		else                           m_Time->Resume( FString( kGamePauseReason ) );
	}

	/** 行の値を時間の担当へ渡す。 */
	void ApplySpeed() noexcept
	{
		if ( m_Time == nullptr || m_Speed == nullptr ) return;

		m_Time->SetSpeed( m_Speed->GetValue() );
	}

	/** 止めたまま 1 フレームだけ進めるよう頼む。 */
	void RequestStep() noexcept
	{
		if ( m_Time == nullptr ) return;

		m_Time->StepOnce();
	}

	/** 値が変わったときに呼ぶもの。 */
	FSimpleDelegate m_OnChanged;

	/** シーンを作り直すときに呼ぶもの。 */
	FSimpleDelegate m_OnReload;

	/** セーブへ書くときに呼ぶもの。 */
	FSimpleDelegate m_OnSaveWrite;

	/** セーブから読むときに呼ぶもの。 */
	FSimpleDelegate m_OnSaveRead;

	/** セーブを消すときに呼ぶもの。 */
	FSimpleDelegate m_OnSaveErase;

	/** 移動速度の行。所有はしない (このページが所有している)。 */
	CDebugTopElementFloat* m_MoveSpeed = nullptr;

	/** 開始レベルの行。所有はしない。 */
	CDebugTopElementInt* m_StartLevel = nullptr;

	/** 文字色の行。所有はしない。 */
	CDebugTopElementColor* m_TextColor = nullptr;

	/** 時間の速さの行。所有はしない。 */
	CDebugTopElementFloat* m_Speed = nullptr;

	/** ゲーム側の理由で止める行。所有はしない。 */
	CDebugTopElementBool* m_GamePause = nullptr;

	/** 全画面の行。所有はしない。 */
	CDebugTopElementBool* m_Fullscreen = nullptr;

	/** 時間の担当。所有はしない (GameInstance の間ずっと生きている)。 */
	CTimeSubsystem* m_Time = nullptr;

	/** 窓の担当。所有はしない。 */
	CScreenSubsystem* m_Screen = nullptr;

	/** 遷移の担当。所有はしない。 */
	CSceneTravelSubsystem* m_Travel = nullptr;

	/** 設定の担当。所有はしない。 */
	CGameSettingsSubsystem* m_Settings = nullptr;

	/** 音量の行。所有はしない。 */
	CDebugTopElementFloat* m_BgmVolume = nullptr;

	/** 音の担当。所有はしない。 */
	CAudioSubsystem* m_Audio = nullptr;

	/** 重ねた回数 (持たせるものへ載せる)。 */
	i32 m_ModalOpenCount = 0;
};

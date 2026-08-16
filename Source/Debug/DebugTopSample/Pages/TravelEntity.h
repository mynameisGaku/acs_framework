#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

// 遷移先を並べるページ。幕の有無を選んで移動できる。

/**
 * 遷移先を並べるページ。
 *
 * @details 項目が増える想定なので EDebugTopAttachMode::Page で画面ごと切り替える。
 */
class ATravelEntity : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name ページ名。
	 * @param OnTravelToSample サンプルシーンへ遷移するデリゲート。
	 */
	ATravelEntity( const FString& Name, FSimpleDelegate OnTravelWithFade, FSimpleDelegate OnTravelWithCut )
		: ADebugTopEntity( Name )
		, m_OnTravelWithFade( OnTravelWithFade )
		, m_OnTravelWithCut( OnTravelWithCut )
	{
	}

protected:
	void OnBuild() noexcept override
	{
		SetHeader( "Select a scene to travel" );

		// 説明文はページごとに差し替わる。ここでは遷移ページ用の注意書きを出す。
		SetDescription( FString( "決定したシーンへ移動します\n" "Enter / 左クリック : 移動\n" "Esc / 右クリック : 戻る" ) );

		Add<CDebugTopElementAction>( "SampleFade", "ASampleScene (暗転して切替)", m_OnTravelWithFade )
			->SetDescription( FString( "暗転しきったところで切り替わり、明転もエンジンが戻します\n" "遷移先はフェード明けのコードを書かなくて済みます" ) );

		Add<CDebugTopElementAction>( "SampleCut", "ASampleScene (幕なしで切替)", m_OnTravelWithCut )
			->SetDescription( FString( "幕を使わずその場で切り替えます\n" "見せ方を遷移先が自分で作りたいとき用です" ) );
	}

private:
	/** 暗転して切り替えるデリゲート。 */
	FSimpleDelegate m_OnTravelWithFade;

	/** 幕なしで切り替えるデリゲート。 */
	FSimpleDelegate m_OnTravelWithCut;
};

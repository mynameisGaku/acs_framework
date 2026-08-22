// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Interaction3D/InteractionHighlight3DParams.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"

using namespace acs;
using namespace acs::game;

/**
 * 遊ぶ人向けUIを3D場面の寿命と描画順へ自動で接続する基底場面。
 *
 * @details
	 * 派生側は `Ui().AddButton(...)`、`Ui().AddText(...)`、`WorldLabels().AddNodeLabel(...)`を
	 * 呼ぶだけでよい。UIの初期化、
 * 入力配送、毎フレーム更新、終了処理、ポスト処理後のHUD描画はこの基底が受け持つ。
 * より複雑な画面ではACSの`AWidget`ツリーを直接利用できる。
 */
class AUi3DScene : public ALegacyScene3DAdapter
{
public:
	/** 空のUI層を持つ3D場面を作る。 */
	AUi3DScene() noexcept = default;

	/** UI層と3D場面を破棄する。実際の終了処理はOnExitで行う。 */
	~AUi3DScene() noexcept override = default;

	/** 場面固有のUI状態を重複所有しないためコピーを禁止する。 */
	AUi3DScene( const AUi3DScene& ) = delete;

	/** 場面固有のUI状態を重複所有しないためコピー代入を禁止する。 */
	AUi3DScene& operator=( const AUi3DScene& ) = delete;

	/**
	 * ボタンや文字を追加するUI層を返す。
	 *
	 * @return この場面が所有するUI層。
	 */
	CUiLayer& Ui() noexcept { return m_Ui; }

	/**
	 * 読み取り専用のUI層を返す。
	 *
	 * @return この場面が所有するUI層。
	 */
	const CUiLayer& Ui() const noexcept { return m_Ui; }

	/**
	 * 3Dノードまたはworld位置へ追従する文字を追加するレイヤーを返す。
	 *
	 * @return この場面が所有し、場面グラフへ接続済みのワールドラベルレイヤー。
	 */
	CWorldLabel3DLayer& WorldLabels() noexcept { return m_WorldLabels; }

	/**
	 * 読み取り専用のワールドラベルレイヤーを返す。
	 *
	 * @return この場面が所有するワールドラベルレイヤー。
	 */
	const CWorldLabel3DLayer& WorldLabels() const noexcept { return m_WorldLabels; }

	/**
	 * 登録対象の視線判定と操作案内を扱う3Dインタラクション窓口を返す。
	 *
	 * @return この場面のグラフとワールドラベルへ接続済みのアダプター。
	 */
	CInteractionFocus3D& InteractionFocus() noexcept { return m_InteractionFocus; }

	/** 読み取り専用の3Dインタラクション窓口を返す。 */
	const CInteractionFocus3D& InteractionFocus() const noexcept { return m_InteractionFocus; }

	/**
	 * 現在カメラから視線判定を1回行い、対象出入りと決定を返す。
	 *
	 * @param bActivateRequested 今回捉えた対象へ決定操作を要求するならtrue。
	 * @return 更新前後の対象と、成立した決定対象。
	 */
	FInteractionFocus3DUpdateResult UpdateInteractionFocus( bool bActivateRequested = false ) noexcept;

	/**
	 * 視線判定と同じ画面位置へ描く照準の色と寸法を返す。
	 *
	 * @return この場面が所有する照準設定。対象登録が0件なら設定にかかわらず描かない。
	 */
	FInteractionReticle3DParams& InteractionReticleParams() noexcept { return m_InteractionReticleParams; }

	/** 読み取り専用の3Dインタラクション照準設定を返す。 */
	const FInteractionReticle3DParams& InteractionReticleParams() const noexcept { return m_InteractionReticleParams; }

	/**
	 * 視線で捉えた3D対象へ自動で重ねる選択輪郭設定を返す。
	 *
	 * @return この場面が所有し、描画直前にACSへ同期する輪郭設定。
	 */
	FInteractionHighlight3DParams& InteractionHighlightParams() noexcept { return m_InteractionHighlightParams; }

	/** 読み取り専用の3Dインタラクション選択輪郭設定を返す。 */
	const FInteractionHighlight3DParams& InteractionHighlightParams() const noexcept { return m_InteractionHighlightParams; }

	/** 通常の3D場面を開始してからUI層を初期化する。 */
	void OnEnter() noexcept override;

	/** UI層を終了してから通常の3D場面を終了する。 */
	void OnExit() noexcept override;

	/** 通常の3D場面とUI層を同じ明示秒で更新する。 */
	void OnUpdate( f32 DeltaSeconds ) noexcept override;

	/** 現在の視線対象を選択輪郭へ同期してから通常の3D場面を描く。 */
	void OnRender( FRenderContext& Context ) noexcept override;

	/**
	 * ウィンドウ入力をUI層と通常の場面へ配送する。
	 *
	 * @param Event 配送された入力イベント。
	 */
	void OnEvent( const FEvent& Event ) noexcept override;

protected:
	/**
	 * ポスト処理後のHUDパスへUI層を描く。
	 *
	 * @param Context 現フレームの描画先と共有フォント。
	 * @param Sprites 開かれているHUD用スプライトバッチ。
	 */
	void OnDrawHud( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept override;

private:
	/** 有効な視線対象と輪郭設定だけをACSの選択マスクへ同期する。 */
	void SyncInteractionHighlight_Internal() noexcept;

	/** 視線判定と同じ正規化画面位置へ、対象状態に応じた照準を描く。 */
	void DrawInteractionReticle_Internal( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept;

	/** この場面のノード位置をHUDへ射影するワールドラベルレイヤー。 */
	CWorldLabel3DLayer m_WorldLabels;

	/** 実形状ピックと操作案内を接続する、この場面所有の視線フォーカス。 */
	CInteractionFocus3D m_InteractionFocus;

	/** 視線位置へ重ねる照準の色とpixel寸法。 */
	FInteractionReticle3DParams m_InteractionReticleParams;

	/** 視線で捉えたメッシュ部分木へ重ねる選択輪郭設定。 */
	FInteractionHighlight3DParams m_InteractionHighlightParams;

	/** ACSへ最後に反映できた選択輪郭設定。不要な全ノード走査を避ける比較元。 */
	FInteractionHighlight3DParams m_AppliedInteractionHighlightParams;

	/** ACSへ最後に反映できた選択部分木の根。 */
	FNodeId m_AppliedInteractionHighlightNode;

	/** 選択輪郭の印をACSへ反映中ならtrue。 */
	bool m_bInteractionHighlightApplied = false;

	/** この場面だけが所有する遊ぶ人向けUI層。 */
	CUiLayer m_Ui;
};

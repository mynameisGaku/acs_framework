// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DLayer.h"
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DLayer.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionHighlight3DParams.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"

using namespace acs;
using namespace acs::game;

struct FAnimatedModel3DSpawnParams;

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
	 * 3D画像板を現在カメラへ向け続ける追従レイヤーを返す。
	 *
	 * @return この場面のグラフへ接続済みのビルボードレイヤー。
	 */
	CBillboard3DLayer& Billboards() noexcept { return m_Billboards; }

	/** 読み取り専用のビルボードレイヤーを返す。 */
	const CBillboard3DLayer& Billboards() const noexcept { return m_Billboards; }

	/**
	 * 画像名からカメラ追従の3D画像板を1回で生成する。
	 *
	 * @param Params 画像名、位置、大きさ、ノード名。
	 * @param Mode 上下も追うか、worldのY軸を保つか。
	 * @param RollDegrees 正面軸まわりへ加える度数。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成して追従へ加えたノード。画像読込や登録に失敗したらnullptr。
	 */
	ANode* SpawnBillboard3D( const FSprite3DSpawnParams& Params,
		EBillboard3DMode Mode = EBillboard3DMode::FaceCamera, f32 RollDegrees = 0.0f,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 骨付き3Dモデルを画像板と同じく、読み込みから初期再生まで1回で場面へ置く。
	 *
	 * @param Params 骨付きモデルの相対path、位置、大きさ、初期animation。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いたノード。asset窓口、読み込み、検証のいずれかに失敗したらnullptr。
	 */
	ANode* SpawnAnimatedModel3D( const FAnimatedModel3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

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

	/**
	 * world座標の線を次の3D描画へ1本登録する。
	 *
	 * @details 線は深度を無視して常に見える。表示を続ける場合は更新ごとに呼ぶ。
	 * @return 値が有効で1フレーム上限内に登録できたらtrue。
	 */
	bool DrawLine3D( FVec3 Start, FVec3 End, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/**
	 * 軸並行境界箱の12辺を次の3D描画へ一括登録する。
	 *
	 * @details 線は深度を無視して常に見える。表示を続ける場合は更新ごとに呼ぶ。
	 * @return 箱と色が有効で、12辺全てを登録できたらtrue。
	 */
	bool DrawAabb3D( const FAabb3& Bounds, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/**
	 * 球を3方向の円として次の3D描画へ一括登録する。
	 *
	 * @details 線は深度を無視して常に見える。表示を続ける場合は更新ごとに呼ぶ。
	 * @return 球、色、分割数が有効で、全ての線を登録できたらtrue。
	 */
	bool DrawSphere3D( const FSphere& Sphere, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

	/** 登録線の消去と拒否数の確認に使う、この場面所有の3Dデバッグ描画層を返す。 */
	CDebugDraw3DLayer& DebugDraw3D() noexcept { return m_DebugDraw3D; }

	/** 読み取り専用の3Dデバッグ描画層を返す。 */
	const CDebugDraw3DLayer& DebugDraw3D() const noexcept { return m_DebugDraw3D; }

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
	 * 登録済みの3Dデバッグ線をACSのHDR透明3Dパスへ描く。
	 *
	 * @return RHI commandだけを追加するため、外部状態復旧が不要なfalse。
	 */
	bool OnRenderTransparent3D( const FScene3DTransparentRenderContext& Context ) noexcept override;

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

	/** 3D画像板の世代付き識別子とカメラ追従を場面寿命で所有する層。 */
	CBillboard3DLayer m_Billboards;

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

	/** 1フレーム線キューとACSのGPU描画器を場面寿命で所有する層。 */
	CDebugDraw3DLayer m_DebugDraw3D;

	/** この場面だけが所有する遊ぶ人向けUI層。 */
	CUiLayer m_Ui;
};

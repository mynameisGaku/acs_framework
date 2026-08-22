// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DLayer.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DLayer.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionHighlight3DParams.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRay.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRayHit.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"

using namespace acs;
using namespace acs::game;

struct FAnimatedModel3DSpawnParams;
struct FLight3DSpawnParams;
struct FModel3DSpawnParams;
struct FSpatialPlayRequest;
struct FSprite3DSpawnParams;
struct FWater3DSpawnParams;
class CThirdPersonCharacter3D;

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
	/** 場面グラフへ接続済みの衝突集合と空のUI層を持つ3D場面を作る。 */
	AUi3DScene() noexcept;

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
	 * この場面の3Dノードへ衝突形状を登録して問い合わせる集合を返す。
	 *
	 * @return 場面グラフへ接続済みで、場面終了時に自動消去する衝突集合。
	 */
	CSceneCollision3D& Collision3D() noexcept { return m_Collision3D; }

	/** 読み取り専用の3D衝突集合を返す。 */
	const CSceneCollision3D& Collision3D() const noexcept { return m_Collision3D; }

	/**
	 * 第三者視点キャラクターを、この場面の衝突集合、カメラ、対象ノードへ接続する。
	 *
	 * @param Controller 呼出側が所有する第三者視点キャラクター制御。
	 * @param Character この場面が所有する移動・追従対象ノード。
	 * @param Params 移動、向き、衝突、追従カメラ設定。
	 * @return 未接続の制御を有効な自場面ノードへ完全に接続できたらtrue。
	 */
	bool BindThirdPersonCharacter3D( CThirdPersonCharacter3D& Controller, ANode& Character,
		const FThirdPersonCharacter3DParams& Params = FThirdPersonCharacter3DParams{} ) noexcept;

	/**
	 * 静的モデルの生成、自己衝突登録、第三者視点操作への接続を1回で完了する。
	 *
	 * @param Controller 呼出側が所有する未接続のキャラクター制御。
	 * @param ModelParams プリミティブまたは静的モデルの配置と見た目。
	 * @param SpawnParams 自己形状、移動、向き、追従カメラの設定。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 必須処理を全て完了したノードと自己形状。失敗時は空で、半端な生成物を残さない。
	 */
	FThirdPersonCharacter3DSpawnResult SpawnThirdPersonCharacter3D(
		CThirdPersonCharacter3D& Controller, const FModel3DSpawnParams& ModelParams,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams = FThirdPersonCharacter3DSpawnParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 骨格モデルの生成、自己衝突登録、第三者視点操作への接続を1回で完了する。
	 *
	 * @details 移動連動アニメーションだけ接続できない場合も必須処理は成功とし、
	 * 結果の`bAnimationBound`をfalseにしてモデル側の初期再生を保つ。
	 * @param Controller 呼出側が所有する未接続のキャラクター制御。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param SpawnParams 自己形状、操作、任意アニメーションの設定。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 必須処理を全て完了したノードと自己形状。失敗時は空で、半端な生成物を残さない。
	 */
	FThirdPersonCharacter3DSpawnResult SpawnThirdPersonCharacter3D(
		CThirdPersonCharacter3D& Controller, const FAnimatedModel3DSpawnParams& ModelParams,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams = FThirdPersonCharacter3DSpawnParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した第三者視点キャラクターを、操作と自己形状を残さず破棄する。
	 *
	 * @param Controller 生成時に接続した、または既に解除済みのキャラクター制御。
	 * @param Character `SpawnThirdPersonCharacter3D`の結果。成功時は空の結果へ置き換える。
	 * @return 自場面の有効な生成結果を破棄予定にして全接続を外せたらtrue。
	 */
	bool DestroyThirdPersonCharacter3D( CThirdPersonCharacter3D& Controller,
		FThirdPersonCharacter3DSpawnResult& Character ) noexcept;

	/**
	 * 左上を0、右下を1とした画面位置から、現在カメラを通る3D判定線を作る。
	 *
	 * @param NormalizedScreenPosition 左上が(0, 0)、右下が(1, 1)の画面位置。
	 * @param MaximumDistance 線が届くworld距離。
	 * @return 現在カメラから伸びる線。位置または距離が無効なら`IsValid()`がfalseの線。
	 */
	FSceneRay MakeScreenRay3D( FVec2 NormalizedScreenPosition,
		f32 MaximumDistance = 1000.0f ) noexcept;

	/**
	 * この場面で実際に描かれる3D形状へ有限の線を当てる。
	 *
	 * @param Ray world座標の始点、正規化済みの向き、届く距離。
	 * @return 最前面のノード、距離、world位置、実表面法線。外れた場合は`IsHit()`がfalse。
	 */
	FSceneRayHit Raycast3D( const FSceneRay& Ray ) noexcept;

	/**
	 * 現在カメラの画面位置から、この場面の最前面にある実形状を1回で選ぶ。
	 *
	 * @param NormalizedScreenPosition 左上が(0, 0)、右下が(1, 1)の画面位置。
	 * @param MaximumDistance 判定が届くworld距離。
	 * @return 最前面の命中結果。入力が無効または外れた場合は`IsHit()`がfalse。
	 */
	FSceneRayHit PickScreen3D( FVec2 NormalizedScreenPosition,
		f32 MaximumDistance = 1000.0f ) noexcept;

	/**
	 * 複数の3Dモデルをまとめて動かすための空ノードを場面へ置く。
	 *
	 * @param Name デバッグと実行中の識別に使う任意名。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 場面の世代付き識別子を持つ空ノード。親または生成に失敗したらnullptr。
	 */
	ANode* SpawnNode3D( FStringView Name = FStringView{}, ANode* Parent = nullptr ) noexcept;

	/**
	 * プリミティブまたは静的3Dモデルを、必要な読み込みを含めて1回で場面へ置く。
	 *
	 * @param Params 形またはモデル名、位置、材質、ノード名。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いたノード。入力、asset窓口、読み込みのいずれかに失敗したらnullptr。
	 */
	ANode* SpawnModel3D( const FModel3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 静的3Dモデルの生成と視線フォーカス対象登録を1回で完了する。
	 *
	 * @param Params 形またはモデル名、位置、材質、ノード名。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	ANode* SpawnInteractableModel3D( const FModel3DSpawnParams& Params,
		FStringView Prompt, FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * `SpawnInteractableModel3D`または骨付き版で生成した操作対象を安全に破棄する。
	 *
	 * @param Model この場面が所有する一括生成モデル。成功時はnullptrになる。
	 * @return 有効な自場面モデルを破棄予定にして操作対象を直ちに外せたらtrue。
	 */
	bool DestroyInteractableModel3D( ANode*& Model ) noexcept;

	/**
	 * 静的3Dモデルの生成、衝突登録、視線フォーカス対象登録を1回で完了する。
	 *
	 * @param Params 形またはモデル名、位置、材質、ノード名。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param CollisionParams 登録形状と衝突レイヤー。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成と2登録を完了したノードと形状。失敗時は空で、半端な生成物を残さない。
	 */
	FCollidableModel3DSpawnResult SpawnInteractableCollidableModel3D(
		const FModel3DSpawnParams& Params, FStringView Prompt,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 衝突付き操作対象モデルを、操作登録と衝突形状ごと安全に破棄する。
	 *
	 * @param Model この場面が所有する一括生成結果。成功時は空の結果になる。
	 * @return 有効な自場面モデルを破棄予定にして2登録を直ちに外せたらtrue。
	 */
	bool DestroyInteractableCollidableModel3D(
		FCollidableModel3DSpawnResult& Model ) noexcept;

	/**
	 * 静的3Dモデルの生成と衝突登録を1回で完了する。
	 *
	 * @details 衝突登録に失敗した場合は生成ノードも破棄予定へ戻す。既定は描画境界を使う。
	 * 厚さのない平面や独自の移動形状には`FCollisionShape3DParams::FromBox`または
	 * `FromSphere`で明示的な形を渡す。
	 * @param Params 形またはモデル名、位置、材質、ノード名。
	 * @param CollisionParams 登録形状と衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いたノードと形状番号。読込、生成、衝突登録のいずれかに失敗したら空の結果。
	 */
	FCollidableModel3DSpawnResult SpawnCollidableModel3D( const FModel3DSpawnParams& Params,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 画像名から向き固定の3D画像板を、必要な読み込みを含めて1回で場面へ置く。
	 *
	 * @param Params 画像名、位置、向き、大きさ、ノード名。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いたノード。入力、asset窓口、読み込みのいずれかに失敗したらnullptr。
	 */
	ANode* SpawnImage3D( const FSprite3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

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
	 * 骨付き3Dモデルの読込、初期再生、視線フォーカス対象登録を1回で完了する。
	 *
	 * @param Params 骨付きモデルの相対path、位置、大きさ、初期animation。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成、再生、対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	ANode* SpawnInteractableAnimatedModel3D(
		const FAnimatedModel3DSpawnParams& Params, FStringView Prompt,
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 骨付き3Dモデルの読込、初期再生、衝突登録、視線対象登録を1回で完了する。
	 *
	 * @param Params 骨付きモデルの相対path、位置、大きさ、初期animation。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param CollisionParams 登録形状と衝突レイヤー。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成、再生、2登録を完了したノードと形状。失敗時は空で、半端な生成物を残さない。
	 */
	FCollidableModel3DSpawnResult SpawnInteractableCollidableAnimatedModel3D(
		const FAnimatedModel3DSpawnParams& Params, FStringView Prompt,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 骨付き3Dモデルの読込、初期再生、衝突登録を1回で完了する。
	 *
	 * @details 衝突登録に失敗した場合は生成ノードも破棄予定へ戻す。既定は読込時の
	 * 描画境界を使う。大きく姿勢が変わる人物には明示箱または明示球を渡す。
	 * @param Params 骨付きモデルの相対path、位置、大きさ、初期animation。
	 * @param CollisionParams 登録形状と衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いたノードと形状番号。読込、生成、再生、登録に失敗したら空の結果。
	 */
	FCollidableModel3DSpawnResult SpawnCollidableAnimatedModel3D(
		const FAnimatedModel3DSpawnParams& Params,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 太陽または点光源を、光の種類に応じた位置・向きと部品を含めて1回で場面へ置く。
	 *
	 * @param Params 光の種類、位置または方向、色、明るさ、届く距離。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いた光ノード。入力または親が無効ならnullptr。
	 */
	ANode* SpawnLight3D( const FLight3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 屈折、反射、泡、波紋へ接続済みの3D水面を1回で場面へ置く。
	 *
	 * @param Params 水面の位置、広さ、見た目、ノード名。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 置いた水面ノード。入力または親が無効ならnullptr。
	 */
	ANode* SpawnWater3D( const FWater3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * この場面が所有する3Dノードを破棄予定にし、成功時だけ呼出側の生ポインタを空にする。
	 *
	 * @details 破棄は次の場面更新で確定する。別場面のノード、root、未登録ノードは拒み、
	 * `Node`を変更しない。すでに破棄予定の自場面ノードは成功としてポインタを空にする。
	 * @param Node 破棄するノードへの生ポインタ。成功時はnullptrになる。
	 * @return 破棄予定へ移したか、すでに破棄予定だった自場面ノードならtrue。
	 */
	bool DestroyNode3D( ANode*& Node ) noexcept;

	/**
	 * 現在カメラを聴取位置として、指定したworld位置から短い3D効果音を1回鳴らす。
	 *
	 * @param AssetPath `Assets`からの相対音声名。
	 * @param WorldPosition 音を鳴らすworld位置。
	 * @param Volume 距離減衰前の音量。0より大きい有限値。
	 * @param MaximumDistance この距離以上で聞こえなくする距離。
	 * @return カメラ、指定、音声窓口、再生の全てが有効ならtrue。
	 */
	bool PlaySound3D( FStringView AssetPath, FVec3 WorldPosition, f32 Volume = 1.0f,
		f32 MaximumDistance = 20.0f ) noexcept;

	/**
	 * 現在カメラを聴取位置として、詳細指定の短い3D効果音を1回鳴らす。
	 *
	 * @param Request 音声名、world位置、速度、音量、距離、減衰、再生速度。
	 * @return カメラ、指定、音声窓口、再生の全てが有効ならtrue。
	 */
	bool PlaySound3D( const FSpatialPlayRequest& Request ) noexcept;

	/**
	 * 現在の3Dカメラを空間音響の聴取位置へ反映する。
	 *
	 * @details ノード追従中の聴取位置は解除し、以後は呼ぶたびに現在カメラへ更新する。
	 * @return カメラ姿勢と空間音響窓口が利用できればtrue。
	 */
	bool RefreshSpatialAudioListener() noexcept;

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
	 * 現在カメラから空間音響へ渡せる聴取位置を作る。
	 *
	 * @param OutListener 作成結果。失敗時は変更しない。
	 * @return カメラ姿勢から有限かつ左右を求められる値を作れたらtrue。
	 */
	bool TryMakeCameraAudioListener( FAudioListener& OutListener ) noexcept;

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

	/** 場面グラフ内のノードとACSの3D衝突形状を場面寿命で結ぶ集合。 */
	CSceneCollision3D m_Collision3D;

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

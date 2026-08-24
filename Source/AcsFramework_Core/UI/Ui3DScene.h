// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DLayer.h"
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DDirection.h"
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3D.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DDirection.h"
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawnResult.h"
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DLayer.h"
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DOrientation.h"
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Fence3D/Fence3DDirection.h"
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionHighlight3DParams.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRay.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRayHit.h"
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DDirection.h"
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawnResult.h"
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3D.h"
#include "AcsFramework_Core/Scene/Visual3D/VisualPreset3D.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"

using namespace acs;
using namespace acs::game;

struct FAnimatedModel3DSpawnParams;
struct FBlock3DSpawnParams;
struct FBridge3DSpawnParams;
struct FCorridor3DSpawnParams;
struct FDoorway3DSpawnParams;
struct FFence3DSpawnParams;
struct FGround3DSpawnParams;
struct FLamp3DParams;
struct FLight3DSpawnParams;
struct FModel3DSpawnParams;
struct FRoom3DSpawnParams;
struct FSphere3DSpawnParams;
struct FStairs3DSpawnParams;
struct FStreetLamp3DSpawnParams;
struct FSpatialPlayRequest;
struct FSprite3DSpawnParams;
struct FStudioLightRig3DParams;
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
	 * 3D場面の遮蔽、反射、間接光、仕上げを一つの見た目へまとめて設定する。
	 *
	 * @details 適用後も個別のACS設定を上書きできる。実行中のGPU参照とフレーム時刻は維持する。
	 * @param Preset 適用する見た目と負荷の組み合わせ。
	 * @return 既知のプリセットを完全に反映できた場合だけtrue。未知値では現在設定を維持する。
	 */
	bool TryApplyVisualPreset3D( EVisualPreset3D Preset = EVisualPreset3D::Balanced ) noexcept;

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
	 * 呼出側所有の近接トリガーを、この場面の衝突集合と基準ノードへ接続する。
	 *
	 * @param Trigger 呼出側が所有する未接続の近接トリガー。
	 * @param Origin この場面が所有する近接範囲の基準ノード。
	 * @param Params ローカル球または箱と検出する衝突レイヤー。
	 * @return 所属と設定を確認して完全に接続できたらtrue。
	 */
	bool BindProximityTrigger3D( CProximityTrigger3D& Trigger, ANode& Origin,
		const FProximityTrigger3DParams& Params = FProximityTrigger3DParams{} ) noexcept;

	/**
	 * 呼出側所有のチェックポイントを、この場面の基準ノードと対象形状へ接続する。
	 *
	 * @param Checkpoint 呼出側が所有する未接続のチェックポイント。
	 * @param Origin この場面が所有する範囲基準ノード。
	 * @param TargetShape 進入だけを追跡する、この場面へ登録済みの衝突形状。
	 * @param Params ローカル範囲、対象レイヤー、一度限りか再進入可能かの設定。
	 * @return 所属、対象形状、設定を確認して完全に接続できたらtrue。
	 */
	bool BindCheckpoint3D( CCheckpoint3D& Checkpoint, ANode& Origin,
		FCollisionShapeId3D TargetShape,
		const FCheckpoint3DParams& Params = FCheckpoint3DParams{} ) noexcept;

	/**
	 * 範囲基準ノードの生成とチェックポイント接続を1回で完了する。
	 *
	 * @param Checkpoint 呼出側が所有する未接続のチェックポイント。
	 * @param TargetShape 進入だけを追跡する、この場面へ登録済みの衝突形状。
	 * @param Position 配置先親から見た範囲基準位置。root直下ではworld位置。
	 * @param Params ローカル範囲、対象レイヤー、一度限りか再進入可能かの設定。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成と接続を完了した範囲基準ノード。失敗時は空で半端なノードを残さない。
	 */
	FCheckpoint3DSpawnResult SpawnCheckpoint3D( CCheckpoint3D& Checkpoint,
		FCollisionShapeId3D TargetShape, FVec3 Position,
		const FCheckpoint3DParams& Params = FCheckpoint3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 球半径を直接指定し、範囲基準ノードの生成とチェックポイント接続を完了する。
	 *
	 * @param Checkpoint 呼出側が所有する未接続のチェックポイント。
	 * @param TargetShape 進入だけを追跡する、この場面へ登録済みの衝突形状。
	 * @param Position 配置先親から見た範囲基準位置。root直下ではworld位置。
	 * @param LocalRadius 0より大きいローカル半径。
	 * @param CollisionMask 検知する衝突レイヤーのビット列。
	 * @param bActivateOnce 最初の進入だけを発火にするならtrue。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 生成と接続を完了した範囲基準ノード。失敗時は空で半端なノードを残さない。
	 */
	FCheckpoint3DSpawnResult SpawnCheckpoint3D( CCheckpoint3D& Checkpoint,
		FCollisionShapeId3D TargetShape, FVec3 Position, f32 LocalRadius,
		u32 CollisionMask = CSceneCollision3D::kAllLayers,
		bool bActivateOnce = true, ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成したチェックポイントの接続を外し、範囲基準ノードを破棄する。
	 *
	 * @param Checkpoint 生成時に接続したチェックポイント。
	 * @param Spawned `SpawnCheckpoint3D`の成功結果。成功時は空の結果になる。
	 * @return 所有関係を確認し、接続解除とノード破棄を完了できたらtrue。
	 */
	bool DestroyCheckpoint3D( CCheckpoint3D& Checkpoint,
		FCheckpoint3DSpawnResult& Spawned ) noexcept;

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
	 * 表示用平面と、その直下に収まる歩ける箱型衝突を1回で場面へ置く。
	 *
	 * @param Params 上面位置、広さ、厚み、見た目、衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 地面ノードと衝突識別子。失敗時は空で、半端な生成物を残さない。
	 */
	FCollidableModel3DSpawnResult SpawnGround3D( const FGround3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の見た目と厚みを使い、広さだけで歩ける3D地面を置く。
	 *
	 * @param Size X方向とZ方向の全幅。
	 * @param Position 配置先親から見た地面上面の中心位置。root直下ではworld位置。
	 * @param CollisionLayer 地面が属する非0の衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 地面ノードと衝突識別子。入力または登録に失敗したら空の結果。
	 */
	FCollidableModel3DSpawnResult SpawnGround3D( FVec2 Size,
		FVec3 Position = FVec3{},
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 表示と箱型衝突の寸法を揃えた3D直方体を1回で置く。
	 *
	 * @param Params 中心位置、回転、全寸法、見た目、衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 立方体ノードと衝突識別子。失敗時は空で、半端な生成物を残さない。
	 */
	FCollidableModel3DSpawnResult SpawnBlock3D( const FBlock3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の見た目を使い、全寸法と中心位置だけで衝突付き3D直方体を置く。
	 *
	 * @param Size X、Y、Z方向の全寸法。
	 * @param Position 配置先親から見た直方体の中心位置。root直下ではworld位置。
	 * @param CollisionLayer 箱が属する非0の衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 立方体ノードと衝突識別子。入力または登録に失敗したら空の結果。
	 */
	FCollidableModel3DSpawnResult SpawnBlock3D( FVec3 Size,
		FVec3 Position = FVec3{},
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 表示半径と球型衝突半径を揃えた3D球を1回で置く。
	 *
	 * @param Params 中心位置、半径、見た目、衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 球ノードと衝突識別子。失敗時は空で、半端な生成物を残さない。
	 */
	FCollidableModel3DSpawnResult SpawnSphere3D( const FSphere3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の見た目を使い、半径と中心位置だけで衝突付き3D球を置く。
	 *
	 * @param Radius 表示と衝突へ共通で使う半径。
	 * @param Position 配置先親から見た球の中心位置。
	 * @param CollisionLayer 球が属する非0の衝突レイヤー。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 球ノードと衝突識別子。入力または登録に失敗したら空の結果。
	 */
	FCollidableModel3DSpawnResult SpawnSphere3D( f32 Radius,
		FVec3 Position = FVec3{},
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 歩ける床板と両側柵を持つ、軸方向の衝突付き3D橋を1回で置く。
	 *
	 * @param Params 入口、方向、床板と柵の寸法、見た目、衝突レイヤー。
	 * @param Parent 全パーツを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 床板と両側柵。失敗時は空で、有効な半端物を残さない。
	 */
	FBridge3DSpawnResult SpawnBridge3D( const FBridge3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の床板・柵寸法と見た目を使い、幅、長さ、柵高だけで3D橋を置く。
	 *
	 * @param Width 両側の床板端を結ぶ全幅。
	 * @param Length 入口境界から出口境界までの床板全長。
	 * @param RailingHeight 床板上面からの柵高。
	 * @param EntranceCenter 配置先親から見た入口境界の床板上中心。
	 * @param Direction 入口から出口へ橋を伸ばす軸方向。省略時はZ正方向。
	 * @param CollisionLayer 全パーツが属する非0の衝突レイヤー。
	 * @param Parent 全パーツを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 床板と両側柵。入力または途中登録に失敗したら空の結果。
	 */
	FBridge3DSpawnResult SpawnBridge3D( f32 Width, f32 Length,
		f32 RailingHeight = 1.15f, FVec3 EntranceCenter = FVec3{},
		EBridge3DDirection Direction = EBridge3DDirection::PositiveZ,
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した3D橋を、床板、支柱、横桟のノード・形状ごと破棄する。
	 *
	 * @details 全要素がこの場面で重複なく対になっていない場合は何も変更しない。
	 * @param Bridge `SpawnBridge3D`の成功結果。成功時は空の結果になる。
	 * @return 全ノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyBridge3D( FBridge3DSpawnResult& Bridge ) noexcept;

	/**
	 * 歩ける床と左右の壁を持つ、両端が開いた3D通路を1回で置く。
	 *
	 * @param Params 入口、方向、内幅、長さ、壁と床の寸法、見た目、衝突レイヤー。
	 * @param Parent 3個のノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 床と側壁2枚。失敗時は空で、有効な半端物を残さない。
	 */
	FCorridor3DSpawnResult SpawnCorridor3D( const FCorridor3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の壁厚、床厚、見た目を使い、内幅と長さだけで3D通路を置く。
	 *
	 * @param InnerWidth 左右の壁内面の間で使える全幅。
	 * @param Length 入口境界から出口境界までの長さ。
	 * @param WallHeight 床上面からの壁高。
	 * @param EntranceCenter 配置先親から見た入口境界の床上中心。
	 * @param Direction 入口から出口へ通路を伸ばす軸方向。省略時はZ正方向。
	 * @param CollisionLayer 床と側壁が属する非0の衝突レイヤー。
	 * @param Parent 3個のノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 床と側壁2枚。入力または途中登録に失敗したら空の結果。
	 */
	FCorridor3DSpawnResult SpawnCorridor3D( f32 InnerWidth, f32 Length,
		f32 WallHeight = 3.0f, FVec3 EntranceCenter = FVec3{},
		ECorridor3DDirection Direction = ECorridor3DDirection::PositiveZ,
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した3D通路を、床と側壁2枚のノード・形状ごと破棄する。
	 *
	 * @details 全3組がこの場面で重複なく対になっていない場合は何も変更しない。
	 * @param Corridor `SpawnCorridor3D`の成功結果。成功時は空の結果になる。
	 * @return 3個のノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyCorridor3D( FCorridor3DSpawnResult& Corridor ) noexcept;

	/**
	 * 床から始まる開口を持つ3D壁枠を、左右柱と上枠の3組で置く。
	 *
	 * @param Params 下辺中央、向き、壁と開口の寸法、見た目、衝突レイヤー。
	 * @param Parent 3個のノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 左右柱と上枠。失敗時は空で、有効な半端物を残さない。
	 */
	FDoorway3DSpawnResult SpawnDoorway3D( const FDoorway3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の見た目を使い、壁と開口の寸法だけで中央開口の3D壁枠を置く。
	 *
	 * @param WallWidth 左右の外端間にある壁全体の幅。
	 * @param WallHeight 床から壁上端までの高さ。
	 * @param OpeningWidth 通り抜けられる開口幅。
	 * @param OpeningHeight 床から上枠下端までの開口高。
	 * @param WallThickness 壁面に直交する方向の厚み。
	 * @param BottomCenter 配置先親から見た壁全体の下辺中央。
	 * @param Orientation 壁幅を伸ばすXまたはZ軸。
	 * @param CollisionLayer 左右柱と上枠が属する非0の衝突レイヤー。
	 * @param Parent 3個のノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 左右柱と上枠。入力または途中登録に失敗したら空の結果。
	 */
	FDoorway3DSpawnResult SpawnDoorway3D( f32 WallWidth, f32 WallHeight,
		f32 OpeningWidth, f32 OpeningHeight, f32 WallThickness = 0.25f,
		FVec3 BottomCenter = FVec3{},
		EDoorway3DOrientation Orientation = EDoorway3DOrientation::AlongX,
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した3D壁枠を、左右柱と上枠のノード・形状ごと破棄する。
	 *
	 * @details 全3組がこの場面で重複なく対になっていない場合は何も変更しない。
	 * @param Doorway `SpawnDoorway3D`の成功結果。成功時は空の結果になる。
	 * @return 3個のノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyDoorway3D( FDoorway3DSpawnResult& Doorway ) noexcept;

	/**
	 * 両端を含む支柱を最大間隔で分け、水平な横桟で繋いだ3D柵を置く。
	 *
	 * @param Params 始点、方向、寸法、支柱間隔、横桟数、見た目、衝突レイヤー。
	 * @param Parent 全支柱と横桟を繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 始点から並ぶ支柱と下から並ぶ横桟。失敗時は空で、有効な半端物を残さない。
	 */
	FFence3DSpawnResult SpawnFence3D( const FFence3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の支柱・横桟寸法と見た目を使い、長さと高さだけで3D柵を置く。
	 *
	 * @param Length 始点支柱と終点支柱の中心間距離。
	 * @param Height 底面から支柱上端までの高さ。
	 * @param MaximumPostSpacing 隣り合う支柱中心の最大間隔。
	 * @param StartPostBottomCenter 配置先親から見た始点支柱の底面中央。
	 * @param Direction 始点支柱から終点支柱へ伸ばす軸方向。
	 * @param CollisionLayer 全支柱と横桟が属する非0の衝突レイヤー。
	 * @param Parent 全支柱と横桟を繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 始点から並ぶ支柱と下から並ぶ横桟。入力または途中登録に失敗したら空の結果。
	 */
	FFence3DSpawnResult SpawnFence3D( f32 Length, f32 Height,
		f32 MaximumPostSpacing = 2.0f,
		FVec3 StartPostBottomCenter = FVec3{},
		EFence3DDirection Direction = EFence3DDirection::PositiveZ,
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した3D柵を、全支柱と横桟のノード・形状ごと破棄する。
	 *
	 * @details 全要素がこの場面で重複なく対になっていない場合は何も変更しない。
	 * @param Fence `SpawnFence3D`の成功結果。成功時は空の結果になる。
	 * @return 全ノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyFence3D( FFence3DSpawnResult& Fence ) noexcept;

	/**
	 * 衝突付き直方体を段差ごとに積み上げた軸方向3D階段を1回で置く。
	 *
	 * @param Params 基準点、方向、段数、寸法、見た目、衝突レイヤー。
	 * @param Parent 全段を繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 低い側から並ぶ全段。失敗時は空で、有効な半端物を残さない。
	 */
	FStairs3DSpawnResult SpawnStairs3D( const FStairs3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の見た目を使い、段数と1段の寸法だけで3D階段を置く。方向を省略するとZ正方向へ上る。
	 *
	 * @param StepCount 1から256までの段数。
	 * @param Width 上る方向と直交する全幅。
	 * @param StepDepth 1段あたりの上る方向への奥行き。
	 * @param StepHeight 1段上がるごとの高さ。
	 * @param BottomEdgeCenter 配置先親から見た最下段手前の床上中心。
	 * @param Direction 低い側から高い側へ段を増やす軸方向。
	 * @param CollisionLayer 全段が属する非0の衝突レイヤー。
	 * @param Parent 全段を繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 低い側から並ぶ全段。入力または途中登録に失敗したら空の結果。
	 */
	FStairs3DSpawnResult SpawnStairs3D( u32 StepCount, f32 Width,
		f32 StepDepth, f32 StepHeight,
		FVec3 BottomEdgeCenter = FVec3{},
		EStairs3DDirection Direction = EStairs3DDirection::PositiveZ,
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した3D階段を、全段のノードと形状ごと高い側から破棄する。
	 *
	 * @details 全段がこの場面で重複なく対になっていない場合は何も変更しない。
	 * @param Stairs `SpawnStairs3D`の成功結果。成功時は空の結果になる。
	 * @return 全段のノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyStairs3D( FStairs3DSpawnResult& Stairs ) noexcept;

	/**
	 * 床位置を基準に、衝突付き金属ポストと見える点光源を1基の3D街灯として置く。
	 *
	 * @details ポストは回転しない直方体で、表示と箱型衝突が同じ位置・寸法になる。
	 * 街灯1基につきACSの点光源枠を1灯使用する。
	 * @param Params 床位置、ポスト寸法・材質、発光球、照明、衝突レイヤー。
	 * @param Parent 3ノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return ポスト、衝突、発光球、点光源を全て配置した結果。失敗時は空。
	 */
	FStreetLamp3DSpawnResult SpawnStreetLamp3D(
		const FStreetLamp3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 床位置だけで、既定の金属ポストと暖色ランプを持つ3D街灯を置く。
	 *
	 * @param BasePosition 配置先親から見たポスト底面中央。
	 * @param Parent 3ノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 既定の衝突付き街灯。入力または親が無効なら空。
	 */
	FStreetLamp3DSpawnResult SpawnStreetLamp3D( FVec3 BasePosition,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * この場面へ一括配置した3D街灯を、3ノードとポスト衝突ごと破棄する。
	 *
	 * @details 場面、root、3ノード、衝突形状、重複を先に検証する。
	 * @param StreetLamp `SpawnStreetLamp3D`の成功結果。成功時は空になる。
	 * @return この場面の街灯と確認し、全て片付けられた場合だけtrue。
	 */
	bool DestroyStreetLamp3D(
		FStreetLamp3DSpawnResult& StreetLamp ) noexcept;

	/**
	 * 歩ける床と四方の壁を持つ、天井なし3D部屋を1回で置く。
	 *
	 * @param Params 床上面位置、内寸、壁と床の寸法、見た目、衝突レイヤー。
	 * @param Parent 5個のノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 床と四方の壁。失敗時は空で、有効な半端物を残さない。
	 */
	FRoom3DSpawnResult SpawnRoom3D( const FRoom3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既定の壁厚、床厚、見た目を使い、内寸と壁高だけで天井なし3D部屋を置く。
	 *
	 * @param InnerSize 壁の内側で使えるX方向とZ方向の全幅。
	 * @param WallHeight 床上面からの壁高。
	 * @param FloorTopPosition 配置先親から見た床上面の中心位置。
	 * @param CollisionLayer 床と壁が属する非0の衝突レイヤー。
	 * @param Parent 5個のノードを繋ぐ、この場面が所有する親。空ならroot直下。
	 * @return 床と四方の壁。入力または途中登録に失敗したら空の結果。
	 */
	FRoom3DSpawnResult SpawnRoom3D( FVec2 InnerSize, f32 WallHeight = 3.0f,
		FVec3 FloorTopPosition = FVec3{},
		u32 CollisionLayer = CCollisionWorld3D::kAllLayers,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した天井なし部屋を、床と四方の壁のノード・形状ごと破棄する。
	 *
	 * @details 全5組がこの場面で重複なく対になっていない場合は何も変更しない。
	 * @param Room `SpawnRoom3D`の成功結果。成功時は空の結果になる。
	 * @return 5個のノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyRoom3D( FRoom3DSpawnResult& Room ) noexcept;

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
	 * 一括生成した通常モデル、骨格モデル、地面、または直方体をノードと衝突形状ごと破棄する。
	 *
	 * @details ノードと形状がこの場面で対になっていない場合は何も変更しない。
	 * @param Model 衝突付き生成APIの成功結果。成功時は空の結果になる。
	 * @return 自場面の対になったノードを破棄予定へ移し、形状も直ちに外せたらtrue。
	 */
	bool DestroyCollidableModel3D( FCollidableModel3DSpawnResult& Model ) noexcept;

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
	 * 見える自己発光球と周囲を照らす点光源を、1個の3Dランプとして配置する。
	 *
	 * @details 発光球と点光源は位置と色を共有する。1個につきACSの点光源枠を1灯使用する。
	 * @param Params 位置、半径、共有色、発光と照明の強さ、到達距離。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 発光球と点光源を両方配置した結果。失敗時は空で、半端なノードを残さない。
	 */
	FLamp3DSpawnResult SpawnLamp3D( const FLamp3DParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 位置だけで暖色の見える3Dランプを配置する。
	 *
	 * @param Position 配置先親から見たランプ中心。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 既定の発光球と点光源を配置した結果。入力または親が無効なら空。
	 */
	FLamp3DSpawnResult SpawnLamp3D( FVec3 Position,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * この場面へ配置した3Dランプの発光球と点光源を同じ指定へ同期更新する。
	 *
	 * @details 別場面の結果、破棄予定の片方、不正な新指定では何も変更しない。
	 * @param Spawned `SpawnLamp3D`の成功結果。
	 * @param Params 新しい位置、半径、共有色、発光と照明の強さ、到達距離。
	 * @return 発光球と点光源を両方更新できた場合だけtrue。
	 */
	bool TryUpdateLamp3D( const FLamp3DSpawnResult& Spawned,
		const FLamp3DParams& Params ) noexcept;

	/**
	 * この場面へ一括配置した3Dランプを、発光球と点光源ごと破棄する。
	 *
	 * @param Spawned `SpawnLamp3D`の成功結果。成功時は空になる。
	 * @return この場面の結果と確認し、残る2ノードを破棄予定へ移せたらtrue。
	 */
	bool DestroyLamp3D( FLamp3DSpawnResult& Spawned ) noexcept;

	/**
	 * 被写体の中心、見る方向、半径からキー、フィル、リムの3灯を一括配置する。
	 *
	 * @details 全て点光源として置くため、時刻連動または既定の太陽と影は置き換えない。
	 * ACSが同時描画する点光源4灯のうち3灯を使用する。
	 * @param Params 被写体中心、見る方向、半径と3灯の見た目。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 3灯を全て配置した結果。失敗時は空で、半端な光を残さない。
	 */
	FStudioLightRig3DSpawnResult SpawnStudioLightRig3D(
		const FStudioLightRig3DParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 被写体の中心、見る方向、半径だけで既定の3点照明を一括配置する。
	 *
	 * @param SubjectCenter 配置先親から見た被写体中心。
	 * @param ViewDirectionToCamera 被写体からカメラへ向かう方向。
	 * @param SubjectRadius 被写体を覆うおおよその半径。
	 * @param Parent この場面が所有する親。空ならroot直下。
	 * @return 3灯を全て配置した結果。入力または親が無効なら空。
	 */
	FStudioLightRig3DSpawnResult SpawnStudioLightRig3D(
		FVec3 SubjectCenter, FVec3 ViewDirectionToCamera,
		f32 SubjectRadius, ANode* Parent = nullptr ) noexcept;

	/**
	 * この場面へ一括配置した3点照明を全て破棄する。
	 *
	 * @param Spawned `SpawnStudioLightRig3D`の成功結果。成功時は空になる。
	 * @return この場面の結果と確認し、残る3灯を破棄予定へ移せたらtrue。
	 */
	bool DestroyStudioLightRig3D(
		FStudioLightRig3DSpawnResult& Spawned ) noexcept;

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
	 * world座標の始点から終点へ向く矢印を次の3D描画へ一括登録する。
	 *
	 * @details 胴体1本と立体的な矢尻4本を使う。表示を続ける場合は更新ごとに呼ぶ。
	 * @param HeadSize world単位の矢尻長。0より大きく、矢印全体の長さ以下でなければならない。
	 * @return 座標、色、長さが有効で、5本全てを登録できたらtrue。
	 */
	bool DrawArrow3D( FVec3 Start, FVec3 End,
		FVec4 Color = FVec4{ 1.0f, 0.72f, 0.16f, 1.0f },
		f32 HeadSize = CDebugDraw3DQueue::kDefaultArrowHeadSize ) noexcept;

	/**
	 * 指定位置と回転のローカルX、Y、Z軸を次の3D描画へ一括登録する。
	 *
	 * @details Xは赤、Yは緑、Zは青の立体矢印で固定する。表示を続ける場合は更新ごとに呼ぶ。
	 * @param Origin 3軸が始まるworld座標。
	 * @param Rotation 3軸へ適用する有限で正規化可能なworld回転。
	 * @param AxisLength 各軸のworld長。0より大きい有限値。
	 * @param HeadSize 各矢尻のworld長。0より大きく、AxisLength以下でなければならない。
	 * @return 位置、回転、寸法が有効で、15本全てを登録できたらtrue。
	 */
	bool DrawAxes3D( FVec3 Origin, FQuat Rotation = FQuat::Identity(),
		f32 AxisLength = CDebugDraw3DQueue::kDefaultAxisLength,
		f32 HeadSize = CDebugDraw3DQueue::kDefaultArrowHeadSize ) noexcept;

	/**
	 * 指定中心の水平XZグリッドを次の3D描画へ一括登録する。
	 *
	 * @details X方向とZ方向へ各Divisions+1本を等間隔で置く。表示を続ける場合は更新ごとに呼ぶ。
	 * @param Center グリッド中央のworld座標。yがグリッド面の高さになる。
	 * @param HalfExtent 中心からX、Z各端までのworld距離。0より大きい有限値。
	 * @param Divisions 各方向を等分する1から128の数。
	 * @param Color 全てのグリッド線へ使う色。
	 * @return 位置、寸法、分割数、色が有効で、全線を登録できたらtrue。
	 */
	bool DrawGrid3D( FVec3 Center = FVec3{},
		f32 HalfExtent = CDebugDraw3DQueue::kDefaultGridHalfExtent,
		u32 Divisions = CDebugDraw3DQueue::kDefaultGridDivisions,
		FVec4 Color = FVec4{ 0.28f, 0.36f, 0.48f, 1.0f } ) noexcept;

	/**
	 * 指定world法線へ直交する閉じた円を次の3D描画へ一括登録する。
	 *
	 * @details 接触面、半径、効果範囲を1本の輪で示す。表示を続ける場合は更新ごとに呼ぶ。
	 * @param Center 円のworld中心。
	 * @param Normal 円が載る面の有限で正規化可能なworld法線。
	 * @param Radius 円のworld半径。0より大きい有限値。
	 * @param Color 全ての円周線へ使う色。
	 * @param Segments 円周を等分する4から128の数。
	 * @return 値が有効で、円周線を全て登録できたらtrue。
	 */
	bool DrawCircle3D( FVec3 Center, FVec3 Normal, f32 Radius,
		FVec4 Color = FVec4{ 1.0f, 0.58f, 0.18f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultCircleSegments ) noexcept;

	/**
	 * 指定world方向へ伸びる円錐を次の3D描画へ一括登録する。
	 *
	 * @details 視野、範囲、スポット方向、ノード正面を底面円と4本の側線で示す。
	 * 表示を続ける場合は更新ごとに呼ぶ。
	 * @param Apex 円錐のworld頂点。
	 * @param Direction 頂点から底面へ向かう有限で正規化可能なworld方向。
	 * @param Length 頂点から底面中心までのworld長。0より大きい有限値。
	 * @param BaseRadius 底面円のworld半径。0より大きい有限値。
	 * @param Color 全ての底面線と側線へ使う色。
	 * @param Segments 底面円を等分する4から128の数。
	 * @return 値が有効で、円錐の全線を登録できたらtrue。
	 */
	bool DrawCone3D( FVec3 Apex, FVec3 Direction, f32 Length, f32 BaseRadius,
		FVec4 Color = FVec4{ 0.72f, 0.35f, 1.0f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultConeSegments ) noexcept;

	/**
	 * 指定world軸へ沿う円柱を次の3D描画へ一括登録する。
	 *
	 * @details 回転軸、体積、センサー範囲を両端円と4本の側線で示す。
	 * 表示を続ける場合は更新ごとに呼ぶ。
	 * @param Center 円柱のworld中心。
	 * @param Axis 一方の端から他方の端へ向かう有限で正規化可能なworld軸。
	 * @param Height 両端中心間のworld高さ。0より大きい有限値。
	 * @param Radius 両端円のworld半径。0より大きい有限値。
	 * @param Color 全ての端面線と側線へ使う色。
	 * @param Segments 各端円を等分する4から128の数。
	 * @return 値が有効で、円柱の全線を登録できたらtrue。
	 */
	bool DrawCylinder3D( FVec3 Center, FVec3 Axis, f32 Height, f32 Radius,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 0.74f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultCylinderSegments ) noexcept;

	/**
	 * 指定world回転を持つ箱の12辺を次の3D描画へ一括登録する。
	 *
	 * @details 回転ノードの見た目、局所範囲、向き付き境界を示す。表示を続ける場合は更新ごとに呼ぶ。
	 * @param Center 箱のworld中心。
	 * @param Rotation 箱へ適用する有限で正規化可能なworld回転。
	 * @param HalfSize 回転前のローカルX、Y、Z各軸へ伸びる有限な非負半サイズ。
	 * @param Color 12辺全てへ使う色。
	 * @return 値が有効で、12辺全てを登録できたらtrue。
	 */
	bool DrawBox3D( FVec3 Center, FQuat Rotation, FVec3 HalfSize,
		FVec4 Color = FVec4{ 1.0f, 0.38f, 0.72f, 1.0f } ) noexcept;

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

	/**
	 * この場面へ登録した衝突形状を現在位置で次の3D描画へ一括登録する。
	 *
	 * @details 問い合わせ対象と同じworld球またはworld軸平行箱を使う。表示を続ける場合は更新ごとに呼ぶ。
	 * @param Shape `Collision3D()`へ登録済みの世代付き形状番号。
	 * @param Color 全ての線へ使う色。
	 * @param SphereSegments 球を構成する各円の分割数。箱では使わない。
	 * @return 生存中かつ問い合わせ対象の形状を取得し、線を全て登録できたらtrue。
	 */
	bool DrawCollisionShape3D( FCollisionShapeId3D Shape,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 SphereSegments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

	/**
	 * この場面で現在問い合わせ対象の衝突形状を、レイヤーで絞って一括登録する。
	 *
	 * @details 各形状を判定と同じworld球またはworld軸平行箱として登録する。表示を続ける場合は更新ごとに呼ぶ。
	 * @param CollisionMask レイヤーとのANDが0でない形状だけを表示するマスク。
	 * @param Color 全ての線へ使う色。
	 * @param SphereSegments 各球を構成する円の分割数。箱では使わない。
	 * @return 線を全て登録できた形状数。同期失敗または0件では0。
	 */
	u32 DrawCollisionShapes3D( u32 CollisionMask = CSceneCollision3D::kAllLayers,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 SphereSegments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

	/**
	 * この場面へ接続した近接トリガーの現在範囲を次の3D描画へ一括登録する。
	 *
	 * @details 判定と同じworld球またはworld軸平行箱を使う。表示を続ける場合は更新ごとに呼ぶ。
	 * @param Trigger `BindProximityTrigger3D`でこの場面へ接続済みのトリガー。
	 * @param Color 全ての線へ使う色。
	 * @param SphereSegments 球を構成する各円の分割数。箱では使わない。
	 * @return 接続、world変換、線の一括登録を全て完了できたらtrue。
	 */
	bool DrawProximityTrigger3D( const CProximityTrigger3D& Trigger,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 SphereSegments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

	/**
	 * この場面へ接続したチェックポイントの現在範囲を次の3D描画へ一括登録する。
	 *
	 * @param Checkpoint `BindCheckpoint3D`または`SpawnCheckpoint3D`で接続済みの対象。
	 * @param Color 全ての線へ使う色。
	 * @param SphereSegments 球を構成する各円の分割数。箱では使わない。
	 * @return 接続、world変換、線の一括登録を全て完了できたらtrue。
	 */
	bool DrawCheckpoint3D( const CCheckpoint3D& Checkpoint,
		FVec4 Color = FVec4{ 0.25f, 1.0f, 0.35f, 1.0f },
		u32 SphereSegments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

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

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneSweepHit3D.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRay.h"

using namespace acs;
using namespace acs::game;

/**
 * シーンノードとACSの3D衝突形状を結び、現在位置で問い合わせる。
 *
 * @details
 * 場面またはゲーム固有の所有者が、対象の`CSceneNodeGraph`より短い期間だけ所有する。
 * 1ノードには1形状を登録でき、複合形状は子ノードへ分ける。問い合わせ前にノードの
 * 世界Transformを自動同期するため、毎フレームの手動更新は要らない。
 */
class CSceneCollision3D
{
public:
	/** 全レイヤーを問い合わせ対象にするマスク。 */
	static constexpr u32 kAllLayers = CCollisionWorld3D::kAllLayers;

	/**
	 * ノードグラフへ接続した空の衝突集合を作る。
	 *
	 * @param Graph ノードを所有するグラフ。この型より長く生存すること。
	 */
	explicit CSceneCollision3D( CSceneNodeGraph& Graph ) noexcept;

	/** 所有権の重複を防ぐためコピーを禁止する。 */
	CSceneCollision3D( const CSceneCollision3D& ) = delete;

	/** 所有権の重複を防ぐためコピー代入を禁止する。 */
	CSceneCollision3D& operator=( const CSceneCollision3D& ) = delete;

	/**
	 * 設定で選んだ描画境界、明示箱、明示球のいずれかをノードへ登録する。
	 *
	 * @param Node 対象グラフへ登録済みのノード。
	 * @param Params 形状の種類、ローカル寸法、衝突レイヤー。
	 * @return 登録形状の番号。入力不正、部品なし、二重登録では無効値。
	 */
	FCollisionShapeId3D TryAdd( ANode& Node, const FCollisionShape3DParams& Params ) noexcept;

	/**
	 * 描画部品のローカル境界を衝突形状として登録する。
	 *
	 * @details 通常の球プリミティブは球、それ以外の通常メッシュと骨付きメッシュは箱にする。
	 * 骨付きメッシュは読み込み時の頂点境界を使うため、大きく飛び出す姿勢には明示的な箱を使う。
	 * @param Node 対象グラフへ登録済みのノード。
	 * @param Layer 形状が属するレイヤーのビット列。0は全問い合わせから外れる。
	 * @return 登録形状の番号。部品なし、不正な境界、同じノードの二重登録では無効値。
	 */
	FCollisionShapeId3D TryAddBounds( ANode& Node, u32 Layer = kAllLayers ) noexcept;

	/**
	 * ノードのローカル座標に箱を登録する。
	 *
	 * @param Node 対象グラフへ登録済みのノード。
	 * @param LocalCenter 箱のローカル中心。
	 * @param LocalHalfSize 各軸の半サイズ。有限かつ0以上。
	 * @param Layer 形状が属するレイヤーのビット列。
	 * @return 登録形状の番号。入力不正または二重登録では無効値。
	 */
	FCollisionShapeId3D TryAddBox( ANode& Node, FVec3 LocalCenter, FVec3 LocalHalfSize,
		u32 Layer = kAllLayers ) noexcept;

	/**
	 * ノードのローカル座標に球を登録する。
	 *
	 * @details 非一様な拡縮では最も大きい軸に合わせ、安全側の球へ広げる。
	 * @param Node 対象グラフへ登録済みのノード。
	 * @param LocalCenter 球のローカル中心。
	 * @param LocalRadius ローカル半径。有限かつ0より大きい値。
	 * @param Layer 形状が属するレイヤーのビット列。
	 * @return 登録形状の番号。入力不正または二重登録では無効値。
	 */
	FCollisionShapeId3D TryAddSphere( ANode& Node, FVec3 LocalCenter, f32 LocalRadius,
		u32 Layer = kAllLayers ) noexcept;

	/**
	 * 登録形状のレイヤーを変える。
	 *
	 * @param Shape 変更する形状番号。
	 * @param Layer 新しいレイヤーのビット列。
	 * @return 生存する登録形状を変更できたらtrue。
	 */
	bool TrySetLayer( FCollisionShapeId3D Shape, u32 Layer ) noexcept;

	/**
	 * 登録形状を外す。ノード自体は変更しない。
	 *
	 * @param Shape 外す形状番号。
	 * @return 生存する登録形状を外せたらtrue。
	 */
	bool Remove( FCollisionShapeId3D Shape ) noexcept;

	/** 全登録を外す。ノード自体は変更しない。 */
	void Clear() noexcept;

	/** 現在登録している形状数を返す。 */
	u32 ShapeCount() const noexcept { return m_World.ShapeCount(); }

	/**
	 * 生存ノードの現在Transformを反映し、破棄済みノードの登録を取り除く。
	 *
	 * @details 通常は問い合わせが自動で呼ぶ。まとめて同期結果だけ確かめたい場合に使う。
	 * @return 全ての生存形状を更新できたらtrue。不正なTransformではfalse。
	 */
	bool Sync() noexcept;

	/**
	 * 世界座標の箱と重なる登録ノードを、ACSの決定的な形状番号順で置き換える。
	 *
	 * @param Box 中心と半サイズで表す世界座標の軸平行箱。
	 * @param OutNodes 重なったノードの受け取り先。失敗時は変更しない。
	 * @param Exclude 除外する自身などの形状番号。無効値なら除外なし。
	 * @param Mask レイヤーとのANDが0でない形状だけを含めるマスク。
	 * @return 入力、同期、結果構築に成功したらtrue。0件でもtrue。
	 */
	bool TryOverlapBox( const FAabb3& Box, TArray<ANode*>& OutNodes,
		FCollisionShapeId3D Exclude = {}, u32 Mask = kAllLayers ) noexcept;

	/**
	 * 世界座標の球と重なる登録ノードを、ACSの決定的な形状番号順で置き換える。
	 *
	 * @param Sphere 中心と半径で表す世界座標の球。
	 * @param OutNodes 重なったノードの受け取り先。失敗時は変更しない。
	 * @param Exclude 除外する自身などの形状番号。無効値なら除外なし。
	 * @param Mask レイヤーとのANDが0でない形状だけを含めるマスク。
	 * @return 入力、同期、結果構築に成功したらtrue。0件でもtrue。
	 */
	bool TryOverlapSphere( const FSphere& Sphere, TArray<ANode*>& OutNodes,
		FCollisionShapeId3D Exclude = {}, u32 Mask = kAllLayers ) noexcept;

	/**
	 * 球を線に沿って動かし、最初に触れる登録ノードを返す。
	 *
	 * @param Ray 正規化済みの向きと最大距離を持つ線。
	 * @param Radius 動かす球の世界半径。有限かつ0以上。
	 * @param OutHit 最初の接触結果。失敗または外れでは変更しない。
	 * @param Exclude 除外する自身などの形状番号。無効値なら除外なし。
	 * @param Mask レイヤーとのANDが0でない形状だけを含めるマスク。
	 * @return 有効な線と球が登録形状へ触れたらtrue。
	 */
	bool TrySweepSphere( const FSceneRay& Ray, f32 Radius, FSceneSweepHit3D& OutHit,
		FCollisionShapeId3D Exclude = {}, u32 Mask = kAllLayers ) noexcept;

	/**
	 * 登録ノードを同期してから、球型3Dキャラクターの次状態を計算する。
	 *
	 * @details ノード位置や入力状態は変更しない。計算後のノード反映は呼出側が明示的に行う。
	 * @param Input 希望水平速度、ジャンプ要求、レイヤーマスク、自己除外形状。
	 * @param State 現在の球中心、速度、接地状態。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @param Params 球半径、重力、ジャンプ初速、接触調整値。
	 * @param OutResult 次状態と接触事象の受け取り先。失敗時は変更しない。
	 * @return 同期と次状態の計算に成功したらtrue。
	 */
	bool TryMoveCharacter( const FKinematicCharacterMovementInput3D& Input, const FKinematicCharacterState3D& State, f32 DeltaSeconds, const FKinematicCharacterMovementParams3D& Params, FKinematicCharacterMovementResult3D& OutResult ) noexcept;

private:
	/** ACSへ登録した形状の種類。 */
	enum class EShapeKind : u8
	{
		/** 世界座標で軸に沿う箱へ同期する。 */
		Box,

		/** 世界座標の球へ同期する。 */
		Sphere,
	};

	/** 1ノードと1衝突形状を結ぶ記録。 */
	struct FRegistration
	{
		/** ACS衝突集合内の形状番号。 */
		FCollisionShapeId3D Shape;

		/** シーングラフ内の世代付きノード番号。 */
		FNodeId Node;

		/** 同期する形状の種類。 */
		EShapeKind Kind = EShapeKind::Box;

		/** 箱または球のローカル中心。 */
		FVec3 LocalCenter;

		/** 箱のローカル半サイズ。 */
		FVec3 LocalHalfSize;

		/** 球のローカル半径。 */
		f32 LocalRadius = 0.0f;

		/** ノードが有効なときに使うレイヤー。 */
		u32 Layer = kAllLayers;
	};

	/** 共通の入力検証、世界形状作成、ACS登録を行う。 */
	FCollisionShapeId3D TryAdd_Internal( ANode& Node, EShapeKind Kind, FVec3 LocalCenter,
		FVec3 LocalHalfSize, f32 LocalRadius, u32 Layer ) noexcept;

	/** 描画部品から有限なローカル境界を求める。 */
	static bool TryFindLocalBounds_Internal( ANode& Node, FVec3& OutMinimum,
		FVec3& OutMaximum, bool& OutIsSphere ) noexcept;

	/** ローカル箱を現在Transformで世界軸に沿う箱へ変換する。 */
	static bool TryMakeWorldBox_Internal( const ANode& Node, FVec3 LocalCenter,
		FVec3 LocalHalfSize, FAabb3& OutBox ) noexcept;

	/** ローカル球を現在Transformで安全側の世界球へ変換する。 */
	static bool TryMakeWorldSphere_Internal( const ANode& Node, FVec3 LocalCenter,
		f32 LocalRadius, FSphere& OutSphere ) noexcept;

	/** ノードと祖先が有効で、破棄予定でなければtrue。非表示は判定へ影響させない。 */
	static bool IsNodeActive_Internal( const ANode& Node ) noexcept;

	/** 全要素が有限ならtrue。 */
	static bool IsFinite_Internal( FVec3 Value ) noexcept;

	/** 指定形状の登録記録を返す。 */
	FRegistration* FindRegistration_Internal( FCollisionShapeId3D Shape ) noexcept;

	/** 指定ノード番号の登録記録を返す。 */
	FRegistration* FindRegistrationByNode_Internal( FNodeId Node ) noexcept;

	/** ACSの形状番号列を、生存するノードポインタ列へ変換する。 */
	bool TryResolveNodes_Internal( const TArray<FCollisionShapeId3D>& Shapes,
		TArray<ANode*>& OutNodes ) noexcept;

	/** グラフのルート交換を検出し、旧ノードの登録を安全に破棄する。 */
	bool RefreshGraphIdentity_Internal() noexcept;

	/** 呼出側より長く生存するノードグラフ。 */
	CSceneNodeGraph* m_Graph = nullptr;

	/** グラフ全置換を見分ける、現在の非所有ルートポインタ。 */
	ANode* m_RootIdentity = nullptr;

	/** 実際の形状計算と世代管理を受け持つACSの衝突集合。 */
	CCollisionWorld3D m_World;

	/** ノード番号、ローカル形状、ACS形状番号の対応表。 */
	TArray<FRegistration> m_Registrations;
};

// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Camera3D/NodeOrbitCamera3DParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 明示した操作量で回りながら、シーンノードの注視点を追う3Dカメラ。
 *
 * @details
 * 場面と追従ノードは所有しない。ACSの決定的な軌道カメラ計算を使い、成功した状態だけを
 * `ALegacyScene3DAdapter`へ反映する。プラットフォーム入力、時刻、追従ノードの移動は所有しない。
 */
class CNodeOrbitCamera3D
{
public:
	/** 接続解除時に、接続前の場面カメラ設定を復元する。 */
	~CNodeOrbitCamera3D() noexcept;

	CNodeOrbitCamera3D() noexcept = default;
	CNodeOrbitCamera3D( const CNodeOrbitCamera3D& ) = delete;
	CNodeOrbitCamera3D& operator=( const CNodeOrbitCamera3D& ) = delete;
	CNodeOrbitCamera3D( CNodeOrbitCamera3D&& ) = delete;
	CNodeOrbitCamera3D& operator=( CNodeOrbitCamera3D&& ) = delete;

	/**
	 * 3D場面と追従ノードへ接続し、軌道カメラを初期位置へ反映する。
	 *
	 * @details 接続中は場面の既定自由操作を止め、軌道カメラ選択と遮蔽物回避設定を所有する。
	 * 再接続失敗時は既存接続と両場面の設定を変更しない。
	 * @param Scene 表示先の3D場面。この型より長く生存すること。
	 * @param Target Sceneのノードグラフが所有する追従ノード。この型より長く生存すること。
	 * @param Params 初期角度、距離、操作速度、遮蔽物回避設定。
	 * @return 全指定を検証し、場面へ反映できたらtrue。
	 */
	bool Bind( ALegacyScene3DAdapter& Scene, ANode& Target, const FNodeOrbitCamera3DParams& Params = FNodeOrbitCamera3DParams{} ) noexcept;

	/** 接続を解除し、場面の自由操作、カメラ選択、遮蔽物回避設定を接続前へ戻す。 */
	void Unbind() noexcept;

	/**
	 * 追従点を読み直し、水平・上下・距離操作を1回反映する。
	 *
	 * @param LookAxes xを右回転、yを見下ろす回転とする操作量。ACS側で[-1,1]へ制限する。
	 * @param ZoomAxis 正で近づき、負で離れる距離操作。ACS側で[-1,1]へ制限する。
	 * @param DeltaSeconds 進める有限かつ0以上の秒数。
	 * @return 接続、追従点、入力、時刻が有効で場面へ反映できたらtrue。
	 */
	bool Update( FVec2 LookAxes, f32 ZoomAxis, f32 DeltaSeconds ) noexcept;

	/** 現在位置の追従点だけを即座に読み直す。 */
	bool RefreshTarget() noexcept { return Update( FVec2{}, 0.0f, 0.0f ); }

	/** 場面と有効な追従ノードへ接続中ならtrue。 */
	bool IsBound() const noexcept { return m_Scene != nullptr && m_Target != nullptr; }

	/** 接続中の追従ノードを返す。未接続ならnullptr。 */
	ANode* Target() const noexcept { return m_Target; }

	/** 次回更新へ渡す現在の軌道カメラ状態を返す。 */
	const COrbitCameraController3D::FOrbitCameraState3D& State() const noexcept { return m_State; }

	/** 現在の検証済み設定を返す。 */
	const FNodeOrbitCamera3DParams& Params() const noexcept { return m_Params; }

private:
	/** 公開指定からACS軌道カメラ計算器と初期状態を作る。 */
	static bool TryBuildController_Internal( const FNodeOrbitCamera3DParams& Params, FVec3 WorldTarget, COrbitCameraController3D& OutController, COrbitCameraController3D::FOrbitCameraState3D& OutState ) noexcept;

	/** 公開指定から場面の遮蔽物回避設定を作る。 */
	static bool TryBuildObstruction_Internal( const FNodeOrbitCamera3DParams& Params, ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D& OutSettings ) noexcept;

	/** ノードの現在変形から有限な世界注視点を作る。 */
	static bool TryWorldTarget_Internal( const ANode& Target, FVec3 LocalOffset, FVec3& OutWorldTarget ) noexcept;

	/** 接続先のカメラ関連設定を接続前へ戻す。 */
	void RestoreSceneSettings_Internal() noexcept;

	/** 呼出側が所有する3D場面。 */
	ALegacyScene3DAdapter* m_Scene = nullptr;

	/** 呼出側のノードグラフが所有する追従先。 */
	ANode* m_Target = nullptr;

	/** 入力、時刻、状態から次の軌道状態を求めるACS計算器。 */
	COrbitCameraController3D m_Controller;

	/** 次回更新へ渡す注視点、角度、距離。 */
	COrbitCameraController3D::FOrbitCameraState3D m_State;

	/** 接続時に検証した操作と表示の指定。 */
	FNodeOrbitCamera3DParams m_Params;

	/** 接続前に場面が既定自由操作を受け付けていたならtrue。 */
	bool m_bPreviousFreeCameraEnabled = true;

	/** 接続前に場面が軌道カメラを明示選択していたならtrue。 */
	bool m_bPreviousOrbitCameraOverrideActive = false;

	/** 接続前に場面が配置済みカメラを明示選択していたならtrue。 */
	bool m_bPreviousAuthoredCameraOverrideActive = false;

	/** 接続前に明示選択されていた配置済みカメラのノード番号。 */
	i32 m_PreviousAuthoredCameraNodeId = -1;

	/** 接続前に明示選択されていた配置済みカメラの安定識別子。 */
	char m_PreviousAuthoredCameraStableId[kScene3DSerializeMaxCameraIdBytes + 1u]{};

	/** 接続前の遮蔽物回避設定。 */
	ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D m_PreviousObstruction;

	/** 接続前の軌道カメラ補間区間。 */
	COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D m_PreviousOrbitSnapshot;

	/** 接続前の軌道カメラ補間区間を復元用に取得できたならtrue。 */
	bool m_bHasPreviousOrbitSnapshot = false;
};

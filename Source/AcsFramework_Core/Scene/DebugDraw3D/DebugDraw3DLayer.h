// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DQueue.h"

class CProximityTrigger3D;

/**
 * 場面の1フレーム線キューをACSのHDR透明3D描画へ接続する所有者。
 *
 * @details 線は深度を無視して常に見えるデバッグオーバーレイになる。GPU資源を場面終了時に
 * 解放するため、所有場面は描画装置を止める前にShutdownを呼ぶ。
 */
class CDebugDraw3DLayer
{
public:
	/** 空の1フレームキューと未初期化の描画器を作る。 */
	CDebugDraw3DLayer() noexcept = default;

	/** GPU資源は所有場面のShutdownで明示解放する。 */
	~CDebugDraw3DLayer() noexcept = default;

	/** GPU資源とキューを単独所有するためコピーを禁止する。 */
	CDebugDraw3DLayer( const CDebugDraw3DLayer& ) = delete;

	/** GPU資源とキューを単独所有するためコピー代入を禁止する。 */
	CDebugDraw3DLayer& operator=( const CDebugDraw3DLayer& ) = delete;

	/** world座標の線を次の3D描画へ1本登録する。 */
	bool DrawLine( FVec3 Start, FVec3 End, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/** 始点から終点へ向く矢印を次の3D描画へ一括登録する。 */
	bool DrawArrow( FVec3 Start, FVec3 End,
		FVec4 Color = FVec4{ 1.0f, 0.72f, 0.16f, 1.0f },
		f32 HeadSize = CDebugDraw3DQueue::kDefaultArrowHeadSize ) noexcept;

	/** 指定位置と回転のローカルX、Y、Z軸を次の3D描画へ一括登録する。 */
	bool DrawAxes( FVec3 Origin, FQuat Rotation = FQuat::Identity(),
		f32 AxisLength = CDebugDraw3DQueue::kDefaultAxisLength,
		f32 HeadSize = CDebugDraw3DQueue::kDefaultArrowHeadSize ) noexcept;

	/** 指定中心の水平XZグリッドを次の3D描画へ一括登録する。 */
	bool DrawGrid( FVec3 Center = FVec3{},
		f32 HalfExtent = CDebugDraw3DQueue::kDefaultGridHalfExtent,
		u32 Divisions = CDebugDraw3DQueue::kDefaultGridDivisions,
		FVec4 Color = FVec4{ 0.28f, 0.36f, 0.48f, 1.0f } ) noexcept;

	/** 指定world法線へ直交する閉じた円を次の3D描画へ一括登録する。 */
	bool DrawCircle( FVec3 Center, FVec3 Normal, f32 Radius,
		FVec4 Color = FVec4{ 1.0f, 0.58f, 0.18f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultCircleSegments ) noexcept;

	/** 指定world方向へ伸びる円錐を次の3D描画へ一括登録する。 */
	bool DrawCone( FVec3 Apex, FVec3 Direction, f32 Length, f32 BaseRadius,
		FVec4 Color = FVec4{ 0.72f, 0.35f, 1.0f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultConeSegments ) noexcept;

	/** 指定world軸へ沿う円柱を次の3D描画へ一括登録する。 */
	bool DrawCylinder( FVec3 Center, FVec3 Axis, f32 Height, f32 Radius,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 0.74f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultCylinderSegments ) noexcept;

	/** 指定world回転を持つ箱の12辺を次の3D描画へ一括登録する。 */
	bool DrawBox( FVec3 Center, FQuat Rotation, FVec3 HalfSize,
		FVec4 Color = FVec4{ 1.0f, 0.38f, 0.72f, 1.0f } ) noexcept;

	/** 軸並行境界箱の12辺を次の3D描画へ一括登録する。 */
	bool DrawAabb( const FAabb3& Bounds, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/** 球を3方向の円として次の3D描画へ一括登録する。 */
	bool DrawSphere( const FSphere& Sphere, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 Segments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

	/**
	 * 接続済み近接トリガーの球または箱を次の3D描画へ一括登録する。
	 *
	 * @param Trigger 表示する有効な近接トリガー。
	 * @param Color 全ての線へ使う色。
	 * @param SphereSegments 球を構成する各円の分割数。箱では使わない。
	 * @return world形状を取得し、必要な線を全て登録できたらtrue。
	 */
	bool DrawProximityTrigger( const CProximityTrigger3D& Trigger,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 SphereSegments = CDebugDraw3DQueue::kDefaultSphereSegments ) noexcept;

	/** 次の3D描画へ登録済みの線数を返す。 */
	usize LineCount() const noexcept { return m_Queue.Num(); }

	/** 不正値、上限、確保失敗により拒否した描画要求の累計を返す。 */
	u64 RejectedDrawCount() const noexcept { return m_Queue.RejectedDrawCount(); }

	/** 次の3D描画へ登録済みの線を全て捨てる。 */
	void Clear() noexcept { m_Queue.Clear(); }

	/**
	 * 登録線を現在のHDR描画先へ描き、成功失敗にかかわらずキューを空にする。
	 *
	 * @return 1本以上のRHI描画commandを追加できたらtrue。
	 */
	bool Render( IRhiDevice& Device, IRhiCommandList& Commands, const CCamera& Camera, IRhiTexture& ColorTarget ) noexcept;

	/** GPU資源と未描画キューを解放し、次の場面開始で再初期化できる状態へ戻す。 */
	void Shutdown() noexcept;

private:
	/** 現在のHDR色形式に合うACS描画器を遅延初期化する。 */
	bool EnsureRenderer_Internal( IRhiDevice& Device, EFormat ColorFormat ) noexcept;

	/** 次の透明3Dパスで消費する、検証済み1フレーム線。 */
	CDebugDraw3DQueue m_Queue;

	/** shader、pipeline、vertex bufferを所有するACS既存描画器。 */
	FDebugDraw3D m_Renderer;

	/** 初期化を試したHDR描画先の色形式。 */
	EFormat m_RenderTargetFormat = EFormat::Unknown;

	/** ACS描画器が現在の色形式へ初期化済みならtrue。 */
	bool m_bRendererReady = false;

	/** 同じ色形式で失敗した初期化を毎フレーム繰り返さないための印。 */
	bool m_bInitializationAttempted = false;

	/** 初期化失敗を同じ場面で1回だけ記録済みならtrue。 */
	bool m_bInitializationWarningIssued = false;

};

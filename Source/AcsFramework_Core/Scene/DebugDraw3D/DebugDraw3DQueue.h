// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/DebugDraw3D/DebugLine3D.h"

/**
 * 1フレーム分の3Dデバッグ線を、GPUへ依存せず上限付きで保持するキュー。
 *
 * @details 描画後にClearする。残したい線は更新ごとに登録し直す。
 */
class CDebugDraw3DQueue
{
public:
	/** 通常の1フレームで保持する線の既定上限。 */
	static constexpr u32 kDefaultCapacity = 16384u;

	/** 方向と先端を立体的に読める矢印の固定線数。 */
	static constexpr u32 kArrowLineCount = 5u;

	/** X、Y、Zの3本の矢印で構成する座標軸の固定線数。 */
	static constexpr u32 kAxesLineCount = kArrowLineCount * 3u;

	/** world単位で指定する矢尻長の既定値。 */
	static constexpr f32 kDefaultArrowHeadSize = 0.25f;

	/** world単位で指定する各座標軸長の既定値。 */
	static constexpr f32 kDefaultAxisLength = 1.0f;

	/** 水平グリッドを面として読める最小分割数。 */
	static constexpr u32 kMinimumGridDivisions = 1u;

	/** 1world単位刻みになる既定の水平グリッド分割数。 */
	static constexpr u32 kDefaultGridDivisions = 10u;

	/** 1要求が線容量と一時領域を過度に消費しない最大分割数。 */
	static constexpr u32 kMaximumGridDivisions = 128u;

	/** 最大分割の水平グリッドを構成する線数。 */
	static constexpr u32 kMaximumGridLineCount = ( kMaximumGridDivisions + 1u ) * 2u;

	/** world原点を中心に既定グリッドが届く片側の距離。 */
	static constexpr f32 kDefaultGridHalfExtent = 5.0f;

	/** 円を輪として読める最小分割数。 */
	static constexpr u32 kMinimumCircleSegments = 4u;

	/** 見た目と線数の釣り合いを取った円の既定分割数。 */
	static constexpr u32 kDefaultCircleSegments = 24u;

	/** 1要求が線容量を過度に消費しない円の最大分割数。 */
	static constexpr u32 kMaximumCircleSegments = 128u;

	/** 円錐の底面へ使う最小分割数。 */
	static constexpr u32 kMinimumConeSegments = kMinimumCircleSegments;

	/** 円錐の底面へ使う既定分割数。 */
	static constexpr u32 kDefaultConeSegments = kDefaultCircleSegments;

	/** 円錐の底面へ使う最大分割数。 */
	static constexpr u32 kMaximumConeSegments = kMaximumCircleSegments;

	/** 円錐の頂点と底面を結ぶ固定側線数。 */
	static constexpr u32 kConeSideLineCount = 4u;

	/** 最大分割の円錐を構成する線数。 */
	static constexpr u32 kMaximumConeLineCount = kMaximumConeSegments + kConeSideLineCount;

	/** 円柱の両端円へ使う最小分割数。 */
	static constexpr u32 kMinimumCylinderSegments = kMinimumCircleSegments;

	/** 円柱の両端円へ使う既定分割数。 */
	static constexpr u32 kDefaultCylinderSegments = kDefaultCircleSegments;

	/** 円柱の両端円へ使う最大分割数。 */
	static constexpr u32 kMaximumCylinderSegments = kMaximumCircleSegments;

	/** 円柱の両端円を結ぶ固定側線数。 */
	static constexpr u32 kCylinderSideLineCount = 4u;

	/** 最大分割の円柱を構成する線数。 */
	static constexpr u32 kMaximumCylinderLineCount = kMaximumCylinderSegments * 2u + kCylinderSideLineCount;

	/** 球の各円へ使う最小分割数。 */
	static constexpr u32 kMinimumSphereSegments = kMinimumCircleSegments;

	/** 球の各円へ使う既定分割数。 */
	static constexpr u32 kDefaultSphereSegments = kDefaultCircleSegments;

	/** 球の各円へ使う最大分割数。 */
	static constexpr u32 kMaximumSphereSegments = kMaximumCircleSegments;

	/**
	 * 空のキューを作る。
	 *
	 * @param Capacity 1フレームで保持できる線の上限。0なら全登録を拒否する。
	 */
	explicit CDebugDraw3DQueue( u32 Capacity = kDefaultCapacity ) noexcept;

	/** キューのメモリを単独所有するためコピーを禁止する。 */
	CDebugDraw3DQueue( const CDebugDraw3DQueue& ) = delete;

	/** キューのメモリを単独所有するためコピー代入を禁止する。 */
	CDebugDraw3DQueue& operator=( const CDebugDraw3DQueue& ) = delete;

	/**
	 * world座標の線を1本登録する。
	 *
	 * @return 値が有効で上限内に保存できたらtrue。
	 */
	bool TryLine( FVec3 Start, FVec3 End, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/**
	 * world座標の始点から終点へ向く矢印を一括登録する。
	 *
	 * @details 胴体1本と立体的な矢尻4本を使う。5本全てを保持できない場合は1本も追加しない。
	 * @param HeadSize world単位の矢尻長。0より大きく、始点から終点までの長さ以下でなければならない。
	 * @return 座標、色、長さが有効で、5本全てを保存できたらtrue。
	 */
	bool TryArrow( FVec3 Start, FVec3 End,
		FVec4 Color = FVec4{ 1.0f, 0.72f, 0.16f, 1.0f },
		f32 HeadSize = kDefaultArrowHeadSize ) noexcept;

	/**
	 * 指定位置と回転のローカルX、Y、Z軸を3本の矢印として一括登録する。
	 *
	 * @details Xは赤、Yは緑、Zは青で固定する。15本全てを保持できない場合は1本も追加しない。
	 * @param Origin 3軸が始まるworld座標。
	 * @param Rotation 3軸へ適用する有限で正規化可能なworld回転。
	 * @param AxisLength 各軸のworld長。0より大きい有限値。
	 * @param HeadSize 各矢尻のworld長。0より大きく、AxisLength以下でなければならない。
	 * @return 位置、回転、寸法が有効で、15本全てを保存できたらtrue。
	 */
	bool TryAxes( FVec3 Origin, FQuat Rotation = FQuat::Identity(),
		f32 AxisLength = kDefaultAxisLength,
		f32 HeadSize = kDefaultArrowHeadSize ) noexcept;

	/**
	 * 指定中心の水平XZ面を等間隔のグリッド線として一括登録する。
	 *
	 * @details X方向とZ方向へ各Divisions+1本を置く。全線を保持できない場合は1本も追加しない。
	 * @param Center グリッド中央のworld座標。yがグリッド面の高さになる。
	 * @param HalfExtent 中心からX、Z各端までのworld距離。0より大きい有限値。
	 * @param Divisions 各方向を等分する1から128の数。
	 * @param Color 全てのグリッド線へ使う色。
	 * @return 位置、寸法、分割数、色が有効で、全線を保存できたらtrue。
	 */
	bool TryGrid( FVec3 Center = FVec3{}, f32 HalfExtent = kDefaultGridHalfExtent,
		u32 Divisions = kDefaultGridDivisions,
		FVec4 Color = FVec4{ 0.28f, 0.36f, 0.48f, 1.0f } ) noexcept;

	/**
	 * 指定world法線へ直交する閉じた円を一括登録する。
	 *
	 * @details Segments本全てを保持できない場合は1本も追加しない。
	 * @param Center 円のworld中心。
	 * @param Normal 円が載る面の有限で正規化可能なworld法線。
	 * @param Radius 円のworld半径。0より大きい有限値。
	 * @param Color 全ての円周線へ使う色。
	 * @param Segments 円周を等分する4から128の数。
	 * @return 中心、法線、半径、色、分割数が有効で、全線を保存できたらtrue。
	 */
	bool TryCircle( FVec3 Center, FVec3 Normal, f32 Radius,
		FVec4 Color = FVec4{ 1.0f, 0.58f, 0.18f, 1.0f },
		u32 Segments = kDefaultCircleSegments ) noexcept;

	/**
	 * 指定world方向へ伸びる円錐を、底面円と4本の側線で一括登録する。
	 *
	 * @details Segments+4本を全て保持できない場合は1本も追加しない。
	 * @param Apex 円錐のworld頂点。
	 * @param Direction 頂点から底面へ向かう有限で正規化可能なworld方向。
	 * @param Length 頂点から底面中心までのworld長。0より大きい有限値。
	 * @param BaseRadius 底面円のworld半径。0より大きい有限値。
	 * @param Color 全ての底面線と側線へ使う色。
	 * @param Segments 底面円を等分する4から128の数。
	 * @return 頂点、方向、寸法、色、分割数が有効で、全線を保存できたらtrue。
	 */
	bool TryCone( FVec3 Apex, FVec3 Direction, f32 Length, f32 BaseRadius,
		FVec4 Color = FVec4{ 0.72f, 0.35f, 1.0f, 1.0f },
		u32 Segments = kDefaultConeSegments ) noexcept;

	/**
	 * 指定world軸へ沿う円柱を、両端円と4本の側線で一括登録する。
	 *
	 * @details 2×Segments+4本を全て保持できない場合は1本も追加しない。
	 * @param Center 円柱のworld中心。
	 * @param Axis 一方の端から他方の端へ向かう有限で正規化可能なworld軸。
	 * @param Height 両端中心間のworld高さ。0より大きい有限値。
	 * @param Radius 両端円のworld半径。0より大きい有限値。
	 * @param Color 全ての端面線と側線へ使う色。
	 * @param Segments 各端円を等分する4から128の数。
	 * @return 中心、軸、寸法、色、分割数が有効で、全線を保存できたらtrue。
	 */
	bool TryCylinder( FVec3 Center, FVec3 Axis, f32 Height, f32 Radius,
		FVec4 Color = FVec4{ 0.20f, 0.95f, 0.74f, 1.0f },
		u32 Segments = kDefaultCylinderSegments ) noexcept;

	/**
	 * 軸並行境界箱の12辺を一括登録する。
	 *
	 * @details 12本全てを保持できない場合は1本も追加しない。
	 * @return 箱と色が有効で、12本全てを保存できたらtrue。
	 */
	bool TryAabb( const FAabb3& Bounds, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/**
	 * 球をXY、XZ、YZの3つの円で一括登録する。
	 *
	 * @details 3×Segments本を全て保持できない場合は1本も追加しない。
	 * @return 球、色、分割数が有効で、全ての線を保存できたらtrue。
	 */
	bool TrySphere( const FSphere& Sphere, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 Segments = kDefaultSphereSegments ) noexcept;

	/** 現在登録されている線の数を返す。 */
	usize Num() const noexcept { return m_Lines.Num(); }

	/** 指定位置に登録された線を返す。IndexはNum未満でなければならない。 */
	const FDebugLine3D& Get( usize Index ) const noexcept { return m_Lines[Index]; }

	/** 登録済みの線を全て捨て、確保済み領域は再利用する。 */
	void Clear() noexcept { m_Lines.Reset(); }

	/** 1フレームで保持できる線の上限を返す。 */
	u32 Capacity() const noexcept { return m_Capacity; }

	/** 不正値、上限、確保失敗により拒否した登録要求の累計を返す。 */
	u64 RejectedDrawCount() const noexcept { return m_RejectedDrawCount; }

private:
	/** 検証済みの複数線を、容量不足や確保失敗で途中状態を残さず追加する。 */
	bool TryAppendLines_Internal( const FDebugLine3D* Lines, usize LineCount ) noexcept;

	/** 指定本数を一括追加できる空きがあるならtrue。 */
	bool HasRoom_Internal( usize LineCount ) const noexcept;

	/** 1フレーム分の検証済み線。 */
	TArray<FDebugLine3D> m_Lines;

	/** 1フレームで保持できる線の上限。 */
	u32 m_Capacity = kDefaultCapacity;

	/** 受け付けられなかった線または形状の要求数。 */
	u64 m_RejectedDrawCount = 0u;
};

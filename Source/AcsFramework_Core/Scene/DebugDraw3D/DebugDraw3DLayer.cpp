// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DLayer.h"

#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3D.h"


bool CDebugDraw3DLayer::DrawLine( FVec3 Start, FVec3 End, FVec4 Color ) noexcept
{
	return m_Queue.TryLine( Start, End, Color );
}


bool CDebugDraw3DLayer::DrawArrow( FVec3 Start, FVec3 End, FVec4 Color, f32 HeadSize ) noexcept
{
	return m_Queue.TryArrow( Start, End, Color, HeadSize );
}


bool CDebugDraw3DLayer::DrawAxes( FVec3 Origin, FQuat Rotation, f32 AxisLength, f32 HeadSize ) noexcept
{
	return m_Queue.TryAxes( Origin, Rotation, AxisLength, HeadSize );
}


bool CDebugDraw3DLayer::DrawGrid( FVec3 Center, f32 HalfExtent, u32 Divisions, FVec4 Color ) noexcept
{
	return m_Queue.TryGrid( Center, HalfExtent, Divisions, Color );
}


bool CDebugDraw3DLayer::DrawCircle( FVec3 Center, FVec3 Normal, f32 Radius,
	FVec4 Color, u32 Segments ) noexcept
{
	return m_Queue.TryCircle( Center, Normal, Radius, Color, Segments );
}


bool CDebugDraw3DLayer::DrawCone( FVec3 Apex, FVec3 Direction, f32 Length,
	f32 BaseRadius, FVec4 Color, u32 Segments ) noexcept
{
	return m_Queue.TryCone( Apex, Direction, Length, BaseRadius, Color, Segments );
}


bool CDebugDraw3DLayer::DrawCylinder( FVec3 Center, FVec3 Axis, f32 Height,
	f32 Radius, FVec4 Color, u32 Segments ) noexcept
{
	return m_Queue.TryCylinder( Center, Axis, Height, Radius, Color, Segments );
}


bool CDebugDraw3DLayer::DrawAabb( const FAabb3& Bounds, FVec4 Color ) noexcept
{
	return m_Queue.TryAabb( Bounds, Color );
}


bool CDebugDraw3DLayer::DrawSphere( const FSphere& Sphere, FVec4 Color, u32 Segments ) noexcept
{
	return m_Queue.TrySphere( Sphere, Color, Segments );
}


bool CDebugDraw3DLayer::DrawProximityTrigger( const CProximityTrigger3D& Trigger,
	FVec4 Color, u32 SphereSegments ) noexcept
{
	return TryQueueProximityTrigger3D( Trigger, m_Queue, Color, SphereSegments );
}


bool CDebugDraw3DLayer::Render( IRhiDevice& Device, IRhiCommandList& Commands, const CCamera& Camera, IRhiTexture& ColorTarget ) noexcept
{
	if ( m_Queue.Num() == 0u ) return false;
	if ( !EnsureRenderer_Internal( Device, ColorTarget.PixelFormat() ) )
	{
		m_Queue.Clear();
		return false;
	}

	m_Renderer.Begin();
	/** FrameworkとACSの固定上限がずれていないことも同時に確認する結果。 */
	bool bAllLinesAccepted = true;
	/** 登録順を保って検証済み線をACSのCPU頂点列へ渡す。 */
	for ( usize Index = 0u; Index < m_Queue.Num(); ++Index )
	{
		/** 現在ACS描画器へ渡す1本。 */
		const FDebugLine3D& Line = m_Queue.Get( Index );
		bAllLinesAccepted = m_Renderer.TryLine( Line.Start, Line.End, Line.Color ) && bAllLinesAccepted;
	}
	m_Queue.Clear();

	if ( !bAllLinesAccepted ) ACS_LOG_WARN( "DebugDraw3D: ACS描画器の線容量とFrameworkキューが一致していません" );
	if ( m_Renderer.LineCount() == 0u ) return false;

	m_Renderer.End( Commands, Camera.ViewProjection() );
	return true;
}


void CDebugDraw3DLayer::Shutdown() noexcept
{
	m_Queue.Clear();
	m_Renderer.Shutdown();
	m_RenderTargetFormat = EFormat::Unknown;
	m_bRendererReady = false;
	m_bInitializationAttempted = false;
	m_bInitializationWarningIssued = false;
}


bool CDebugDraw3DLayer::EnsureRenderer_Internal( IRhiDevice& Device, EFormat ColorFormat ) noexcept
{
	if ( m_bRendererReady && m_RenderTargetFormat == ColorFormat ) return true;

	if ( m_RenderTargetFormat != ColorFormat )
	{
		m_Renderer.Shutdown();
		m_RenderTargetFormat = ColorFormat;
		m_bRendererReady = false;
		m_bInitializationAttempted = false;
		m_bInitializationWarningIssued = false;
	}

	if ( m_bInitializationAttempted ) return false;
	m_bInitializationAttempted = true;

	/** shader、pipeline、bufferを全て作れた場合だけ成功する初期化結果。 */
	const TResult<void> Result = m_Renderer.Init( Device, ColorFormat, CDebugDraw3DQueue::kDefaultCapacity );
	if ( Result.IsErr() )
	{
		if ( !m_bInitializationWarningIssued )
		{
			/** ACSが返した失敗理由。文字列が無い場合だけ日本語の代替表示を使う。 */
			const char* const Message = Result.Error().message != nullptr ? Result.Error().message : "理由不明";
			ACS_LOG_WARN( "DebugDraw3D: 描画器を初期化できません (%s)", Message );
			m_bInitializationWarningIssued = true;
		}
		return false;
	}

	m_bRendererReady = true;
	return true;
}

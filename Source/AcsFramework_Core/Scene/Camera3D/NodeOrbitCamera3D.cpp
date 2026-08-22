// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Camera3D/NodeOrbitCamera3D.h"

#include <cmath>
#include <cstring>


CNodeOrbitCamera3D::~CNodeOrbitCamera3D() noexcept
{
	Unbind();
}


bool CNodeOrbitCamera3D::Bind( ALegacyScene3DAdapter& Scene, ANode& Target, const FNodeOrbitCamera3DParams& Params ) noexcept
{
	if ( Target.IsPendingDestroy() || !Scene.Graph().IdOf( &Target ).IsValid() ) return false;

	FVec3 WorldTarget;
	if ( !TryWorldTarget_Internal( Target, Params.LocalTargetOffset, WorldTarget ) ) return false;

	COrbitCameraController3D CandidateController;
	COrbitCameraController3D::FOrbitCameraState3D CandidateState;
	if ( !TryBuildController_Internal( Params, WorldTarget, CandidateController, CandidateState ) ) return false;

	ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D CandidateObstruction;
	if ( !TryBuildObstruction_Internal( Params, CandidateObstruction ) ) return false;

	const bool bSameScene = m_Scene == &Scene;
	const bool bPreviousFreeCameraEnabled = Scene.FreeCameraEnabled();
	const bool bPreviousOrbitCameraOverrideActive = Scene.OrbitCameraOverrideActive();
	const bool bPreviousAuthoredCameraOverrideActive = Scene.AuthoredCameraOverrideActive();
	const FScene3DCameraState* const PreviousAuthoredCamera = Scene.AuthoredCamera();
	const i32 PreviousAuthoredCameraNodeId = bPreviousAuthoredCameraOverrideActive && PreviousAuthoredCamera != nullptr ? PreviousAuthoredCamera->NodeId : -1;
	// 実行時に置かれたカメラも復元するために複製する安定識別子。
	char PreviousAuthoredCameraStableId[kScene3DSerializeMaxCameraIdBytes + 1u]{};
	if ( bPreviousAuthoredCameraOverrideActive && PreviousAuthoredCamera != nullptr ) std::memcpy( PreviousAuthoredCameraStableId, PreviousAuthoredCamera->StableId, sizeof( PreviousAuthoredCameraStableId ) );
	const ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D PreviousObstruction = Scene.OrbitCameraObstructionSettings();
	COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D PreviousOrbitSnapshot;
	if ( !bSameScene && bPreviousAuthoredCameraOverrideActive && ( PreviousAuthoredCamera == nullptr || PreviousAuthoredCameraStableId[0] == '\0' ) ) return false;
	if ( !bSameScene && !Scene.TryCaptureOrbitCameraSnapshot( PreviousOrbitSnapshot ) ) return false;
	if ( !Scene.TrySetOrbitCameraObstructionSettings( CandidateObstruction ) ) return false;

	if ( m_Scene != nullptr && !bSameScene ) RestoreSceneSettings_Internal();
	if ( !bSameScene )
	{
		m_bPreviousFreeCameraEnabled = bPreviousFreeCameraEnabled;
		m_bPreviousOrbitCameraOverrideActive = bPreviousOrbitCameraOverrideActive;
		m_bPreviousAuthoredCameraOverrideActive = bPreviousAuthoredCameraOverrideActive;
		m_PreviousAuthoredCameraNodeId = PreviousAuthoredCameraNodeId;
		std::memcpy( m_PreviousAuthoredCameraStableId, PreviousAuthoredCameraStableId, sizeof( m_PreviousAuthoredCameraStableId ) );
		m_PreviousObstruction = PreviousObstruction;
		m_PreviousOrbitSnapshot = PreviousOrbitSnapshot;
		m_bHasPreviousOrbitSnapshot = true;
	}

	m_Scene = &Scene;
	m_Target = &Target;
	m_Controller = CandidateController;
	m_State = CandidateState;
	m_Params = Params;
	m_ShakeCamera.StopShake();
	m_Scene->SetFreeCameraEnabled( false );
	m_Scene->SetOrbitCameraActive( true );
	m_Scene->SetOrbit( m_State.target, m_State.yaw_radians, m_State.pitch_radians, m_State.distance );
	return true;
}


void CNodeOrbitCamera3D::Unbind() noexcept
{
	RestoreSceneSettings_Internal();
	m_Scene = nullptr;
	m_Target = nullptr;
	m_Controller = COrbitCameraController3D{};
	m_State = COrbitCameraController3D::FOrbitCameraState3D{};
	m_ShakeCamera.StopShake();
	m_Params = FNodeOrbitCamera3DParams{};
	m_bPreviousFreeCameraEnabled = true;
	m_bPreviousOrbitCameraOverrideActive = false;
	m_bPreviousAuthoredCameraOverrideActive = false;
	m_PreviousAuthoredCameraNodeId = -1;
	std::memset( m_PreviousAuthoredCameraStableId, 0, sizeof( m_PreviousAuthoredCameraStableId ) );
	m_PreviousObstruction = ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D{};
	m_PreviousOrbitSnapshot = COrbitCameraController3D::FOrbitCameraFixedStepSnapshot3D{};
	m_bHasPreviousOrbitSnapshot = false;
}


bool CNodeOrbitCamera3D::Update( FVec2 LookAxes, f32 ZoomAxis, f32 DeltaSeconds ) noexcept
{
	if ( m_Scene == nullptr || m_Target == nullptr || m_Target->IsPendingDestroy() ) return false;

	FVec3 WorldTarget;
	if ( !TryWorldTarget_Internal( *m_Target, m_Params.LocalTargetOffset, WorldTarget ) ) return false;

	COrbitCameraController3D::FOrbitCameraInput3D Input;
	Input.look_yaw = LookAxes.x;
	Input.look_pitch = LookAxes.y;
	Input.zoom = ZoomAxis;
	COrbitCameraController3D::FOrbitCameraState3D Candidate = m_State;
	Candidate.target = WorldTarget;
	if ( !m_Controller.TryStep( Input, DeltaSeconds, Candidate ) ) return false;

	m_ShakeCamera.Tick( DeltaSeconds );
	const FVec3 ShakeEye = m_ShakeCamera.EffectiveEye();
	const FVec3 ShakeBaseEye = m_ShakeCamera.Position();
	const FVec3 PresentationTarget{ Candidate.target.x + ShakeEye.x - ShakeBaseEye.x, Candidate.target.y + ShakeEye.y - ShakeBaseEye.y, Candidate.target.z + ShakeEye.z - ShakeBaseEye.z };
	m_State = Candidate;
	m_Scene->SetOrbit( PresentationTarget, m_State.yaw_radians, m_State.pitch_radians, m_State.distance );
	return true;
}


bool CNodeOrbitCamera3D::TryShakePreset( EShakePreset Preset ) noexcept
{
	if ( Preset == EShakePreset::Custom ) return false;
	return TryAddShake( CCameraShakePresets::GetPreset( Preset ) );
}


bool CNodeOrbitCamera3D::TryAddShake( const FShakeParams& Params ) noexcept
{
	if ( !IsBound() ) return false;
	if ( !std::isfinite( Params.trauma ) || !std::isfinite( Params.amplitude ) || !std::isfinite( Params.decay_rate ) || !std::isfinite( Params.frequency ) || !std::isfinite( Params.duration_hint ) ) return false;
	if ( Params.trauma <= 0.0f || Params.amplitude < 0.0f || Params.decay_rate <= 0.0f || Params.frequency <= 0.0f || Params.duration_hint < 0.0f ) return false;

	m_ShakeCamera.SetShakeAmplitude( Params.amplitude );
	m_ShakeCamera.SetShakeDecayRate( Params.decay_rate );
	m_ShakeCamera.SetShakeFrequency( Params.frequency );
	m_ShakeCamera.AddShake( Params.trauma );
	return true;
}


void CNodeOrbitCamera3D::StopShake() noexcept
{
	m_ShakeCamera.StopShake();
	if ( m_Scene != nullptr ) m_Scene->SetOrbit( m_State.target, m_State.yaw_radians, m_State.pitch_radians, m_State.distance );
}


bool CNodeOrbitCamera3D::TryBuildController_Internal( const FNodeOrbitCamera3DParams& Params, FVec3 WorldTarget, COrbitCameraController3D& OutController, COrbitCameraController3D::FOrbitCameraState3D& OutState ) noexcept
{
	COrbitCameraController3D::FOrbitCameraSettings3D Settings;
	Settings.yaw_radians_per_second = Params.YawDegreesPerSecond * kDeg2Rad;
	Settings.pitch_radians_per_second = Params.PitchDegreesPerSecond * kDeg2Rad;
	Settings.pitch_limit_radians = Params.PitchLimitDegrees * kDeg2Rad;
	Settings.zoom_distance_scale_per_second = Params.ZoomDistanceScalePerSecond;
	Settings.minimum_distance = Params.MinimumDistance;
	Settings.maximum_distance = Params.MaximumDistance;

	// 場面のSetOrbitが保持できる角度と距離の既定範囲。
	const COrbitCameraController3D SceneController;
	const auto& SceneSettings = SceneController.Settings();
	if ( Settings.pitch_limit_radians > SceneSettings.pitch_limit_radians || Settings.minimum_distance < SceneSettings.minimum_distance || Settings.maximum_distance > SceneSettings.maximum_distance ) return false;

	COrbitCameraController3D Controller;
	if ( !Controller.TryConfigure( Settings ) ) return false;
	if ( !std::isfinite( Params.InitialYawDegrees ) || !std::isfinite( Params.InitialPitchDegrees ) || !std::isfinite( Params.InitialDistance ) ) return false;
	if ( std::abs( Params.InitialPitchDegrees ) > Params.PitchLimitDegrees || Params.InitialDistance < Params.MinimumDistance || Params.InitialDistance > Params.MaximumDistance ) return false;

	COrbitCameraController3D::FOrbitCameraState3D State;
	State.target = WorldTarget;
	State.yaw_radians = Params.InitialYawDegrees * kDeg2Rad;
	State.pitch_radians = Params.InitialPitchDegrees * kDeg2Rad;
	State.distance = Params.InitialDistance;
	if ( !Controller.TryStep( {}, 0.0f, State ) ) return false;

	OutController = Controller;
	OutState = State;
	return true;
}


bool CNodeOrbitCamera3D::TryBuildObstruction_Internal( const FNodeOrbitCamera3DParams& Params, ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D& OutSettings ) noexcept
{
	if ( !std::isfinite( Params.TargetClearance ) || !std::isfinite( Params.CameraClearance ) || !std::isfinite( Params.ProbeRadius ) || !std::isfinite( Params.RecoverySharpness ) ) return false;
	if ( Params.TargetClearance <= 0.0f || Params.CameraClearance < 0.0f || Params.ProbeRadius < 0.0f || Params.RecoverySharpness < 0.0f ) return false;
	// 注視点側とカメラ側の余白を差し引いた有効距離。
	const f32 ResolvedClearance = Params.TargetClearance - Params.CameraClearance;
	if ( !std::isfinite( ResolvedClearance ) || ResolvedClearance < 0.01f ) return false;

	ALegacyScene3DAdapter::FOrbitCameraObstructionSettings3D Settings;
	Settings.Enabled = Params.bAvoidObstructions;
	Settings.TargetClearance = Params.TargetClearance;
	Settings.CameraClearance = Params.CameraClearance;
	Settings.ProbeRadius = Params.ProbeRadius;
	Settings.RecoverySharpness = Params.RecoverySharpness;
	OutSettings = Settings;
	return true;
}


bool CNodeOrbitCamera3D::TryWorldTarget_Internal( const ANode& Target, FVec3 LocalOffset, FVec3& OutWorldTarget ) noexcept
{
	if ( !std::isfinite( LocalOffset.x ) || !std::isfinite( LocalOffset.y ) || !std::isfinite( LocalOffset.z ) ) return false;
	const FVec3 WorldTarget = TransformPoint( LocalOffset, Target.World().ToMat4() );
	if ( !std::isfinite( WorldTarget.x ) || !std::isfinite( WorldTarget.y ) || !std::isfinite( WorldTarget.z ) ) return false;
	OutWorldTarget = WorldTarget;
	return true;
}


void CNodeOrbitCamera3D::RestoreSceneSettings_Internal() noexcept
{
	if ( m_Scene == nullptr ) return;
	m_Scene->TrySetOrbitCameraObstructionSettings( m_PreviousObstruction );
	m_Scene->SetFreeCameraEnabled( m_bPreviousFreeCameraEnabled );
	if ( m_bPreviousOrbitCameraOverrideActive )
	{
		m_Scene->SetOrbitCameraActive( true );
	}
	else if ( m_bPreviousAuthoredCameraOverrideActive )
	{
		// 読み込み済みノード番号を優先し、実行時カメラは安定識別子で探す。
		bool bRestored = m_PreviousAuthoredCameraNodeId >= 0 && m_Scene->SetActiveCamera( m_PreviousAuthoredCameraNodeId );
		if ( !bRestored && m_PreviousAuthoredCameraStableId[0] != '\0' ) bRestored = m_Scene->SetActiveCamera( m_PreviousAuthoredCameraStableId );
		if ( !bRestored ) m_Scene->UseAutomaticCameraSelection();
	}
	else
	{
		m_Scene->UseAutomaticCameraSelection();
	}
	if ( m_bHasPreviousOrbitSnapshot ) m_Scene->TryRestoreOrbitCameraSnapshot( m_PreviousOrbitSnapshot );
}

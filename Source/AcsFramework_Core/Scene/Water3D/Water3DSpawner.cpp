// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Water3D/Water3DSpawner.h"

#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

ANode* CWater3DSpawner::SpawnInto( CSceneNodeGraph& Graph, const FWater3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;

	FModel3DSpawnParams Plane = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, Params.Position );
	Plane.Scale = FVec3{ Params.Size.x, 1.0f, Params.Size.y };
	Plane.Roughness = Params.Roughness;
	Plane.bCastsShadow = false;
	Plane.Name = Params.Name;

	ANode* const Node = CModel3DSpawner::SpawnInto( Graph, Plane, Parent );
	if ( Node == nullptr ) return nullptr;

	AWaterSurface3DComponent& Water = Node->AddComponent<AWaterSurface3DComponent>();
	Water.shallowColor = Params.ShallowColor;
	Water.deepColor = Params.DeepColor;
	Water.flowDirection = Params.FlowDirection;
	Water.roughness = Params.Roughness;
	Water.normalStrength = Params.NormalStrength;
	Water.normalTiling = Params.NormalTiling;
	Water.waveAmplitude = Params.WaveAmplitude;
	Water.waveScale = Params.WaveScale;
	Water.waveSpeed = Params.WaveSpeed;
	Water.rippleSpeed = Params.RippleSpeed;
	Water.rippleWavelength = Params.RippleWavelength;
	Water.rippleLifetime = Params.RippleLifetime;
	Water.rippleDamping = Params.RippleDamping;
	Water.refractionStrength = Params.RefractionStrength;
	Water.opticalDepth = Params.OpticalDepth;
	Water.foamIntensity = Params.FoamIntensity;

	return Node;
}

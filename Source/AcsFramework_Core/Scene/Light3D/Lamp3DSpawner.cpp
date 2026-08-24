// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawner.h"

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"


FLamp3DSpawnResult CLamp3DSpawner::SpawnInto(
	CSceneNodeGraph& Graph, const FLamp3DParams& Params,
	ANode* Parent ) noexcept
{
	return FLamp3DSpawnResult::TrySpawnInto( Graph, Params, Parent );
}


bool CLamp3DSpawner::TryApplyTo( CSceneNodeGraph& Graph,
	const FLamp3DSpawnResult& Spawned,
	const FLamp3DParams& Params ) noexcept
{
	FModel3DSpawnParams BulbParams;
	FLight3DSpawnParams LightParams;
	if ( !Params.TryBuildParts( BulbParams, LightParams )
		|| !CanApply_Internal( Graph, Spawned ) ) return false;

	ANode* const Bulb = Spawned.Bulb();
	ANode* const Light = Spawned.Light();
	AMeshComponent3D* const Mesh = Bulb->GetComponent<AMeshComponent3D>();
	if ( !CLight3DSpawner::TryApplyTo( *Light, LightParams ) ) return false;

	ApplyBulb_Internal( *Bulb, *Mesh, BulbParams );
	if ( BulbParams.Name.Data() != nullptr && BulbParams.Name.Size() > 0u )
		Bulb->SetName( BulbParams.Name );
	if ( LightParams.Name.Data() != nullptr && LightParams.Name.Size() > 0u )
		Light->SetName( LightParams.Name );
	return true;
}


bool CLamp3DSpawner::Destroy( CSceneNodeGraph& Graph,
	FLamp3DSpawnResult& Spawned ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Spawned ) ) return false;

	const FNodeId NodeIds[]
	{
		Spawned.LightId(),
		Spawned.BulbId(),
	};
	for ( const FNodeId NodeId : NodeIds )
	{
		ANode* const Node = Graph.Get( NodeId );
		if ( Node != nullptr && !Node->IsPendingDestroy()
			&& !Graph.Destroy( NodeId ) ) return false;
	}

	Spawned.Reset();
	return true;
}


bool CLamp3DSpawner::CanApply_Internal( CSceneNodeGraph& Graph,
	const FLamp3DSpawnResult& Spawned ) noexcept
{
	if ( !Spawned || !Graph.HasRoot() || !Spawned.IsOwnedBy( Graph )
		|| !Spawned.IsFromRoot( Graph.Root() )
		|| Spawned.BulbId() == Spawned.LightId() ) return false;

	ANode* const Bulb = Spawned.Bulb();
	ANode* const Light = Spawned.Light();
	return Bulb != nullptr && Light != nullptr
		&& Bulb != &Graph.Root() && Light != &Graph.Root()
		&& Graph.Get( Spawned.BulbId() ) == Bulb
		&& Graph.Get( Spawned.LightId() ) == Light
		&& Bulb->GetComponent<AMeshComponent3D>() != nullptr
		&& Light->GetComponent<ALightComponent3D>() != nullptr;
}


void CLamp3DSpawner::ApplyBulb_Internal( ANode& Bulb,
	AMeshComponent3D& Mesh,
	const FModel3DSpawnParams& Params ) noexcept
{
	Bulb.Local().position = Params.Position;
	Bulb.Local().rotation = FQuat::Identity();
	Bulb.Local().scale = Params.Scale;
	Mesh.SetMeshAsset( TSharedPtr<AAsset>{} );
	Mesh.SetMeshPath( FStringView{} );
	Mesh.SetPrimitive( EMeshPrimitive3D::Sphere );
	Mesh.SetColor( Params.Color );
	Mesh.SetCastsShadow( Params.bCastsShadow );

	FMaterial2D Material{};
	Material.pbr.roughness = Params.Roughness;
	Material.pbr.emissive = Params.EmissiveColor;
	Material.pbr.emissiveStrength = Params.EmissiveStrength;
	Mesh.SetMaterial( Material );
}


bool CLamp3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	const FLamp3DSpawnResult& Spawned ) noexcept
{
	if ( !Spawned || !Graph.HasRoot() || !Spawned.IsOwnedBy( Graph )
		|| !Spawned.IsFromRoot( Graph.Root() ) ) return false;

	const FNodeId BulbId = Spawned.BulbId();
	const FNodeId LightId = Spawned.LightId();
	if ( BulbId == LightId ) return false;

	ANode* const Bulb = Graph.Get( BulbId );
	ANode* const Light = Graph.Get( LightId );
	if ( Bulb == &Graph.Root() || Light == &Graph.Root() ) return false;
	if ( Bulb != nullptr && Bulb->GetComponent<AMeshComponent3D>() == nullptr ) return false;
	if ( Light != nullptr && Light->GetComponent<ALightComponent3D>() == nullptr ) return false;
	return true;
}

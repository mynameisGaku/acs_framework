// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"

FModel3DSpawnParams FModel3DSpawnParams::FromMesh( FStringView Path, FVec3 InPosition ) noexcept
{
	FModel3DSpawnParams Params;
	Params.MeshPath = Path;
	Params.Primitive = EMeshPrimitive3D::Mesh;
	Params.Position = InPosition;

	return Params;
}


FModel3DSpawnParams FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition ) noexcept
{
	FModel3DSpawnParams Params;
	Params.Primitive = InPrimitive;
	Params.Position = InPosition;

	return Params;
}


bool FModel3DSpawnParams::IsValid() const noexcept
{
	// 0 倍は「置いたのに見えない」になる。負は鏡写しとして使うので通す。
	if ( Scale.x == 0.0f || Scale.y == 0.0f || Scale.z == 0.0f ) return false;

	// モデルを指しているのに場所も読込済みモデルも無ければ、何も描けない。
	const bool bWantsMesh = Primitive == EMeshPrimitive3D::Mesh;
	const bool bHasMeshPath = MeshPath.Data() != nullptr && MeshPath.Size() > 0u;
	if ( bWantsMesh && !bHasMeshPath && !MeshAsset ) return false;
	if ( MeshAsset && MeshAsset->Type() != AMeshAsset::StaticType() ) return false;

	return true;
}

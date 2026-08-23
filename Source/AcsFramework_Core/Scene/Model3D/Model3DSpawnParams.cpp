// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 自己発光色として使える有限な0から1のRGBか返す。 */
	bool IsEmissiveColorValid_Internal( FVec3 Color ) noexcept
	{
		return std::isfinite( Color.x ) && std::isfinite( Color.y ) && std::isfinite( Color.z )
			&& Color.x >= 0.0f && Color.x <= 1.0f
			&& Color.y >= 0.0f && Color.y <= 1.0f
			&& Color.z >= 0.0f && Color.z <= 1.0f;
	}

	/** 材質の割合として使える有限な0から1の値か返す。 */
	bool IsMaterialRatioValid_Internal( f32 Value ) noexcept
	{
		return std::isfinite( Value ) && Value >= 0.0f && Value <= 1.0f;
	}
}

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


FModel3DSpawnParams FModel3DSpawnParams::FromToonPrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor ) noexcept
{
	FModel3DSpawnParams Params = FromPrimitive( InPrimitive, InPosition );
	Params.Color = FVec4{ InColor.x, InColor.y, InColor.z, 1.0f };
	Params.bToonShading = true;
	return Params;
}


FModel3DSpawnParams FModel3DSpawnParams::FromCoatedPrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor, f32 InCoatRoughness ) noexcept
{
	FModel3DSpawnParams Params = FromPrimitive( InPrimitive, InPosition );
	Params.Color = FVec4{ InColor.x, InColor.y, InColor.z, 1.0f };
	Params.Clearcoat = 1.0f;
	Params.ClearcoatRoughness = InCoatRoughness;
	return Params;
}


FModel3DSpawnParams FModel3DSpawnParams::FromEmissivePrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor, f32 InStrength ) noexcept
{
	FModel3DSpawnParams Params = FromPrimitive( InPrimitive, InPosition );
	Params.Color = FVec4{ InColor.x, InColor.y, InColor.z, 1.0f };
	Params.EmissiveColor = InColor;
	Params.EmissiveStrength = InStrength;
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
	if ( !IsMaterialRatioValid_Internal( Clearcoat ) ) return false;
	if ( !IsMaterialRatioValid_Internal( ClearcoatRoughness ) ) return false;
	if ( !IsEmissiveColorValid_Internal( EmissiveColor ) ) return false;
	if ( !std::isfinite( EmissiveStrength ) || EmissiveStrength < 0.0f || EmissiveStrength > 10.0f ) return false;

	return true;
}

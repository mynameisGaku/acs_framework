// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimator3D.h"


bool CCharacterAnimator3D::Bind( ASkinnedMeshComponent3D& Component,
	const FCharacterAnimation3DProfile& Profile ) noexcept
{
	if ( !CanBind_Internal( Component, Profile ) ) return false;
	if ( !Component.PlayByName( Profile.IdleClip.View(), true ) ) return false;

	m_Component = &Component;
	m_Profile = Profile;
	m_CurrentState = EAnimationGraphState::Idle;
	return true;
}


bool CCharacterAnimator3D::Bind( ANode& Node,
	const FCharacterAnimation3DProfile& Profile ) noexcept
{
	ASkinnedMeshComponent3D* const Component = Node.GetComponent<ASkinnedMeshComponent3D>();
	return Component != nullptr && Bind( *Component, Profile );
}


void CCharacterAnimator3D::Unbind() noexcept
{
	m_Component = nullptr;
	m_Profile = FCharacterAnimation3DProfile{};
	m_CurrentState = EAnimationGraphState::Idle;
}


bool CCharacterAnimator3D::Update( const FCharacterAnimation3DInput& Input ) noexcept
{
	if ( m_Component == nullptr ) return false;

	EAnimationGraphState NextState = m_CurrentState;
	if ( !m_Profile.TrySelectState( Input, m_CurrentState, NextState ) ) return false;
	if ( NextState == m_CurrentState ) return true;
	if ( !m_Component->BlendToByName(
		m_Profile.ClipFor( NextState ), m_Profile.BlendSeconds, m_Profile.LoopsFor( NextState ) ) ) return false;

	m_CurrentState = NextState;
	return true;
}


bool CCharacterAnimator3D::HasAnimation_Internal(
	const ASkinnedMeshAsset& Mesh, FStringView Name ) noexcept
{
	const TArray<FAnimation>& Animations = Mesh.Animations();
	for ( usize Index = 0u; Index < Animations.Num(); ++Index )
	{
		if ( Animations[Index].name.View() == Name ) return true;
	}
	return false;
}


bool CCharacterAnimator3D::CanBind_Internal( const ASkinnedMeshComponent3D& Component,
	const FCharacterAnimation3DProfile& Profile ) noexcept
{
	if ( !Profile.IsValid() || !Component.IsRenderable() ) return false;

	const TSharedPtr<ASkinnedMeshAsset>& Mesh = Component.MeshAsset();
	if ( !Mesh ) return false;
	return HasAnimation_Internal( *Mesh, Profile.IdleClip.View() )
		&& HasAnimation_Internal( *Mesh, Profile.WalkClip.View() )
		&& HasAnimation_Internal( *Mesh, Profile.RunClip.View() )
		&& HasAnimation_Internal( *Mesh, Profile.JumpClip.View() );
}

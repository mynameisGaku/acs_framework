// SPDX-License-Identifier: Apache-2.0
#include "SaveSubsystem.h"

// GameInstanceの寿命で所有し、シーンを跨いで同じ保存設定を維持する。
ACS_REGISTER_SUBSYSTEM( CSaveSubsystem, ESubsystemScope::GameInstance )


void CSaveSubsystem::Configure( const FString& Directory, const FString& BaseName, i32 SlotCount )
{
	m_Store.Configure( Directory, BaseName, SlotCount );
}


FString CSaveSubsystem::GetSlotPath( i32 Slot ) const
{
	return m_Store.GetSlotPath( Slot );
}


bool CSaveSubsystem::Exists( i32 Slot ) const
{
	return m_Store.Exists( Slot );
}


FSaveSlotInfo CSaveSubsystem::GetSlotInfo( i32 Slot ) const
{
	return m_Store.GetSlotInfo( Slot );
}


bool CSaveSubsystem::Erase( i32 Slot )
{
	return m_Store.Erase( Slot );
}

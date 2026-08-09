#include "SaveSubsystem.h"

namespace
{
	/** セーブの拡張子 (エンジンの書式に合わせる)。 */
	constexpr const char* kSaveExtension = ".acssave";
}

// GameInstance スコープへ登録する。シーンを跨いでも同じ置き場所を指す。
ACS_REGISTER_SUBSYSTEM( CSaveSubsystem, ESubsystemScope::GameInstance )


void CSaveSubsystem::Configure( const FString& Directory, const FString& BaseName, i32 SlotCount )
{
	m_Directory = Directory;
	m_BaseName = BaseName;
	m_SlotCount = SlotCount > 0 ? SlotCount : 0;
}


FString CSaveSubsystem::GetSlotPath( i32 Slot ) const
{
	if ( Slot < 0 || Slot >= m_SlotCount ) return FString();

	FString Path = m_Directory;
	if ( !Path.IsEmpty() && !CFileSystem::IsPathSeparator( Path.Data()[Path.Size() - 1] ) ) Path.Append( '/' );

	Path.Append( m_BaseName.View() );
	Path.AppendFormat( "%d", Slot );
	Path.Append( kSaveExtension );
	return Path;
}


bool CSaveSubsystem::BuildSlotPath( i32 Slot, TArray<wchar_t>& OutWide ) const
{
	const FString Path = GetSlotPath( Slot );
	if ( Path.IsEmpty() ) return false;

	return AcsToWide( Path, OutWide );
}


void CSaveSubsystem::EnsureDirectory() const
{
	if ( m_Directory.IsEmpty() ) return;

	TArray<wchar_t> Wide;
	if ( !AcsToWide( m_Directory, Wide ) ) return;

	if ( CFileSystem::DirectoryExists( Wide.GetData() ) ) return;

	// 作れなくても書き込みの側で失敗が出るので、ここでは黙って進む。
	( void )CFileSystem::CreateDirectory( Wide.GetData() );
}


bool CSaveSubsystem::Exists( i32 Slot ) const
{
	TArray<wchar_t> Wide;
	if ( !BuildSlotPath( Slot, Wide ) ) return false;

	return CFileSystem::Exists( Wide.GetData() );
}


FSaveSlotInfo CSaveSubsystem::GetSlotInfo( i32 Slot ) const
{
	FSaveSlotInfo Info;
	Info.Index = Slot;
	Info.Path = GetSlotPath( Slot );
	if ( Info.Path.IsEmpty() ) return Info;

	TArray<wchar_t> Wide;
	if ( !AcsToWide( Info.Path, Wide ) ) return Info;

	// 中身は読まない。header だけ見れば、一覧に要ることは揃う。
	const auto Version = CSaveArchive::PeekVersion( Wide.GetData() );
	if ( !Version.IsOk() ) return Info;

	Info.bExists = true;
	Info.Version = Version.Value();

	const auto Size = CSaveArchive::PeekPayloadSize( Wide.GetData() );
	if ( Size.IsOk() ) Info.SizeBytes = Size.Value();

	return Info;
}


bool CSaveSubsystem::Erase( i32 Slot )
{
	TArray<wchar_t> Wide;
	if ( !BuildSlotPath( Slot, Wide ) ) return false;

	if ( !CFileSystem::Exists( Wide.GetData() ) ) return true;

	return CFileSystem::Delete( Wide.GetData() ).IsOk();
}

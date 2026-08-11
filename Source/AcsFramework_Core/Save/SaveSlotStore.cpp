// SPDX-License-Identifier: Apache-2.0
#include "SaveSlotStore.h"

namespace
{
	/** Engineの保存書式へ渡すファイル拡張子。 */
	constexpr const char* kSaveExtension = ".acssave";
}


void FSaveSlotStore::Configure( const FString& Directory, const FString& BaseName, i32 SlotCount )
{
	m_Directory = Directory;
	m_BaseName = BaseName;
	m_SlotCount = SlotCount > 0 ? SlotCount : 0;
}


FString FSaveSlotStore::GetSlotPath( i32 Slot ) const
{
	if ( Slot < 0 || Slot >= m_SlotCount ) return FString();

	// 指定枠へ拡張子まで加えるUTF-8パス。
	FString Path = m_Directory;
	if ( !Path.IsEmpty() && !CFileSystem::IsPathSeparator( Path.Data()[Path.Size() - 1] ) ) Path.Append( '/' );

	Path.Append( m_BaseName.View() );
	Path.AppendFormat( "%d", Slot );
	Path.Append( kSaveExtension );
	return Path;
}


bool FSaveSlotStore::BuildSlotPath( i32 Slot, TArray<wchar_t>& OutWide ) const
{
	// native文字列へ変換する指定枠のUTF-8パス。
	const FString Path = GetSlotPath( Slot );
	if ( Path.IsEmpty() ) return false;

	return AcsToWide( Path, OutWide );
}


void FSaveSlotStore::EnsureDirectory() const
{
	if ( m_Directory.IsEmpty() ) return;

	// ファイルシステムへ渡す保存先のnativeパス。
	TArray<wchar_t> Wide;
	if ( !AcsToWide( m_Directory, Wide ) ) return;

	if ( CFileSystem::DirectoryExists( Wide.GetData() ) ) return;

	// 保存処理で最終的な失敗を返すため、作成結果は保持しない。
	( void )CFileSystem::CreateDirectory( Wide.GetData() );
}


bool FSaveSlotStore::Exists( i32 Slot ) const
{
	// ファイル存在を調べる指定枠のnativeパス。
	TArray<wchar_t> Wide;
	if ( !BuildSlotPath( Slot, Wide ) ) return false;

	return CFileSystem::Exists( Wide.GetData() );
}


FSaveSlotInfo FSaveSlotStore::GetSlotInfo( i32 Slot ) const
{
	// 指定枠について返す一覧用の値。
	FSaveSlotInfo Info;
	Info.Index = Slot;
	Info.Path = GetSlotPath( Slot );
	if ( Info.Path.IsEmpty() ) return Info;

	// ヘッダーだけを読む指定枠のnativeパス。
	TArray<wchar_t> Wide;
	if ( !AcsToWide( Info.Path, Wide ) ) return Info;

	// ファイルを有効と判断するEngine書式の版。
	const auto Version = CSaveArchive::PeekVersion( Wide.GetData() );
	if ( !Version.IsOk() ) return Info;

	Info.bExists = true;
	Info.Version = Version.Value();

	// 一覧へ載せるEngine書式の内容サイズ。
	const auto Size = CSaveArchive::PeekPayloadSize( Wide.GetData() );
	if ( Size.IsOk() ) Info.SizeBytes = Size.Value();

	return Info;
}


bool FSaveSlotStore::Erase( i32 Slot )
{
	// 削除する指定枠のnativeパス。
	TArray<wchar_t> Wide;
	if ( !BuildSlotPath( Slot, Wide ) ) return false;

	if ( !CFileSystem::Exists( Wide.GetData() ) ) return true;

	return CFileSystem::Delete( Wide.GetData() ).IsOk();
}

// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Effects/Effect3D/Effect3DPlayer.h"

#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"
#include "AcsFramework_Core/Text/StringConvert.h"

#include <Effekseer.h>
#include <EffekseerRendererDX12.h>

#include <cmath>
#include <cstring>
#include <d3d12.h>
#include <dxgiformat.h>

namespace
{
	/** 同時に生かせるparticle instance数。通常の戦闘演出に余裕を持たせる。 */
	constexpr i32 kMaximumInstances = 4096;

	/** 1回の描画で確保する最大sprite数。 */
	constexpr i32 kMaximumSprites = 16384;

	/** Windowsの絶対パスを受ける固定長。CAssetRootと同じ失敗の仕方に揃える。 */
	constexpr usize kMaximumPathCharacters = 2048u;

	/**
	 * ACS形式をD3D12形式へ直す。
	 *
	 * @param Format ACSの論理形式。
	 * @return 対応するD3D12形式。未対応ならUNKNOWN。
	 */
	DXGI_FORMAT ToDxgiFormat( EFormat Format ) noexcept
	{
		switch ( Format )
		{
		case EFormat::R8G8B8A8_UNorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case EFormat::R8G8B8A8_UNorm_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case EFormat::B8G8R8A8_UNorm: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case EFormat::R16G16_Float: return DXGI_FORMAT_R16G16_FLOAT;
		case EFormat::R16G16B16A16_Float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case EFormat::R11G11B10_Float: return DXGI_FORMAT_R11G11B10_FLOAT;
		case EFormat::R32G32_Float: return DXGI_FORMAT_R32G32_FLOAT;
		case EFormat::R32G32B32_Float: return DXGI_FORMAT_R32G32B32_FLOAT;
		case EFormat::R32G32B32A32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case EFormat::D24_UNorm_S8_UInt: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case EFormat::D32_Float: return DXGI_FORMAT_D32_FLOAT;
		default: return DXGI_FORMAT_UNKNOWN;
		}
	}

	/**
	 * 2つの所有文字列が同じか返す。
	 *
	 * @param Left 左辺。
	 * @param Right 右辺。
	 * @return 内容が同じなら true。
	 */
	bool IsSameString( const FString& Left, const FString& Right ) noexcept
	{
		return Left == FStringView( Right.Data(), Right.Size() );
	}

	/**
	 * Effekseerへ渡せるUTF-16へ直す。
	 *
	 * @param Source UTF-8の絶対パス。
	 * @param OutPath 終端文字を含む受け取り先。
	 * @return 長さと変換が有効なら true。
	 */
	bool ToUtf16Path( const FString& Source, char16_t ( &OutPath )[kMaximumPathCharacters] ) noexcept
	{
		wchar_t Wide[kMaximumPathCharacters] = {};
		if ( !AcsToWide( Source, Wide, kMaximumPathCharacters ) ) return false;

		usize Index = 0u;
		for ( ; Index + 1u < kMaximumPathCharacters && Wide[Index] != L'\0'; ++Index )
		{
			OutPath[Index] = static_cast<char16_t>( Wide[Index] );
		}

		if ( Wide[Index] != L'\0' ) return false;
		OutPath[Index] = u'\0';
		return true;
	}

	/**
	 * ACSの行ベクトル行列を同じ規約のEffekseer行列へ移す。
	 *
	 * @param Source ACS行列。
	 * @return 成分を行優先のまま移した行列。
	 */
	Effekseer::Matrix44 ToEffekseerMatrix( const FMat4& Source ) noexcept
	{
		Effekseer::Matrix44 Result;
		static_assert( sizeof( Result.Values ) == sizeof( Source.m ) );
		std::memcpy( Result.Values, Source.m, sizeof( Result.Values ) );
		return Result;
	}

	/** 3成分がすべて有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}
}


struct CEffect3DPlayer::FImpl
{
	/** 読込済み素材。 */
	struct FCachedEffect
	{
		/** `Assets`から解決済みの絶対パス。 */
		FString FullPath;

		/** Effekseerが所有参照する素材。 */
		Effekseer::EffectRef Effect;
	};

	/** 受け付けた1つの再生。 */
	struct FInstance
	{
		/** framework側の安定した発行値。 */
		u32 PublicValue = 0u;

		/** Effekseer側の番号。負値はまだ始めていない。 */
		Effekseer::Handle NativeHandle = -1;

		/** `Assets`から解決済みの絶対パス。 */
		FString FullPath;

		/** 開始前の保持と、後からの変更確認に使う再生指定。 */
		FEffect3DPlayParams Params;
	};

	/** D3D12を借りるACSデバイス。所有はしない。 */
	IRhiDevice* Device = nullptr;

	/** EffekseerのD3D12デバイスadapter。 */
	Effekseer::Backend::GraphicsDeviceRef GraphicsDevice;

	/** エフェクト描画器。 */
	EffekseerRenderer::RendererRef Renderer;

	/** エフェクトのinstanceと時間を管理する。 */
	Effekseer::ManagerRef Manager;

	/** 毎フレーム使い捨てるGPUメモリ。 */
	Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> MemoryPool;

	/** ACSのcommand listへ記録を橋渡しする。 */
	Effekseer::RefPtr<EffekseerRenderer::CommandList> CommandList;

	/** 読込済み素材。 */
	TArray<FCachedEffect> CachedEffects;

	/** 準備待ちと再生中をまとめた一覧。 */
	TArray<FInstance> Instances;

	/** 次に発行する値。0は飛ばす。 */
	u32 NextHandleValue = 1u;

	/** このbackendでは初期化できないと判定済み。 */
	bool bBackendRejected = false;

	/** command list不足を既に知らせたか。 */
	bool bCommandListWarningIssued = false;

	/** D3D12描画器を遅延初期化する。 */
	bool InitializeBackend_Internal( IRhiDevice& InDevice, IRhiTexture& ColorTarget, IRhiTexture* DepthTarget ) noexcept;

	/** 読込済み素材を探し、無ければ読む。 */
	Effekseer::EffectRef LoadEffect_Internal( const FString& FullPath ) noexcept;

	/** 準備待ち1件をEffekseerで開始する。 */
	bool StartInstance_Internal( FInstance& Instance ) noexcept;

	/** 準備待ちを開始し、失敗した項目を除く。 */
	void StartPending_Internal() noexcept;

	/** 終了済みの再生を一覧から除く。 */
	void RemoveFinished_Internal() noexcept;

	/** handleに対応する一覧位置を返す。 */
	usize FindInstanceIndex_Internal( FEffect3DHandle Handle ) const noexcept;

	/** 衝突しない発行値を返す。 */
	u32 AllocateHandleValue_Internal() noexcept;

	/** GPU完了を待ち、backendだけを解放する。 */
	void ReleaseBackend_Internal() noexcept;
};


bool CEffect3DPlayer::FImpl::InitializeBackend_Internal( IRhiDevice& InDevice, IRhiTexture& ColorTarget, IRhiTexture* DepthTarget ) noexcept
{
	if ( Renderer != nullptr ) return true;
	if ( bBackendRejected ) return false;

	FRhiD3D12DeviceInterop Interop;
	if ( !InDevice.TryGetD3D12Interop( Interop ) || !Interop.IsValid() )
	{
		ACS_LOG_WARN( "Effect3D: 現在の描画backendからD3D12を借りられません" );
		bBackendRejected = true;
		return false;
	}

	const DXGI_FORMAT ColorFormat = ToDxgiFormat( ColorTarget.PixelFormat() );
	const DXGI_FORMAT DepthFormat = DepthTarget != nullptr ? ToDxgiFormat( DepthTarget->PixelFormat() ) : DXGI_FORMAT_UNKNOWN;
	if ( ColorFormat == DXGI_FORMAT_UNKNOWN || ( DepthTarget != nullptr && DepthFormat == DXGI_FORMAT_UNKNOWN ) )
	{
		ACS_LOG_WARN( "Effect3D: HDRまたはdepthの形式にEffekseerが対応していません" );
		bBackendRejected = true;
		return false;
	}

	auto* const NativeDevice = static_cast<ID3D12Device*>( Interop.Device );
	auto* const NativeQueue = static_cast<ID3D12CommandQueue*>( Interop.GraphicsQueue );
	const i32 FrameCount = Interop.FramesInFlight > 0u ? static_cast<i32>( Interop.FramesInFlight ) : 2;

	GraphicsDevice = EffekseerRendererDX12::CreateGraphicsDevice( NativeDevice, NativeQueue, FrameCount );
	if ( GraphicsDevice == nullptr )
	{
		ACS_LOG_WARN( "Effect3D: EffekseerのD3D12 deviceを作れません" );
		bBackendRejected = true;
		return false;
	}

	DXGI_FORMAT MutableColorFormat = ColorFormat;
	Renderer = EffekseerRendererDX12::Create( GraphicsDevice, &MutableColorFormat, 1, DepthFormat, false, kMaximumSprites );
	Manager = Effekseer::Manager::Create( kMaximumInstances );
	if ( Renderer == nullptr || Manager == nullptr )
	{
		ACS_LOG_WARN( "Effect3D: rendererまたはmanagerを作れません" );
		ReleaseBackend_Internal();
		bBackendRejected = true;
		return false;
	}

	MemoryPool = EffekseerRenderer::CreateSingleFrameMemoryPool( Renderer->GetGraphicsDevice() );
	CommandList = EffekseerRenderer::CreateCommandList( Renderer->GetGraphicsDevice(), MemoryPool );
	if ( MemoryPool == nullptr || CommandList == nullptr )
	{
		ACS_LOG_WARN( "Effect3D: frame用の描画資源を作れません" );
		ReleaseBackend_Internal();
		bBackendRejected = true;
		return false;
	}

	Manager->SetCoordinateSystem( Effekseer::CoordinateSystem::LH );
	Manager->SetSpriteRenderer( Renderer->CreateSpriteRenderer() );
	Manager->SetRibbonRenderer( Renderer->CreateRibbonRenderer() );
	Manager->SetRingRenderer( Renderer->CreateRingRenderer() );
	Manager->SetTrackRenderer( Renderer->CreateTrackRenderer() );
	Manager->SetModelRenderer( Renderer->CreateModelRenderer() );
	Manager->SetTextureLoader( Renderer->CreateTextureLoader() );
	Manager->SetModelLoader( Renderer->CreateModelLoader() );
	Manager->SetMaterialLoader( Renderer->CreateMaterialLoader() );
	Manager->SetCurveLoader( Effekseer::MakeRefPtr<Effekseer::CurveLoader>() );
	Renderer->SetRestorationOfStatesFlag( false );

	Device = &InDevice;
	ACS_LOG_INFO( "Effect3D: D3D12 backend ready (%u frames)", Interop.FramesInFlight );
	return true;
}


Effekseer::EffectRef CEffect3DPlayer::FImpl::LoadEffect_Internal( const FString& FullPath ) noexcept
{
	for ( const FCachedEffect& Cached : CachedEffects )
	{
		if ( IsSameString( Cached.FullPath, FullPath ) ) return Cached.Effect;
	}

	char16_t Path[kMaximumPathCharacters] = {};
	if ( !ToUtf16Path( FullPath, Path ) )
	{
		ACS_LOG_WARN( "Effect3D: 素材pathをUTF-16へ変換できません: %s", FullPath.Data() );
		return nullptr;
	}

	Effekseer::EffectRef Effect = Effekseer::Effect::Create( Manager, Path );
	if ( Effect == nullptr )
	{
		ACS_LOG_WARN( "Effect3D: 素材を読めません: %s", FullPath.Data() );
		return nullptr;
	}

	FCachedEffect Cached;
	Cached.FullPath = FullPath;
	Cached.Effect = Effect;
	if ( !CachedEffects.TryAdd( Move( Cached ) ) )
	{
		ACS_LOG_WARN( "Effect3D: 素材cacheを確保できません" );
		return nullptr;
	}

	return Effect;
}


bool CEffect3DPlayer::FImpl::StartInstance_Internal( FInstance& Instance ) noexcept
{
	Effekseer::EffectRef Effect = LoadEffect_Internal( Instance.FullPath );
	if ( Effect == nullptr ) return false;

	const FEffect3DPlayParams& Params = Instance.Params;
	Instance.NativeHandle = Manager->Play( Effect, Effekseer::Vector3D( Params.Position.x, Params.Position.y, Params.Position.z ), Params.StartFrame );
	if ( Instance.NativeHandle < 0 )
	{
		ACS_LOG_WARN( "Effect3D: 再生instanceを作れません: %s", Instance.FullPath.Data() );
		return false;
	}

	Manager->SetRotation( Instance.NativeHandle, ToRadians( Params.RotationDeg.x ), ToRadians( Params.RotationDeg.y ), ToRadians( Params.RotationDeg.z ) );
	Manager->SetScale( Instance.NativeHandle, Params.Scale.x, Params.Scale.y, Params.Scale.z );
	Manager->SetSpeed( Instance.NativeHandle, Params.Speed );
	return true;
}


void CEffect3DPlayer::FImpl::StartPending_Internal() noexcept
{
	usize Index = 0u;
	while ( Index < Instances.Num() )
	{
		FInstance& Instance = Instances[Index];
		if ( Instance.NativeHandle >= 0 || StartInstance_Internal( Instance ) )
		{
			++Index;
			continue;
		}

		Instances.RemoveAt( Index );
	}
}


void CEffect3DPlayer::FImpl::RemoveFinished_Internal() noexcept
{
	if ( Manager == nullptr ) return;

	usize Index = 0u;
	while ( Index < Instances.Num() )
	{
		const Effekseer::Handle NativeHandle = Instances[Index].NativeHandle;
		if ( NativeHandle < 0 || Manager->Exists( NativeHandle ) )
		{
			++Index;
			continue;
		}

		Instances.RemoveAt( Index );
	}
}


usize CEffect3DPlayer::FImpl::FindInstanceIndex_Internal( FEffect3DHandle Handle ) const noexcept
{
	if ( !Handle.IsValid() ) return Instances.Num();

	for ( usize Index = 0u; Index < Instances.Num(); ++Index )
	{
		if ( Instances[Index].PublicValue == Handle.Value() ) return Index;
	}

	return Instances.Num();
}


u32 CEffect3DPlayer::FImpl::AllocateHandleValue_Internal() noexcept
{
	for ( usize Attempt = 0u; Attempt <= Instances.Num(); ++Attempt )
	{
		const u32 Candidate = NextHandleValue++;
		if ( NextHandleValue == 0u ) NextHandleValue = 1u;
		if ( Candidate == 0u ) continue;

		bool bUsed = false;
		for ( const FInstance& Instance : Instances )
		{
			if ( Instance.PublicValue == Candidate )
			{
				bUsed = true;
				break;
			}
		}
		if ( !bUsed ) return Candidate;
	}

	return 0u;
}


void CEffect3DPlayer::FImpl::ReleaseBackend_Internal() noexcept
{
	if ( Manager != nullptr ) Manager->StopAllEffects();
	if ( Device != nullptr ) Device->WaitIdle();

	Instances.Reset();
	CachedEffects.Reset();
	Manager = nullptr;
	CommandList = nullptr;
	MemoryPool = nullptr;
	Renderer = nullptr;
	GraphicsDevice = nullptr;
	Device = nullptr;
	bCommandListWarningIssued = false;
}


CEffect3DPlayer::CEffect3DPlayer() noexcept : m_Impl( MakeUnique<FImpl>() )
{
}


CEffect3DPlayer::~CEffect3DPlayer() noexcept
{
	Shutdown();
}


FEffect3DHandle CEffect3DPlayer::Play( FStringView AssetPath, FVec3 WorldPosition ) noexcept
{
	return Play( AssetPath, FEffect3DPlayParams::At( WorldPosition ) );
}


FEffect3DHandle CEffect3DPlayer::Play( FStringView AssetPath, const FEffect3DPlayParams& Params ) noexcept
{
	if ( !m_Impl )
	{
		ACS_LOG_WARN( "Effect3D: player実装を確保できていません" );
		return {};
	}
	if ( !Params.IsValid() )
	{
		ACS_LOG_WARN( "Effect3D: 不正な再生指定を拒否しました" );
		return {};
	}

	FString FullPath;
	if ( !CAssetRoot::Resolve( AssetPath, FullPath ) )
	{
		ACS_LOG_WARN( "Effect3D: Assetsからの相対pathを解決できません" );
		return {};
	}

	const u32 PublicValue = m_Impl->AllocateHandleValue_Internal();
	if ( PublicValue == 0u )
	{
		ACS_LOG_WARN( "Effect3D: 再生handleを発行できません" );
		return {};
	}

	FImpl::FInstance Instance;
	Instance.PublicValue = PublicValue;
	Instance.FullPath = Move( FullPath );
	Instance.Params = Params;
	if ( !m_Impl->Instances.TryAdd( Move( Instance ) ) )
	{
		ACS_LOG_WARN( "Effect3D: 再生一覧を確保できません" );
		return {};
	}

	const FEffect3DHandle Handle = FEffect3DHandle::FromValue( PublicValue );
	if ( m_Impl->Manager != nullptr )
	{
		m_Impl->StartPending_Internal();
		if ( m_Impl->FindInstanceIndex_Internal( Handle ) >= m_Impl->Instances.Num() ) return {};
	}
	return Handle;
}


bool CEffect3DPlayer::Stop( FEffect3DHandle Handle ) noexcept
{
	if ( !m_Impl ) return false;

	const usize Index = m_Impl->FindInstanceIndex_Internal( Handle );
	if ( Index >= m_Impl->Instances.Num() ) return false;

	const Effekseer::Handle NativeHandle = m_Impl->Instances[Index].NativeHandle;
	if ( NativeHandle >= 0 && m_Impl->Manager != nullptr ) m_Impl->Manager->StopEffect( NativeHandle );
	m_Impl->Instances.RemoveAt( Index );
	return true;
}


void CEffect3DPlayer::StopAll() noexcept
{
	if ( !m_Impl ) return;
	if ( m_Impl->Manager != nullptr ) m_Impl->Manager->StopAllEffects();
	m_Impl->Instances.Reset();
}


bool CEffect3DPlayer::IsPlaying( FEffect3DHandle Handle ) const noexcept
{
	if ( !m_Impl ) return false;
	const usize Index = m_Impl->FindInstanceIndex_Internal( Handle );
	if ( Index >= m_Impl->Instances.Num() ) return false;

	const Effekseer::Handle NativeHandle = m_Impl->Instances[Index].NativeHandle;
	return NativeHandle < 0 || ( m_Impl->Manager != nullptr && m_Impl->Manager->Exists( NativeHandle ) );
}


bool CEffect3DPlayer::SetPosition( FEffect3DHandle Handle, FVec3 WorldPosition ) noexcept
{
	if ( !m_Impl || !IsFinite( WorldPosition ) ) return false;
	const usize Index = m_Impl->FindInstanceIndex_Internal( Handle );
	if ( Index >= m_Impl->Instances.Num() ) return false;

	FImpl::FInstance& Instance = m_Impl->Instances[Index];
	Instance.Params.Position = WorldPosition;
	if ( Instance.NativeHandle >= 0 && m_Impl->Manager != nullptr )
	{
		m_Impl->Manager->SetLocation( Instance.NativeHandle, WorldPosition.x, WorldPosition.y, WorldPosition.z );
	}
	return true;
}


bool CEffect3DPlayer::SetRotationDeg( FEffect3DHandle Handle, FVec3 RotationDeg ) noexcept
{
	if ( !m_Impl || !IsFinite( RotationDeg ) ) return false;
	const usize Index = m_Impl->FindInstanceIndex_Internal( Handle );
	if ( Index >= m_Impl->Instances.Num() ) return false;

	FImpl::FInstance& Instance = m_Impl->Instances[Index];
	Instance.Params.RotationDeg = RotationDeg;
	if ( Instance.NativeHandle >= 0 && m_Impl->Manager != nullptr )
	{
		m_Impl->Manager->SetRotation( Instance.NativeHandle, ToRadians( RotationDeg.x ), ToRadians( RotationDeg.y ), ToRadians( RotationDeg.z ) );
	}
	return true;
}


bool CEffect3DPlayer::SetScale( FEffect3DHandle Handle, FVec3 Scale ) noexcept
{
	if ( !m_Impl ) return false;
	const usize Index = m_Impl->FindInstanceIndex_Internal( Handle );
	if ( Index >= m_Impl->Instances.Num() ) return false;

	FImpl::FInstance& Instance = m_Impl->Instances[Index];
	FEffect3DPlayParams Candidate = Instance.Params;
	Candidate.Scale = Scale;
	if ( !Candidate.IsValid() ) return false;

	Instance.Params.Scale = Scale;
	if ( Instance.NativeHandle >= 0 && m_Impl->Manager != nullptr )
	{
		m_Impl->Manager->SetScale( Instance.NativeHandle, Scale.x, Scale.y, Scale.z );
	}
	return true;
}


bool CEffect3DPlayer::SetSpeed( FEffect3DHandle Handle, f32 Speed ) noexcept
{
	if ( !m_Impl ) return false;
	const usize Index = m_Impl->FindInstanceIndex_Internal( Handle );
	if ( Index >= m_Impl->Instances.Num() ) return false;

	FImpl::FInstance& Instance = m_Impl->Instances[Index];
	FEffect3DPlayParams Candidate = Instance.Params;
	Candidate.Speed = Speed;
	if ( !Candidate.IsValid() ) return false;

	Instance.Params.Speed = Speed;
	if ( Instance.NativeHandle >= 0 && m_Impl->Manager != nullptr )
	{
		m_Impl->Manager->SetSpeed( Instance.NativeHandle, Speed );
	}
	return true;
}


void CEffect3DPlayer::Update( f32 DeltaSeconds ) noexcept
{
	if ( !m_Impl || m_Impl->Manager == nullptr ) return;
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f )
	{
		ACS_LOG_WARN( "Effect3D: 不正な経過秒を無視しました" );
		return;
	}

	m_Impl->Manager->Update( DeltaSeconds * 60.0f );
	m_Impl->RemoveFinished_Internal();
}


bool CEffect3DPlayer::Render( IRhiDevice& Device, IRhiCommandList& Commands, const CCamera& Camera, IRhiTexture& ColorTarget, IRhiTexture* DepthTarget ) noexcept
{
	if ( !m_Impl || m_Impl->Instances.IsEmpty() ) return false;
	if ( !m_Impl->InitializeBackend_Internal( Device, ColorTarget, DepthTarget ) )
	{
		if ( m_Impl->bBackendRejected ) m_Impl->Instances.Reset();
		return false;
	}

	m_Impl->StartPending_Internal();
	if ( m_Impl->Instances.IsEmpty() ) return false;

	auto* const NativeCommandList = static_cast<ID3D12GraphicsCommandList*>( Commands.D3D12GraphicsCommandList() );
	if ( NativeCommandList == nullptr )
	{
		if ( !m_Impl->bCommandListWarningIssued )
		{
			ACS_LOG_WARN( "Effect3D: 現在のD3D12 command listを借りられません" );
			m_Impl->bCommandListWarningIssued = true;
		}
		return false;
	}

	m_Impl->MemoryPool->NewFrame();
	EffekseerRendererDX12::BeginCommandList( m_Impl->CommandList, NativeCommandList );
	m_Impl->Renderer->SetCommandList( m_Impl->CommandList );
	m_Impl->Renderer->SetCameraMatrix( ToEffekseerMatrix( Camera.View() ) );
	m_Impl->Renderer->SetProjectionMatrix( ToEffekseerMatrix( Camera.Projection() ) );
	m_Impl->Renderer->BeginRendering();
	m_Impl->Manager->Draw();
	m_Impl->Renderer->EndRendering();
	m_Impl->Renderer->SetCommandList( nullptr );
	EffekseerRendererDX12::EndCommandList( m_Impl->CommandList );
	return true;
}


bool CEffect3DPlayer::IsReady() const noexcept
{
	return m_Impl && m_Impl->Renderer != nullptr;
}


void CEffect3DPlayer::Shutdown() noexcept
{
	if ( !m_Impl ) return;
	m_Impl->ReleaseBackend_Internal();
	m_Impl->NextHandleValue = 1u;
	m_Impl->bBackendRejected = false;
}

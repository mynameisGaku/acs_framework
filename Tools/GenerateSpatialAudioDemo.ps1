# SPDX-License-Identifier: Apache-2.0
# 3D左右定位デモ用の短いモノラル効果音を決定論的に生成する。
param(
    [string]$OutputPath = ''
)
$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $repositoryRoot 'Assets\Audio\SpatialPulse.wav'
}

$sampleRate = 44100
$durationSeconds = 0.45
$sampleCount = [int]($sampleRate * $durationSeconds)
$channelCount = 1
$bitsPerSample = 16
$dataByteCount = $sampleCount * ($bitsPerSample / 8)

$outputDirectory = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

$stream = [System.IO.FileStream]::new($OutputPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write)
$writer = [System.IO.BinaryWriter]::new($stream)
try {
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes('RIFF'))
    $writer.Write([int](36 + $dataByteCount))
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes('WAVE'))
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes('fmt '))
    $writer.Write([int]16)
    $writer.Write([int16]1)
    $writer.Write([int16]$channelCount)
    $writer.Write([int]$sampleRate)
    $writer.Write([int]($sampleRate * $channelCount * $bitsPerSample / 8))
    $writer.Write([int16]($channelCount * $bitsPerSample / 8))
    $writer.Write([int16]$bitsPerSample)
    $writer.Write([System.Text.Encoding]::ASCII.GetBytes('data'))
    $writer.Write([int]$dataByteCount)

    for ($sampleIndex = 0; $sampleIndex -lt $sampleCount; ++$sampleIndex) {
        $time = $sampleIndex / [double]$sampleRate
        $attack = [Math]::Min(1.0, $time / 0.012)
        $decay = [Math]::Exp(-7.2 * $time)
        $tone = [Math]::Sin(2.0 * [Math]::PI * 523.251 * $time)
        $tone += 0.38 * [Math]::Sin(2.0 * [Math]::PI * 783.991 * $time)
        $sample = 0.42 * $attack * $decay * $tone
        $writer.Write([int16][Math]::Round([Math]::Max(-1.0, [Math]::Min(1.0, $sample)) * 32767.0))
    }
}
finally {
    $writer.Dispose()
    $stream.Dispose()
}

Write-Host "generated: $OutputPath"

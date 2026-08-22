# SPDX-License-Identifier: Apache-2.0
#
# Launch the built application, screenshot its render window, and close it.
#
# 3D work cannot be checked by tests alone. A wrong light direction, a missing shadow
# pass or a flipped matrix all compile and pass, and only show up as "the picture looks
# off". This makes looking at the picture a single command.
#
# The render window is found by title, not by MainWindowHandle: the process also owns a
# console window, and that is the one MainWindowHandle returns.
#
# Usage:
#   .\Tools\CaptureApp.ps1
#   .\Tools\CaptureApp.ps1 -Out Docs\demo3d.png -Wait 10
#   .\Tools\CaptureApp.ps1 -Exe path\to\acs_framework.exe -Title "ACS"
[CmdletBinding()]
param(
    [string]$Exe   = '',
    [string]$Out   = '',
    [string]$Title = 'ACS',
    [int]$Wait     = 9,
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)
$ErrorActionPreference = 'Stop'

$repo = Split-Path $PSScriptRoot -Parent

if ([string]::IsNullOrWhiteSpace($Exe)) {
    $Exe = Join-Path $repo "x64\$Configuration\acs_framework.exe"
}
if (-not (Test-Path $Exe)) { throw "executable not found: $Exe (build it first)" }

if ([string]::IsNullOrWhiteSpace($Out)) {
    $Out = Join-Path $env:TEMP ("acs-capture-" + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.png')
}
$outDir = Split-Path $Out -Parent
if ($outDir -and -not (Test-Path $outDir)) { New-Item -ItemType Directory -Force -Path $outDir | Out-Null }

Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class AcsCapture {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int command);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int width, int height, uint flags);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    public struct RECT { public int Left, Top, Right, Bottom; }
    public const int ShowNormally = 5;
    public const uint KeepPositionAndSize = 0x43;
    public static readonly IntPtr TopMost = new IntPtr(-1);

    // Pick the top-level window of this process whose title contains the marker.
    public static IntPtr FindRenderWindow(uint pid, string marker) {
        IntPtr hit = IntPtr.Zero;
        EnumWindows((h, l) => {
            uint owner; GetWindowThreadProcessId(h, out owner);
            if (owner != pid) return true;
            var sb = new StringBuilder(256); GetWindowText(h, sb, 256);
            if (sb.ToString().Contains(marker)) { hit = h; return false; }
            return true;
        }, IntPtr.Zero);
        return hit;
    }
}
'@
Add-Type -AssemblyName System.Windows.Forms, System.Drawing

$workDir = Split-Path $Exe -Parent
# ログ用consoleは隠し、アプリが明示表示する描画windowだけを撮影対象にする。
$proc = Start-Process $Exe -WorkingDirectory $workDir -WindowStyle Hidden -PassThru

try {
    Start-Sleep -Seconds $Wait
    $proc.Refresh()
    if ($proc.HasExited) { throw "the application exited before it could be captured (exit=$($proc.ExitCode))" }

    $window = [AcsCapture]::FindRenderWindow([uint32]$proc.Id, $Title)
    if ($window -eq [IntPtr]::Zero) { throw "no window whose title contains '$Title'" }

    # 起動時に隠したconsoleの影響を受けず、描画windowだけを表示して手前へ出す。
    [void][AcsCapture]::ShowWindow($window, [AcsCapture]::ShowNormally)
    [void][AcsCapture]::SetWindowPos($window, [AcsCapture]::TopMost, 0, 0, 0, 0, [AcsCapture]::KeepPositionAndSize)
    Start-Sleep -Seconds 2

    $rect = New-Object AcsCapture+RECT
    [void][AcsCapture]::GetWindowRect($window, [ref]$rect)
    $width  = $rect.Right - $rect.Left
    $height = $rect.Bottom - $rect.Top
    if ($width -le 0 -or $height -le 0) { throw "the window has no area ($width x $height)" }

    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $canvas = [System.Drawing.Graphics]::FromImage($bitmap)
    try {
        $canvas.CopyFromScreen(
            (New-Object System.Drawing.Point($rect.Left, $rect.Top)),
            [System.Drawing.Point]::Empty,
            (New-Object System.Drawing.Size($width, $height)))
        $bitmap.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally { $canvas.Dispose(); $bitmap.Dispose() }

    Write-Host ("captured {0}  ({1} x {2})" -f $Out, $width, $height)
}
finally {
    # Always close it. A left-behind window keeps the GPU busy and blocks the next build.
    if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue }
}

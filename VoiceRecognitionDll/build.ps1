$ErrorActionPreference = "Stop"

$projectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$outDir = Join-Path $projectDir "bin\Release"
$outDll = Join-Path $outDir "VoiceRecognitionDll.dll"
$compiler = Join-Path $env:WINDIR "Microsoft.NET\Framework\v4.0.30319\csc.exe"
$speechDll = Join-Path $env:WINDIR "Microsoft.NET\assembly\GAC_MSIL\System.Speech\v4.0_4.0.0.0__31bf3856ad364e35\System.Speech.dll"

New-Item -ItemType Directory -Force -Path $outDir | Out-Null

& $compiler `
  /target:library `
  /out:$outDll `
  /reference:$speechDll `
  (Join-Path $projectDir "VoiceRecognizer.cs") `
  (Join-Path $projectDir "Properties\AssemblyInfo.cs")

Write-Host "Built: $outDll"

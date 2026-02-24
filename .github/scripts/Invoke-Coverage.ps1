#Requires -Version 5.1
<#
.SYNOPSIS
    Runs OpenCppCoverage against the UE5 test suite for the configured project.
.DESCRIPTION
    Invokes OpenCppCoverage as an external debugger against UnrealEditor-Cmd.exe,
    scoped to the project module only. Produces CoverageReports\coverage.xml
    (Cobertura format) and CoverageReports\html (HTML report).
    Exits non-zero if coverage data is not flushed (lines-valid = 0).

    Reads PROJECT_NAME from environment variable. Falls back to the default project
    name if not set (see the else branch in the param block below).

    Validated on UE5.7.2 with OpenCppCoverage 0.9.9.0 (COV-01 spike, 2026-02-22).
    Graceful quit via -ExecCmds="Automation RunTests {ProjectName};quit" correctly
    flushes coverage data. The -ExecCmds value MUST have inner double quotes because
    OpenCppCoverage passes child arguments to CreateProcess as a flat command line --
    without quotes, spaces split the value into separate argv entries.
    Project path must be passed unquoted to avoid argument parsing issues with
    OpenCppCoverage on Windows paths containing spaces.
.NOTES
    Set PROJECT_NAME in the workflow env block (see coverage.yml FORK CONFIGURATION).
    The script reads $env:PROJECT_NAME and falls back to a local default for standalone use.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$ProjectName   = if ($env:PROJECT_NAME) { $env:PROJECT_NAME } else { 'ClaudeCodeTesting' }
$ue5Root       = if ($env:UE5_ROOT) { $env:UE5_ROOT } else { 'C:\Program Files\Epic Games\UE_5.7' }
$workspaceRoot = if ($env:GITHUB_WORKSPACE) { $env:GITHUB_WORKSPACE } else { (Get-Location).Path }
$editorCmd     = Join-Path $ue5Root 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$projectFile   = Join-Path $workspaceRoot "$ProjectName.uproject"
$sourceDir     = Join-Path $workspaceRoot "Source\$ProjectName"
$reportDir     = Join-Path $workspaceRoot 'CoverageReports'

if (-not (Test-Path $editorCmd)) {
    Write-Host "::error::UnrealEditor-Cmd.exe not found at: $editorCmd"
    exit 1
}

# Verify game module DLL exists (built by build.yml or local compilation)
$binDir = Join-Path $workspaceRoot 'Binaries\Win64'
$gameDll = Join-Path $binDir "UnrealEditor-$ProjectName.dll"
Write-Host "Checking for game module DLL..."
Write-Host "  Binaries dir: $binDir"
if (Test-Path $gameDll) {
    $dllInfo = Get-Item $gameDll
    Write-Host "  Found: $($dllInfo.Name) ($([math]::Round($dllInfo.Length / 1MB, 1)) MB, modified $($dllInfo.LastWriteTime))"
} else {
    Write-Host "::warning::Game module DLL not found at: $gameDll"
    Write-Host "  Available DLLs in Binaries\Win64:"
    if (Test-Path $binDir) {
        Get-ChildItem $binDir -Filter '*.dll' | ForEach-Object { Write-Host "    $($_.Name)" }
    } else {
        Write-Host "    (Binaries\Win64 directory does not exist)"
    }
}

New-Item -ItemType Directory -Force -Path $reportDir | Out-Null

$coberturaXml = Join-Path $reportDir 'coverage.xml'
$htmlDir      = Join-Path $reportDir 'html'

Write-Host "Running OpenCppCoverage against $ProjectName test suite..."
Write-Host "  Source: $sourceDir"
Write-Host "  Output: $coberturaXml"

# OpenCppCoverage passes arguments after -- to CreateProcess as a flat command line.
# Multi-layer quoting (PowerShell -> OpenCppCoverage -> CreateProcess -> editor) is
# fragile: spaces in -ExecCmds get split into separate argv entries. To guarantee
# correct quoting, we write a .cmd wrapper with the exact command line we want and
# let OpenCppCoverage debug cmd.exe + its child editor via --cover_children.
$wrapperCmd = Join-Path $reportDir 'run-tests.cmd'
$cmdLine = "`"$editorCmd`" `"$projectFile`" -ExecCmds=`"Automation RunTests $ProjectName;quit`" -unattended -nopause -NullRHI -nosplash -nosound"
"@echo off`r`n$cmdLine" | Out-File -FilePath $wrapperCmd -Encoding ascii -NoNewline

Write-Host "  Wrapper: $wrapperCmd"
Write-Host "  Command: $cmdLine"

& OpenCppCoverage.exe `
    --modules $ProjectName `
    --sources $sourceDir `
    --excluded_sources '.gen.' `
    --excluded_sources '\Tests\' `
    "--export_type=cobertura:$coberturaXml" `
    "--export_type=html:$htmlDir" `
    --cover_children `
    --continue_after_cpp_exception `
    -- `
    $wrapperCmd

if (-not (Test-Path $coberturaXml)) {
    Write-Host '::error::coverage.xml was not created - OpenCppCoverage may have failed'
    exit 1
}

[xml]$cov     = Get-Content $coberturaXml -Encoding UTF8
$linesValid   = [int]$cov.coverage.GetAttribute('lines-valid')
$linesCovered = [int]$cov.coverage.GetAttribute('lines-covered')
$lineRate     = [double]$cov.coverage.GetAttribute('line-rate')
$coveragePct  = [math]::Round($lineRate * 100, 2)

if ($linesValid -eq 0) {
    Write-Host '::error::Coverage XML has lines-valid=0 - OpenCppCoverage did not capture data'
    Write-Host "::error::Check that $ProjectName DLL is built and module name matches --modules filter"
    exit 1
}

Write-Host "Coverage: $coveragePct% ($linesCovered / $linesValid lines covered)"

if ($env:GITHUB_ENV) {
    "COVERAGE_PCT=$coveragePct"    | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    "LINES_COVERED=$linesCovered"  | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    "LINES_VALID=$linesValid"      | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
}

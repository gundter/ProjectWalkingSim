# Invoke-AssetValidation.ps1
# Orchestrates all 7 UE5 asset validation checks and produces structured output.
# Used by validate.yml (standalone) and package.yml (pre-package gate).
#
# Checks:
#   1. Missing Asset References (DataValidation commandlet - log parsing)
#   2. Blueprint Compilation (CompileAllBlueprints commandlet - exit code + log parsing)
#   3. Circular Dependencies (parse DataValidation log for dependency symptoms)
#   4. Asset Naming Conventions (PowerShell filesystem check)
#   5. Unused/Orphaned Assets (PowerShell heuristic - informational only)
#   6. Large Asset Sizes (PowerShell filesystem check)
#   7. Map Validation (parse DataValidation log for map errors + verify map files)
#
# Severity gating:
#   Development: all issues are warnings (exit 0)
#   Shipping:    validation errors are hard failures (exit 1)
#
# Usage:
#   powershell -File Invoke-AssetValidation.ps1 `
#     -ProjectPath "C:\Project\MyGame.uproject" `
#     -Configuration "Development" `
#     -OutputDir "C:\Project\ValidationOutput"

param(
    [Parameter(Mandatory=$true)]
    [string]$ProjectPath,

    [Parameter(Mandatory=$true)]
    [ValidateSet("Development", "Shipping")]
    [string]$Configuration,

    [Parameter(Mandatory=$true)]
    [string]$OutputDir,

    [Parameter(Mandatory=$false)]
    [string]$UE5Root
)

$ErrorActionPreference = "Stop"

# ============================================================================
# Parameter Validation and Path Setup
# ============================================================================

if (-not $UE5Root) {
    $UE5Root = $env:UE5_ROOT
}
if (-not $UE5Root) {
    $UE5Root = "H:\UE5-Source\UnrealEngine"
}

$editorCmd = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $editorCmd)) {
    Write-Host "::error::UnrealEditor-Cmd.exe not found at: $editorCmd"
    exit 1
}

if (-not (Test-Path $ProjectPath)) {
    Write-Host "::error::Project file not found at: $ProjectPath"
    exit 1
}

# Resolve project directory and content path
$projectDir = Split-Path $ProjectPath -Parent
$contentDir = Join-Path $projectDir "Content"
if (-not (Test-Path $contentDir)) {
    Write-Host "::error::Content directory not found at: $contentDir"
    exit 1
}

# Ensure output directory exists
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
}

Write-Host "========================================"
Write-Host "UE5 Asset Validation"
Write-Host "========================================"
Write-Host "Project: $ProjectPath"
Write-Host "Configuration: $Configuration"
Write-Host "UE5 Root: $UE5Root"
Write-Host "Content Dir: $contentDir"
Write-Host "Output Dir: $OutputDir"
Write-Host "========================================"
Write-Host ""

# ============================================================================
# Results Structure
# ============================================================================

# Each check produces a result object with: name, status, errors, warnings, details
$checkResults = @()

# Timeout for UE5 commandlets (10 minutes in milliseconds)
$commandletTimeoutMs = 600000

# ============================================================================
# Check 1: Missing Asset References (DataValidation Commandlet)
# ============================================================================
# CRITICAL: Do NOT rely on exit code. The commandlet always returns 0 even with
# errors (Pitfall 1 from RESEARCH.md). Parse log output instead.

Write-Host "=== Check 1: Missing Asset References (DataValidation) ==="
Write-Host ""

$dataValLogFile = Join-Path $OutputDir "DataValidation.log"
$check1 = @{
    name     = "DataValidation"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

try {
    $dvStdoutFile = Join-Path $OutputDir "DataValidation-stdout.txt"
    $dvStderrFile = Join-Path $OutputDir "DataValidation-stderr.txt"

    $dvArgs = @(
        "`"$ProjectPath`"",
        "-run=DataValidation",
        "-unattended",
        "-NullRHI",
        "-nosplash",
        "-nosound",
        "-stdout",
        "-FullStdOutLogOutput"
    )

    Write-Host "Running DataValidation commandlet..."
    Write-Host "Command: $editorCmd $($dvArgs -join ' ')"

    # Use -RedirectStandardOutput to capture log to a file for parsing.
    # The -stdout and -FullStdOutLogOutput flags tell UE5 to write log to stdout.
    $dvProcess = Start-Process -FilePath $editorCmd -ArgumentList $dvArgs `
        -Wait -PassThru -NoNewWindow `
        -RedirectStandardOutput $dvStdoutFile `
        -RedirectStandardError $dvStderrFile
    $dvTimedOut = $false

    # Check for timeout (Start-Process -Wait doesn't have a built-in timeout,
    # so we use a separate approach if needed)
    if (-not $dvProcess.HasExited) {
        Write-Host "::warning::DataValidation commandlet still running, waiting up to 10 minutes..."
        $dvProcess.WaitForExit($commandletTimeoutMs)
        if (-not $dvProcess.HasExited) {
            $dvTimedOut = $true
            $dvProcess.Kill()
            Write-Host "::warning::DataValidation commandlet timed out after 10 minutes"
        }
    }

    $dvExitCode = $dvProcess.ExitCode
    Write-Host "DataValidation exit code: $dvExitCode (note: unreliable per Pitfall 1)"

    # Copy stdout to the expected log file location for consistency
    if (Test-Path $dvStdoutFile) {
        Copy-Item $dvStdoutFile $dataValLogFile -Force
    }

    # Parse log for validation results
    if (Test-Path $dataValLogFile) {
        $dvLogContent = Get-Content $dataValLogFile -ErrorAction SilentlyContinue

        # Check for successful completion
        $completionLine = $dvLogContent | Where-Object {
            $_ -match "LogDataValidation.*Successfully finished running DataValidation Commandlet"
        }
        if ($completionLine) {
            Write-Host "DataValidation commandlet completed successfully"
        }

        # Parse for errors
        $dvErrors = @($dvLogContent | Where-Object { $_ -match "LogContentValidation: Error:" })
        $dvWarnings = @($dvLogContent | Where-Object { $_ -match "LogContentValidation: Warning:" })

        # Also check for the overall failure message
        $overallFailure = $dvLogContent | Where-Object {
            $_ -match "LogDataValidation: Warning: Errors occurred while validating data"
        }

        $check1.errors = $dvErrors.Count
        $check1.warnings = $dvWarnings.Count

        foreach ($err in $dvErrors) {
            $check1.details += @{
                asset    = "Unknown"
                severity = "error"
                message  = $err.Trim()
            }
        }
        foreach ($warn in $dvWarnings) {
            $check1.details += @{
                asset    = "Unknown"
                severity = "warning"
                message  = $warn.Trim()
            }
        }

        if ($dvErrors.Count -gt 0 -or $overallFailure) {
            $check1.status = "fail"
            Write-Host "DataValidation found $($dvErrors.Count) error(s), $($dvWarnings.Count) warning(s)"
        } elseif ($dvWarnings.Count -gt 0) {
            $check1.status = "warn"
            Write-Host "DataValidation found $($dvWarnings.Count) warning(s)"
        } else {
            Write-Host "DataValidation passed (no errors or warnings)"
        }
    } else {
        Write-Host "::warning::DataValidation log file not found at: $dataValLogFile"
        $check1.status = "warn"
        $check1.warnings = 1
        $check1.details += @{
            asset    = "N/A"
            severity = "warning"
            message  = "DataValidation log file not generated"
        }
    }

    if ($dvTimedOut) {
        $check1.status = "fail"
        $check1.errors += 1
        $check1.details += @{
            asset    = "N/A"
            severity = "error"
            message  = "DataValidation commandlet timed out after 10 minutes"
        }
    }
} catch {
    Write-Host "::warning::DataValidation commandlet failed to execute: $_"
    $check1.status = "fail"
    $check1.errors = 1
    $check1.details += @{
        asset    = "N/A"
        severity = "error"
        message  = "DataValidation commandlet execution error: $_"
    }
}

$checkResults += $check1
Write-Host ""

# ============================================================================
# Check 2: Blueprint Compilation (CompileAllBlueprints Commandlet)
# ============================================================================
# Exit code = TotalNumFatalIssues + TotalNumFailedLoads (Pitfall 4)
# Must use -IgnoreFolder=/Engine to exclude engine Blueprints (Pitfall 2)

Write-Host "=== Check 2: Blueprint Compilation (CompileAllBlueprints) ==="
Write-Host ""

$bpLogFile = Join-Path $OutputDir "CompileBlueprints.log"
$check2 = @{
    name     = "BlueprintCompilation"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

try {
    $bpStdoutFile = Join-Path $OutputDir "CompileBlueprints-stdout.txt"
    $bpStderrFile = Join-Path $OutputDir "CompileBlueprints-stderr.txt"

    $bpArgs = @(
        "`"$ProjectPath`"",
        "-run=CompileAllBlueprints",
        "-IgnoreFolder=/Engine",
        "-SimpleAssetList",
        "-unattended",
        "-NullRHI",
        "-nosplash",
        "-nosound",
        "-stdout",
        "-FullStdOutLogOutput"
    )

    Write-Host "Running CompileAllBlueprints commandlet..."
    Write-Host "Command: $editorCmd $($bpArgs -join ' ')"

    # Use -RedirectStandardOutput to capture log to a file for parsing.
    $bpProcess = Start-Process -FilePath $editorCmd -ArgumentList $bpArgs `
        -Wait -PassThru -NoNewWindow `
        -RedirectStandardOutput $bpStdoutFile `
        -RedirectStandardError $bpStderrFile

    if (-not $bpProcess.HasExited) {
        Write-Host "::warning::CompileAllBlueprints still running, waiting up to 10 minutes..."
        $bpProcess.WaitForExit($commandletTimeoutMs)
        if (-not $bpProcess.HasExited) {
            $bpProcess.Kill()
            Write-Host "::warning::CompileAllBlueprints commandlet timed out after 10 minutes"
            $check2.status = "fail"
            $check2.errors = 1
            $check2.details += @{
                asset    = "N/A"
                severity = "error"
                message  = "CompileAllBlueprints commandlet timed out after 10 minutes"
            }
        }
    }

    $bpExitCode = $bpProcess.ExitCode
    Write-Host "CompileAllBlueprints exit code: $bpExitCode"

    # Copy stdout to the expected log file location for consistency
    if (Test-Path $bpStdoutFile) {
        Copy-Item $bpStdoutFile $bpLogFile -Force
    }

    # Exit code != 0 means errors (Pitfall 4: value = TotalNumFatalIssues + TotalNumFailedLoads)
    if ($bpExitCode -ne 0) {
        $check2.status = "fail"
        $check2.errors = $bpExitCode
        $check2.details += @{
            asset    = "N/A"
            severity = "error"
            message  = "CompileAllBlueprints returned $bpExitCode error(s)/failed load(s)"
        }
    }

    # Parse log for the summary line
    if (Test-Path $bpLogFile) {
        $bpLogContent = Get-Content $bpLogFile -ErrorAction SilentlyContinue

        $summaryMatch = $bpLogContent | Where-Object {
            $_ -match "Compiling Completed with (\d+) errors? and (\d+) warnings? and (\d+) blueprints? that failed to load"
        }
        if ($summaryMatch) {
            if ($summaryMatch -match "(\d+) errors? and (\d+) warnings? and (\d+) blueprints? that failed to load") {
                $bpErrorCount = [int]$Matches[1]
                $bpWarningCount = [int]$Matches[2]
                $bpFailedLoads = [int]$Matches[3]

                $check2.errors = $bpErrorCount + $bpFailedLoads
                $check2.warnings = $bpWarningCount

                Write-Host "Blueprint compilation: $bpErrorCount errors, $bpWarningCount warnings, $bpFailedLoads failed loads"
            }
        }

        # Parse individual Blueprint error lines
        $bpErrors = @($bpLogContent | Where-Object {
            $_ -match "LogCompileAllBlueprintsCommandlet: Error:" -or
            $_ -match "LogBlueprint: Error:"
        })
        foreach ($err in $bpErrors) {
            $check2.details += @{
                asset    = "Unknown"
                severity = "error"
                message  = $err.Trim()
            }
        }

        # Parse warning lines
        $bpWarns = @($bpLogContent | Where-Object {
            $_ -match "LogCompileAllBlueprintsCommandlet: Warning:" -or
            $_ -match "LogBlueprint: Warning:"
        })
        foreach ($warn in $bpWarns) {
            $check2.details += @{
                asset    = "Unknown"
                severity = "warning"
                message  = $warn.Trim()
            }
        }

        if ($check2.errors -eq 0 -and $bpExitCode -eq 0) {
            if ($check2.warnings -gt 0) {
                $check2.status = "warn"
            } else {
                $check2.status = "pass"
            }
            Write-Host "Blueprint compilation passed"
        } else {
            $check2.status = "fail"
            Write-Host "Blueprint compilation failed"
        }
    } else {
        Write-Host "::warning::CompileBlueprints log file not found at: $bpLogFile"
    }
} catch {
    Write-Host "::warning::CompileAllBlueprints commandlet failed to execute: $_"
    $check2.status = "fail"
    $check2.errors = 1
    $check2.details += @{
        asset    = "N/A"
        severity = "error"
        message  = "CompileAllBlueprints execution error: $_"
    }
}

$checkResults += $check2
Write-Host ""

# ============================================================================
# Check 3: Circular Dependencies (Log Parsing)
# ============================================================================
# Parse the DataValidation log for circular dependency symptoms.
# Full graph traversal is deferred; this catches symptoms.

Write-Host "=== Check 3: Circular Dependencies ==="
Write-Host ""

$check3 = @{
    name     = "CircularDependencies"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

if (Test-Path $dataValLogFile) {
    $dvLogContent = Get-Content $dataValLogFile -ErrorAction SilentlyContinue

    # Look for circular dependency indicators in log lines.
    # Match specific UE5 log patterns that indicate dependency issues, NOT asset names
    # that happen to contain the word "circular" (e.g., SM_CircularBand).
    $circularLines = @($dvLogContent | Where-Object {
        $_ -match "LogLinker: Warning: CreateImport" -or
        $_ -match "(?i)circular\s+(dependency|reference|import)" -or
        $_ -match "failed to load.*dependency" -or
        $_ -match "LogLinker: Warning:.*cycle" -or
        $_ -match "LogLinker: Error:" -or
        $_ -match "(?i)dependency\s+cycle"
    })

    if ($circularLines.Count -gt 0) {
        $check3.status = "warn"
        $check3.warnings = $circularLines.Count
        foreach ($line in $circularLines) {
            $check3.details += @{
                asset    = "Unknown"
                severity = "warning"
                message  = $line.Trim()
            }
        }
        Write-Host "Found $($circularLines.Count) potential circular dependency indicator(s)"
    } else {
        Write-Host "No circular dependency indicators found"
    }
} else {
    Write-Host "DataValidation log not available for circular dependency analysis"
}

$checkResults += $check3
Write-Host ""

# ============================================================================
# Check 4: Asset Naming Conventions (PowerShell)
# ============================================================================
# Scan project Content/ for assets that don't follow Epic's naming conventions.
# Excludes Characters/Mannequins (Epic starter content) and .umap files.

Write-Host "=== Check 4: Asset Naming Conventions ==="
Write-Host ""

$check4 = @{
    name     = "NamingConventions"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

# Directories to exclude from naming checks (starter/engine content)
$excludeDirs = @(
    "Characters",
    "LevelPrototyping"
)

# Directory-specific prefix expectations
$directoryPrefixes = @{
    "Blueprints"     = @("BP_", "ABP_", "WBP_")
    "Input\\Actions" = @("IA_")
}

# General recognized prefixes (any asset with these is fine)
$recognizedPrefixes = @(
    "BP_", "ABP_", "WBP_", "BI_", "AC_",
    "SM_", "SK_", "SKM_",
    "M_", "MI_", "MF_", "MPC_",
    "T_",
    "AS_", "AM_", "BS_", "MM_",
    "Rig_", "SKEL_", "CR_", "PA_",
    "DT_", "CT_", "E_", "F_",
    "FXS_", "FXE_", "NS_",
    "PHYS_", "PM_",
    "MS_", "MO_", "MP_",
    "PPM_", "HDR_",
    "IA_", "IMC_",
    "AO_"
)

# Get all .uasset files in Content/ (exclude starter content directories)
$allAssets = Get-ChildItem -Path $contentDir -Recurse -Filter "*.uasset" -ErrorAction SilentlyContinue
$projectAssets = @()
foreach ($asset in $allAssets) {
    $relativePath = $asset.FullName.Substring($contentDir.Length + 1)
    $excluded = $false
    foreach ($excludeDir in $excludeDirs) {
        if ($relativePath -like "$excludeDir\*") {
            $excluded = $true
            break
        }
    }
    if (-not $excluded) {
        $projectAssets += $asset
    }
}

Write-Host "Checking $($projectAssets.Count) project assets for naming conventions..."

foreach ($asset in $projectAssets) {
    $relativePath = $asset.FullName.Substring($contentDir.Length + 1)
    $fileName = $asset.BaseName

    # Check directory-specific prefixes
    $directoryChecked = $false
    foreach ($dirPattern in $directoryPrefixes.Keys) {
        if ($relativePath -match "^$dirPattern\\") {
            $directoryChecked = $true
            $expectedPrefixes = $directoryPrefixes[$dirPattern]
            $hasExpectedPrefix = $false
            foreach ($prefix in $expectedPrefixes) {
                if ($fileName.StartsWith($prefix)) {
                    $hasExpectedPrefix = $true
                    break
                }
            }
            # Also accept Input Mapping Contexts at the Input level
            if (-not $hasExpectedPrefix -and $relativePath -match "^Input\\" -and $fileName.StartsWith("IMC_")) {
                $hasExpectedPrefix = $true
            }
            if (-not $hasExpectedPrefix) {
                $check4.warnings += 1
                $check4.details += @{
                    asset    = $relativePath
                    severity = "warning"
                    message  = "Expected prefix: $($expectedPrefixes -join ' or ') (found: $fileName)"
                }
                Write-Host "  Warning: $relativePath - expected prefix: $($expectedPrefixes -join ' or ')"
            }
            break
        }
    }

    # If not in a directory-specific check, verify it has any recognized prefix
    if (-not $directoryChecked) {
        $hasRecognized = $false
        foreach ($prefix in $recognizedPrefixes) {
            if ($fileName.StartsWith($prefix)) {
                $hasRecognized = $true
                break
            }
        }
        if (-not $hasRecognized) {
            $check4.warnings += 1
            $check4.details += @{
                asset    = $relativePath
                severity = "warning"
                message  = "No recognized naming convention prefix (found: $fileName)"
            }
            Write-Host "  Warning: $relativePath - no recognized prefix"
        }
    }
}

if ($check4.warnings -gt 0) {
    $check4.status = "warn"
    Write-Host "Naming convention check: $($check4.warnings) warning(s)"
} else {
    Write-Host "Naming convention check passed"
}

$checkResults += $check4
Write-Host ""

# ============================================================================
# Check 5: Unused/Orphaned Assets (PowerShell - Simplified/Informational)
# ============================================================================
# Simplified heuristic: check for .uasset files not in directories used by maps
# or known Blueprint directories. Informational only -- never errors.

Write-Host "=== Check 5: Unused/Orphaned Assets (Informational) ==="
Write-Host ""

$check5 = @{
    name     = "UnusedAssets"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

# Known directories that are expected to contain used assets
$knownUsedDirs = @(
    "Blueprints",
    "Input",
    "Maps"
)

# Get all project-owned .uasset files (excluding starter content)
$orphanCandidates = @()
foreach ($asset in $projectAssets) {
    $relativePath = $asset.FullName.Substring($contentDir.Length + 1)
    $topDir = ($relativePath -split "\\")[0]
    $inKnownDir = $false
    foreach ($knownDir in $knownUsedDirs) {
        if ($topDir -eq $knownDir) {
            $inKnownDir = $true
            break
        }
    }
    if (-not $inKnownDir) {
        $orphanCandidates += $relativePath
    }
}

if ($orphanCandidates.Count -gt 0) {
    $check5.status = "warn"
    $check5.warnings = $orphanCandidates.Count
    foreach ($candidate in $orphanCandidates) {
        $check5.details += @{
            asset    = $candidate
            severity = "warning"
            message  = "Potentially unused asset (not in Blueprints, Input, or Maps directory)"
        }
    }
    Write-Host "Found $($orphanCandidates.Count) potentially unused asset(s) (informational)"
} else {
    Write-Host "No potentially unused assets found"
}

$checkResults += $check5
Write-Host ""

# ============================================================================
# Check 6: Large Asset Sizes (PowerShell)
# ============================================================================
# Warn on assets exceeding size thresholds.

Write-Host "=== Check 6: Large Asset Sizes ==="
Write-Host ""

$check6 = @{
    name     = "LargeAssets"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

# Size thresholds in bytes
$sizeThresholds = @{
    "T_"  = 100MB   # Textures: warn above 100 MB
    "SM_" = 200MB   # Static Meshes: warn above 200 MB
    "SK_" = 200MB   # Skeletal Meshes: warn above 200 MB
}
$defaultAssetThreshold = 500MB   # Any single .uasset: warn above 500 MB
$defaultMapThreshold   = 500MB   # Any single .umap: warn above 500 MB

# Check all files in Content/ (including starter content for size)
$allContentFiles = Get-ChildItem -Path $contentDir -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -eq ".uasset" -or $_.Extension -eq ".umap" }

foreach ($file in $allContentFiles) {
    $fileName = $file.BaseName
    $fileSize = $file.Length
    $relativePath = $file.FullName.Substring($contentDir.Length + 1)
    $threshold = $null
    $thresholdLabel = ""

    if ($file.Extension -eq ".umap") {
        $threshold = $defaultMapThreshold
        $thresholdLabel = "500 MB (map)"
    } else {
        # Check prefix-specific thresholds
        foreach ($prefix in $sizeThresholds.Keys) {
            if ($fileName.StartsWith($prefix)) {
                $threshold = $sizeThresholds[$prefix]
                $thresholdLabel = "$([math]::Round($threshold / 1MB)) MB ($prefix prefix)"
                break
            }
        }
        # Fallback to default threshold
        if (-not $threshold) {
            $threshold = $defaultAssetThreshold
            $thresholdLabel = "500 MB (default)"
        }
    }

    if ($fileSize -gt $threshold) {
        $fileSizeMB = [math]::Round($fileSize / 1MB, 2)
        $check6.warnings += 1
        $check6.details += @{
            asset    = $relativePath
            severity = "warning"
            message  = "File size ${fileSizeMB} MB exceeds threshold of $thresholdLabel"
        }
        Write-Host "  Warning: $relativePath - ${fileSizeMB} MB (threshold: $thresholdLabel)"
    }
}

if ($check6.warnings -gt 0) {
    $check6.status = "warn"
    Write-Host "Large asset check: $($check6.warnings) warning(s)"
} else {
    Write-Host "Large asset check passed (no oversized assets)"
}

$checkResults += $check6
Write-Host ""

# ============================================================================
# Check 7: Map Validation (Log Parsing)
# ============================================================================
# Parse the DataValidation commandlet log for map-specific errors.
# Also verify all .umap files in Content/Maps/ are present.

Write-Host "=== Check 7: Map Validation ==="
Write-Host ""

$check7 = @{
    name     = "MapValidation"
    status   = "pass"
    errors   = 0
    warnings = 0
    details  = @()
}

# Verify all .umap files exist in Content/Maps/
$mapsDir = Join-Path $contentDir "Maps"
if (Test-Path $mapsDir) {
    $mapFiles = Get-ChildItem -Path $mapsDir -Filter "*.umap" -ErrorAction SilentlyContinue
    Write-Host "Found $($mapFiles.Count) map file(s) in Content/Maps/"
    foreach ($mapFile in $mapFiles) {
        Write-Host "  Map: $($mapFile.Name)"
    }

    if ($mapFiles.Count -eq 0) {
        $check7.warnings += 1
        $check7.details += @{
            asset    = "Content/Maps/"
            severity = "warning"
            message  = "No .umap files found in Content/Maps/"
        }
    }
} else {
    $check7.warnings += 1
    $check7.details += @{
        asset    = "Content/Maps/"
        severity = "warning"
        message  = "Maps directory not found"
    }
}

# Parse DataValidation log for map-specific errors
if (Test-Path $dataValLogFile) {
    $dvLogContent = Get-Content $dataValLogFile -ErrorAction SilentlyContinue

    $mapErrors = @($dvLogContent | Where-Object {
        ($_ -match "LogLoad: Error:" -and $_ -match "\.umap") -or
        ($_ -match "LogWorld: Error:") -or
        ($_ -match "LogMapCheck: Error:") -or
        ($_ -match "Error:.*map.*package" -and $_ -notmatch "LogContentValidation")
    })

    $mapWarnings = @($dvLogContent | Where-Object {
        ($_ -match "LogLoad: Warning:" -and $_ -match "\.umap") -or
        ($_ -match "LogWorld: Warning:") -or
        ($_ -match "LogMapCheck: Warning:")
    })

    if ($mapErrors.Count -gt 0) {
        $check7.status = "fail"
        $check7.errors = $mapErrors.Count
        foreach ($err in $mapErrors) {
            $check7.details += @{
                asset    = "Unknown"
                severity = "error"
                message  = $err.Trim()
            }
        }
        Write-Host "Map validation found $($mapErrors.Count) error(s)"
    }

    if ($mapWarnings.Count -gt 0) {
        if ($check7.status -eq "pass") { $check7.status = "warn" }
        $check7.warnings = $mapWarnings.Count
        foreach ($warn in $mapWarnings) {
            $check7.details += @{
                asset    = "Unknown"
                severity = "warning"
                message  = $warn.Trim()
            }
        }
        Write-Host "Map validation found $($mapWarnings.Count) warning(s)"
    }

    if ($mapErrors.Count -eq 0 -and $mapWarnings.Count -eq 0) {
        Write-Host "Map validation passed (no map-specific errors in log)"
    }
} else {
    Write-Host "DataValidation log not available for map error analysis"
}

if ($check7.warnings -gt 0 -and $check7.status -eq "pass") {
    $check7.status = "warn"
}

$checkResults += $check7
Write-Host ""

# ============================================================================
# Aggregate Results
# ============================================================================

Write-Host "========================================"
Write-Host "Validation Summary"
Write-Host "========================================"

$totalErrors = 0
$totalWarnings = 0
$totalPassed = 0
$totalFailed = 0
$totalWarned = 0

foreach ($check in $checkResults) {
    $totalErrors += $check.errors
    $totalWarnings += $check.warnings
    switch ($check.status) {
        "pass" { $totalPassed++ }
        "fail" { $totalFailed++ }
        "warn" { $totalWarned++ }
    }
    $statusIcon = switch ($check.status) {
        "pass" { "[PASS]" }
        "fail" { "[FAIL]" }
        "warn" { "[WARN]" }
    }
    Write-Host "  $statusIcon $($check.name) - Errors: $($check.errors), Warnings: $($check.warnings)"
}

$overallStatus = "pass"
if ($totalFailed -gt 0) {
    $overallStatus = "fail"
} elseif ($totalWarned -gt 0) {
    $overallStatus = "warn"
}

Write-Host ""
Write-Host "Overall: $overallStatus (Passed: $totalPassed, Warnings: $totalWarned, Failed: $totalFailed)"
Write-Host "Total errors: $totalErrors, Total warnings: $totalWarnings"
Write-Host "========================================"
Write-Host ""

# ============================================================================
# Output -- JSON Report (validation-report.json)
# ============================================================================

$timestamp = (Get-Date -Format "o")
$projectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectPath)

# Build checks array for JSON
$jsonChecks = @()
foreach ($check in $checkResults) {
    $jsonDetails = @()
    foreach ($detail in $check.details) {
        $jsonDetails += @{
            asset    = $detail.asset
            severity = $detail.severity
            message  = $detail.message
        }
    }
    $jsonChecks += @{
        name     = $check.name
        status   = $check.status
        errors   = $check.errors
        warnings = $check.warnings
        details  = $jsonDetails
    }
}

$report = @{
    timestamp     = $timestamp
    project       = $projectName
    configuration = $Configuration
    checks        = $jsonChecks
    summary       = @{
        total_checks = $checkResults.Count
        passed       = $totalPassed
        warnings     = $totalWarned
        failed       = $totalFailed
        overall      = $overallStatus
    }
}

$reportJson = $report | ConvertTo-Json -Depth 10
$reportPath = Join-Path $OutputDir "validation-report.json"
$reportJson | Out-File -FilePath $reportPath -Encoding utf8
Write-Host "Validation report written to: $reportPath"

# ============================================================================
# Output -- GITHUB_STEP_SUMMARY (Markdown)
# ============================================================================
# Per CONTEXT.md decision: silent on success (no output when all pass)

$hasIssues = ($totalFailed -gt 0) -or ($totalWarned -gt 0)

if ($hasIssues -and $env:GITHUB_STEP_SUMMARY) {
    $lines = @(
        "## Asset Validation Results",
        "",
        "**Configuration:** $Configuration | **Overall:** $overallStatus",
        "",
        "| Check | Status | Errors | Warnings | Details |",
        "|-------|--------|--------|----------|---------|"
    )

    foreach ($check in $checkResults) {
        if ($check.status -ne "pass") {
            $icon = if ($check.status -eq "fail") { ":x:" } else { ":warning:" }
            $detailCount = $check.details.Count
            $lines += "| $($check.name) | $icon | $($check.errors) | $($check.warnings) | $detailCount issue(s) |"
        }
    }

    # Add details section for failed checks
    $failedChecks = @($checkResults | Where-Object { $_.status -eq "fail" })
    if ($failedChecks.Count -gt 0) {
        $codeBlock = '```'
        $lines += @("", "<details>", "<summary>Error Details</summary>", "")
        foreach ($check in $failedChecks) {
            $lines += "**$($check.name):**"
            $lines += ""
            $lines += $codeBlock
            foreach ($detail in $check.details) {
                if ($detail.severity -eq "error") {
                    $lines += "[$($detail.severity)] $($detail.asset): $($detail.message)"
                }
            }
            $lines += $codeBlock
            $lines += ""
        }
        $lines += "</details>"
    }

    $lines -join "`n" | Out-File -Append -FilePath $env:GITHUB_STEP_SUMMARY -Encoding utf8
    Write-Host "Step summary written to GITHUB_STEP_SUMMARY"
} elseif ($hasIssues) {
    Write-Host "GITHUB_STEP_SUMMARY not set (running locally) - skipping step summary"
} else {
    Write-Host "All checks passed - step summary suppressed (silent on success)"
}

# ============================================================================
# Output -- GitHub Actions Annotations
# ============================================================================

foreach ($check in $checkResults) {
    foreach ($detail in $check.details) {
        if ($detail.severity -eq "error" -and $Configuration -eq "Shipping") {
            Write-Host "::error::[$($check.name)] $($detail.asset): $($detail.message)"
        } elseif ($detail.severity -eq "error" -or $detail.severity -eq "warning") {
            Write-Host "::warning::[$($check.name)] $($detail.asset): $($detail.message)"
        }
    }
}

# ============================================================================
# Exit Code
# ============================================================================
# Development: exit 0 always (warnings only)
# Shipping: exit 1 if any errors found

if ($Configuration -eq "Shipping" -and $totalFailed -gt 0) {
    Write-Host ""
    Write-Host "::error::Asset validation failed in Shipping configuration ($totalErrors error(s)). Fix validation errors before packaging."
    exit 1
} elseif ($totalFailed -gt 0) {
    Write-Host ""
    Write-Host "::warning::Asset validation found $totalErrors error(s) in Development configuration (warnings only - not blocking)."
    exit 0
} else {
    Write-Host ""
    Write-Host "Asset validation completed successfully."
    exit 0
}

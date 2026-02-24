# Convert-GauntletToJUnit.ps1
# Converts Gauntlet E2E test output to JUnit XML format for dorny/test-reporter.
#
# Gauntlet outputs a structured results directory (GauntletResults/) containing:
#   index.json          - Overall pass/fail + per-test summary (Gauntlet format)
#   <TestName>/
#     Artifacts/        - Logs, screenshots
#     <Platform>/
#       *.log
#
# The Gauntlet index.json format differs from the UE5 Automation Test JSON that
# Convert-UETestsToJUnit.ps1 handles:
#   - Gauntlet:    Tests[].TestResult (Passed/Failed/TimedOut), Tests[].TimeToComplete
#   - UE Automation: report.tests[].state (Fail/Pass/NotRun), report.tests[].entries[]
#
# This script handles both formats (detected by JSON structure), plus the crash
# case where no JSON was produced before the process terminated.
#
# Usage:
#   .\Convert-GauntletToJUnit.ps1 -ReportDir "GauntletResults/" -OutputPath "junit-gauntlet-results.xml"
#
# Returns: Failure count (0 = all passed, >0 = some failed)

param(
    [Parameter(Mandatory=$true)]
    [string]$ReportDir,       # Path to GauntletResults/ directory

    [Parameter(Mandatory=$true)]
    [string]$OutputPath       # Path to output JUnit XML file
)

$ErrorActionPreference = "Stop"

Write-Host "Converting Gauntlet results to JUnit XML..."
Write-Host "  Report dir: $ReportDir"
Write-Host "  Output:     $OutputPath"

# ---------------------------------------------------------------------------
# Helper: Read JSON with BOM stripping (UE5 outputs with UTF-8 BOM)
# ---------------------------------------------------------------------------
function Read-JsonWithBOM {
    param([string]$Path)
    $content = Get-Content -Path $Path -Raw -Encoding UTF8
    # Strip UTF-8 BOM (both byte sequence patterns)
    $content = $content.TrimStart([char]0xFEFF)
    $content = $content -replace '^\xEF\xBB\xBF', ''
    return $content | ConvertFrom-Json
}

# ---------------------------------------------------------------------------
# Helper: Build XML testcase element for a passed test
# ---------------------------------------------------------------------------
function New-PassedTestcase {
    param(
        [System.Xml.XmlDocument]$Xml,
        [string]$Name,
        [string]$Classname,
        [double]$Time
    )
    $testcase = $Xml.CreateElement("testcase")
    $testcase.SetAttribute("name", $Name)
    $testcase.SetAttribute("classname", $Classname)
    $testcase.SetAttribute("time", [string]$Time)
    return $testcase
}

# ---------------------------------------------------------------------------
# Helper: Build XML testcase element for a failed test
# ---------------------------------------------------------------------------
function New-FailedTestcase {
    param(
        [System.Xml.XmlDocument]$Xml,
        [string]$Name,
        [string]$Classname,
        [double]$Time,
        [string]$FailureMessage,
        [string]$FailureType = "TestFailure"
    )
    $testcase = $Xml.CreateElement("testcase")
    $testcase.SetAttribute("name", $Name)
    $testcase.SetAttribute("classname", $Classname)
    $testcase.SetAttribute("time", [string]$Time)

    $failure = $Xml.CreateElement("failure")
    $failure.SetAttribute("message", $FailureMessage)
    $failure.SetAttribute("type", $FailureType)
    $failure.InnerText = $FailureMessage
    $testcase.AppendChild($failure) | Out-Null

    return $testcase
}

# ---------------------------------------------------------------------------
# Helper: Build XML testcase element for an error (crash/process error)
# ---------------------------------------------------------------------------
function New-ErrorTestcase {
    param(
        [System.Xml.XmlDocument]$Xml,
        [string]$Name,
        [string]$Classname,
        [string]$ErrorMessage
    )
    $testcase = $Xml.CreateElement("testcase")
    $testcase.SetAttribute("name", $Name)
    $testcase.SetAttribute("classname", $Classname)
    $testcase.SetAttribute("time", "0")

    $errorElement = $Xml.CreateElement("error")
    $errorElement.SetAttribute("message", $ErrorMessage)
    $errorElement.SetAttribute("type", "GameCrash")
    $errorElement.InnerText = $ErrorMessage
    $testcase.AppendChild($errorElement) | Out-Null

    return $testcase
}

# ---------------------------------------------------------------------------
# Collect all JSON files in the report directory
# ---------------------------------------------------------------------------
$allJsonFiles = @()
if (Test-Path $ReportDir) {
    $allJsonFiles = @(Get-ChildItem $ReportDir -Recurse -Filter "*.json" -ErrorAction SilentlyContinue)
}

Write-Host "Found $($allJsonFiles.Count) JSON file(s) in $ReportDir"

# ---------------------------------------------------------------------------
# Parse all JSON files - detect format and collect testcases
# ---------------------------------------------------------------------------
$allTestcases = [System.Collections.Generic.List[System.Xml.XmlElement]]::new()
$totalTests   = 0
$totalFailed  = 0
$totalErrors  = 0
$totalSkipped = 0
$totalTime    = 0.0

# Create XML document early (needed by helper functions)
$xml = [System.Xml.XmlDocument]::new()
$declaration = $xml.CreateXmlDeclaration("1.0", "UTF-8", $null)
$xml.AppendChild($declaration) | Out-Null

$testsuite = $xml.CreateElement("testsuite")
$testsuite.SetAttribute("name", "Gauntlet E2E Tests")

$processedFiles = 0

foreach ($jsonFile in $allJsonFiles) {
    Write-Host "  Processing: $($jsonFile.FullName)"

    try {
        $data = Read-JsonWithBOM -Path $jsonFile.FullName
    } catch {
        Write-Host "  ::warning::Could not parse $($jsonFile.Name): $_"
        continue
    }

    # -----------------------------------------------------------------------
    # DETECTION: Is this a Gauntlet index.json or UE Automation report.json?
    # Gauntlet: has Tests[] array with TestResult property
    # UE Auto:  has tests[] array with state property, plus succeeded/failed counts
    # -----------------------------------------------------------------------

    if ($data.PSObject.Properties["Tests"] -and $data.Tests -is [array]) {
        # ===================================================================
        # GAUNTLET FORMAT: index.json from RunUAT RunUnreal
        # Structure:
        #   Tests[].TestDisplayName  - human-readable name (e.g. "BootTest")
        #   Tests[].TestResult       - "Passed", "Failed", "TimedOut", "NotRun"
        #   Tests[].TimeToComplete   - seconds as float
        #   Tests[].Events[]         - log entries (optional, may be sparse)
        # ===================================================================
        Write-Host "  Format: Gauntlet (Tests[].TestResult)"

        foreach ($test in $data.Tests) {
            $testName   = if ($test.PSObject.Properties["TestDisplayName"]) { $test.TestDisplayName } else { "UnknownTest" }
            $testResult = if ($test.PSObject.Properties["TestResult"]) { $test.TestResult } else { "NotRun" }
            $testTime   = if ($test.PSObject.Properties["TimeToComplete"]) { [double]$test.TimeToComplete } else { 0.0 }
            $classname  = "Gauntlet.E2E"

            $totalTests++
            $totalTime += $testTime

            switch ($testResult) {
                "Passed" {
                    $tc = New-PassedTestcase -Xml $xml -Name $testName -Classname $classname -Time $testTime
                }
                "Failed" {
                    $totalFailed++
                    # Extract failure message from Events if available
                    $failMsg = "Test failed"
                    if ($test.PSObject.Properties["Events"] -and $test.Events) {
                        $errorEvents = @($test.Events | Where-Object { $_.PSObject.Properties["Type"] -and $_.Type -eq "Error" })
                        if ($errorEvents.Count -gt 0 -and $errorEvents[0].PSObject.Properties["Message"]) {
                            $failMsg = $errorEvents[0].Message
                        }
                    }
                    $tc = New-FailedTestcase -Xml $xml -Name $testName -Classname $classname -Time $testTime -FailureMessage $failMsg
                }
                "TimedOut" {
                    $totalFailed++
                    $tc = New-FailedTestcase -Xml $xml -Name $testName -Classname $classname -Time $testTime `
                        -FailureMessage "Test timed out after $testTime seconds" -FailureType "Timeout"
                }
                default {
                    # NotRun or unknown state
                    $totalSkipped++
                    $tc = $xml.CreateElement("testcase")
                    $tc.SetAttribute("name", $testName)
                    $tc.SetAttribute("classname", $classname)
                    $tc.SetAttribute("time", "0")
                    $skipped = $xml.CreateElement("skipped")
                    $tc.AppendChild($skipped) | Out-Null
                }
            }

            $allTestcases.Add($tc)
        }
        $processedFiles++

    } elseif ($data.PSObject.Properties["tests"] -and $data.PSObject.Properties["succeeded"]) {
        # ===================================================================
        # UE AUTOMATION FORMAT: standard automation test report
        # Structure matches Convert-UETestsToJUnit.ps1 expectations:
        #   .succeeded, .failed, .notRun, .totalDuration
        #   .tests[].testDisplayName, .tests[].state ("Pass"/"Fail"/"NotRun")
        #   .tests[].entries[].event.type, .entries[].event.message
        # ===================================================================
        Write-Host "  Format: UE Automation (tests[].state)"

        $totalTests   += $data.succeeded + $data.succeededWithWarnings + $data.failed + $data.notRun
        $totalFailed  += $data.failed
        $totalSkipped += $data.notRun
        $totalTime    += if ($data.PSObject.Properties["totalDuration"]) { [double]$data.totalDuration } else { 0.0 }

        foreach ($test in $data.tests) {
            $testName  = $test.testDisplayName
            $classname = if ($test.PSObject.Properties["fullTestPath"]) { $test.fullTestPath } else { "UE5.AutomationTest" }
            $testTime  = if ($test.PSObject.Properties["duration"]) { [double]$test.duration } else { 0.0 }

            if ($test.state -eq "Fail") {
                $failMsg = "Test failed"
                foreach ($entry in $test.entries) {
                    if ($entry.event.type -eq "Error") {
                        $failMsg = $entry.event.message
                        break
                    }
                }
                $tc = New-FailedTestcase -Xml $xml -Name $testName -Classname $classname -Time $testTime -FailureMessage $failMsg
            } elseif ($test.state -eq "NotRun" -or $test.state -eq "Skipped") {
                $tc = $xml.CreateElement("testcase")
                $tc.SetAttribute("name", $testName)
                $tc.SetAttribute("classname", $classname)
                $tc.SetAttribute("time", "0")
                $skipped = $xml.CreateElement("skipped")
                $tc.AppendChild($skipped) | Out-Null
            } else {
                # Passed or succeeded with warnings
                $tc = New-PassedTestcase -Xml $xml -Name $testName -Classname $classname -Time $testTime
            }

            $allTestcases.Add($tc)
        }
        $processedFiles++

    } else {
        Write-Host "  ::warning::Unrecognized JSON format in $($jsonFile.Name) - skipping"
    }
}

# ---------------------------------------------------------------------------
# Fallback: No JSON results produced (crash before Gauntlet wrote results)
# ---------------------------------------------------------------------------
if ($processedFiles -eq 0) {
    Write-Host "No parseable JSON results found - creating fallback crash report"

    # Determine status from env var set by the test step
    $gauntletStatus = $env:GAUNTLET_STATUS
    if (-not $gauntletStatus) { $gauntletStatus = "crashed" }

    $errorMessage = switch ($gauntletStatus) {
        "crashed"     { "Game crashed before Gauntlet produced test results. No test data available." }
        "test_failed" { "Gauntlet test session ended without producing result files. Check game logs." }
        default       { "No test results produced - Gauntlet did not complete successfully." }
    }

    $tc = New-ErrorTestcase -Xml $xml -Name "Gauntlet Session" -Classname "Gauntlet.E2E" -ErrorMessage $errorMessage
    $allTestcases.Add($tc)

    $totalTests  = 1
    $totalErrors = 1
    $totalFailed = 0
}

# ---------------------------------------------------------------------------
# Build testsuite attributes and append all testcases
# ---------------------------------------------------------------------------
$testsuite.SetAttribute("tests",    [string]$totalTests)
$testsuite.SetAttribute("failures", [string]$totalFailed)
$testsuite.SetAttribute("errors",   [string]$totalErrors)
$testsuite.SetAttribute("skipped",  [string]$totalSkipped)
$testsuite.SetAttribute("time",     [string][math]::Round($totalTime, 3))

foreach ($tc in $allTestcases) {
    # Import element into this document if it was created in a different context
    $testsuite.AppendChild($tc) | Out-Null
}

$xml.AppendChild($testsuite) | Out-Null

# ---------------------------------------------------------------------------
# Save JUnit XML output
# ---------------------------------------------------------------------------
$outputDir = Split-Path $OutputPath -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$xml.Save($OutputPath)

Write-Host ""
Write-Host "Gauntlet to JUnit conversion complete:"
Write-Host "  Output: $OutputPath"
Write-Host "  Tests:   $totalTests"
Write-Host "  Passed:  $($totalTests - $totalFailed - $totalErrors - $totalSkipped)"
Write-Host "  Failed:  $totalFailed"
Write-Host "  Errors:  $totalErrors"
Write-Host "  Skipped: $totalSkipped"

# Return failure count for exit code handling
return ($totalFailed + $totalErrors)

@{
    # Block Warning severity and above (Error, Warning)
    # Satisfies AUDIT-02: zero Warning+ findings
    Severity = @('Error', 'Warning')

    ExcludeRules = @(
        # --- PSAvoidUsingWriteHost ---
        # What it catches: Write-Host bypasses the PowerShell output pipeline,
        #   making output uncapturable by callers. In reusable modules, this
        #   prevents downstream processing of output.
        # Why it does not apply here: These are CI tool scripts executed directly
        #   by GitHub Actions runner steps, not reusable modules. Output goes to
        #   the runner log. GitHub Actions annotations (::error::, ::warning::)
        #   MUST use Write-Host to be intercepted by the Actions runner.
        #   Write-Output goes to stdout (captured as step output), losing
        #   annotation syntax.
        # Alternative safeguard: Scripts are invoked directly via `pwsh -File`,
        #   never dot-sourced or imported as modules.
        'PSAvoidUsingWriteHost'

        # --- PSUseShouldProcessForStateChangingFunctions ---
        # What it catches: Functions using 'New-', 'Set-', 'Remove-' verbs that
        #   modify system state (files, registry, services) should support
        #   -WhatIf and -Confirm via SupportsShouldProcess, enabling callers
        #   to preview or interactively confirm changes.
        # Why it does not apply here: The flagged functions (New-PassedTestcase,
        #   New-FailedTestcase, New-ErrorTestcase) are pure in-memory XML element
        #   builders. They create and return System.Xml.XmlElement objects using
        #   XmlDocument.CreateElement() - no files, registry keys, or system
        #   resources are modified. ShouldProcess would add interactive confirmation
        #   prompts to functions that have no side effects, contradicting CI
        #   unattended execution requirements.
        # Alternative safeguard: Functions are private helpers called only within
        #   the same script in a pipeline context where -WhatIf is irrelevant.
        'PSUseShouldProcessForStateChangingFunctions'
    )
}

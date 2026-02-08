# Testing Patterns

**Analysis Date:** 2026-02-07

## Current State

**No automated tests detected.** This is an early-stage project with minimal C++ code and no test infrastructure.

## Test Framework

**Available (Unreal Engine 5.7):**
- Automation Framework (built-in)
- Functional Testing Framework (built-in)
- Gauntlet (performance/stress testing)

**Currently Configured:**
- None

**Run Commands (when tests exist):**
```bash
# Run all tests via editor
UnrealEditor.exe ProjectWalkingSim.uproject -ExecCmds="Automation RunTests Now"

# Run specific test
UnrealEditor.exe ProjectWalkingSim.uproject -ExecCmds="Automation RunTests MyTestName"

# Run via command line (headless)
UnrealEditor-Cmd.exe ProjectWalkingSim.uproject -ExecCmds="Automation RunAll; Quit"

# Run via Session Frontend in editor
Window > Developer Tools > Session Frontend > Automation
```

## Recommended Test File Organization

**Location:**
- Co-located with source: `Source/ProjectWalkingSim/Private/Tests/`
- Or separate module: `Source/ProjectWalkingSimTests/`

**Naming:**
- `{ClassName}Test.cpp` for unit tests
- `{FeatureName}FunctionalTest.cpp` for functional tests

**Structure:**
```
Source/ProjectWalkingSim/
├── Private/
│   ├── Tests/                    # Test implementations
│   │   ├── SereneGameModeTest.cpp
│   │   └── ...
│   └── ...
├── Public/
└── ...
```

## Test Structure (Unreal Automation Framework)

**Simple Test:**
```cpp
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSereneGameModeTest,
    "ProjectSerene.GameMode.BasicTest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSereneGameModeTest::RunTest(const FString& Parameters)
{
    // Test implementation
    TestTrue(TEXT("Description"), bCondition);
    TestEqual(TEXT("Description"), Actual, Expected);

    return true;
}
```

**Complex Test (with setup/teardown):**
```cpp
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FSereneComplexTest,
    "ProjectSerene.Feature.ComplexTest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

void FSereneComplexTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
    OutBeautifiedNames.Add(TEXT("TestCase1"));
    OutTestCommands.Add(TEXT("Case1"));

    OutBeautifiedNames.Add(TEXT("TestCase2"));
    OutTestCommands.Add(TEXT("Case2"));
}

bool FSereneComplexTest::RunTest(const FString& Parameters)
{
    if (Parameters == TEXT("Case1"))
    {
        // Test case 1
    }
    else if (Parameters == TEXT("Case2"))
    {
        // Test case 2
    }

    return true;
}
```

**Latent Test (async/multi-frame):**
```cpp
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
    FWaitForConditionCommand,
    TFunction<bool()>, Condition
);

bool FWaitForConditionCommand::Update()
{
    return Condition();
}

bool FSereneLatentTest::RunTest(const FString& Parameters)
{
    // Start async operation
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForConditionCommand([this]() {
        return bOperationComplete;
    }));

    return true;
}
```

## Mocking

**Framework:** Manual mocking (no built-in framework)

**Patterns:**
```cpp
// Interface-based mocking
class ISereneService
{
public:
    virtual void DoSomething() = 0;
};

class FMockSereneService : public ISereneService
{
public:
    int32 DoSomethingCallCount = 0;

    virtual void DoSomething() override
    {
        DoSomethingCallCount++;
    }
};

// In test
FMockSereneService MockService;
SystemUnderTest.SetService(&MockService);
SystemUnderTest.Execute();
TestEqual(TEXT("Service called once"), MockService.DoSomethingCallCount, 1);
```

**What to Mock:**
- External services/APIs
- File system operations
- Network calls
- Time-dependent operations

**What NOT to Mock:**
- Core engine classes
- Simple data structures
- Pure functions

## Fixtures and Factories

**Test Data (recommended pattern):**
```cpp
namespace SereneTestHelpers
{
    // Factory function for test objects
    ASereneGameMode* CreateTestGameMode(UWorld* World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("TestGameMode");
        return World->SpawnActor<ASereneGameMode>(SpawnParams);
    }

    // Fixture data
    struct FTestPlayerData
    {
        FString PlayerName = TEXT("TestPlayer");
        int32 StartingHealth = 100;
    };
}
```

**Location:**
- `Source/ProjectWalkingSim/Private/Tests/TestHelpers.h`
- `Source/ProjectWalkingSim/Private/Tests/TestFixtures.h`

## Coverage

**Requirements:** None enforced

**View Coverage:**
- Not built-in to Unreal
- Use external tools (Visual Studio Code Coverage, llvm-cov)

## Test Types

**Unit Tests:**
- Scope: Individual classes/functions
- Framework: `IMPLEMENT_SIMPLE_AUTOMATION_TEST`
- Fast execution, no world required
- Test Flags: `EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter`

**Functional Tests:**
- Scope: Gameplay features requiring a world
- Framework: `AFunctionalTest` actor in levels
- Place in `Content/Tests/` map files
- Runs in Play-In-Editor mode

**Integration Tests:**
- Scope: Multiple systems working together
- Framework: `IMPLEMENT_COMPLEX_AUTOMATION_TEST` with latent commands
- May require specific test levels

**Performance Tests:**
- Framework: Gauntlet
- Scope: Frame time, memory usage, load times
- Test Flags: Include `EAutomationTestFlags::PerfFilter`

## Common Patterns

**Async Testing:**
```cpp
// Wait for condition
ADD_LATENT_AUTOMATION_COMMAND(FWaitForConditionCommand(Condition));

// Wait for frames
ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(FramesToWait));

// Wait for seconds
ADD_LATENT_AUTOMATION_COMMAND(FDelayedCallbackLatentCommand(
    [this]() { /* callback */ },
    SecondsToWait
));
```

**Error Testing:**
```cpp
// Expect log error
AddExpectedError(TEXT("Expected error message"), EAutomationExpectedErrorFlags::Contains);

// Trigger the error
SystemUnderTest.OperationThatLogsError();

// Framework validates error was logged
```

**World-Dependent Tests:**
```cpp
bool FWorldTest::RunTest(const FString& Parameters)
{
    // Create test world
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(World);

    // Run tests...

    // Cleanup
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);

    return true;
}
```

## Build.cs Configuration for Tests

**Enable testing in module:**
```csharp
public class ProjectWalkingSim : ModuleRules
{
    public ProjectWalkingSim(ReadOnlyTargetRules Target) : base(Target)
    {
        // ... existing config ...

        // Add for automation tests
        if (Target.bBuildDeveloperTools || Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PrivateDependencyModuleNames.Add("AutomationController");
        }
    }
}
```

## Recommended First Tests

**Priority 1 - Game Mode:**
- `Source/ProjectWalkingSim/Private/Tests/SereneGameModeTest.cpp`
- Verify game mode spawns correctly
- Verify default pawn class
- Verify player controller class

**Priority 2 - Functional Test Map:**
- `Content/Tests/FunctionalTestMap.umap`
- Basic player spawn test
- Movement test

---

*Testing analysis: 2026-02-07*

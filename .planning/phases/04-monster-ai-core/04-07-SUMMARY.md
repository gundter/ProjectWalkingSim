# Summary: 04-07 NavMesh, Blueprints, State Tree, PIE Verification

## Result: COMPLETE (with fixes)

**Tasks:** 2/2
**Duration:** ~45 min (interactive checkpoint with PIE debugging)

## Commits

| Hash | Description |
|------|-------------|
| 114c046 | feat(04-07): add NavMesh agent configuration for tall Wendigo character |
| 109a712 | fix(04-07): PIE verification fixes for AI system integration |
| 8e0e865 | feat(04-07): add editor assets for AI system (Blueprints, State Tree, NavMesh) |

## Deliverables

- **NavMesh config:** Wendigo agent (Radius=60, Height=275, Step=35) in DefaultEngine.ini
- **BP_WendigoCharacter:** Blueprint subclass with skeletal mesh placeholder
- **BP_PatrolRoute:** Blueprint subclass for level-placed patrol waypoints
- **BP_WendigoAIController:** Blueprint subclass of AI controller
- **ST_WendigoAI:** State Tree asset with hierarchical Patrol/Suspicious/Investigate structure
- **TestMap:** Updated with patrol route, Wendigo placement, NavMesh bounds

## Issues Found and Fixed During PIE Verification

| Issue | Fix | Commit |
|-------|-----|--------|
| STC_SuspicionLevel "missing instance value" State Tree compiler error | Added empty instance data struct + GetInstanceDataType() override | 109a712 |
| Waypoints stored as local-space but used as world-space | GetWaypoint() now transforms through actor transform | 109a712 |
| CurrentWaypointIndex reset on State Tree state re-entry | Moved index from task instance data to AWendigoCharacter | 109a712 |
| Sight perception not detecting player | Added AIPerceptionStimuliSourceComponent to player character | 109a712 |
| Sight didn't track player location for investigation | Added SetStimulusLocation() to SuspicionComponent | 109a712 |
| VisibilityScore too low (0.16 in full light) for 0.3 threshold | Lowered VisibilityThreshold to 0.05 | 109a712 |
| Sight suspicion accumulated too slowly for testing | Raised BaseSuspicionRate to 1.5, lowered SuspiciousThreshold to 0.2 | 109a712 |

## State Tree Structure (Final)

```
Root
└── Patrol (container, no task)
    ├── Suspicious (enter condition: STC_SuspicionLevel >= Suspicious)
    │   ├── Orient Toward Stimulus → On Succeeded → Investigate
    │   └── Investigate → On Succeeded → Patrol
    ├── Move to Waypoint (default, transition: On Tick STC_SuspicionLevel → Suspicious)
    │   └── STT_PatrolMoveToWaypoint → On Succeeded → Patrol Idle
    └── Patrol Idle (transition: On Tick STC_SuspicionLevel → Suspicious)
        └── STT_PatrolIdle → On Succeeded → Move to Waypoint
```

## Verified Behaviors

- [x] Wendigo patrols between waypoints (loop and ping-pong modes)
- [x] Wendigo pauses at waypoints with look-around behavior
- [x] Wendigo detects player via sight (visibility score → suspicion accumulation)
- [x] Wendigo reacts to sprint noise (hearing → immediate suspicion bump)
- [x] State Tree transitions: Patrol → Suspicious → Investigate → Patrol
- [x] Ping-pong patrol mode works for hallways (bLoopRoute = false)

## Notes

- AI tuning constants (VisibilityThreshold, BaseSuspicionRate, SuspiciousThreshold) are placeholder values — final calibration happens after Phase 6 lighting
- On Tick transitions work for demo but should be replaced with event-driven triggers for performance at scale
- Spline-based patrol routes noted as future consideration for polish/main release

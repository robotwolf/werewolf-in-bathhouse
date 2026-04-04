# High Priority To-Do

Operational queue only. If it is not helping us write the game, it does not belong in `Do Now`.

## Do Now

| Item | Reason | Status | Owner | Exit Condition |
| --- | --- | --- | --- | --- |
| Fix `BP_Room_WashShower` local shell leak | It fails in the Blueprint viewport, so Mason is lying locally before the generator even enters the room | In Progress | Codex | `BP_Room_WashShower` shows no exterior slit and stays within `RoomBoundsBox` |
| Fix hallway intrusion and gap behavior for `PublicHallStraight` and `PublicHallCorner` | Halls are still the repeat offenders at joins and are poisoning the whole slice | In Progress | Codex | Representative seeds show no hall intrusion into adjacent rooms and no daylight leaks at the same joins |
| Reject candidate rooms that overlap the room they are connecting to | The generator was skipping overlap checks against the target room, which let hallway-on-room intrusion sneak through the front door wearing a nametag | In Progress | Codex | Generator placement rejects candidates that intrude into the attached room instead of only checking bystanders |
| Restore one stable canonical bathhouse slice map for gameplay work | We need one map we can trust more than our mood | In Progress | Codex | `L_BathhouseSlice` regenerates into an honest graybox with no hall-room overlap artifacts in the representative room set |
| Fix PIE/runtime generation mismatch when editor preview looks healthy | Runtime regeneration has to agree with editor preview or the slice becomes a liar with good posture | In Progress | Codex | PIE regenerates the same layout family instead of collapsing to the entry facade because of false validation failures |
| Remove horizontal grid snap from connector alignment | The room audit cleared `ColdPlunge` and `PublicHallStraight`, so the remaining likely liar is X/Y snapping in `AlignRoomToConnector` | In Progress | Codex | Connected rooms align by exact connector position on X/Y and no longer intrude because of horizontal snap drift |
| Verify nav, staging, and Gideon still behave on stabilized geometry | Honest geometry that quietly breaks runtime is still bait store inventory | Pending | Codex | PIE confirms staging/Gideon runtime still works on the stabilized slice |

## Do Soon

| Item | Reason | Status | Owner | Exit Condition |
| --- | --- | --- | --- | --- |
| Add a quick room-technique regression pass for representative room classes | We need a repeatable “did the bathhouse start lying again?” check | Pending | Codex | A small repeatable validation pass covers LockerHall, WashShower, PublicHallStraight, PublicHallCorner, and EntryFacadeNight |
| Audit any remaining room-technique joins beyond the known hall offenders | Once the obvious overlap liars are fixed, we need to confirm no quieter liar is still in the walls | Pending | Codex | Representative room classes can connect without overlap or daylight gaps across multiple seeds |
| Fix or quarantine `BP_Room_EntryFacade_UserRemix` connector drift | The room audit found its north connector almost 900 uu off the claimed face, which makes it a future trap even if it is not the current canonical culprit | Pending | Codex | The user remix room is either corrected to the shared RoomBoundsBox contract or explicitly kept out of canonical generation |

## Parked But Important

| Item | Reason | Status | Owner | Exit Condition |
| --- | --- | --- | --- | --- |
| Mason `BoxShell + UnifiedDynamicMesh` path | It looks promising enough to keep, but not important enough to sabotage the bathhouse again | Parked | Codex | Stable isolated repro path exists and no canonical gameplay content depends on it |
| Authored-shell `ObjectShell` path with envelope anchors and opening sockets | Useful once the stock graybox contract is honest | Parked | Codex | First authored-shell pilot aligns cleanly to Ginny bounds without destabilizing stock generation |
| Mason material-region/profile work | Worth doing after coordinate truth, not before | Parked | Codex | Region/material policy is implemented on top of a stable shell contract |
| Mason recipe / inflation work | Good future architecture, terrible present distraction | Parked | Codex | We return to it only after the bathhouse slice is stable and gameplay work is flowing |

## Completed Items

| Item | Reason | Status | Owner | Exit Condition |
| --- | --- | --- | --- | --- |
| Stop editor-selection stalls in room Blueprints and the slice map | Selection churn was wasting time before we even touched gameplay | Completed | Codex | Room preview rebuilds now early-out on unchanged editor selection and heavy staging refresh is no longer automatic |
| Add automated geometry-within-bounds validation for stock Mason output | The generator trusted rooms more than rooms deserved | Completed | Codex | Mason bounds audit exists on room builds and generator validation now reports out-of-bounds stock geometry |
| Strip or gate remaining editor-only debug refreshes that fire on selection | Hidden editor work was stealing time and confidence | Completed | Codex | Staging debug refresh is explicit or opt-in, and viewport-only label/debug ticking is no longer the default |
| Audit all room blueprints and inherited room contract for connector/bounds drift | If `ColdPlunge` is lying, chances are it has cousins | Completed | Codex | `RoomContractAudit.md` categorizes the bathhouse room family and shows only `BP_Room_EntryFacade_UserRemix` as connector-suspect while `ColdPlunge` and `PublicHallStraight` audit clean |
| Closed-by-default blocked doors for doorway-like openings | Doorways should start visibly blocked and only open when generation actually earns the connection | Completed | Codex | Doorway-like openings now show a red blocker until a real room or hall connection removes it in editor preview and PIE |
| Rebuild connected rooms after successful joins so blockers actually disappear | PIE needed a runtime-safe geometry refresh that preserved connector identity instead of rerunning construction scripts and forgetting who was connected to whom | Completed | Codex | Newly connected doorways now rebuild immediately after registration and no longer keep stale blockers in PIE or editor generation |

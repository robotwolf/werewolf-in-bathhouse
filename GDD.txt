# Werewolf in Bathhouse

## Prototype Game Design Document

### High Concept
*Werewolf in Bathhouse* is a first-person horror/comedy mystery game set in an urban bathhouse that sits on top of an ancient supernatural ritual site. The player works the night shift and must keep the bathhouse running while investigating which named patron is the werewolf this run, what strange force is reshaping the building, and whether the player can survive the full moon without accusing the wrong person.

The tone is a straight-faced blend of dread, absurdity, and social embarrassment. Horror comes from uncertainty, distorted visibility, and sudden violence. Comedy comes from misread situations, awkward patron behavior, and management treating supernatural incidents like routine maintenance issues.

### Player Fantasy
The player is the only person in the building practical enough to notice that something is deeply wrong and still professional enough to keep handing out towels anyway.

### Core Pillars
#### 1. Social Deduction
The player studies named NPCs, compares stories, gathers clues, and decides who to trust, protect, accuse, or frame.

#### 2. Environmental Uncertainty
Steam, reflections, wet surfaces, muffled sound, and shifting architecture make the player unsure of what they saw and where danger really is.

#### 3. Crisis Management
The player is not just solving a mystery. They must also do their job, respond to guest needs, maintain key spaces, and stop panic from taking over the bathhouse.

### Core Gameplay Loop
The player loops through five activities during a run:

1. Receive or choose a maintenance task that takes them to a part of the bathhouse.
2. Observe NPC behavior and search for clues while completing that task.
3. Interpret suspicious evidence that may be true, misleading, or planted.
4. Decide who to question, help, monitor, or misdirect.
5. React to a supernatural event that changes access, danger, or NPC behavior.

This loop repeats across the night as the moon rises and the bathhouse becomes less stable.

### Run Structure
Each run represents one night shift during a full moon incident.

#### Phase 1: Opening Hours
- The bathhouse layout is generated from authored room modules.
- Named NPCs enter with routines, needs, and secrets.
- The player learns the current layout while handling normal tasks.

#### Phase 2: First Signs
- Strange clues begin to appear.
- NPC stories conflict.
- Steam, reflections, and noises create false alarms.
- Suspicion starts to build around multiple possible suspects.

#### Phase 3: Moonrise Event
- The building partially reconfigures.
- Some room connections change.
- One route closes and another opens.
- NPC routines break down into panic, hiding, aggression, or ritual behavior.

#### Phase 4: Hunt and Deduction
- The werewolf becomes more active and dangerous.
- Evidence becomes stronger but the environment grows less reliable.
- The player chooses whether to keep investigating, protect someone, or move toward an accusation or escape.

#### Phase 5: Resolution
- The player accuses a suspect, survives until dawn, joins the supernatural order, or is overwhelmed by the night.

### Procedural Generation Rules
The bathhouse is semi-procedural, not fully random. The goal is replayability with readability.

#### Authored Elements
- Room modules are hand-built.
- Named NPCs are authored characters with stable identities.
- Core clue types are designed in advance.
- Major moonrise events follow readable rules.

#### Procedural Elements
- Room arrangement and corridor connectivity.
- Which optional rooms appear in a run.
- Which doors are locked or restricted.
- NPC start locations and routine variations.
- Which named NPC is the werewolf.
- Which clues are true, misleading, planted, or ambiguous.
- Which room connections shift during the moonrise event.

#### Procedural Design Rules
- Every run must include all critical rooms needed to complete the game.
- The layout must remain traversable after each map shift.
- Reconfiguration should change adjacency, not erase room identity.
- The player should feel uncertain, not disoriented beyond recovery.

### Setting and Room List
#### Required Rooms
- Lobby / reception
- Locker room
- Steam room
- Dry sauna
- Showers
- Hot pool
- Cold plunge
- Staff laundry
- Boiler / maintenance room
- Staff corridor

#### Optional Rooms
- Lounge / juice bar
- Massage hall
- Private rental room
- Roof access
- Shrine niche
- Hidden undercroft entrance

#### Endgame Space
- Roman-era ritual chamber beneath the bathhouse

### Player Role
For the prototype, the player is a night-shift towel attendant with maintenance access.

This role supports:
- access to public and staff areas
- believable reasons to move through the whole bathhouse
- interaction with towels, lockers, laundry, spills, and guest complaints
- a grounded point of view inside an absurd situation

### NPC Design
The game features many named NPCs over time, but the first prototype should use five.

Each named NPC has:
- a public personality
- a private secret
- a normal nightly routine
- one suspicious but innocent behavior
- one behavior that changes if they are the actual werewolf

Example archetypes for the prototype:
- The flirty regular
- The anxious first-timer
- The smug fitness obsessive
- The overly calm manager
- The occult scholar who claims he is only here to relax

### Werewolf Role Assignment
One named NPC is secretly chosen as the werewolf at the start of each run.

Rules:
- Any major suspect NPC can become the werewolf.
- No suspect should feel obviously impossible or obviously correct.
- The chosen werewolf gains altered behaviors, clue traces, and transformation risk events.
- At least one other NPC should behave suspiciously for unrelated reasons.

This keeps each run uncertain even after the player learns the cast.

### Clue System
Clues come in three categories.

#### Physical Clues
- fur on towels
- claw marks
- broken lockers
- wet prints
- blood mixed with bath water

#### Social Clues
- overheard confessions
- contradictory statements
- sudden panic reactions
- apologetic notes
- rumors about where someone was seen

#### Supernatural Clues
- distorted reflections
- steam visions
- scent or breath effects near the full moon
- impossible noises through walls
- ritual symbols appearing in condensation

#### Clue Truth States
Not every clue is reliable.

A clue may be:
- true
- misleading
- planted
- ambiguous

The player solves the mystery by building a convincing case, not by finding one perfect proof.

### Core Resources and Meters
#### Steam Density
Controls visibility and may reveal or hide evidence.

#### Suspicion
Tracks who the player believes may be the werewolf. May also affect how the player acts toward NPCs.

#### Moon Influence
Represents rising supernatural pressure. Higher moon influence causes stronger manifestations, more aggressive werewolf behavior, and layout shifts.

#### Towel Inventory
A practical and comedic resource used to complete tasks, calm patrons, and inspect evidence.

#### Bathhouse Stability
Represents how well operations are being maintained. Low stability increases panic, locked-down spaces, and management pressure.

### Tone and Horror/Comedy Balance
The game should play its absurd premise seriously. Characters usually treat the bathhouse like a real workplace and social environment, even while bizarre things happen around them.

Comedy comes from:
- mistaken identity
- awkward timing
- NPC vanity and denial
- management refusing to close during obvious danger
- mundane explanations colliding with supernatural events

Horror comes from:
- not being sure what was seen
- being trapped in a shifting maze
- social trust breaking down
- sudden transformation or chase events
- discovering that the bathhouse may itself be alive

### The Bathhouse as an Active Force
The building may be an ancient spirit or supernatural organism that enjoys drama and escalation.

Possible functions:
- shifting room connections
- releasing steam at suspicious moments
- jamming or opening doors
- emphasizing certain clues while hiding others
- guiding the player toward or away from the truth

For the prototype, this force should be implied through the moonrise reconfiguration rather than fully explained.

### Failure States and Endings
Prototype endings:
- Correct accusation: the player identifies the werewolf and survives the night.
- Incorrect accusation: the wrong NPC is blamed, causing social fallout and leaving the real threat active.
- Survival without certainty: the player reaches dawn or escapes but does not resolve the mystery.

Future full-game endings may include:
- becoming the new alpha
- joining the bathhouse as permanent supernatural staff
- destroying the ritual site
- protecting a sympathetic werewolf
- discovering the bathhouse itself orchestrated the whole night

### First Playable Scope
The first Unreal prototype should prove the game's core identity with the smallest viable system set.

#### Include
- 8 to 10 modular rooms
- 5 named NPCs
- 1 hidden werewolf assigned at runtime
- 3 clue categories
- 2 to 3 maintenance tasks
- 1 moonrise map reconfiguration event
- 1 pursuit or attack sequence
- 3 simple endings

#### Exclude
- combat
- multiple werewolves
- large dialogue trees
- fully dynamic rebuilding every few minutes
- huge maps
- full narrative campaign
- advanced inventory systems

### Prototype Success Criteria
The prototype is successful if players can:
- understand the bathhouse layout quickly enough to investigate
- notice distinct behaviors in named NPCs
- gather clues that support multiple suspect theories
- feel tension when the map shifts mid-run
- make a meaningful accusation, even if they are wrong
- want to replay because the next run may tell a different story

### Technical Direction For Unreal 5.7
The prototype should favor simplicity and iteration speed.

#### Recommended Approach
- Build room modules as reusable level pieces or Blueprint actors.
- Use Data Assets for room definitions, NPC profiles, and clue data.
- Use Blueprint for most prototype logic.
- Use PCG mainly for dressing, prop variation, and spawn placement at first.
- Add deeper procedural logic only after the hand-authored modular flow is working.

#### Visual Priorities
- volumetric fog
- wet reflective materials
- frosted glass and silhouette-friendly lighting
- strong ambient audio and sound occlusion
- readable but moody color contrast

### Open Questions For Later
- Is the player ever secretly the werewolf?
- How much direct conversation should the player have with NPCs?
- Should accusation happen through a formal system or a final confrontation?
- How often can the building shift before it becomes frustrating?
- How much of the comedy is authored versus emergent?

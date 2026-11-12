# Tribute (working title)

Our entry for the 2026 Epic AI MegaJam. **Unreal Engine 5.8+**, solo.

You fly a saucer over a farm valley at night. You fill a manifest by beaming up cows and people while
the town's Heat climbs. Then a mothership the size of the sky comes down. To dock and ride up into its
hull, you have to answer its calls, played in light and tone.

The design doc is the source of truth for what the game is, its scope and its schedule. Read
`docs/DESIGN.md` before starting anything. It's a copy of the live doc at
https://claude.ai/artifact/3hYvvzuMMNN44rBDLQzn2E, which Leland edits. When a decision changes, update both.

## The deadline

- **Submit by Sunday, Nov 15, 2:00 PM PST.** The jam's hard deadline is 5:59 PM PST, and late
  entries are disqualified. The gap is a buffer, not spare time.
- **Feature freeze is Sunday at 11:00 AM.** After that, only fixes.
- **Nothing from "Should" starts until every "Must" works in a packaged build.** If something has
  to be cut, cut in this order: the helicopter, the named object, then the tribute down to two
  rounds. Never cut the arrival or the ascent: they are the entry.

## Jam rules that touch the code

- **Everything is built from Nov 12.** Extraction Point (Godot) is where the idea comes from, not a
  source. Don't copy its code, models, audio or Meshy prompts. Write new prompts.
- **Gameplay is ours.** We start from a Blank project, and no system comes from a template.
- **Assets:** Fab and Megascans only, with a licence that covers this use. Log every one in
  `docs/ASSETS.md` (name, source, licence) as you add it, because the submission has to list them.
  Log AI-generated assets there too, with the tool used.
- **No one else's IP.** The original sequence borrowed a famous film's five-note motif, and none of
  that comes here:
  - The eight sector notes all sit **within one octave**, so the motif's octave drop can't happen.
  - The pattern generator **rejects any call shaped +2, −4, −12, +7 semitones**, in any key. That
    check has a test.
  - The tribute rounds are **3, 4 and 6 notes**, never ending on a five-note call.
  - The tones have **our own voice** (glassy bells, hull resonance), not tuba or synth brass.
  - The ship is **our own design**: a dark, angular hull with one ring of 8 sectors. No tiered
    underside covered in lights, no keyboard-style light board, no famous mountain.
  - **Never name the film:** not in code, comments, commits, docs, the itch page or the dev log.

## House rules (carried over from INFALL, Sungrazer, Wake Cycle and Extraction Point)

- **Controls:** the controls list on the title and pause cards, in the compact format (groups side
  by side; keys and pad). WASD or the left stick to fly, Space or A to hold the beam (tap to pulse),
  Esc or Start to pause.
- **Options:** an Options card with three sound levels (overall, music, effects; 80 is the mix as it
  comes) and sound on/off. These are the Sound Classes `Overall`, `Music` and `Effects`.
- **Sound:** everything has a sound, and nothing drones. The beam hums only while held. The ship's
  presence is pulses, not a bed.
- **Feedback:** big recognition, with a sound and a burst for each lift, each manifest line filled,
  and each note answered.
- **Readable without hand-holding:** the saucer's rim shows Heat (blue to red). The HUD stays small:
  Power, Heat, the manifest and the clock. No floating quest markers.
- **Limited things need a refill you can find:** Power refills over a transformer, which blacks out
  that farm and raises Heat.
- **Copyright:** every title card ends with "© 2026 Leland Rangel. All rights reserved."
- **Names:** the game is Tribute for now. Page copy never names the games it's in the vein of.
- **Testing:** mute it, and close what you open.
- **Committing:** commit after each step, each with a playable build. End commit messages with the
  same Co-Authored-By trailer as the other repos.
- **Code:** plain names, and comments that say what a thing is for.

## How it's built

The rules are in C++ and the look is in Blueprint subclasses. C++ is text, which an agent can read,
diff and review. The Blueprints keep the tuning, meshes and effects editable in the editor.

| System | Class / asset | Notes |
| --- | --- | --- |
| Saucer | `ASaucerPawn` (C++) → `BP_Saucer` | `FloatingPawnMovement`, a down trace for the beam, a lift that turns the target as it rises |
| Cows and people | `ATargetCharacter` → `BP_Cow`, `BP_Person` | AIController: wander with `MoveTo`, flee when the beam is near |
| Power and Heat | `UPowerHeatComponent` on the saucer | Police at 45% Heat, the helicopter at 80%. Heat falls when you lie low |
| Responders | `AResponder` → `BP_PoliceCar`, `BP_Helicopter` | Chase on the NavMesh, with a spotlight cone |
| Manifest and clock | `ATributeGameState`, row in `DT_Manifests` | 3 cows, 2 people, 1 named object; the ship arrives at 7:00 |
| Run flow | `ATributeGameMode` | Raid → arrival → tribute → dock and ascent, or saucer down, or left behind |
| Mothership | `BP_Mothership` | 8 sectors on dynamic material instances (emissive colour and intensity) |
| Tribute | `UTributeSequencer` (C++) | Spin, catch, pulse, stop, new colour. Reads `DT_Patterns` with a seed and runs the motif check |
| Arrival and ascent | `LS_Arrival`, `LS_Ascent` | Level Sequences, started by the GameMode |
| Sound | MetaSounds, `SC_Overall` / `SC_Music` / `SC_Effects` | One MetaSound per sector tone |
| UI | `WBP_Title`, `WBP_Controls`, `WBP_Options`, `WBP_HUD`, `WBP_Debrief` | UMG |
| Input | `IA_*`, `IMC_Default` | Enhanced Input, keys and pad |

Content lives under `Content/Tribute/` (`Maps`, `Saucer`, `Targets`, `Mothership`, `UI`, `Audio`,
`Data`). Keep this table up to date as classes are added.

## Working with the editor (Unreal MCP)

- **The editor has to be open before `claude`.** The MCP server (`http://127.0.0.1:8000/mcp`) lives
  inside it.
- **Only one session drives the editor at a time.** Tool calls run one at a time on the game
  thread. Other sessions can write C++ on their own branches, without the MCP.
- **Keep each request small.** Claude Code times out a tool call after about 120 seconds.
- **Commit before each MCP task**, so a bad edit is one `git restore` away. Save the assets you
  changed: an unsaved asset is lost when the editor closes.
- **Live Coding misses new reflected C++** (new classes, `UPROPERTY`, `UFUNCTION`). Close the editor,
  build, and reopen.
- **Never widen the server.** Leave Additional Allowed Origins empty.

## Verifying

- **Verify by looking, not by reasoning about the code.** For anything visual, take a screenshot in
  play (`HighResShot 1`, saved under `Saved/Screenshots/`) and look at it before saying it works.
- **Play the whole run** after any change to the run flow: raid, arrival, tribute, ascent, and both
  failure endings.
- **Tests:** Automation tests for the rules (Power and Heat thresholds, the manifest, the tribute
  state machine, and the motif check). Run them from Session Frontend or with
  `-ExecCmds="Automation RunTests Tribute"`.
- **Package early.** A Shipping build Friday night and Saturday night, not first on Sunday.

## Packaging

- **Shipping configuration**, never the project folder. Windows:
  `Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -project="<path>\Tribute.uproject" -platform=Win64 -clientconfig=Shipping -build -cook -stage -pak -archive -archivedirectory="<out>"`
  On Mac, use `RunUAT.sh` and `-platform=Mac`.
- Zip as `TeamName_GameName.zip`, **under 1 GB**. Run it from a clean folder before uploading.
- Upload to our own itch.io page, then submit it to the jam.

## The dev log

Add a few lines to `docs/DEVLOG.md` at the end of each step: what was built, what an agent got
wrong, and what got cut. It is written up for the Glass Box prize, and it gets written as we go,
because Sunday has no time for it.

## Things that bit, and why the code looks the way it does

(Add each one as it happens: what broke, how it showed up, and the fix.)

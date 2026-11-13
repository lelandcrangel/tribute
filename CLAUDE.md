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
| Saucer | `ASaucerPawn` (C++) → `BP_Saucer` | Hovers 9 m up, leans, holds a beam (tap = pulse). Input is built in code until `IA_`/`IMC_` assets exist. `BP_Saucer` not made yet |
| Liftable things | `ULiftableComponent`, `ALiftableProp` | The component marks what the beam can take and what it counts as (cow, person, named object); the prop is a physics box for grey boxes and the named object |
| Cows and people | `ATargetCharacter` (`ACowCharacter`, `APersonCharacter`) → `BP_Cow`, `BP_Person` | `ATargetAIController`: wander about home with pauses, run from a lit beam within the panic radius, go still while lifted, bolt when dropped. Grey-box tinted shapes until the Blueprints exist |
| Power and Heat | `UPowerHeatComponent` on the saucer | Police at 45% Heat, the helicopter at 80% (called off 5 below). Heat falls after 4 s lying low. The saucer steps it every frame; tests in `Source/Tribute/Tests/PowerHeatTests.cpp`. The rim goes blue to red, with a ground glow |
| Transformers | `ATransformer` | Refill within 6 m; the first draw blacks the farm out (its yard light and any `FarmLights`); 120 Power each. One at each of the Valley's three farms |
| Responders | `AResponder` (`APoliceCar`) → `BP_PoliceCar`, `BP_Helicopter` | `AResponderAIController` drives toward the ground under the saucer at 9 m/s (the saucer is faster). The searchlight holds the saucer within 40 m unless a barn is in the way; lit, Heat climbs and can't fall. `ATributeGameMode` sends one from the actor tagged `PoliceStart` at the police level and calls it home when calm. Helicopter not built |
| Manifest and clock | `ATributeGameState` with `FManifest` and `FRaidClock` (`Manifest.h`), row `Default` in `Content/Tribute/Data/DT_Manifests` | 3 cows, 2 people, the scarecrow; the ship arrives at 7:00. The GameMode counts each lift; `OnShipArrived` fires when the clock runs out. Tests in `Tests/ManifestTests.cpp`. `/Game/Tribute/Data` is always cooked |
| Run flow | `ATributeGameMode` | Raid → arrival → tribute → dock and ascent, or saucer down, or left behind. For now it spawns the saucer and sends the police in and home by Heat level |
| Map | `Content/Tribute/Maps/Valley` | Grey box: the engine template, floor scaled to 500 m, a NavMesh over all of it, 14 cows, 6 people, six liftable boxes, two barns, three transformers, a `PoliceStart` point. Startup and game default map |
| Mothership | `AMothership` (C++) → `BP_Mothership`, `M_SectorGlow` | Two dark 200 m slabs crossed at 45 degrees; one ring of 8 sector panels (unlit emissive `M_SectorGlow`, a `Color` parameter), each with a pool of light on the ground at 60 m from the centre. Placed above the valley's centre (`ValleyFloorZ` says where the floor is); comes down to 120 m over 14 s. `PlayTone`, `NoteAnswered`, `CallMissed` hooks wait for the MetaSounds |
| Tribute | `FTributeRun`, `FTributeCalls`, `FTributeScale` (`TributeRules.h`) | Spin, catch, pulse, stop, new colour; rounds of 3, 4, 6; a pulse in the right pool answers, a wrong pool or 12 s is a miss, three misses and the ship leaves. Calls are seeded and refuse the motif shape. `DT_Patterns` isn't made: calls are generated. Tests in `Tests/TributeTests.cpp` |
| Arrival and ascent | `ATributeGameMode` | No Level Sequences yet: the GameMode cuts to a fixed camera far out and low for the arrival and for the dock and ascent, and the saucer's camera pulls up wide for the tribute. Police leave, farms black out, the moon dims at arrival. Endings: docked, left behind, saucer down, each with the debrief (`UTributeDebriefWidget`) |
| Sound | MetaSounds, `SC_Overall` / `SC_Music` / `SC_Effects` | One MetaSound per sector tone |
| UI | `WBP_Title`, `WBP_Controls`, `WBP_Options`, `WBP_HUD`, `WBP_Debrief` | UMG. The HUD is built: `ATributeHUD` shows `UTributeHUDWidget`, which lays itself out in code (Power, Heat, the countdown, the manifest) until a `WBP_HUD` restyles it. The cards and debrief aren't made |
| Input | `IA_*`, `IMC_Default` | Enhanced Input, keys and pad |

Content lives under `Content/Tribute/` (`Maps`, `Saucer`, `Targets`, `Mothership`, `UI`, `Audio`,
`Data`). Keep this table up to date as classes are added.

## Working with the editor (Unreal MCP)

- **The editor has to be open before `claude`.** The MCP server (`http://127.0.0.1:8000/mcp`) lives
  inside it.
- **Checking the server:** `lsof -nP -iTCP:8000 -sTCP:LISTEN` should show `UnrealEditor`. If it shows
  nothing, run `ModelContextProtocol.StartServer` in the editor's Cmd box. Our 5.8 install has no
  Agentic Workflows settings panel, so the console commands are the controls.
- **A new session after `.mcp.json` changes.** Claude Code loads its MCP servers when a session starts.
- **Only one session drives the editor at a time.** Tool calls run one at a time on the game
  thread. Other sessions can write C++ on their own branches, without the MCP.
- **Keep each request small.** Claude Code times out a tool call after about 120 seconds.
- **Commit before each MCP task**, so a bad edit is one `git restore` away. Save the assets you
  changed: an unsaved asset is lost when the editor closes.
- **Live Coding misses new reflected C++** (new classes, `UPROPERTY`, `UFUNCTION`). Close the editor,
  build, and reopen.
- **Never widen the server.** Leave Additional Allowed Origins empty.
- **Opening the editor from a shell:** `open -n -a "$HOME/Games/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app" --args
  "$PWD/Tribute.uproject" -ExecCmds="ModelContextProtocol.StartServer"`. The project has to go after `--args`.
- **Testing the beam without a keyboard:** the editor's Cmd box runs `ke * StartBeam` and `ke * StopBeam`
  (`ke * StartBeam | ke * StopBeam` is a tap, so a pulse). Slate's PressKey only taps, and the game viewport
  isn't a Slate ref, so holds go through the console. Lifts and pulses log under `LogTribute`.

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

The game is built on Leland's Windows PC (64 GB), so the entry is a **Windows 10/11 build**. The jam
also accepts macOS, but its page doesn't say what the judges play on, and Windows reaches the most of
them. A Mac build is an optional bonus download, made on the Mac (below), and only once the Windows
entry is safe.

### The Windows entry

1. **Package** in the Shipping configuration, archived outside the repo (say
   `..\TributeBuilds\<date>`), never in the project folder. With a launcher install:

   ```
   "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
     -project="<path>\Tribute.uproject" -platform=Win64 -clientconfig=Shipping ^
     -build -cook -stage -pak -archive -prereqs -nodebuginfo ^
     -archivedirectory="<out>" -unattended -utf8output
   ```

   That's `cmd` syntax. Claude Code's shell on Windows is Git Bash, where the same command takes
   forward slashes and `\` line breaks. Change `UE_5.8` to the installed version. `-prereqs` puts the runtime installer in the build, so
   a judge's PC that lacks the Visual C++ runtime can still run it. It takes a while; run it in the
   background and log to a file. If it fails, read the log's "Warning/Error Summary": one error fails
   the build even when the cook says `Done!`.
2. **Check it:** copy `<out>\Windows\` to a clean folder away from the project and run `Tribute.exe`.
   It must reach the title card and play.
3. **Zip for itch** as `TeamName_GameName.zip`, **under 1 GB**, with the contents of `Windows\` at
   the top of the zip:

   ```
   Compress-Archive -Path "<out>\Windows\*" -DestinationPath TeamName_GameName.zip
   ```

4. **Download it back from itch** and run it once from the download, so it goes through what a judge's
   copy will. It isn't code-signed, so Windows shows "Windows protected your PC". Click More info, then
   Run anyway. Say that on the itch page.
5. Upload to our own itch.io page, then submit it to the jam.

Do the whole run, the download-back included, on **Friday night's** build, so Sunday's is the second
time rather than the first.

### The Mac bonus build (optional)

Only after the Windows entry is uploaded. On the Mac (an Apple Silicon Mac with 16 GB; close the
editor while it cooks), pull the repo and:

1. **Package.** The engine on the Mac is at `~/Games/UE_5.8` (not the launcher's usual
   `/Users/Shared/Epic Games`). Archive outside the repo, under `../TributeBuilds/<date>`:

   ```
   "$HOME/Games/UE_5.8/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
     -project="$PWD/Tribute.uproject" -platform=Mac -clientconfig=Shipping \
     -build -cook -stage -pak -archive -archivedirectory="<out>" -unattended -utf8output
   ```

   **Ship the staged app, not the archived one.** On 5.8 the archive step copies the bare
   `Binaries/Mac/Tribute-Mac-Shipping.app` (no content, no dylibs), which crashes at launch. The real
   app is `Saved/StagedBuilds/Mac/Tribute-Mac-Shipping.app`; sign, zip and test that one.

2. **Sign** the app with the hardened runtime. Copy the staged app out as `Tribute.app` (renaming it is
   fine), then sign every dylib inside it first and the app last. `--deep` alone can miss the dylibs Unreal
   keeps under `Contents/UE/...`, and notarization rejects unsigned code:

   ```
   ID="Developer ID Application: Leland Rangel (ACLJK383JD)"
   find Tribute.app -type f -name "*.dylib" -exec codesign --force --options runtime --timestamp --sign "$ID" {} \;
   codesign --force --options runtime --timestamp --sign "$ID" Tribute.app
   codesign --verify --deep --strict --verbose=2 Tribute.app
   ```

   The bundle id is `com.lelandrangel.Tribute` (`CodeSigningPrefix` in `Config/DefaultEngine.ini`). On
   Nov 12 the signed raid ran under the hardened runtime with no entitlements. If a later signed build
   crashes on launch and the unsigned one doesn't, add an entitlements file with
   `com.apple.security.cs.disable-library-validation` and pass it with `--entitlements`, and only then.
3. **Notarize and staple.** The `notary` keychain profile is set up once with
   `xcrun notarytool store-credentials "notary" --apple-id <Apple ID> --team-id <TEAMID>`, using an
   app-specific password. Leland does that step himself; never put credentials in this repo or a
   command line.

   ```
   ditto -c -k --keepParent Tribute.app Tribute-notarize.zip
   xcrun notarytool submit Tribute-notarize.zip --keychain-profile "notary" --wait
   xcrun stapler staple Tribute.app
   ```

   Notarization usually takes minutes but can take over an hour. If it's rejected, read the reason
   with `xcrun notarytool log <submission-id> --keychain-profile "notary"`.
4. **Check it:** `spctl -a -vv -t exec Tribute.app` must say `accepted` and
   `source=Notarized Developer ID`. Anything else is not done.
5. **Zip** with `ditto -c -k --keepParent Tribute.app TeamName_GameName_Mac.zip`, under 1 GB, and add
   it to the itch page as a second download.

## The dev log

Add a few lines to `docs/DEVLOG.md` at the end of each step: what was built, what an agent got
wrong, and what got cut. It is written up for the Glass Box prize, and it gets written as we go,
because Sunday has no time for it.

## Things that bit, and why the code looks the way it does

(Add each one as it happens: what broke, how it showed up, and the fix.)

- **On the Mac, a fresh Xcode can't compile Unreal's shaders.** Recent Xcode ships without the Metal shader
  compiler, so the editor failed on first launch with `Xcode Metal Compiler error: cannot execute tool
  'metal' due to missing Metal Toolchain`. Fix: `xcodebuild -downloadComponent MetalToolchain` (about
  700 MB), check with `xcrun metal --version`, then relaunch. If the error survives the install, delete
  the project's `DerivedDataCache/` and relaunch. Any new Mac or Xcode update needs it checked again.
- **Epic's MCP docs describe 5.9, and 5.8 differs.** There was no Edit > Editor Preferences > Agentic
  Workflows panel, so Generate Client Config wasn't a button. The console command
  `ModelContextProtocol.GenerateClientConfig ClaudeCode` wrote `.mcp.json` instead. Claude Code here runs
  in the desktop app, so `claude mcp list` isn't available in Terminal; `lsof` on port 8000 plus a test
  prompt ("spawn a cube at the origin") is how to tell whether it's connected.
- **5.8's MCP server doesn't start on its own.** Auto Start Server is off by default in 5.8, so a
  fresh editor has no server and `claude` finds nothing. On Nov 12 it only ran because it was started
  by hand (`ModelContextProtocol.StartServer` in the console). Fix: Edit > Editor Preferences >
  Model Context Protocol > Auto Start Server on. It's a per-user setting under `Saved/`, so it isn't in
  git and a new machine needs it again. Agents are blocked from changing the MCP server's own settings,
  so Leland flips it.
- **The first package failed its cook because of the MCP toolsets.** UAT stopped with
  `ExitCode=25 (Error_UnknownCookFailure)` even though the cook log says `Done!`. The cause was two
  errors: `Asset manager settings do not include a rule for assets of type GameFeatureData`. The
  toolsets that AllToolsets turns on are Game Feature plugins, and a Blank project has no rule for
  them. Fix: the `GameFeatureData` entry under `[/Script/Engine.AssetManagerSettings]` in
  `Config/DefaultGame.ini`. Look for real errors in the log's "Warning/Error Summary"; one error is
  enough to fail the build.
- **The archived Mac app crashed at launch.** After a successful BuildCookRun, the app in the archive
  folder died at once with `Library not loaded: @rpath/libtbb.12.dylib`. The archive step copied
  `Binaries/Mac/...app` (272 MB, executable only) instead of the staged app (958 MB, with `Contents/UE`
  holding the paks and dylibs). Its rpaths also point back into the engine install by relative path,
  so a test next to the project can mislead; always test from a clean folder. Fix: zip
  `Saved/StagedBuilds/Mac/Tribute-Mac-Shipping.app` instead. That one ran from a clean folder.
- **An empty project already zips to 611 MB on the Mac.** About 505 MB is the pak, mostly shaders (ray
  tracing and Substrate are on), plus a 272 MB executable. The jam limit is 1 GB, so check the size of
  every package, and turn off ray tracing or Substrate if they aren't earning their space.
- **`open -a UnrealEditor.app Tribute.uproject` opens the Project Browser.** `open` hands the project over as
  a document event, which the editor ignores, so it started with no project. Pass the path after `--args`.
- **A session that starts before the editor can't reach the MCP later.** `unreal-mcp` failed with
  ECONNREFUSED at session start, and reconnecting in /mcp didn't bring it back. The server itself was fine
  and answers plain HTTP, so `Scripts/ue_mcp_call.py` and `Scripts/ue_mcp_describe.py` call the same tools.
  Better: open the editor first.
- **The saucer sat on the floor instead of hovering.** Its ground trace looked only for `WorldStatic`, and
  the template map's floor is `WorldDynamic` (BlockAllDynamic, Movable). The trace now takes both. Anything
  that's ground to the saucer must be one of those two; props are `PhysicsBody` and cows `Pawn`.
- **A box let go mid-lift flew upward.** Turning physics back on kept the speed the beam had been moving
  it at. `ULiftableComponent::EndLift` clears the velocity so it drops.
- **Nobody wandered: the navmesh held three tiles.** A NavMeshBoundsVolume placed through the MCP starts
  2 m across, and the navmesh sized its tile pool then (`3 tile limit reached!` in the log). Scaling the
  volume up didn't resize it, so `GetRandomReachablePointInRadius` failed everywhere. Fix: run
  `RebuildNavigation` after resizing a nav volume, check the log says it recreated the dtNavMesh, and save.
- **Tinting the basic shapes did nothing.** In 5.8 `/Engine/BasicShapes/Cube` uses `WorldGridMaterial`,
  which has no parameters. Tint from `/Engine/BasicShapes/BasicShapeMaterial` (its vector parameter is
  `Color`), and set it in `OnConstruction`: placed instances keep their saved material overrides, so a
  constructor-only change never reaches them.
- **The editor's widget screenshot misses on-screen debug text.** SlateInspector's `Screenshot` showed no
  Power/Heat readout, but `shot showui` in the Cmd box (saved to `Saved/Screenshots/MacEditor/`) did. Use
  that to check anything drawn on the game's canvas or HUD.
- **Closing the editor during play crashes it, and then the MCP port is taken.** Killing the editor while a
  play session ran crashed it in `EndPlayMap`; its `CrashReportClient` kept port 8000, so the next editor
  logged `HttpListener unable to bind to 127.0.0.1:8000` and had no MCP. Stop play first, and quit any
  `CrashReportClient` before reopening.
- **An empty C++ widget never ticks.** The debrief built its layout on its first `NativeTick`, but a widget
  with no content isn't ticked or drawn, so it never appeared. Build the layout before adding it to the
  screen (`SetResult` builds it).
- **The unity build merges test files.** Three test files each had a `Flags` constant in their own
  namespace with `using namespace`, and the merged file found them ambiguous. Give each file's names their
  own prefix.
- **There's no MCP "Instructions" setting in 5.8.** The design doc's step comes from the 5.9 docs.
  Claude Code reads this `CLAUDE.md` from the project folder anyway.
- **A viewport capture is too big to read inline.** `CaptureViewport` returns the PNG as base64 (about
  2.6 MB), which overflows the tool result and gets saved to a file instead. Decode it with Python
  (`json` then `base64`), shrink it with `sips -Z 1024`, and look at that. It also refuses to run
  without an `annotations` argument; pass one with `gridSpacing: 0` for a plain shot.

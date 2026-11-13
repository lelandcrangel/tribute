# Dev log

Notes for the Glass Box prize, written as we go: what was built, what an agent got wrong, what got cut.
Each day becomes one itch.io post with screenshots.

## Day 1, Thursday Nov 12: the cut

- The mothership, its tribute and its light rig were cut from Extraction Point on 2026-08-21. This is the
  version it was meant to be. (The old sequence borrowed a famous melody; this one is our own.)
- Setup: UE version, Unreal MCP, Claude Code, the first empty Shipping build.
- First wall: Unreal wouldn't launch. The editor stopped on `cannot execute tool 'metal' due to missing
  Metal Toolchain`, because recent Xcode no longer ships the Metal shader compiler. One command fixed it
  (`xcodebuild -downloadComponent MetalToolchain`), then a long first shader compile. Nothing in the
  launcher says this; the error only names the fix once you get to it.
- Second wall: Epic's MCP docs describe 5.9, and our 5.8 editor has no Agentic Workflows settings panel.
  The console command `ModelContextProtocol.GenerateClientConfig ClaudeCode` wrote the client config,
  a new Claude Code session picked it up, and the first thing an agent did in the editor was spawn a cube
  at the origin. The agents can reach the editor now.
- Unreal MCP works. From Claude Code, beside the open editor, `unreal-mcp` listed about fifty toolsets
  (scene, actors, Blueprints, materials, Niagara, Sequencer, UMG, automation tests, config). It spawned
  a cube at the origin, took a viewport capture we could look at, and deleted the cube. With tool
  search on, the server exposes three tools (`list_toolsets`, `describe_toolset`, `call_tool`) and the
  agent finds the rest when it needs them.
- What the agent got wrong, or what the docs did:
  - The design doc's "MCP Instructions" setting doesn't exist in 5.8. The agent went looking through
    every settings section and then the engine headers before concluding it's a 5.9 feature.
    Claude Code reads `CLAUDE.md` from the project folder, so nothing's missing.
  - Auto Start Server was off. The server was up only because it had been started by hand, so the
    next editor launch would have had no MCP. The agent tried to flip it and was blocked (an agent
    can't change the MCP server's own settings), which is the right call; Leland flips it.
  - The first viewport capture failed for a missing `annotations` argument, and the second came back
    as 2.6 MB of base64, too big to read inline. Decoding it to a PNG and shrinking it worked.
  - CLAUDE.md had the launcher's usual engine path; this Mac's engine is in `~/Games/UE_5.8`.
  - The first empty Shipping package failed after 8.5 minutes. The cook finished, but the AllToolsets
    plugin pulls in Game Feature plugins, a Blank project has no Asset Manager rule for them, and
    those two errors fail the whole build. The agent's first wrapper also reported success, because
    the exit code it returned was from `tail`, not from the build. One line in `DefaultGame.ini` fixed
    the cook.
  - The second package said BUILD SUCCESSFUL, and the archived app crashed the moment it opened from a
    clean folder (`libtbb.12.dylib` missing). The archive step had copied the bare executable, not the
    staged app. The staged app, zipped and unzipped somewhere clean, opened and drew the default
    level. Lesson: "success" from the build tool means nothing until the zip opens somewhere else.
  - The empty project zips to 611 MB on the Mac, which is most of the 1 GB budget before a single
    asset goes in. Shaders are most of it.

## Day 2, Friday Nov 13: the raid in grey boxes

- Started a night early, on the Mac (the PC setup wasn't working yet). The project got its C++ module and
  the saucer: it floats 9 m up and follows the ground, its hull bobs, spins and leans into movement, and
  holding the beam catches the nearest liftable thing and lifts it, turning, into the hull. A tap under a
  quarter second is a pulse instead, which is how the tribute will be answered. Input is built in code, so
  it plays before any input assets exist.
- The grey-box Valley map is the engine's template with the floor stretched to 500 m, six boxes to lift
  and two barns.
- How it was tested: the agent can't hold a key through the editor's tools, so it moved the saucer over
  each box and held the beam from the console (`ke * StartBeam`), reading heights back and taking
  screenshots from inside the editor. The lift was slowed for one session to get a picture of it.
- What the agent got wrong:
  - Its ground trace only looked for static geometry, and the template's floor is movable, so the
    first play test had the saucer sitting on the floor. Reading the floor's collision settings found it.
  - Letting go of a box mid-lift threw it upward: physics picked up the beam's lift speed. Now it drops.
  - It launched the editor with `open -a ... Tribute.uproject`, which opened the Project Browser instead of
    the project, and it waited three minutes on a port that was never going to open.
  - This session started before the editor, so its MCP connection failed and wouldn't come back. The
    server was fine, so a small script talks to it over HTTP instead.
- Cows and people: one character class with a cow and a person kind, and a small controller instead of a
  behaviour tree. They wander around where they started and pause; a lit beam within 12 m (cows) or 18 m
  (people) sends them running straight away from it until it has been gone a few seconds; lifted, they
  go still; dropped, they land and bolt. People wander further and run faster. Tested by measuring: two
  cows 6 m from the beam were 14 and 23 m away three seconds later; a person dropped from 3.8 m landed
  and ran 25 m.
- What the agent got wrong, part two:
  - Nothing walked at first. The navmesh had been sized while its bounds volume was still the 2 m cube
    the editor spawns, and scaling the volume up left it at three tiles. A full navigation rebuild fixed it.
  - The tints didn't show twice: the 5.8 basic shapes use a material with no colour parameter, and then
    the cows already placed in the map kept their old material. It's set on construction now.
  - Its first console command went nowhere because the text box it typed into had gone stale when play
    stopped. It noticed only because the log never showed the command.
- The camera moved back to 50 m and down to 48 degrees after the first play test, so a fleeing herd
  stays in frame.
- Power and Heat: the rules sit in one component that the saucer steps each frame, so they can be tested
  without a world: eight automation tests cover the drain and refill rates, the police at 45 and the
  helicopter at 80, lying low, and the saucer going down. Three farm transformers refill the saucer and
  black their farms out. The saucer's rim runs from blue to red with Heat and throws that colour on the
  ground. Play-tested by reading the numbers back: eight seconds of beam took Power to 78 and Heat to
  21; beaming over a transformer brought the police; running Power out sank the saucer to the ground.
- What the agent got wrong, part three:
  - It charged Heat for hanging over a transformer even with a full tank. Play showed Heat climbing
    while nothing was being drawn; the rule now charges for what's actually drawn, and a test says so.
  - Its first rim glow was ten times too bright and turned the valley purple.
  - It decided the Power readout wasn't drawing, because the editor's screenshot tool doesn't capture
    on-screen text. An in-game `shot showui` showed it was there all along.
- The police car: when Heat passes 45 the GameMode sends one from the edge of town. It drives the
  NavMesh toward the ground under the saucer, slower than the saucer, and keeps a searchlight on it from
  up to 40 m unless a barn is in the way. Under the light Heat climbs and can't cool, so the answer is to
  run. When the town calms down it drives home and disappears. Play-tested end to end: sent at 45,
  parked 8 m off and lit the saucer, Heat pinned at 100 under the light with the beam off, lost the
  saucer when it fled 150 m, called off at calm, gone once home.
- The agent tried to test the police by setting Heat directly; Heat is read-only from outside on purpose,
  so it raised it the way a player would, by beaming over a transformer.
- Manifest, clock and HUD: the ship's order is a row in a data table (3 cows, 2 people, the scarecrow, 7
  minutes); the counting and the countdown are plain structs with their own tests, so 13 tests now guard
  the rules. The HUD stays small and in the corners: Power and Heat, "SHIP IN 5:41", and the manifest,
  where a line blinks on each lift and turns green when it's filled. Play-tested by lifting cows, a person
  and a box and reading the manifest back from the log and the screen.
- The HUD is laid out in C++ rather than a widget asset for now, so it was testable tonight; a WBP_HUD
  can restyle it later.
- Packaged the raid as a Mac Shipping build: it built in under two minutes on top of the earlier cook,
  and the staged app, zipped (601 MB) and unzipped in a clean folder, opened on the Valley with the herds,
  the saucer and the HUD counting down. Listing the package confirmed the map, DT_Manifests and the engine
  shapes the grey boxes are made of all went in. Not signed or notarized yet.
- The zip is 601 MB of a 1 GB budget, almost all of it shaders and engine content; the game itself adds
  a few megabytes so far.
- Still to check by hand: WASD and the stick, and how the lean looks in motion.
- Noted for the art pass: auto-exposure lifts the moonlit night to daylight, the beam's pool of light is
  blown out, and from the three-quarter camera the hull hides the thing being lifted.

## Day 3, Saturday Nov 14: the ship

## Day 4, Sunday Nov 15: shipping it

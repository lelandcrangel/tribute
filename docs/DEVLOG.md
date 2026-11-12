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

## Day 3, Saturday Nov 14: the ship

## Day 4, Sunday Nov 15: shipping it

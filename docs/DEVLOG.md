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

## Day 2, Friday Nov 13: the raid in grey boxes

## Day 3, Saturday Nov 14: the ship

## Day 4, Sunday Nov 15: shipping it

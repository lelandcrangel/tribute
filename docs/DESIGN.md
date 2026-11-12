# Mothership: MegaJam Design Doc

A snapshot of the design doc as of Nov 12, 2026. The live, editable version is
https://claude.ai/artifact/3hYvvzuMMNN44rBDLQzn2E. When a decision changes, update that one and copy the change
here, so agents on this machine read the same thing.

## Pitch

**Tribute** (working title) is one night's raid over a farm valley. It ends with the set piece Extraction
Point cut: a mothership the size of the sky comes down to collect what you took.

You fly a saucer over farms, beam up cows and people, and keep your Power up while the town's Heat climbs.
The mothership has sent a manifest and a deadline. When it arrives, it calls in light and tone across a ring
of 64 segments, and you have to answer it to dock. Get it right and you ride up into the hull. That ascent
ends the game.

**Why it's a Director's Cut:** this exact sequence was built in Extraction Point (the light rig, the tribute,
the dock into the hull) and then deleted on 2026-08-21. This version is the one it was meant to be. Building
it fresh in UE5 also gives it the scale the original's isometric camera never could.

## What was cut

Three connected pieces left Extraction Point on 2026-08-21, all on Leland's call. Two of them were never
finished.

| Piece | What it was | What happened |
| --- | --- | --- |
| The mothership set piece | Spawner, shadow, a 64-segment light rig, a five-tone sequence, and a dock into the hull on the way up | Deleted: about 350 lines out of `Raid3D`, plus `mothership.glb` and eleven test harnesses |
| The tribute | A contract with a clock: a manifest to fill (a demand roll, a fill bonus, one named object) before the ship arrives | Deleted with the contracts, and the manifest went from the HUD |
| The light-pattern table | The patterns the rig would play ("i have more ideas so we can randomize") | Never designed. Marked obsolete on 2026-08-24 |

The tribute's five phases were Leland's brief: **spin, catch, pulse, stop, new colour**. It passed review on
2026-08-24, and then the mothership was gone. In Extraction Point the mothership survives only as fiction:
specimens still go up to it, but nobody sees it.

Sources: `extraction-point/docs/DONE.md` (lines 5927, 9634 and 9844) and
`docs/design/ui-handoff-review-2026-08-21.md`.

## The game

One run takes about 10 minutes in one valley: seven minutes of raid, then the mothership.

```
The raid ──> The arrival ──> The tribute ──answered──> Dock and ascent (debrief and score)
   │                              │
   └─ Power hits zero             └─ three missed calls
      ──> Saucer down                ──> Left behind
```

An unfilled manifest doesn't end the run: it only lowers the score at the debrief.

1. **The raid (0:00–7:00).** The manifest asks for three cows, two people and one named object (a scarecrow,
   a tractor, the preacher). A clock counts down to the ship's arrival.
   - **Power** drains while you hover and faster while you beam. Refill it by hanging over a transformer,
     which blacks out that farm and raises Heat. This follows the house rule that a limited thing needs a
     refill you can find.
   - **Heat** climbs while you beam, faster with people. A police car comes at 45% and a searchlight
     helicopter at 80%. Heat falls when you lie low. The saucer's rim goes from blue to red as Heat rises, as
     in the original.
2. **The arrival (7:00).** The clouds open and the mothership comes down over the valley. The police
   scatter, the valley's lights die, and the sound drops away.
3. **The tribute (about 2 minutes), in the five phases:**
   - **Spin:** the ring spins up.
   - **Catch:** it plays a call, lighting segments and sounding a tone for each.
   - **Pulse:** you answer by flying the saucer under each lit segment in order and pulsing the beam.
   - **Stop:** the ring holds.
   - **New colour:** it changes colour and the next call is longer.

   There are three rounds of 3, 4 and 6 notes. The calls come from a randomised pattern table, which is the
   table that never got designed. All eight sector notes sit within one octave, so no call can make the film
   motif's octave drop. The generator also rejects any call with the motif's interval shape: up a step, down
   a third, down an octave, up a fifth.
4. **Dock and ascent.** The saucer rises into the hull while the valley falls away below. The debrief shows
   the manifest fill, the named-object bonus, how accurate the tribute was, and the Heat you left behind.

**Failing:** Power at zero brings the saucer down. An unfilled manifest still gets the ship, but at a lower
score. Three missed calls and the mothership leaves without you, which is the other ending.

**Controls:** WASD or the left stick to fly, Space or A to hold the beam (tap it to pulse), and Esc or Start
to pause. The ring is read as 8 sectors rather than 64 separate segments, so a call is readable at a glance.

## Scope

The must-haves make a complete, submittable game on their own. Nothing in "Should" starts until every
must-have works in a packaged build.

**Must**

- One valley, about 500 m across: three farms, a road, the edge of a town, and transformers
- The saucer: hover, tilt, beam, and lifting a target into the hull
- Cows and people that wander, and panic when the beam is near
- Power, Heat and the police car at 45%
- The manifest and the arrival clock on the HUD
- The mothership arriving, the 8-sector ring, three tribute rounds, and the dock and ascent
- The debrief, and the "left behind" ending
- Title, controls, pause and Options cards: three sound levels at 80 plus sound on and off, and
  "© 2026 Leland Rangel. All rights reserved." on the title card
- A sound for everything, and nothing drones
- A packaged macOS Shipping build under 1 GB, signed with Developer ID and notarized

**Should**

- The searchlight helicopter at 80%
- The named-object errand
- A randomised pattern table, with a seed shown on the debrief
- Full gamepad support
- Camera moves for the arrival and the ascent in Sequencer

**Cut (not in this jam)**

- Upgrades, contracts, more than one map, saves
- Extraction Point's perks (Cloak, Echo, Chronal Wake), time dilation, the Hunter, and Cold Snap
- Web and mobile builds
- Any reuse of Extraction Point code, models or audio

## Schedule

Aim to submit by **2:00 PM PST on Sunday**. The hard deadline is 5:59 PM PST, and late entries are
disqualified, so the gap is a buffer, not spare time.

**Thursday, Nov 12 (tonight): tools**

- [ ] Install UE5 and Xcode (open Xcode once to accept the licence, then run `xcode-select --install`)
- [ ] Join the jam on itch.io
- [ ] Package an empty project as a Shipping build and run the zip on a clean folder
- [ ] Claude Code and the Unreal MCP working beside the editor
- [ ] Dev log entry 1: the cut, and why it's coming back

**Friday, Nov 13: the raid in grey boxes**

- [ ] The saucer pawn: hover, tilt, beam, lift
- [ ] Cows and people: wander, panic, get lifted
- [ ] Power, transformers, Heat, the police car
- [ ] Manifest, clock and HUD
- [ ] **Package a build tonight** (the jam page's tip for day two), signed and notarized end to end

**Saturday, Nov 14: the mothership**

- [ ] Mothership mesh and arrival: clouds, scale, the valley going dark
- [ ] The ring, the call player, and three tribute rounds
- [ ] Dock, ascent, debrief, and the left-behind ending
- [ ] Art pass on the valley: Fab assets, lighting, fog
- [ ] Sound pass: every action, the tones, the mix
- [ ] Package a build

**Sunday, Nov 15: finish and submit**

- [ ] Morning: title, controls, Options cards, polish, bug fixes
- [ ] 11:00: feature freeze, then the final Shipping build named `TeamName_GameName.zip`
- [ ] 12:00: record the 30–60 second video
- [ ] 13:00: itch page, upload, the jam form, and the last dev log entry
- [ ] 14:00: submitted

If Saturday slips, cut in this order: the helicopter, the named object, and then the tribute down to two
rounds. Never cut the arrival or the ascent: they are the entry.

## Tech plan

The rules go in C++ and the look goes in Blueprints. C++ is plain text, which Claude Code can read, edit and
diff directly. Blueprint subclasses keep the tuning and visuals editable in the editor. If C++ setup fights
back on Thursday night, go Blueprint-only and let the agents work through the MCP.

| System | UE pieces | Where |
| --- | --- | --- |
| Saucer | Pawn + `FloatingPawnMovement`, a down trace for the beam, a physics handle or lerp for the lift | C++ class, Blueprint for the mesh and effects |
| Cows and people | Character + AIController: wander with `MoveTo` random points, flee when the beam is near | C++ AI, Blueprint per kind |
| Power and Heat | One component on the saucer, with thresholds that spawn responders | C++ |
| Police and helicopter | A chase AI on the NavMesh, a spotlight cone | C++ base, Blueprint per vehicle |
| Manifest and clock | GameState, with the manifest as a DataTable row | C++ |
| Mothership ring | 8 sectors driven by dynamic material instances (emissive colour and intensity) | Blueprint |
| Tribute | A state machine (spin, catch, pulse, stop, new colour) reading a pattern DataTable with a seed; rejects any call shaped +2, −4, −12, +7 semitones | C++ |
| Arrival and ascent | Level Sequences, triggered from the GameMode | Sequencer |
| Sound | MetaSounds for the tones and the saucer; Sound Classes Overall, Music and Effects for the Options card | Assets + Blueprint |
| UI | UMG: title, controls, Options, HUD, debrief | Blueprint widgets |
| Input | Enhanced Input, keys and pad | Assets |

**Directing the agents.** Only one Claude Code session drives the editor through the MCP at a time, because
the editor runs tool calls one at a time on its game thread. Other sessions can write C++ in parallel on their
own branches of one git repo, without the MCP. Commit after each step with a playable build. Keep a
`CLAUDE.md` in the project, with the house rules carried over from the Godot repos, so every agent works to
them.

## Unreal MCP setup (Thursday night)

Unreal MCP is built into the editor: a server inside Unreal at `http://127.0.0.1:8000/mcp` that Claude Code
connects to. It is **experimental** and arrived in UE 5.8, so install 5.8 or newer.

**1. The project and Claude Code**

- [ ] Install Claude Code on the same machine as the editor, and sign in
- [ ] Create a Blank project, then `git init` (this folder's `.gitignore` and `.gitattributes` cover the
      ignores and Git LFS)
- [ ] Commit the empty project before any agent touches it

**2. Turn it on in the editor**

- [ ] **Edit > Plugins:** check that "Unreal MCP" is enabled (it is by default), and enable "All Toolsets".
      "Terminal" is optional. Restart when asked
- [ ] **Edit > Editor Preferences > Agentic Workflows > Model Context Protocol:** check Auto Start Server is
      on, port 8000, path `/mcp`
- [ ] Under **MCP Client**, set Client Selection to Claude Code and click **Generate Client Config**. A
      launcher install writes `.mcp.json` to the project root. The console command
      `ModelContextProtocol.GenerateClientConfig ClaudeCode` does the same
- [ ] In the **Output Log**, look for `LogModelContextProtocol` reporting the address and port it bound to

**3. Connect Claude Code**

- [ ] With the editor open, open a terminal in the project folder and run `claude`. Approve the `unreal-mcp`
      server from `.mcp.json` when asked
- [ ] Type `/mcp` and check that `unreal-mcp` shows as connected. If there's no `.mcp.json`, add it by hand:
      `claude mcp add --transport http unreal-mcp http://127.0.0.1:8000/mcp`
- [ ] Ask "What toolsets are available, and what can you do with them?", then "Spawn a cube at the origin"
      and watch it appear

**4. Make it yours**

- [ ] In **Project Settings > Agentic Workflows > Model Context Protocol > Instructions**, add a line pointing
      at `CLAUDE.md`
- [ ] Commit, then package the empty project (the Schedule's Thursday item)

**Things that will bite**

- **The editor has to be running first.** The server lives inside it, so `claude` started before the editor
  finds nothing.
- **One MCP session at a time.** Tool calls queue on the game thread, and overlapping calls aren't supported.
- **Long calls time out** after about 120 seconds in Claude Code. Ask for big jobs in smaller steps.
- **Live Coding misses new reflected C++** (new classes, `UPROPERTY`, `UFUNCTION`). Close the editor, build,
  and reopen.
- **Localhost only, with no authentication.** Leave Additional Allowed Origins empty.
- **It's experimental.** Commit before each agent task, so a bad edit is one `git restore` away.
- **Debugging the connection:** `npx @modelcontextprotocol/inspector`, then connect over Streamable HTTP to
  `http://127.0.0.1:8000/mcp`.

Source: Epic's [Unreal MCP in Unreal Editor](https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?lang=en-US)
(5.9 docs). The 5.8 version of that page lists Auto Start Server as off by default, so check the setting
rather than assume it.

## Packaging on the Mac

The entry is a macOS build, signed with Leland's Developer ID and notarized, so a judge's Mac opens it with no
warning. The full commands are in `CLAUDE.md` under Packaging.

1. Package a Shipping build with `RunUAT.sh BuildCookRun -platform=Mac -clientconfig=Shipping`.
2. Sign `Tribute.app` with `codesign --options runtime --timestamp` and the Developer ID Application certificate.
3. Notarize with `xcrun notarytool submit --wait`, then `xcrun stapler staple`. Leland sets the `notary`
   keychain profile up once himself; credentials never go in the repo.
4. Check that `spctl -a -vv -t exec Tribute.app` says `accepted` and `source=Notarized Developer ID`.
5. Zip it with `ditto -c -k --keepParent` as `TeamName_GameName.zip`, upload it, then download it back and
   open it once.

Run all of this on Friday night's build. Notarization can take over an hour, and Sunday should be the second
run, not the first. If the signed app crashes but the unsigned one runs, the hardened runtime is blocking
something. Add the `com.apple.security.cs.disable-library-validation` entitlement only then.

## Art and audio

The valley is dark and small, so the mothership has something to be enormous against.

**Look**

- Night under a low moon. The light comes from sodium porch lights, headlights and the beam, with volumetric
  fog in the low ground. Lumen does the bounce.
- **The saucer:** a plasma rim that goes from blue to red with Heat. That is a world-space readout, so the
  HUD stays small.
- **The beam:** a volumetric cone, dust in Niagara, and the lifted target turning slowly as it rises.
- **The mothership:** a dark hull that fills the sky, unlit except for the ring. Its arrival is the only time
  the whole valley is lit, and it is lit by the ship.
- **The camera:** a high three-quarter view over the saucer for the raid, close to the original's isometric
  read. It pulls far back and low for the arrival, so the scale shows.

**Sound**

- Everything has a sound, and nothing drones: the beam hums only while it is held, and the ship's presence is
  pulses, not a bed.
- **The tones:** our own scale of eight notes within one octave, one per sector, made in MetaSounds. Not the
  five-note figure the original was named after (see Jam compliance).
- **The arrival:** the valley's sound ducks out (crickets, dogs, the town), then the ship's first call. This
  takes the "snow deadens a town" idea from Cold Snap and spends it here.
- **Feedback:** a sound and a burst for each lift, each manifest line filled, and each note answered.

## Jam compliance

The biggest risk is IP. The original sequence was a quote of a famous film's five-note motif, and the jam
forbids other people's IP. Our tones, ship and name must be our own.

**Pre-jam declaration (a draft for the form)**

> The idea comes from a set piece I cut from my game Extraction Point (Godot, cut on 2026-08-21): a
> mothership, a tribute with a manifest, and a dock into the hull. No code, models, audio or prompts from that
> game were reused. Everything was built in Unreal Engine 5 from Nov 12. Made before the jam: [list any
> CLAUDE.md, agent setup or prompts written earlier].

**Rules to keep**

- [ ] **No borrowed IP:** no film tones or score, no film ship design, and no named franchise in the title or
      page copy
- [ ] **The tones can't make the motif:** eight sector notes within one octave, a generator that rejects the
      motif's interval shape, and a last round of 6 notes rather than 5 (see The game)
- [ ] **Never name the film:** not in code, commits, the itch page, the video or the dev log. The dev log can
      still say "the old sequence borrowed a famous melody; this one is our own."
- [ ] **Assets:** Fab and Megascans assets only with their licence, each listed in `docs/ASSETS.md` and on
      the submission. The Synty packs need checking for Unreal use under their licence (see Open questions)
- [ ] **AI-made assets** (Meshy, Tripo or Rodin for the hull, for example): made during the jam, with the
      tools listed in "Tools you used". The Meshy prompts in Extraction Point's docs predate the jam, so write
      new ones or declare them
- [ ] **Custom gameplay:** start from a blank project. Every system above is ours, not a template's
- [ ] **Packaging:** a Shipping build, never the project; zipped as `TeamName_GameName`, under 1 GB, for
      macOS, signed and notarized so it opens with no Gatekeeper warning
- [ ] Upload to our own itch.io page, then submit it to the jam
- [ ] 18 or older, with an Epic account; the account email goes on the form

The full Official Rules on Box override the jam page's summary. Read them once before submitting.

## Submission

The 60-second video and the dev log are what the judges see first, so both are planned now rather than on
Sunday.

**Video shot list (about 60 s, gameplay in the first 5)**

| Time | Shot |
| --- | --- |
| 0–5 s | The beam lifts a cow while the rim goes from blue to red |
| 5–20 s | A person, a transformer blacking out a farm, the police car's lights on the road |
| 20–35 s | The clouds open; a wide, low shot as the mothership comes down over the valley |
| 35–50 s | A call and its answer: the sectors light, the tones sound, the saucer pulses under each |
| 50–58 s | The dock and ascent, with the valley falling away |
| 58–60 s | The title card |

**Dev log, for the Glass Box prize.** One entry a day, with screenshots (notes collect in `docs/DEVLOG.md`):

1. The cut: what was deleted and why it's coming back.
2. The raid in grey boxes, and directing the agents.
3. The ship: scale, the ring, the tones.
4. Shipping it: what got cut again.

**Prizes this entry can reach:** Best Overall (Unreal Engine), Solo Driver, Render Royalty for visuals, and
Glass Box for the dev log. Partner prizes need their tools named in "Tools you used": Claude Code and the
Unreal MCP, plus any of Meshy, Tripo or Rodin used.

## Open questions

- [ ] **Decided: Mac (Apple Silicon).** The entry is a macOS build, signed with Leland's Developer ID and
      notarized. Unreal can't package Windows from a Mac.
- [ ] **The name.** Keep "Tribute", or something else? Also choose a team name for the zip.
- [ ] **C++ or Blueprints?** How comfortable is Leland in each? It changes how the agents work.
- [ ] **Synty.** Do the packs' licences cover Unreal, and are the Unreal versions owned? If not, it's Fab and
      Megascans.
- [ ] **The camera.** A high three-quarter view, as planned, or third-person behind the saucer?
- [ ] **The tribute input.** Flying under each lit sector, as planned, or four colour buttons, which is quicker
      to build and less interesting?

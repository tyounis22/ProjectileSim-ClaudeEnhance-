# AI_WORKLOG.md — Projectile Sim Enhancement Log

## Project Summary

A raylib C++ cannon game built for EC-327. The player aims a cannon with arrow keys,
charges power by holding space, then releases to fire a cannonball at a bullseye target.
Wind changes every 3 hits. Target shrinks and changes color every 3 hits. 10 misses →
game over. Background 8-bit music with a mute button.

Build: `make && ./sim` (macOS/Homebrew raylib at `/opt/homebrew`)

---

## Architecture Map

| File | Purpose |
|---|---|
| `src/main.cpp` | Game loop, input, scoring, HUD drawing, world drawing |
| `src/Cannon.cpp` / `include/Cannon.h` | Aims and fires the cannon; draws base+barrel |
| `src/Projectile.cpp` / `include/Projectile.h` | Flying cannonball; inherits PhysicsBody |
| `src/Target.cpp` / `include/Target.h` | Flat disk target with 5-ring bullseye scoring |
| `src/Debris.cpp` / `include/Debris.h` | Explosion debris pieces; inherits PhysicsBody |
| `src/PhysicsBody.cpp` / `include/PhysicsBody.h` | Gravity + wind integration, bouncing, GenerateWind |
| `include/Entity.h` | Pure-virtual base: position, Update, Draw |
| `include/Constants.h` | Gravity, angle limits, power limits, debris cap, miss count |
| `Makefile` | Cross-platform build (macOS/Linux/Windows) |
| `music/8bit_music.mp3` | Background BGM |
| `images/start_screen.png` | Unused image asset (no menu yet) |

### Inheritance tree
```
Entity
  └─ PhysicsBody      (gravity + wind + bounce)
       ├─ Projectile  (the cannonball)
       └─ Debris      (hit fragments)
  └─ Target           (disk bullseye, static for now)
Cannon                (standalone, not an Entity)
```

---

## Enhancement Roadmap

### Phase 1 — Visual Polish (low risk, high impact)
- [x] Main menu screen (use existing `images/start_screen.png` + simple state machine)
- [x] Cannon recoil animation on fire (barrel snaps back, eases forward)
- [x] Muzzle flash (bright 2D quad drawn at barrel tip for 2-3 frames)
- [x] Smoke puff particles at muzzle on fire (small white/gray spheres, fade out)
- [x] Screen shake on fire and on hit
- [x] Hit flash (target briefly glows white on impact frame)
- [x] Polished HUD: stylized score box, combo text, misses display with icons

### Phase 2 — Gameplay Polish
- [x] Moving targets: horizontal patrol / arc path, speed increases per difficulty tier
- [x] Combo multiplier: consecutive hits multiply score, resets on miss
- [x] Difficulty ramp: target speed and size scale with hit count
- [x] "Wind change incoming" warning (flashes wind HUD before wind rerolls)
- [x] Ball trail (short alpha-faded sphere chain behind the projectile)

### Phase 3 — Environment Polish
- [x] Time-of-day system: random at launch (day/sunset/night), changes sky color + sun/moon/stars
- [x] Animated cloud drift
- [x] Rustling trees (subtle scale oscillation on leaves)
- [x] Add simple explosion sound at cannonfire, and target strike sound when target struck.

### Phase 4 — Ambitious (only if Phases 1-3 are stable)
- [ ] Moving truck/platform: cannon mounted on a slow-moving flatbed; adds lateral drift
- [ ] Larger world with camera pan

### Phase 5 — Final Cleanup
- [ ] Build verification (`make clean && make`)
- [ ] Remove dead/debug code
- [ ] Update this worklog with completed status

---

## Current State (baseline)

All Phase 0 (assignment) features are working:
- Azimuth/elevation/power aiming with arrow keys + space
- Wind physics + HUD
- Bullseye hit detection (5 rings, scored 1–5)
- Debris explosion on hit
- Target respawn with brief disappear
- Target shrinks + color-cycles every 3 hits
- Wind rerolls every 3 hits
- 10-miss game-over
- Background music + mute button

**NOT yet implemented:** main menu, VFX, moving targets, combo system, day-night, screen shake.

---

## Key Constants (Constants.h)
- `GRAVITY = -9.81`
- `MISSES_ALLOWED = 10`
- `MAX_DEBRIS = 300`
- `TARGET_RESPAWN_DELAY = 1.2f` seconds
- Azimuth range: -90 to 90°; Elevation: 0 to 89°; Power: 0 to 100 m/s
- Rotation rate: 30 deg/sec; Power charge rate: 60 m/s per sec

---

## Resume Instructions

1. Open this file first to recall current phase and what's done.
2. Read only the files needed for the next task — do not load the whole repo at once.
3. Build and test after every phase: `make && ./sim`
4. After each change, update the checklist above (mark `[x]`).
5. If the session stops mid-phase, the "Files Changed" section at the bottom records the last save point.

---

## Change Log

### Session 1 — Inspection only
- No files modified.
- Worklog created.
- Plan written below.

### Session 4 — Phase 3 complete
**Files changed:** `src/main.cpp` only

- **Time-of-day** — `TimeOfDay` enum (DAY/SUNSET/NIGHT); picked randomly at launch, re-rolled on restart. Sky colour via `ClearBackground`: day=sky-blue, sunset=warm-orange (220,110,55), night=deep-navy (5,5,30). Ground plane tinted to match. DAY draws yellow sun, SUNSET draws a large orange-red sun near the horizon, NIGHT draws a pale moon + 90 procedurally generated stars in a golden-angle dome distribution (four colour classes: blue-white, warm-yellow, red, violet). Small "DAY / SUNSET / NIGHT" badge added to HUD below the TIER badge.
- **Animated cloud drift** — all 8 cloud spheres Z-offset by `sin(t * 0.08) * 20 m`, giving a gentle 78-second back-and-forth cycle. Cloud colour tinted warm-orange for sunset, very dark for night.
- **Rustling trees** — each of the 6 leaf spheres multiplies its base radius by `1 + 0.045 * sin(t * 1.8 + i * 1.7)`, giving ±4.5% size oscillation with a unique phase per tree.
- **Sound effects** — `GenBoom()` and `GenHit()` synthesize 16-bit mono PCM waves at 44100 Hz using hash-based deterministic noise (no `rand()` required). Boom: white-noise + sub-bass at 55 Hz with 18× exponential decay (~0.35 s). Hit: chirping sine from 380→130 Hz with noise attack (~0.5 s). Both are played through raylib `Sound` objects; muted when the Mute button is active.

### Session 3 — Phase 2 complete
**Files changed:** `include/Target.h`, `src/Target.cpp`, `src/main.cpp`

- **Moving targets** — `Target::Update()` implemented: `patrolSpeed/patrolDir/patrolBound` for linear Z-patrol; `arcMode/arcPhase/baseY/baseZ` for circular Y-Z arc; Y clamped to stay above ground.
- **Combo multiplier** — `score += hitValue * comboCount` (first hit = ×1, second consecutive = ×2, …); `hitPopupValue` shows actual points earned; combo resets on miss.
- **Difficulty ramp** — `getDifficultyTier(hitCount)`: tier 0 (0-5 hits) static; tier 1 (6-11) 4 m/s patrol; tier 2 (12-17) 9 m/s patrol; tier 3 (18+) 1.2 rad/s arc. `target.ApplyDifficulty()` called after every `randomizeTarget()`.
- **Wind warning** — `windChangeWarning = (hitCount % 3 == 2)` after each hit; `DrawWindHUD` flashes ring+label red at 5 Hz when warning is active.
- **Ball trail** — `std::deque<Vector3> ballTrail` (max 12 positions); each frame ball is active, prepend position; drawn with `BLEND_ALPHA` as ghost spheres scaled and faded by age.
- **Difficulty tier indicator** — small colored "TIER N" badge below wind HUD (gray/green/yellow/orange).

### Session 2 — Phase 1 complete
**Files changed:** `include/Cannon.h`, `src/Cannon.cpp`, `src/Target.cpp`, `src/main.cpp`

- **Main menu** — `GameState` enum (`MENU/PLAYING/GAME_OVER`); draws `start_screen.png` scaled to window; SPACE/ENTER/click to start.
- **Cannon recoil** — `_recoilOffset` + `_recoilVelocity` private members; `Cannon::Update(dt)` runs a spring-damper (k=50, c=12) that eases the barrel back to rest after a -0.5m kick on fire.
- **Muzzle flash** — `_flashFrames` counter; two overlapping yellow/white spheres at barrel tip drawn for 3 frames post-fire.
- **Smoke puff** — `SmokeParticle` struct in `main.cpp`; 10 particles spawned at `cannon.GetMuzzlePos()` on each fire; expand + fade over ~0.2–0.55 sec; drawn with `BLEND_ALPHA`.
- **Screen shake** — `shakeTimer` + `shakeMagnitude`; random camera position jitter while timer > 0; 0.12 s / 0.06 mag on fire, 0.25 s / 0.12 mag on hit.
- **Hit flash** — `hitFlashTimer`; white `DrawCylinderEx` disk layered over target for ~6 frames on hit.
- **Polished HUD** — `DrawRectangleRounded` score box (top-left); gold combo text (top-center, fades 2 s, resets on miss); 10 colored block miss icons (bottom-left); "+N" popup now GOLD.
- **Extras** — `Target::Reset()` implemented (was declared but missing); fixed `ChangeColor()` sizeof bug; "Press R to Play Again" restart on game over.

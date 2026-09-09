<p align="center">
  <img src="media/logo-lockup.png" alt="Three Density" width="520">
</p>

Third-person combat action. Fight through enemy pressure, master combos and charged strikes, and survive lava arenas.

## Download

**[Download the Windows launcher](https://github.com/LqnPeraL/threedensity/releases/latest/download/ThreeDensitySetup.exe)**

The launcher installs the game, creates a desktop shortcut, and **checks GitHub for updates on every launch** before starting.

Game page: **[LqnPeraL.github.io/threedensity](https://LqnPeraL.github.io/threedensity/)**

Portable ZIP (optional): [ThreeDensity-Win64.zip](https://github.com/LqnPeraL/threedensity/releases/latest/download/ThreeDensity-Win64.zip)

Every push to `main` publishes a release you can download to test that commit. The game website is served by **GitHub Pages from the repository root** (`index.html` + `media/`).

## In-game

- **Esc / Start** — pause menu with full **controls cookbook** and **graphics settings**
- Onboarding tips appear in the first moments of play
- First launch **auto-detects your GPU/RAM**, runs a short **FPS benchmark**, and applies Low / Medium / High / Epic (you can retune anytime)
- **Mouse wheel** (or `-` / `=`) zooms the camera in and out

## Controls

| Action | Keyboard | Gamepad |
|---|---|---|
| Move | W A S D | Left Stick |
| Look | Mouse | Right Stick |
| Jump | Space | A / Cross |
| Combo attack | Left Mouse | RT / R2 |
| Charged strike | Hold Right Mouse | Hold RB / R1 |
| Camera shoulder | Q | LB / L1 |
| Zoom in / out | Mouse wheel · `-` / `=` | — |
| Menu | Esc | Start |

## System Requirements

| | Minimum | Recommended |
|---|---|---|
| **OS** | Windows 10 64-bit | Windows 11 64-bit |
| **CPU** | Quad-core 2.5 GHz | 6-core 3.0 GHz |
| **RAM** | 8 GB | 16 GB |
| **GPU** | DirectX 12, 4 GB VRAM | DirectX 12, 8 GB VRAM |
| **Storage** | 1 GB | 1 GB SSD |

## Build from Source

Open `threedensity.uproject`, or package with `RunUAT BuildCookRun`.

To attach a playable Win64 build to the auto-release for a commit, put the cooked zip at `release-assets/ThreeDensity-Win64.zip` (see `scripts/package-win64.ps1`) before pushing, or upload it onto the release after CI finishes.

## License

Template content © Epic Games, Inc.

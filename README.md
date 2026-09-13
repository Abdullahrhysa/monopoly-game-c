# MONOPOLY-LK — Automated Monopoly Simulation (C)

A fully automated, Sri Lanka-themed simulation of Monopoly written in C. Four AI-controlled
players — each following a distinct investment strategy — play the game end-to-end with no
human input, across a 40-square board covering real Sri Lankan locations (Pettah to Galle
Face), while a set of interacting economic systems (loans, insurance, disasters, inflation,
government regulation) evolve the game state each round.

## Features

- **40-square board** modeled on real Sri Lankan cities and landmarks, with railways,
  utilities, a bank, an insurance office, tax and event squares.
- **Four AI strategies** — Aggressive, Conservative, Risk-Taker, and Opportunistic — each with
  different rules for buying property, bidding at auction, and developing houses/hotels.
- **Full property lifecycle**: purchase, auctions on decline, rent calculation (scaled by
  houses/hotels, building condition, market conditions, and active event cards), mortgaging,
  and bankruptcy with automatic asset liquidation.
- **Building condition system**: houses and hotels decay over time, require maintenance,
  and can suffer structural damage from prolonged neglect.
- **Banking system**: loans with interest, refinancing, foreclosure, and insurance policies
  against property damage.
- **Dynamic economy**: recurring inflation updates, market booms/declines by property color
  group, regional development cards, random economic events, and periodic government
  regulation changes — all of which feed back into rent and cost calculations.
- **20-card national event deck** and random disaster events that can damage or destroy
  buildings.
- Runs fully automatically for up to 1000 rounds or until only one solvent player remains,
  printing a full narrated log and a per-round financial summary for every player.

## File Structure

| File | Responsibility |
|---|---|
| `main.c` | Entry point — seeds the RNG, initializes players and the board, starts the game loop. |
| `types.h` | Shared `Property`/`Player` structs, enums, constants, and all function declarations. |
| `board.c` | Board setup — initializes all 40 squares with their names, prices, and rent values. |
| `players.c` | Player initialization, strategy-based buy/bid decisions, and building construction logic. |
| `finance.c` | Core financial engine — rent calculation, auctions, bankruptcy, loans, mortgages, insurance, taxes, and the event card deck. |
| `events.c` | Recurring economic events, government regulations, and market condition reporting. |
| `game.c` | Turn order, dice rolls, movement, jail handling, the main round loop, and end-of-game scoring. |

## Build & Run

Compile with GCC:

```bash
gcc -o monopoly main.c board.c events.c finance.c game.c players.c -Wall
./monopoly
```

Compiles cleanly with no warnings under `-Wall`. Tested on Linux (GCC) — no platform-specific
code is used, so it also builds under MinGW/MSVC on Windows.

## Sample Output

```
MONOPOLY-LK Simulation
Player 1 : Aggressive Investor
Player 2 : Conservative Banker
Player 3 : Risk Taker
Player 4 : Opportunistic Trader
Each player begins with LKR 30000.

Conservative Banker rolls 11.
Conservative Banker will begin the game.
Turn order:
Conservative Banker
Risk Taker
Opportunistic Trader
Aggressive Investor

Conservative Banker rolled 5.
Conservative Banker moves from Square 0 to Square 5.
Conservative Banker landed on Colombo Fort Railway Station.
Conservative Banker purchased Colombo Fort Railway Station for LKR 2000.
Remaining Balance : LKR 28000.
```

## Design Notes

- **Strategy-driven AI**: buying, bidding, and building decisions are all routed through a
  `Strategy` enum (`AGGRESSIVE`, `CONSERVATIVE`, `RISK_TAKER`, `OPPORTUNISTIC`), so each
  player behaves consistently differently without any branching in the core game loop.
- **Layered rent calculation**: `calculateRent()` composes multiple independent modifiers —
  building level, condition decay, structural damage, active event cards, market booms/declines,
  regional cards, and inflation — applied in sequence to a base rent value.
- **Shared mutable game state** (inflation, active market events, regulation multipliers) is
  kept as global variables in `game.c`, declared `extern` in `types.h`, since many independent
  subsystems (rent, events, regulations) need to read and update them each round.
- **Self-contained bankruptcy handling**: `checkBankruptcy()` attempts mortgaging before
  forcing `declareBankrupt()`, which then auctions off the player's entire portfolio back to
  the remaining players automatically.

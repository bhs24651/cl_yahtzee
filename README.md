# CL_Yahtzee

**CL_Yahtzee** is a command-line implementation of the classic dice game *Yahtzee*, written in C++ with a retro-style UI for Windows consoles.  
It supports keyboard controls, dice rolling animations, a scorecard, and confirmation prompts to make the gameplay experience smooth and intuitive.

## Features
- **Full Yahtzee gameplay**: Upper and lower sections with correct scoring rules.
- **Interactive dice rolling**: Lock/unlock dice between rolls, up to 3 rolls per turn.
- **Color-coded UI**:
  - Red highlight when no rolls remain.
  - Yellow highlight for finalized totals.
- **Clean navigation**: Return to menus without breaking turn flow.
- **Cross-compatibility**: Works in modern PowerShell, Windows Terminal, and supports fallback for legacy consoles.

## How to Play
Yahtzee is played over 13 rounds. In each round, you:
1. Roll up to five dice.
2. Optionally re-roll any number of dice up to two more times (three rolls total per turn).
3. Choose a category from the scorecard to assign your points.  
   Once a category is chosen, it cannot be used again.

### Scoring
- **Upper Section**: Sum of the chosen face value (1s–6s).  
  Score 63+ points here for a **35-point bonus**.
- **Lower Section**:
  - *Three of a Kind*: Sum of all dice if three match.
  - *Four of a Kind*: Sum of all dice if four match.
  - *Full House*: 25 points for a 3-of-a-kind + a pair.
  - *Small Straight*: 30 points for a run of four.
  - *Large Straight*: 40 points for a run of five.
  - *Yahtzee*: 50 points for five of a kind.
  - *Chance*: Sum of all dice.

The player’s **Grand Total** is the sum of all categories.

## Controls
- **Spacebar**: Roll dice (roll all if first roll, otherwise only unlocked dice).
- **Number keys**: Select dice to lock/unlock, or choose a scorecard category.
- **0 key**: Submit score to the selected category (with confirmation).
- **Escape / Menu options**: Return to menus, restart, or quit.

## Building
This project is written in C++17 and tested with MinGW-w64.

### Build (MinGW, static linking for portability)
```bash
g++ -O2 -static -static-libstdc++ -static-libgcc cl_yahtzee.cpp -o yahtzee.exe -lwinpthread

# MONOPOLY-LK: Deep-Dive Auction Engine Viva & Code Guide (`auction.c`)

This guide explains **every single line, data structure, loop, condition, and AI behavior** in [`auction.c`](file:///c:/Users/ACER/Documents/Projects/C%20Assignment/auction.c). It is designed to prepare you to answer any oral question (viva) from your lecturers or examiners with 100% confidence.

---

## 1. High-Level Architecture & What `auction.c` Does

In **MONOPOLY-LK**, properties enter an auction under three specific game events:
1. **Purchase Decline**: A player lands on an unowned property/railway/utility but refuses or cannot afford to purchase it.
2. **Foreclosure**: A player defaults on a bank loan, and their pledged collateral is seized by the Bank of Ceylon and immediately put up for auction.
3. **Bankruptcy Liquidation**: When a player is eliminated, their assets are auctioned off.

The auction system is implemented in [`auction.c`](file:///c:/Users/ACER/Documents/Projects/C%20Assignment/auction.c) and consists of three key components:
1. **`get_market_value()`**: Extracts the listed purchase price of any board tile.
2. **`decide_bid()`**: The AI decision brain that calculates whether an AI player bids or permanently withdraws.
3. **`run_auction()`**: The round-robin state machine that coordinates turns, manages withdrawals, collects bids, handles early termination, transfers ownership, and deducts money.

```
       [ Trigger: Decline / Foreclosure / Liquidation ]
                            │
                            ▼
              ┌───────────────────────────┐
              │       run_auction()       │
              │  • Check Loan-Lock        │
              │  • Starting Bid = 50%     │
              │  • Init still_in[] Array  │
              └─────────────┬─────────────┘
                            │
                            ▼
     ┌──────────────────────────────────────────────┐
     │           Round-Robin Bidding Loop           │ ◄───┐
     │                                              │     │
     │  For each active player (Turn 1 to 4):       │     │
     │    Call decide_bid()                         │     │
     │      ├─ AI Bids  ──> update current_bid      │     │ Next
     │      │               winning_player_index    │     │ Round
     │      └─ AI Drops ──> still_in[i] = 0         │     │
     │                      active_count--          │     │
     │                                              │     │
     │  Are there >= 2 active bidders? ─────────────┼─────┘
     └──────────────────────┬───────────────────────┘
                            │ (active_count <= 1)
                            ▼
              ┌───────────────────────────┐
              │       auction_done:       │
              │  • Deduct Winner's Cash   │
              │  • Update Board & Union   │
              │  • Print Victory/Bank Log │
              └───────────────────────────┘
```

---

## 2. Global Includes & Forward Declarations (Lines 1–8)

```c
1: #include "types.h"
2: #include <stdio.h>
3: 
4: // Forward declarations
5: int find_player_index(GameState *game, int player_id);
6: void purchase_property(GameState *game, int player_index, int square_index);
7: void purchase_railway(GameState *game, int player_index, int square_index);
8: void purchase_utility(GameState *game, int player_index, int square_index);
```

### Explanation
* **Line 1 (`#include "types.h"`)**: Imports all custom structures (`GameState`, `Square`, `Player`, `SquareType`, `PropertyGroup`). Without this, the compiler will not know what `GameState` or `SQUARE_PROPERTY` means.
* **Line 2 (`#include <stdio.h>`)**: Imports standard input/output library functions like `printf`.
* **Lines 4–8 (Forward declarations)**: Tells the compiler that these functions exist in other `.c` files (`helper_functions.c`, `finance.c`) so we can call them without compiler warnings before the linker links them together.

---

## 3. Function 1: `get_market_value()` (Lines 15–24)

```c
15: double get_market_value(GameState *game, int square_index) {
16:   if (game->board[square_index].type == SQUARE_PROPERTY) {
17:     return game->board[square_index].data.property.price;
18:   } else if (game->board[square_index].type == SQUARE_RAILWAY) {
19:     return game->board[square_index].data.railway.price;
20:   } else if (game->board[square_index].type == SQUARE_UTILITY) {
21:     return game->board[square_index].data.utility.price;
22:   }
23:   return 0;
24: }
```

### Step-by-Step Breakdown
* **Parameters**:
  * `GameState *game`: A pointer to the shared game memory structure.
  * `int square_index`: The board tile index (from `0` to `39`).
* **Why this function is needed**:
  In `types.h`, the `Square` struct uses a **C Union** (`union { PropertyData property; RailwayData railway; UtilityData utility; } data;`).
  Because properties, railways, and utilities store their `price` inside different struct members inside the union, we need an accessor helper that checks the `SquareType` enum and extracts the correct `price`.
* **Lines 16–17**: If the square is a standard color property (e.g. *Galle Face*, *Pettah*), return `data.property.price`.
* **Lines 18–19**: If it is a railway station (e.g. *Colombo Fort*), return `data.railway.price`.
* **Lines 20–21**: If it is a utility (e.g. *CEB* or *Water Board*), return `data.utility.price`.
* **Line 23 (`return 0`)**: Fallback for non-purchasable tiles (e.g. *GO*, *Income Tax*, *Jail*, *Chance*).

---

## 4. Function 2: `decide_bid()` (Lines 33–92)

This function represents the **AI cognitive strategy** for each player during each step of an auction.

```c
33: double decide_bid(GameState *game, int player_index, int square_index,
34:                   double current_bid, double market_value) {
35: 
36:   double new_bid = current_bid + 250;
37:   int player_id = game->players[player_index].id;
38:   double cash = game->players[player_index].money;
39: 
40:   // Cannot bid more than cash on hand (Rule-LK 22)
41:   if (new_bid > cash) return 0;
```

### Line-by-Line Mechanics
* **Line 36 (`double new_bid = current_bid + 250;`)**:
  * **Rule-LK 20 Compliance**: The minimum bid increment is fixed at **LKR 250**. Every new bid must be at least `current_bid + 250`.
* **Line 37 & 38**: Extracts the `player_id` (1 to 4) and the current player's liquid `cash`.
* **Line 41 (`if (new_bid > cash) return 0;`)**:
  * **Rule-LK 22 Compliance**: A player is strictly prohibited from bidding more cash than they currently hold. Players cannot take bank loans during auctions. If the minimum required bid exceeds their wallet, they must immediately return `0` (withdraw).

---

### The 4 AI Player Personalities (Lines 43–90)

Each player implements the exact bidding strategy mandated in the assignment specifications:

#### 1. Player 1 — Aggressive Investor (`player_id == 1`)
```c
47:   if (player_id == 1) {
48:     double max_bid = market_value * 1.20;
49:     if (new_bid <= max_bid && new_bid <= cash) {
50:       return new_bid;
51:     }
52:     return 0;
53:   }
```
* **Strategy**: Will bid aggressively up to **120% of the property's market value** (`market_value * 1.20`).
* **Condition**: If `new_bid <= max_bid` and he has the cash, he bids `new_bid`. Otherwise, returns `0` (withdraws).

#### 2. Player 2 — Conservative Banker (`player_id == 2`)
```c
60:   else if (player_id == 2) {
61:     if (new_bid <= market_value && new_bid <= cash * 0.5) {
62:       return new_bid;
63:     }
64:     return 0;
65:   }
```
* **Strategy**: Bargain hunter. Refuses to pay even 1 Rupee over the market value (`new_bid <= market_value`).
* **Safety Reserve**: Also enforces the Conservative Banker's core rule: never spend more than 50% of available cash on an asset (`new_bid <= cash * 0.5`).

#### 3. Player 3 — Risk Taker (`player_id == 3`)
```c
71:   else if (player_id == 3) {
72:     if (new_bid <= cash) {
73:       return new_bid;
74:     }
75:     return 0;
76:   }
```
* **Strategy**: "All In". Ignores market value completely. Continues bidding round after round until his cash is completely exhausted (`new_bid <= cash`).

#### 4. Player 4 — Opportunistic Trader (`player_id == 4`)
```c
83:   else if (player_id == 4) {
84:     double max_bid = market_value * 0.90;
85:     if (new_bid <= max_bid && (cash - new_bid) >= 500) {
86:       return new_bid;
87:     }
88:     return 0;
89:   }
```
* **Strategy**: Prefers heavily discounted acquisitions. Only bids if the auction price is at least **10% below market value** (max bid ceiling = `market_value * 0.90`).
* **Cash Buffer**: Must retain a minimum reserve of **LKR 500** after purchase to pay future rents (`cash - new_bid >= 500`).

---

## 5. Function 3: `run_auction()` (Lines 101–254)

This is the main state machine that runs the entire auction from start to finish.

### Phase 1: Prerequisite & Validation Guards (Lines 103–128)

```c
101: void run_auction(GameState *game, int square_index, int triggering_player_id) {
102: 
103:   // Loan-locked properties cannot be auctioned (Rule-LK 3)
104:   if (game->board[square_index].type == SQUARE_PROPERTY &&
105:       game->board[square_index].data.property.is_loan_locked) {
106:     return;
107:   }
108:   if (game->board[square_index].type == SQUARE_RAILWAY &&
109:       game->board[square_index].data.railway.is_loan_locked) {
110:     return;
111:   }
112:   if (game->board[square_index].type == SQUARE_UTILITY &&
113:       game->board[square_index].data.utility.is_loan_locked) {
114:     return;
115:   }
116: 
117:   // Must be unowned to auction
118:   if (game->board[square_index].owner_id != -1) return;
119: 
120:   double market_value = get_market_value(game, square_index);
121:   if (market_value <= 0) return;
122: 
123:   // Starting bid = 50% of market value (Rule-LK 19)
124:   double current_bid = market_value * 0.50;
```

#### Explanation
* **Lines 103–115 (Rule-LK 3 Guard)**: Assets pledged as collateral for active bank loans are **Loan Locked**. The bank holds the deed in escrow. They are legally forbidden from being auctioned until foreclosure occurs.
* **Line 118**: Ensures we only auction unowned properties (`owner_id == -1`).
* **Lines 123–124 (Rule-LK 19)**: Sets the initial opening bid at exactly **50% of listed market value** (`market_value * 0.50`).

---

### Phase 2: Bidder Tracking Initialization (Lines 130–152)

```c
130:   // Initialize bidder tracking
131:   int still_in[MAX_PLAYERS];
132:   int active_count = 0;
133: 
134:   for (int i = 0; i < game->num_players; i++) {
135:     if (game->players[i].is_bankrupt) {
136:       still_in[i] = 0;
137:     } else {
138:       still_in[i] = 1;
139:       active_count++;
140:     }
141:   }
142: 
143:   // Need at least 1 bidder
144:   if (active_count == 0) {
145:     printf("No eligible bidders — %s remains with the Bank\n",
146:            game->board[square_index].name);
147:     printf("========== AUCTION END ==========\n\n");
148:     return;
149:   }
150: 
151:   int winning_player_index = -1;
152:   int rounds_with_no_new_bid = 0;
```

#### Key Variables & Why They Exist
* `int still_in[MAX_PLAYERS]`: A boolean flag array. `still_in[i] = 1` means player `i` is currently active. `still_in[i] = 0` means player `i` has **permanently withdrawn**.
* `int active_count`: Keeps track of how many solvent players are still participating in the auction.
* `int winning_player_index = -1`: Holds the index of the highest bidder. Defaults to `-1` (meaning no bids placed yet).
* `int rounds_with_no_new_bid = 0`: A safety watchdog counter to prevent infinite loops.

---

### Phase 3: The Round-Robin Bidding Loop (Lines 155–220)

```c
155:   while (active_count > 0) {
156:     int any_bid_this_round = 0;
157: 
158:     for (int i = 0; i < game->num_players; i++) {
159:       if (!still_in[i]) continue;
```
* The `while (active_count > 0)` loop continues iterating rounds until bidders drop out.
* The inner `for` loop cycles through each player `i = 0, 1, 2, 3` in turn order.
* `if (!still_in[i]) continue;`: If this player previously withdrew, skip them immediately.

---

#### 1. Early Termination Check (Lines 162–164)
```c
162:       if (active_count == 1 && winning_player_index == i) {
163:         goto auction_done;
164:       }
```
* If there is only 1 active bidder left (`active_count == 1`) AND that player is already the highest bidder (`winning_player_index == i`), the auction is over! We jump straight to `auction_done`.

---

#### 2. Handling a Withdrawal (`bid <= 0`) (Lines 166–203)
```c
166:       double bid = decide_bid(game, i, square_index, current_bid, market_value);
167: 
168:       if (bid <= 0) {
169:         // Player withdraws permanently
170:         printf("  %s withdraws from the auction\n", game->players[i].name);
171:         still_in[i] = 0;
172:         active_count--;
```
* If the AI decides not to bid (returns `0`), we log the withdrawal.
* `still_in[i] = 0; active_count--;`: **Permanent Withdrawal (Rule-LK 21)**. The player is marked out and cannot re-enter this auction.

##### What if only 1 player remains after this withdrawal? (Lines 174–203)
```c
174:         if (active_count <= 1) {
175:           // Check if the remaining bidder already has a winning bid
176:           if (winning_player_index != -1) {
177:             goto auction_done;
178:           }
179:           // If no one has bid yet and only 1 remains, give them a chance
180:           if (active_count == 1) {
181:             for (int j = 0; j < game->num_players; j++) {
182:               if (still_in[j]) {
183:                 double last_bid = decide_bid(game, j, square_index,
184:                                              current_bid, market_value);
185:                 if (last_bid > 0) {
186:                   current_bid = last_bid;
187:                   winning_player_index = j;
188:                   printf("  %s bids LKR %.0lf\n", game->players[j].name,
189:                          current_bid);
190:                 } else {
191:                   printf("  %s withdraws from the auction\n",
192:                          game->players[j].name);
193:                   still_in[j] = 0;
194:                   active_count--;
195:                 }
196:                 goto auction_done;
197:               }
198:             }
199:           }
200:           goto auction_done;
201:         }
```
* **Line 176**: If a previous bid already exists (`winning_player_index != -1`), the remaining player has already won! Jump to `auction_done`.
* **Lines 180–198**: If all other players withdrew immediately on Round 1 and **nobody has placed a bid yet**, the single remaining player `j` gets one final chance to place an opening bid at 50% + 250. If they bid, they win. If they withdraw too, the property stays with the Bank.

---

#### 3. Handling a Valid Bid (Lines 204–211)
```c
204:       } else {
205:         // Player places a bid
206:         current_bid = bid;
207:         winning_player_index = i;
208:         any_bid_this_round = 1;
209:         printf("  %s bids LKR %.0lf\n", game->players[i].name, current_bid);
210:       }
```
* Updates the high-water mark: `current_bid` is updated to the new bid amount.
* Sets `winning_player_index = i` so player `i` is the current leader.
* Sets `any_bid_this_round = 1` to signal to the watchdog that the auction is making progress.

---

#### 4. Watchdog Loop Breaker (Lines 213–220)
```c
213:     // Safety: if a full rotation passed with no new bids, stop
214:     if (!any_bid_this_round) {
215:       rounds_with_no_new_bid++;
216:       if (rounds_with_no_new_bid >= 2) break;
217:     } else {
218:       rounds_with_no_new_bid = 0;
219:     }
220:   }
```
* If a full round passes where every remaining player passes without placing a bid, `rounds_with_no_new_bid` increments. If this happens for 2 full cycles, the loop safely breaks.

---

### Phase 4: Ownership Transfer & Settlement (Lines 222–254)

```c
222: auction_done:
223: 
224:   // Determine winner and transfer ownership
225:   if (winning_player_index != -1) {
226:     // Deduct the winning bid amount
227:     game->players[winning_player_index].money -= current_bid;
228: 
229:     // Transfer ownership
230:     game->board[square_index].owner_id =
231:         game->players[winning_player_index].id;
232: 
233:     if (game->board[square_index].type == SQUARE_PROPERTY) {
234:       game->board[square_index].data.property.owner_id =
235:           game->players[winning_player_index].id;
236:     } else if (game->board[square_index].type == SQUARE_RAILWAY) {
237:       game->board[square_index].data.railway.owner_id =
238:           game->players[winning_player_index].id;
239:     } else if (game->board[square_index].type == SQUARE_UTILITY) {
240:       game->board[square_index].data.utility.owner_id =
241:           game->players[winning_player_index].id;
242:     }
243: 
244:     printf("  >>> %s WINS the auction for %s at LKR %.0lf (saved LKR %.0lf)\n",
245:            game->players[winning_player_index].name,
246:            game->board[square_index].name, current_bid,
247:            market_value - current_bid);
248:   } else {
249:     printf("  No bids placed — %s remains with the Bank\n",
250:            game->board[square_index].name);
251:   }
252: 
253:   printf("========== AUCTION END ==========\n\n");
```

#### Step-by-Step Resolution
1. **Deduct Winning Bid (Line 227)**: `game->players[winning_player_index].money -= current_bid;`. Deducts the final bid price from the winner's cash balance.
2. **Synchronize Dual Ownership**:
   * **Outer Level (Line 230)**: `game->board[square_index].owner_id = player_id;` (used by quick board checkers).
   * **Inner Union Level (Lines 233–242)**: Sets `owner_id` inside `data.property.owner_id`, `data.railway.owner_id`, or `data.utility.owner_id`. This prevents desynchronization bugs when other systems check specific property structs.
3. **Log Winner & Discount (Lines 244–247)**: Prints the winning player, final sale price, and the discount achieved (`market_value - current_bid`).
4. **No-Bid Case (Lines 248–251)**: If `winning_player_index == -1` (everyone declined), the property remains unowned (`owner_id` is already `-1`), so ownership stays with the Bank of Ceylon.

---

## 6. Comprehensive Viva Q&A (Frequently Asked Questions)

### Q1: What data structures did you use in `auction.c` and why?
> **Answer**: 
> "We used `GameState*` to access the global state of the game. To track bidders dynamically without complex dynamic memory allocation, we used a local fixed-size array `int still_in[MAX_PLAYERS]` and an integer counter `active_count`. Since `MAX_PLAYERS` is constant (4), a stack-allocated array is $O(1)$ in space and extremely fast."

### Q2: Why is `goto auction_done;` used, and isn't `goto` considered bad practice in C?
> **Answer**: 
> "In structured C programming, `goto` is standard and idiomatic when exiting deeply nested loops to a single cleanup/resolution point (often called *error handling/single-exit pattern*). In our auction loop, we have an outer `while` loop and an inner `for` loop. A simple `break` only exits the inner `for` loop. Using `goto auction_done` allows clean, immediate jumping to the resolution block when an auction concludes early without needing multiple boolean flags."

### Q3: How do you enforce the rule that players cannot take loans during auctions?
> **Answer**: 
> "Rule-LK 22 states that players cannot obtain bank loans during auctions. We enforce this in `decide_bid()` with the guard `if (new_bid > cash) return 0;`. We check only the player's liquid `money` field, and never invoke `obtain_loan()` or bank visit routines from within the auction engine."

### Q4: Why do properties with `is_loan_locked == 1` get skipped?
> **Answer**: 
> "Under Rule-LK 3, when a player pledges a property as loan collateral to the Bank of Ceylon, that property is locked in escrow. It cannot be traded, mortgaged, or auctioned. Only if the player defaults does foreclosure occur, at which point the lock is stripped and the asset is auctioned."

### Q5: How do the 4 AI player strategies differ in math?
> **Answer**: 
> * **Player 1 (Aggressive)**: $\text{Max Bid} = \text{Market Value} \times 1.20$
> * **Player 2 (Conservative)**: $\text{Max Bid} = \min(\text{Market Value}, \text{Cash} \times 0.50)$
> * **Player 3 (Risk Taker)**: $\text{Max Bid} = \text{Cash}$ (No price ceiling)
> * **Player 4 (Opportunistic)**: $\text{Max Bid} = \min(\text{Market Value} \times 0.90, \text{Cash} - 500)$

### Q6: What happens if all players immediately withdraw without placing a single bid?
> **Answer**: 
> "If every player returns `0` from `decide_bid()`, `winning_player_index` remains `-1`. The `if (winning_player_index != -1)` condition evaluates to false, jumping to the `else` branch which prints *'No bids placed — property remains with the Bank'*, leaving `owner_id = -1`."

---

## 7. Summary Table of Rules Mapped to Code

| Game Rule | Specification Requirement | Code Implementation in `auction.c` |
| :--- | :--- | :--- |
| **Rule-LK 19** | Starting bid is 50% of market value | `double current_bid = market_value * 0.50;` (Line 124) |
| **Rule-LK 20** | Minimum bid increment is LKR 250 | `double new_bid = current_bid + 250;` (Line 36) |
| **Rule-LK 21** | Withdrawal is permanent | `still_in[i] = 0; active_count--;` (Line 171) |
| **Rule-LK 22** | Cannot bid more than available cash | `if (new_bid > cash) return 0;` (Line 41) |
| **Rule-LK 3** | Loan-locked assets cannot be auctioned | `if (data.property.is_loan_locked) return;` (Lines 103–115) |
| **Bankrupt Rule** | Bankrupt players excluded | `if (game->players[i].is_bankrupt) still_in[i] = 0;` (Line 136) |

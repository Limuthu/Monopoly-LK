# MONOPOLY-LK: Dynamic Property Market Viva Guide

This document is a comprehensive guide to understanding exactly how the **Dynamic Property Market** was implemented in the codebase. It details the "Getter Architecture" we used, the logic introduced in the new files, and how it translates to the specific rules of the assignment.

Use this guide to prepare for your viva by understanding "how" and "why" everything works!

---

## 1. The Core Problem: Temporary vs. Permanent

The rules state that the Dynamic Property Market causes a "Market Boom" and "Market Decline" that last for **exactly 10 rounds**. 

When we built the *Inflation* system, we permanently modified the values inside the `Square` struct (e.g., `game->board[i].data.property.price *= 1.15`). We could do this because Inflation permanently alters the economy.

However, if we used that same approach for a temporary Market Boom, we would have to remember to "undo" the math 10 rounds later (e.g., divide the price by 1.15). If an Inflation cycle happened *while* a Market Boom was active, the math to undo the boom would become incredibly complicated and likely result in bugs. 

**The Solution:** We used a **"Getter Architecture"**. Instead of changing the database (the structs), we created helper functions that calculate the buffs and debuffs *on the fly* whenever the game asks for a value. Once the 10-round boom expires, the game naturally goes back to reading the normal baseline values.

---

## 2. File-by-File Code Breakdown

Here is exactly what was changed across the codebase to make the market work safely.

### `types.h` (State & Data Management)
We introduced global variables in the `GameState` to track the current market conditions. 
```c
// Dynamic Property Market
PropertyGroup market_boom_group;
PropertyGroup market_decline_group;
int market_rounds_left;
int group_cooldowns[9]; // Track 30-round cooldowns (index 1 to 8 map to groups)
```
*Note: Because there are 8 property colors (Groups 1 through 8), we made the `group_cooldowns` array size 9 so we can directly map `group_cooldowns[GROUP_BROWN]` without off-by-one errors.*

### `market.c` (The Engine & The Getters)
This new file contains all the logic.

**1. The Market Update Logic:**
The `update_property_market(GameState *game)` function handles the 10-round cycle:
*   It loops through the `group_cooldowns` array and subtracts 10 from any active cooldowns.
*   It compiles a list of "eligible groups" (any group where `cooldown <= 0`).
*   It randomly selects one group to Boom and a distinct second group to Decline.
*   It immediately slaps a 30-round cooldown on both selected groups to enforce Rule-LK 33.

**2. The Getter Functions:**
This is where the magic happens. We created six getter functions:
*   `get_dynamic_price()`
*   `get_dynamic_rent()`
*   `get_dynamic_mortgage()`
*   `get_dynamic_build_cost()`
*   `get_dynamic_property_value()`
*   `get_dynamic_auction_start()`

Every single one of these functions follows the exact same safe pattern. For example:
```c
double get_dynamic_rent(GameState *game, int square_index) {
  // 1. Read the base value
  double base = game->board[square_index].data.property.base_rent;
  PropertyGroup group = game->board[square_index].data.property.group;

  // 2. Apply on-the-fly math if the group is active
  if (group == game->market_boom_group) {
    return base * 1.25; // Boom: +25% Rental Income
  } else if (group == game->market_decline_group) {
    return base * 0.80; // Decline: -20% Rental Income
  }
  
  // 3. Return normal base value if no market event
  return base; 
}
```

### `finance.c`, `loans.c`, `auction.c` (The Refactor)
To make the getters work, we had to act like a copy editor and intercept the code anytime it tried to read a property's value. 

For example, in `finance.c`, when a player lands on a property and pays rent, we changed:
```diff
-  double base_rent = game->board[square_index].data.property.base_rent;
+  double base_rent = get_dynamic_rent(game, square_index);
```
We did this across all the financial files. By making this one-line change, the entire codebase (including the building multiplier logic and the auction logic) automatically inherited the complex dynamic market buffs without having to be rewritten!

### `building.c` (AI Strategy)
The specification explicitly mentioned that the **Opportunistic Trader** AI evaluates market conditions and *"will intentionally sell properties expected to decrease in value following economic events."*

To satisfy this rule, we injected a special check at the very top of `player_build_decision()` just for Player 4. 

```c
// Inside player_build_decision() for Player 4:
for (int i = 0; i < TOTAL_SQUARES; i++) {
  if (game->board[i].owner_id == player_id && game->board[i].type == SQUARE_PROPERTY) {
    // If we own a property that just entered a Market Decline...
    if (game->board[i].data.property.group == game->market_decline_group) {
      
      // Sell it back to the bank for half price immediately!
      double sell_price = game->board[i].data.property.price * 0.5;
      game->players[player_index].money += sell_price;
      
      // Reset ownership
      game->board[i].owner_id = -1;
      
      printf("[Opportunistic Trader] Sold %s back to the bank for LKR %.0lf to avoid Market Decline.\n", 
             game->board[i].name, sell_price);
    }
  }
}
```

### `main.c` (The Loop & UI)
Finally, we hooked everything up in the main game loop:
1.  **Trigger:** `update_property_market(&game)` is called every 10 rounds.
2.  **UI Requirement:** Rule-LK 36 requires the simulation to print the active market conditions at the end of every round. We added a check at the end of the `while` loop that decrements `game.market_rounds_left` and prints exactly what is booming and declining to the console.

---

## 3. How to Talk About This in Your Viva

If the examiner asks:

**"How did you implement the temporary buffs for the Dynamic Property Market without breaking the baseline values?"**
> "I used what I call a 'Getter Architecture'. Instead of permanently mutating the actual `price` or `rent` fields in the `Square` struct like I did for Inflation, I created helper functions in `market.c` like `get_dynamic_rent()`. These functions read the base value, check if the property's color group is currently in a Boom or Decline, and return the `+25%` or `-20%` value on the fly. This guarantees that when the 10-round cycle ends, the game safely reverts to the baseline numbers without doing any complex 'undo' math."

**"How did you handle the 30-round cooldown requirement?"**
> "I added an integer array called `group_cooldowns[9]` to the `GameState`. Every time the 10-round market update triggers, it loops through that array and subtracts 10 from any active cooldowns. It then creates a list of 'eligible groups'—which are any groups with a cooldown of zero or less—and randomly picks the boom and decline from that list. Once picked, those two groups get their index in the array set to 30."

**"How does the Opportunistic Trader AI interact with this system?"**
> "At the start of their turn in `building.c`, the Opportunistic Trader scans the board for any properties they own. If they find a property that belongs to the group currently stored in `game->market_decline_group`, they immediately trigger a fire-sale, selling the property back to the bank for 50% of its base price to avoid taking the massive rent and value losses."

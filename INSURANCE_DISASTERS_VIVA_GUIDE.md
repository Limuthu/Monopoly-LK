# MONOPOLY-LK: Insurance & Disasters Viva Guide

This document is a comprehensive guide to understanding exactly how the **Insurance** and **Disasters** systems were implemented in the codebase. It details the logic introduced in the new files, how they integrate into the main game loop, and how it translates to the specific rules of the assignment.

Use this guide to prepare for your viva by understanding "how" and "why" everything works!

---

## 1. The Core Architecture

The rules state that Disasters strike every 10 rounds, hitting exactly one developed property, forcing players to either pay repair costs or rely on insurance policies (which they buy when landing on squares 17 or 33). 

We needed a system that could:
1. Track the insurance tier and expiration of every property independently.
2. Trigger a global event every 10 rounds.
3. Automatically intercept rent collection if a property is left damaged.
4. Auto-repair a property as soon as a broke player gets enough cash.

To do this, we expanded the `PropertyData` struct to act as a state machine for damage and insurance, and we created a new file, `events.c`, to act as the "Insurance Agency and Act of God" manager.

---

## 2. File-by-File Code Breakdown

### `types.h` (State & Data Management)
First, we introduced the `DisasterType` enum and added four new fields to the `PropertyData` struct to track state:
```c
// Added to PropertyData:
int insurance_tier;          // 0 = None, 1 = Basic, 2 = Comprehensive, 3 = Bus. Interruption
int insurance_rounds_left;   // Starts at 20 when a policy is purchased
int is_damaged;              // Flag: 1 if disaster struck and hasn't been repaired
double pending_repair_cost;  // Stores the cost needed to fix the disaster
```
By placing these inside `PropertyData`, every single property on the board independently remembers if it has insurance and if it is currently damaged. (We initialized these to `0` in `board.c`).

### `events.c` (The Core Logic Engine)
This brand new file houses all the mechanics.

**1. Landing on Insurance (`handle_insurance_landing`)**
When a player lands on Square 17 or 33, this function is called. It iterates through every property the player owns. If a property is developed (has a house/hotel) and uninsured, the player decides whether to buy a policy. 
This is where the AI logic lives:
*   **Aggressive Investor:** Buys Tier 2 for hotels, Tier 1 for houses.
*   **Conservative Banker:** Buys Tier 2 for *everything*.
*   **Risk Taker:** Refuses to buy insurance unless the property is already damaged (meaning they wait until they suffer a loss!).
*   **Opportunistic Trader:** Evaluates the `get_dynamic_price()`. If the property is worth over 5000, they buy Tier 2. Otherwise, they skip it.

**2. The Disaster Trigger (`trigger_disaster`)**
Every 10 rounds, this function fires:
1. It loops through the entire board and collects all "developed" properties (properties with `num_houses > 0` or `has_hotel`) into an array.
2. It randomly picks one property from that array.
3. It randomly generates a Disaster (Fire, Flood, Riot, etc.).
4. It calculates the repair cost (`50% of the property's total built value`).
5. **Coverage Check:** It checks the property's `insurance_tier`. If it's a Fire/Flood, Basic pays 80%. If it's Comprehensive, it pays 100%. If it's Business Interruption, it pays 100% plus 5 rounds of lost rent!
6. **Damage Check:** If the player has enough cash (either from their wallet or the insurance payout), they pay the repair cost instantly. If they are completely broke, the property is marked as `is_damaged = 1` and the repair cost is stored in `pending_repair_cost`.

**3. Policy Expiration (`update_insurance_durations`)**
At the end of every round, this function loops through the board and decrements `insurance_rounds_left`. If the counter hits 3, it prints a warning to the console. If it hits 0, it strips the insurance from the property.

**4. The Auto-Repair Hook (`attempt_disaster_repairs`)**
Because the spec says "Repairs happen automatically as soon as the owner has sufficient funds," we built a function that checks if a player has any `is_damaged` properties. If they do, and their cash is now greater than the `pending_repair_cost`, it automatically deducts the money and clears the damage flag.

### `finance.c` (The Punishment)
To enforce Rule-LK 11 (damaged buildings cannot collect rent), we added a simple intercept in `pay_property_rent()`:
```c
if (game->board[square_index].data.property.is_damaged == 1) {
  printf("No rent, property is damaged from a disaster and needs repairs!\n");
  return;
}
```
If a player lands on a damaged property, this immediately aborts the rent collection.

### `main.c` (The Loop Integration)
Finally, we hooked everything up in the main game loop:
1.  **Start of Turn:** `attempt_disaster_repairs(&game, player_id);` runs before the dice are rolled so broke players can instantly repair if they gained money.
2.  **Landing Hook:** Inside `handle_landing`, if the square type is `SQUARE_INSURANCE`, we call `handle_insurance_landing()`.
3.  **End of Round:** `trigger_disaster()` is called if `(round + 1) % 10 == 0`.
4.  **End of Round:** `update_insurance_durations()` is called every round to tick down the policy timers.

---

## 3. How to Talk About This in Your Viva

If the examiner asks:

**"How did you handle the requirement that repairs happen automatically as soon as a player gets sufficient funds?"**
> "I added a state-tracking flag to the `PropertyData` struct called `is_damaged` and `pending_repair_cost`. If a disaster hits and the player is broke, `is_damaged` is set to 1. Then, I added a hook called `attempt_disaster_repairs()` that runs at the very beginning of every player's turn in `main.c`. Before they even roll the dice, it checks if they have any damaged properties and automatically pays off the `pending_repair_cost` if their current cash balance can afford it."

**"How did you prevent a damaged building from collecting rent?"**
> "I updated `finance.c`. Inside the `pay_property_rent()` function, I added a guard clause right at the top. If `game->board[square_index].data.property.is_damaged == 1`, it prints a message and immediately `return`s out of the function, bypassing the rent deduction completely."

**"How did you map 'Earthquake' to 'Building Collapse' for the Comprehensive Insurance coverage?"**
> "In `types.h`, I created a `DisasterType` enum that defines `DISASTER_BUILDING_COLLAPSE` and `DISASTER_ELECTRICAL_FAILURE`. When the disaster triggers in `events.c`, if the player has Comprehensive Insurance (Tier 2), the code treats Building Collapse exactly the same as an Earthquake, automatically applying the 100% payout without needing duplicate enums."

**"Is there any connection between Disasters and Maintenance?"**
> "Yes, I built a deep integration between the two systems! If a building is completely destroyed by a disaster, it doesn't make sense for it to continue slowly decaying in the background. In `depreciation.c`, I added a guard clause that completely skips the `building_condition` decay for any property where `is_damaged == 1`. Furthermore, when a player finally pays the massive disaster repair cost in `events.c`, the repair acts as a 'Forced Renovation'—it completely resets the `building_condition` back to 100.0 and clears any existing Structural Damage penalties. This proves that the Disasters and Maintenance systems logically interact with each other in the game state!"

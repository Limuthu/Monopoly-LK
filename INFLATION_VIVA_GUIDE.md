# MONOPOLY-LK: Inflation & Economy Viva Guide

This document is a comprehensive guide to understanding exactly how the **Inflation System** was implemented in the codebase. It details all the files changed, the logic introduced, and how it maps back to the assignment specification.

Use this guide to prepare for your viva by understanding "how" and "why" everything works!

---

## 1. Core Mechanics & The Compounding Formula

Unlike traditional Monopoly, the Sri Lankan economy in this simulation is dynamic. Every 10 rounds, an inflation rate is generated.

> [!NOTE]
> **The Compounding Formula (Rule-LK 14)**
> Every time the inflation cycle triggers, we recalculate the base values using:
> `New Value = Previous Value * (1 + Inflation Rate)`
> 
> Because this modifies the *base value* each time, inflation (or deflation) compounds over the duration of the 300-round simulation.

The following values are affected by this formula:
*   **Property Prices** (cost to buy)
*   **Building Costs** (house & hotel costs)
*   **Rental Values** (property base rent, and railway/utility rents)
*   **Mortgage Values** (the collateral players can borrow against)

---

## 2. File-by-File Code Breakdown

Here is exactly what was changed across the codebase to make the economy work.

### `types.h` (State & Data Management)
We needed to introduce global variables to track the current economic state, as well as ensure that *fixed* rents (like railways and utilities) could become *dynamic* based on inflation.

1.  **Global Economy State:**
    ```c
    // Added to GameState struct
    double current_inflation_rate;
    double current_interest_rate;
    double railway_rent_base[4]; // Inflating array for railway rents
    double utility_rent_base[2]; // Inflating array for utility rent multipliers
    ```
2.  **Loan Interest Freeze:**
    According to Rule-LK 13, existing active loan rates remain *unchanged* even if a new inflation spike occurs. To implement this, we added a field to track the specific rate at the time a player takes out a loan.
    ```c
    // Added to Player struct
    double loan_interest_rate; 
    ```

### `inflation.c` (The Engine)
This is a brand new file created specifically to handle the logic. The primary function is `handle_inflation(GameState *game)`.

*   **Generating the Rate:** It randomly selects from the predefined pool: `-3%`, `0%`, `2%`, `5%`, `8%`, `12%`.
*   **Updating Interest Rates:** It maps the selected inflation rate to the baseline loan interest rate:
    *   $8\%$ or $12\%$ Inflation $\rightarrow$ **12% Interest Rate**
    *   $2\%$ or $5\%$ Inflation $\rightarrow$ **10% Interest Rate**
    *   $0\%$ or $-3\%$ Inflation $\rightarrow$ **8% Interest Rate**
*   **Applying the Multiplier:** It loops through all 40 squares and directly modifies their data structs (`price`, `base_rent`, `house_cost`, etc.) by multiplying them by `(1.0 + current_inflation_rate)`. It also inflates the global arrays `railway_rent_base` and `utility_rent_base`.

### `main.c` (The Loop)
The inflation engine must be hooked into the game loop.
```c
// Inside the main while (j < 300) loop
// Step 7: Inflation Cycle (every 10 rounds)
if ((j + 1) % 10 == 0) {
  handle_inflation(&game);
}
```
This ensures that at the very end of Rounds 10, 20, 30, etc., the entire economy shifts and players are notified via the console.

### `finance.c` (Dynamic Rents)
Previously, if a player landed on a Railway they paid a hardcoded `250`, `500`, `1000`, or `2000` LKR depending on ownership. Because these numbers are hardcoded, they couldn't inflate.

We changed these functions to use the new dynamic arrays in `GameState` that get compounded by `inflation.c`:
```c
// In pay_railway_rent()
double rent = 0;
if (owned > 0 && owned <= 4) {
  rent = game->railway_rent_base[owned - 1]; // Pulls the inflated value!
}

// In pay_utility_rent()
double rent = 0;
if (owned > 0 && owned <= 2) {
  rent = game->utility_rent_base[owned - 1] * dice_roll; // Pulls the inflated multiplier!
}
```
*Note: Because rent is now dynamically compounded, the variables were changed from `int` to `double` and the print statements were updated to `%.0lf` to handle decimal outputs cleanly.*

### `loans.c` (Locking the Interest Rate)
To enforce the "Crucial Exception" where existing loans are protected from inflation spikes:
1.  When a loan is taken in `obtain_loan()`, we capture the current global interest rate and lock it to the player:
    ```c
    game->players[idx].loan_interest_rate = game->current_interest_rate;
    ```
2.  When applying compound interest at the end of every round in `apply_interest_all()`, we use the player's personal locked rate instead of a global constant:
    ```c
    double interest = old_balance * game->players[i].loan_interest_rate;
    ```

### `building.c` (AI Strategy)
The **Opportunistic Trader** AI (Player 4) is designed to monitor the economy. Their specific instruction is to *"Delay construction during inflation."*

We added a check right at the top of their logic inside `player_build_decision()`:
```c
else if (player_id == 4) {
  if (game->current_inflation_rate > 0.0) {
    printf("[Opportunistic Trader] Delaying construction due to inflation (%.0lf%%)\n", 
           game->current_inflation_rate * 100);
    return; // Exits the function, skipping building entirely this turn
  }
  // ... existing build logic ...
```

---

## 3. How to Talk About This in Your Viva

If the examiner asks:

**"How did you implement the compounding formula for inflation?"**
> "I created a dedicated `inflation.c` file that hooks into `main.c` and runs every 10 rounds. It generates a random rate, calculates a multiplier as `1.0 + rate`, and then loops over every square on the board, updating the `price`, `base_rent`, and `house_costs` permanently in the `GameState`. Because these struct values are modified directly, the next time inflation hits, it naturally compounds on top of the previously inflated value."

**"How did you handle the hardcoded railway and utility rents inflating?"**
> "In `finance.c`, the rents were originally hardcoded in switch statements. I refactored this by moving those base values into `GameState` as global arrays (`railway_rent_base` and `utility_rent_base`). This allowed the `handle_inflation` loop to compound those values over time, and `finance.c` simply references the dynamic array when charging a player."

**"How did you ensure existing loans aren't affected by a sudden 12% inflation spike?"**
> "I added a `loan_interest_rate` variable directly to the `Player` struct. When `obtain_loan()` is called, it takes a snapshot of the current global economy's interest rate and saves it to the player. The `apply_interest_all()` function uses that frozen rate to compound their debt, completely ignoring any fluctuations in the global economy."

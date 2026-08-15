#include "types.h"
#include <stdio.h>

#define LOAN_DURATION 20
#define COLLATERAL_RATIO 0.75

int find_player_index(GameState *game, int player_id);
int has_monopoly(GameState *game, int player_id, int square_index);

// =============================================================
// COLLATERAL HELPERS
// =============================================================

// Sums mortgage_value of all owned, unmortgaged, non-loan-locked assets
// Properties, railways, and utilities are eligible collateral
// Houses and hotels are NOT eligible

double calculate_total_collateral(GameState *game, int player_id) {
  double total = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id != player_id) continue;

    if (game->board[i].type == SQUARE_PROPERTY) {
      if (!game->board[i].data.property.is_mortgaged &&
          !game->board[i].data.property.is_loan_locked) {
        total += game->board[i].data.property.mortgage_value;
      }
    } else if (game->board[i].type == SQUARE_RAILWAY) {
      if (!game->board[i].data.railway.is_mortgaged &&
          !game->board[i].data.railway.is_loan_locked) {
        total += game->board[i].data.railway.mortgage_value;
      }
    } else if (game->board[i].type == SQUARE_UTILITY) {
      if (!game->board[i].data.utility.is_mortgaged &&
          !game->board[i].data.utility.is_loan_locked) {
        total += game->board[i].data.utility.mortgage_value;
      }
    }
  }
  return total;
}

// Returns 75% of total collateral value - the maximum loan a player can take

double calculate_max_loan(GameState *game, int player_id) {
  return calculate_total_collateral(game, player_id) * COLLATERAL_RATIO;
}

// =============================================================
// LOCK / UNLOCK COLLATERAL
// =============================================================

// Helper: gets the mortgage value of a square (property, railway, or utility)

static double get_mortgage_value(GameState *game, int square_index) {
  if (game->board[square_index].type == SQUARE_PROPERTY) {
    return game->board[square_index].data.property.mortgage_value;
  } else if (game->board[square_index].type == SQUARE_RAILWAY) {
    return game->board[square_index].data.railway.mortgage_value;
  } else if (game->board[square_index].type == SQUARE_UTILITY) {
    return game->board[square_index].data.utility.mortgage_value;
  }
  return 0;
}

// Helper: checks if a square is eligible collateral (owned, unmortgaged, not already locked)

static int is_eligible_collateral(GameState *game, int square_index, int player_id) {
  if (game->board[square_index].owner_id != player_id) return 0;

  if (game->board[square_index].type == SQUARE_PROPERTY) {
    return !game->board[square_index].data.property.is_mortgaged &&
           !game->board[square_index].data.property.is_loan_locked;
  } else if (game->board[square_index].type == SQUARE_RAILWAY) {
    return !game->board[square_index].data.railway.is_mortgaged &&
           !game->board[square_index].data.railway.is_loan_locked;
  } else if (game->board[square_index].type == SQUARE_UTILITY) {
    return !game->board[square_index].data.utility.is_mortgaged &&
           !game->board[square_index].data.utility.is_loan_locked;
  }
  return 0;
}

// Helper: locks a single square as collateral

static void lock_square(GameState *game, int square_index) {
  if (game->board[square_index].type == SQUARE_PROPERTY) {
    game->board[square_index].data.property.is_loan_locked = 1;
  } else if (game->board[square_index].type == SQUARE_RAILWAY) {
    game->board[square_index].data.railway.is_loan_locked = 1;
  } else if (game->board[square_index].type == SQUARE_UTILITY) {
    game->board[square_index].data.utility.is_loan_locked = 1;
  }
}

// SELECTIVE COLLATERAL LOCKING
// Picks assets one-by-one until (total pledged mortgage value) * 0.75 >= loan_amount
// Only pledges what is needed - remaining assets stay freely tradable
// Prints the names of pledged assets as "Collateral :" output

static void lock_sufficient_collateral(GameState *game, int player_id, double loan_amount) {
  double pledged_total = 0;
  double needed_collateral = loan_amount / COLLATERAL_RATIO;

  printf("Collateral :\n");

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (pledged_total >= needed_collateral) break;

    if (!is_eligible_collateral(game, i, player_id)) continue;

    double mv = get_mortgage_value(game, i);
    lock_square(game, i);
    pledged_total += mv;

    printf("  %s\n", game->board[i].name);
  }
}

// Unlocks all loan-locked assets when a loan is fully repaid

static void unlock_all_collateral(GameState *game, int player_id) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id != player_id) continue;

    if (game->board[i].type == SQUARE_PROPERTY) {
      game->board[i].data.property.is_loan_locked = 0;
    } else if (game->board[i].type == SQUARE_RAILWAY) {
      game->board[i].data.railway.is_loan_locked = 0;
    } else if (game->board[i].type == SQUARE_UTILITY) {
      game->board[i].data.utility.is_loan_locked = 0;
    }
  }
}

// =============================================================
// CORE LOAN OPERATIONS
// =============================================================

// OBTAIN LOAN
// Validates: no existing loan, amount > 0, amount <= max loan
// Credits cash, locks ONLY enough collateral to cover the loan, sets 20-round timer

void obtain_loan(GameState *game, int player_id, double amount) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  // Cannot have more than one active loan
  if (game->players[idx].has_loan) {
    printf("%s already has an active loan (LKR %.0lf remaining)\n",
           game->players[idx].name, game->players[idx].loan_amount);
    return;
  }

  double max_loan = calculate_max_loan(game, player_id);

  // Must have collateral
  if (max_loan <= 0) {
    printf("%s has no eligible collateral for a loan\n",
           game->players[idx].name);
    return;
  }

  // Cap amount at maximum allowed
  if (amount > max_loan) {
    amount = max_loan;
  }

  if (amount <= 0) return;

  // Credit the loan
  game->players[idx].money += amount;
  game->players[idx].has_loan = 1;
  game->players[idx].loan_amount = amount;
  game->players[idx].loan_rounds_left = LOAN_DURATION;
  game->players[idx].loan_start_round = game->current_turn;
  game->players[idx].loan_interest_rate = game->current_interest_rate;

  // Output matching PDF format
  printf("%s obtained a secured loan.\n", game->players[idx].name);
  printf("Loan Amount : LKR %.0lf.\n", amount);

  // Lock only enough collateral to cover this specific loan amount
  lock_sufficient_collateral(game, player_id, amount);
}

// REPAY LOAN (FULL)
// Pays entire outstanding balance, unlocks all collateral, resets loan fields

void repay_loan_full(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  double balance = game->players[idx].loan_amount;

  if (game->players[idx].money < balance) {
    printf("%s cannot afford full repayment (needs LKR %.0lf, has LKR %.0lf)\n",
           game->players[idx].name, balance, game->players[idx].money);
    return;
  }

  game->players[idx].money -= balance;

  // Clear loan state
  game->players[idx].has_loan = 0;
  game->players[idx].loan_amount = 0;
  game->players[idx].loan_rounds_left = 0;
  game->players[idx].loan_start_round = 0;

  // Release all locked assets
  unlock_all_collateral(game, player_id);

  printf("LOAN REPAID: %s paid off LKR %.0lf - all collateral unlocked\n",
         game->players[idx].name, balance);
}

// REPAY LOAN (PARTIAL)
// Deducts a partial payment from the outstanding balance
// If this reduces balance to 0 or below, calls repay_loan_full logic

void repay_loan_partial(GameState *game, int player_id, double amount) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  // Can't pay more than you have
  if (amount > game->players[idx].money) {
    amount = game->players[idx].money;
  }

  if (amount <= 0) return;

  // If paying enough to clear the whole loan, just do full repayment
  if (amount >= game->players[idx].loan_amount) {
    repay_loan_full(game, player_id);
    return;
  }

  game->players[idx].money -= amount;
  game->players[idx].loan_amount -= amount;

  printf("PARTIAL REPAYMENT: %s paid LKR %.0lf - remaining balance: LKR %.0lf\n",
         game->players[idx].name, amount, game->players[idx].loan_amount);
}

// REFINANCE LOAN
// Resets loan duration to 20 rounds (keeps current balance)
// If player has gained new collateral since the loan, they can borrow
// the difference between new max and current balance

void refinance_loan(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  double old_balance = game->players[idx].loan_amount;

  // Reset the timer
  game->players[idx].loan_rounds_left = LOAN_DURATION;
  game->players[idx].loan_start_round = game->current_turn;

  // Check if new collateral allows borrowing more
  double new_max = calculate_max_loan(game, player_id);
  double extra = new_max - old_balance;

  if (extra > 0) {
    game->players[idx].money += extra;
    game->players[idx].loan_amount += extra;

    printf("REFINANCE: %s reset loan to %d rounds + borrowed extra LKR %.0lf "
           "(new balance: LKR %.0lf)\n",
           game->players[idx].name, LOAN_DURATION, extra,
           game->players[idx].loan_amount);

    // Lock additional collateral for the extra amount only
    lock_sufficient_collateral(game, player_id, extra);
  } else {
    printf("REFINANCE: %s reset loan timer to %d rounds "
           "(balance unchanged: LKR %.0lf)\n",
           game->players[idx].name, LOAN_DURATION, old_balance);
  }
}

// EXTEND LOAN
// Adds extra rounds to the loan deadline without changing the balance

void extend_loan(GameState *game, int player_id, int extra_rounds) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  game->players[idx].loan_rounds_left += extra_rounds;

  printf("LOAN EXTENDED: %s got %d extra rounds - now %d rounds remaining\n",
         game->players[idx].name, extra_rounds,
         game->players[idx].loan_rounds_left);
}

// INCREASE LOAN
// If the player gained new collateral since the loan was taken,
// calculates the new max loan and credits the difference.
// Locks the new collateral assets.

void increase_loan(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  double current_balance = game->players[idx].loan_amount;
  double new_max = calculate_max_loan(game, player_id);
  double extra = new_max - current_balance;

  if (extra <= 0) {
    printf("%s has no new collateral to increase the loan\n",
           game->players[idx].name);
    return;
  }

  game->players[idx].money += extra;
  game->players[idx].loan_amount += extra;

  printf("LOAN INCREASE: %s borrowed extra LKR %.0lf "
         "(new balance: LKR %.0lf)\n",
         game->players[idx].name, extra,
         game->players[idx].loan_amount);

  // Lock additional collateral for the extra amount only
  lock_sufficient_collateral(game, player_id, extra);
}

// =============================================================
// FORECLOSURE
// =============================================================
// Transfers all loan-locked assets back to the bank (owner_id = -1)
// Demolishes all buildings, cancels insurance, clears debt

void run_auction(GameState *game, int square_index, int triggering_player_id);

void foreclosure(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  printf("\n*** FORECLOSURE: %s has defaulted on their loan! ***\n",
         game->players[idx].name);
  printf("All pledged assets are being seized by the Bank of Ceylon...\n");

  // Track seized squares for post-seizure auction
  int seized_indices[TOTAL_SQUARES];
  int seized_count = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id != player_id) continue;

    if (game->board[i].type == SQUARE_PROPERTY &&
        game->board[i].data.property.is_loan_locked) {

      printf("  Seized: %s", game->board[i].name);
      if (game->board[i].data.property.has_hotel) {
        printf(" (hotel demolished)");
      } else if (game->board[i].data.property.num_houses > 0) {
        printf(" (%d house(s) demolished)",
               game->board[i].data.property.num_houses);
      }
      printf("\n");

      // Reset the property completely
      game->board[i].data.property.num_houses = 0;
      game->board[i].data.property.has_hotel = 0;
      game->board[i].data.property.is_insured = 0;
      game->board[i].data.property.is_loan_locked = 0;
      game->board[i].data.property.is_mortgaged = 0;
      game->board[i].data.property.owner_id = -1;
      game->board[i].owner_id = -1;

      seized_indices[seized_count++] = i;

    } else if (game->board[i].type == SQUARE_RAILWAY &&
               game->board[i].data.railway.is_loan_locked) {

      printf("  Seized: %s\n", game->board[i].name);

      game->board[i].data.railway.is_loan_locked = 0;
      game->board[i].data.railway.is_mortgaged = 0;
      game->board[i].data.railway.owner_id = -1;
      game->board[i].owner_id = -1;

      seized_indices[seized_count++] = i;

    } else if (game->board[i].type == SQUARE_UTILITY &&
               game->board[i].data.utility.is_loan_locked) {

      printf("  Seized: %s\n", game->board[i].name);

      game->board[i].data.utility.is_loan_locked = 0;
      game->board[i].data.utility.is_mortgaged = 0;
      game->board[i].data.utility.owner_id = -1;
      game->board[i].owner_id = -1;

      seized_indices[seized_count++] = i;
    }
  }

  // Clear the player's loan state
  game->players[idx].has_loan = 0;
  game->players[idx].loan_amount = 0;
  game->players[idx].loan_rounds_left = 0;
  game->players[idx].loan_start_round = 0;

  printf("*** %s's debt has been cleared - assets forfeited ***\n\n",
         game->players[idx].name);

  // Auction all seized assets (Rule: foreclosed assets go to auction)
  for (int s = 0; s < seized_count; s++) {
    run_auction(game, seized_indices[s], player_id);
  }
}

// =============================================================
// END-OF-ROUND PROCESSING
// =============================================================

// APPLY INTEREST
// Called once at the end of every round, after all 4 players have moved.
// For each player with an active loan:
//   loan_amount = loan_amount * (1 + 0.08)
//   loan_rounds_left--

void apply_interest_all(GameState *game) {
  for (int i = 0; i < game->num_players; i++) {
    if (!game->players[i].has_loan) continue;

    double old_balance = game->players[i].loan_amount;
    double interest = old_balance * game->players[i].loan_interest_rate;

    game->players[i].loan_amount = old_balance + interest;
    game->players[i].loan_rounds_left--;

    printf("INTEREST: %s - LKR %.0lf + %.0lf interest = LKR %.0lf "
           "(%d rounds left)\n",
           game->players[i].name, old_balance, interest,
           game->players[i].loan_amount,
           game->players[i].loan_rounds_left);
  }
}

// CHECK LOAN DEFAULTS
// Called after apply_interest_all.
// If any player's loan_rounds_left <= 0 and they still have an active loan,
// trigger foreclosure.

void check_loan_defaults(GameState *game) {
  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].has_loan && game->players[i].loan_rounds_left <= 0) {
      foreclosure(game, game->players[i].id);
    }
  }
}

// =============================================================
// PLAYER LOAN STRATEGIES
// =============================================================
// Called when a player lands on Bank of Ceylon (index 38)

// Helper: checks if a player has any monopoly on the board

static int player_has_any_monopoly(GameState *game, int player_id) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY &&
        game->board[i].owner_id == player_id) {
      if (has_monopoly(game, player_id, i)) {
        return 1;
      }
    }
  }
  return 0;
}

// Helper: checks if a player can't afford to build houses on any monopolised group

static int needs_building_funds(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY &&
        game->board[i].owner_id == player_id) {
      if (has_monopoly(game, player_id, i)) {
        // If they have a monopoly but can't afford even one house
        if (game->players[idx].money <
            game->board[i].data.property.house_cost) {
          return 1;
        }
      }
    }
  }
  return 0;
}

void player_loan_decision(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  // ---------------------------------------------------------
  // Player 1 - Aggressive Investor
  // Uses debt as a tool for expansion
  // Takes loans specifically to fund construction on monopolised groups
  // Only repays when cash is double the debt
  // ---------------------------------------------------------

  if (player_id == 1) {
    if (game->players[idx].has_loan) {
      // Repay only when sitting on 2x the debt in cash
      if (game->players[idx].money > 2 * game->players[idx].loan_amount) {
        printf("[Aggressive Investor] Cash is 2x debt - repaying in full\n");
        repay_loan_full(game, player_id);
      } else {
        // Try to increase loan if new collateral is available
        double new_max = calculate_max_loan(game, player_id);
        if (new_max > game->players[idx].loan_amount) {
          printf("[Aggressive Investor] New collateral available - increasing loan\n");
          increase_loan(game, player_id);
        } else {
          // Must take an action - extend the loan to buy more time
          printf("[Aggressive Investor] Extending loan by 5 rounds for breathing room\n");
          extend_loan(game, player_id, 5);
        }
      }
    } else {
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan <= 0) {
        printf("[Aggressive Investor] No collateral available - cannot take a loan\n");
        return;
      }
      // Take a loan if we have a monopoly but can't afford to build
      if (player_has_any_monopoly(game, player_id) &&
          needs_building_funds(game, player_id)) {
        printf("[Aggressive Investor] Has monopoly but needs building funds - taking max loan\n");
        obtain_loan(game, player_id, max_loan);
      } else {
        // Must take an action - take a strategic loan for future investment
        double strategic_amount = max_loan * 0.5;
        printf("[Aggressive Investor] Taking strategic loan for future expansion\n");
        obtain_loan(game, player_id, strategic_amount);
      }
    }
  }

  // ---------------------------------------------------------
  // Player 2 - Conservative Banker
  // Hates debt. Only borrows if near bankruptcy (money < 500)
  // Always repays immediately if they can afford it
  // Otherwise makes partial payments of 50% of current cash
  // ---------------------------------------------------------

  else if (player_id == 2) {
    if (game->players[idx].has_loan) {
      if (game->players[idx].money >= game->players[idx].loan_amount) {
        printf("[Conservative Banker] Can afford full repayment - paying off loan\n");
        repay_loan_full(game, player_id);
      } else {
        // Pay 50% of current cash towards the loan
        double payment = game->players[idx].money * 0.5;
        if (payment > 0) {
          printf("[Conservative Banker] Making partial repayment of 50%% cash\n");
          repay_loan_partial(game, player_id, payment);
        }
      }
    } else {
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan <= 0) {
        printf("[Conservative Banker] No collateral available - cannot take a loan\n");
        return;
      }
      // Must take an action at Bank - borrows reluctantly
      if (game->players[idx].money < 500) {
        // Emergency: take a modest loan
        double loan_amount = 2000;
        if (loan_amount > max_loan) loan_amount = max_loan;
        printf("[Conservative Banker] Dangerously low cash (LKR %.0lf) - emergency loan\n",
               game->players[idx].money);
        obtain_loan(game, player_id, loan_amount);
      } else {
        // Cash is healthy but must take action - take the minimum possible loan
        double min_loan = 1000;
        if (min_loan > max_loan) min_loan = max_loan;
        printf("[Conservative Banker] Reluctantly taking minimum loan (bank requires action)\n");
        obtain_loan(game, player_id, min_loan);
      }
    }
  }

  // ---------------------------------------------------------
  // Player 3 - Risk Taker
  // Maximum leverage, maximum risk
  // Always refinances, always borrows maximum
  // Will always be in debt if possible
  // ---------------------------------------------------------

  else if (player_id == 3) {
    if (game->players[idx].has_loan) {
      // Refinance to reset the clock
      printf("[Risk Taker] Refinancing to buy more time\n");
      refinance_loan(game, player_id);

      // Try to increase to maximum
      double new_max = calculate_max_loan(game, player_id);
      if (new_max > game->players[idx].loan_amount) {
        printf("[Risk Taker] More collateral available - increasing loan\n");
        increase_loan(game, player_id);
      }
    } else {
      // Always take the maximum loan if any collateral exists
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan > 0) {
        printf("[Risk Taker] Leveraging everything - taking max loan\n");
        obtain_loan(game, player_id, max_loan);
      } else {
        printf("[Risk Taker] No collateral to leverage yet\n");
      }
    }
  }

  // ---------------------------------------------------------
  // Player 4 - Opportunistic Trader
  // Only borrows when the numbers make sense (potential rent > borrowing cost)
  // Repays when deadline is approaching (5 rounds or less)
  // ---------------------------------------------------------

  else if (player_id == 4) {
    if (game->players[idx].has_loan) {
      // Repay if nearing the default deadline (5 or fewer rounds left)
      if (game->players[idx].loan_rounds_left <= 5 &&
          game->players[idx].money >= game->players[idx].loan_amount) {
        printf("[Opportunistic Trader] Deadline approaching - repaying loan\n");
        repay_loan_full(game, player_id);
      } else if (game->players[idx].loan_rounds_left <= 5) {
        // Deadline close but can't afford full - pay as much as possible
        double payment = game->players[idx].money * 0.7;
        printf("[Opportunistic Trader] Deadline close (%d rounds) - making large partial repayment\n",
               game->players[idx].loan_rounds_left);
        repay_loan_partial(game, player_id, payment);
      } else {
        // Deadline not urgent - make a small partial repayment to reduce interest
        double payment = game->players[idx].money * 0.2;
        printf("[Opportunistic Trader] Making strategic partial repayment to reduce interest\n");
        repay_loan_partial(game, player_id, payment);
      }
    } else {
      // Calculate if borrowing would be profitable
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan <= 0) {
        printf("[Opportunistic Trader] No collateral available - cannot take a loan\n");
        return;
      }

      // Estimate total borrowing cost over 20 rounds at 8% compound
      // Total interest ≈ max_loan * ((1.08^20) - 1) ≈ max_loan * 3.66
      double borrowing_cost = max_loan * 3.66;

      // Estimate potential rent gain from building on monopolised groups
      double potential_rent_gain = 0;
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].type == SQUARE_PROPERTY &&
            game->board[i].owner_id == player_id &&
            has_monopoly(game, player_id, i)) {
          // Conservative estimate: base_rent * 4 houses * 20 rounds
          potential_rent_gain +=
              game->board[i].data.property.base_rent * 4 * LOAN_DURATION;
        }
      }

      if (potential_rent_gain > borrowing_cost && potential_rent_gain > 0) {
        // Only borrow enough to build, not the max
        double build_cost = 0;
        for (int i = 0; i < TOTAL_SQUARES; i++) {
          if (game->board[i].type == SQUARE_PROPERTY &&
              game->board[i].owner_id == player_id &&
              has_monopoly(game, player_id, i)) {
            build_cost +=
                game->board[i].data.property.house_cost * 4; // 4 houses
          }
        }

        double needed = build_cost - game->players[idx].money;
        if (needed <= 0) needed = 1000; // take a small amount if can already afford
        if (needed > max_loan) needed = max_loan;
        printf("[Opportunistic Trader] ROI is positive - borrowing LKR %.0lf to build\n",
               needed);
        obtain_loan(game, player_id, needed);
      } else {
        // ROI doesn't justify it, but must take action at bank - take minimum loan
        double min_loan = 1000;
        if (min_loan > max_loan) min_loan = max_loan;
        printf("[Opportunistic Trader] Taking small strategic loan (bank requires action)\n");
        obtain_loan(game, player_id, min_loan);
      }
    }
  }
}

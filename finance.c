#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#define LOAN_DURATION 20
#define COLLATERAL_RATIO 0.75

double get_dynamic_price(GameState *game, int square_index);
double get_dynamic_rent(GameState *game, int square_index);
double get_dynamic_mortgage(GameState *game, int square_index);
double get_dynamic_property_value(GameState *game, int square_index);
double get_dynamic_build_cost(GameState *game, int square_index, int is_hotel);
int find_player_index(GameState *game, int player_id);
int count_owned_railways(GameState *game, int owner_id);
int count_owned_utilities(GameState *game, int owner_id);
int has_monopoly(GameState *game, int player_id, int square_index);
void run_auction(GameState *game, int square_index);

double get_dynamic_interest_rate(GameState *game);
double calculate_max_loan(GameState *game, int player_id);
void obtain_loan(GameState *game, int player_id, double amount);
void repay_loan_full(GameState *game, int player_id);
void repay_loan_partial(GameState *game, int player_id, double amount);
void extend_loan(GameState *game, int player_id, int extra_rounds);
void increase_loan(GameState *game, int player_id);
void refinance_loan(GameState *game, int player_id);
double get_condition_multiplier(double condition);
double calculate_net_worth(GameState *game, int player_id);
void declare_bankruptcy(GameState *game, int player_id);
int attempt_raise_funds(GameState *game, int player_id, double amount_needed);

//---PROPERTY, RAILWAY & UTILITY PURCHASES---

int count_undeveloped_properties(GameState *game, int player_id) {
  int count = 0;
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && 
        game->board[i].owner_id == player_id) {
      if (game->board[i].data.property.num_houses == 0 && 
          !game->board[i].data.property.has_hotel) {
        count++;
      }
    }
  }
  
  return count;
}

void purchase_property(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    double purchase_price = get_dynamic_price(game, square_index);
    game->players[player_index].money -= purchase_price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.property.owner_id = game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name, purchase_price);

    // Apply Anti-Speculation Act
    if (game->active_regulation == REGULATION_ANTI_SPECULATION) {
      int undeveloped = count_undeveloped_properties(game, game->players[player_index].id);
      if (undeveloped > 3) {
        game->board[square_index].data.property.forced_development_rounds_left = 5;
        printf("[ANTI-SPECULATION ACT] %s owns >3 undeveloped properties! Must develop %s in 5 rounds.\n", 
               game->players[player_index].name, game->board[square_index].name);
      }
    }
  }
}

void purchase_railway(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    game->players[player_index].money -=
        game->board[square_index].data.railway.price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.railway.owner_id =
        game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name,
           game->board[square_index].data.railway.price);
  }
}

void purchase_utility(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    game->players[player_index].money -=
        game->board[square_index].data.utility.price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.utility.owner_id =
        game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name,
           game->board[square_index].data.utility.price);
  }
}

// ---RENT COLLECTION & DYNAMIC MULTIPLIERS---

void pay_property_rent(GameState *game, int player_index, int square_index) {
  if(game->board[square_index].data.property.is_mortgaged == 1) {
    printf("No rent, property is mortgaged\n");
    return;
  }

  if (game->board[square_index].data.property.is_damaged == 1) {
    printf("No rent, property is damaged from a disaster and needs repairs!\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  double base_rent = get_dynamic_rent(game, square_index);
  int num_houses = game->board[square_index].data.property.num_houses;
  int hotel = game->board[square_index].data.property.has_hotel;

  // Rent multiplier 
  double rent = base_rent;

  if (hotel) {
    rent = base_rent * 10;
  } else if (num_houses == 4) {
    rent = base_rent * 7;
  } else if (num_houses == 3) {
    rent = base_rent * 5;
  } else if (num_houses == 2) {
    rent = base_rent * 3;
  } else if (num_houses == 1) {
    rent = base_rent * 2;
  }

  // 1. Apply property depreciation to base_rent
  rent = rent * (1.0 - game->board[square_index].data.property.depreciation_pct / 100.0);

  // 2. Apply building condition multiplier
  double condition_mult = get_condition_multiplier(game->board[square_index].data.property.building_condition);
  rent = rent * condition_mult;

  if (condition_mult == 0.0) {
    printf("No rent, building is closed due to poor condition!\n");
    return;
  }

  // Display what multiplier was applied
  if (hotel) {
    printf("%s owes LKR %.0lf rent to %s for %s (HOTEL - 10x)\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name);
  } else if (num_houses > 0) {
    printf("%s owes LKR %.0lf rent to %s for %s (%d house(s))\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name,
           num_houses);
  } else {
    printf("%s owes LKR %.0lf rent to %s for %s\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name);
  }

  if (game->players[player_index].money >= rent) {
    game->players[player_index].money -= rent;
    game->players[owner_index].money += rent;
  } else {
    printf("%s cannot afford the rent! Attempting to raise funds...\n", game->players[player_index].name);
    if (attempt_raise_funds(game, game->players[player_index].id, rent)) {
      game->players[player_index].money -= rent;
      game->players[owner_index].money += rent;
      printf("%s successfully raised funds and paid the rent.\n", game->players[player_index].name);
    } else {
      declare_bankruptcy(game, game->players[player_index].id);
    }
  }
}

void pay_railway_rent(GameState *game, int player_index, int square_index) {
  if(game->board[square_index].data.railway.is_mortgaged == 1) {
    printf("No rent, railway is mortgaged\n");
    return;
  }
  
  if (game->board[square_index].data.railway.closed_rounds_left > 0) {
    printf("No rent, railway is closed due to Political Rally!\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  int owned = count_owned_railways(game, owner_id);
  double rent = 0;
  if (owned > 0 && owned <= 4) {
    rent = game->railway_rent_base[owned - 1];
  }
  
  if (game->active_economic_event == EVENT_FUEL_CRISIS) {
    rent *= 2.0; // Railway rent doubles
  }
  
  if (game->active_regional_card == CARD_TRANSPORT_STRIKE) {
    rent *= 0.60; // Railway revenue reduced by 40%
  }
  
  if (game->active_regulation == REGULATION_RAILWAY_MODERNIZATION) {
    rent *= 1.25; // Railway rent +25%
  }

  printf("%s owes LKR %.0lf railway rent to %s (%s owns %d station(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, game->players[owner_index].name, owned);

  if (game->players[player_index].money >= rent) {
    game->players[player_index].money -= rent;
    game->players[owner_index].money += rent;
  } else {
    printf("%s cannot afford the rent! Attempting to raise funds...\n", game->players[player_index].name);
    if (attempt_raise_funds(game, game->players[player_index].id, rent)) {
      game->players[player_index].money -= rent;
      game->players[owner_index].money += rent;
      printf("%s successfully raised funds and paid the rent.\n", game->players[player_index].name);
    } else {
      declare_bankruptcy(game, game->players[player_index].id);
    }
  }
}

void pay_utility_rent(GameState *game, int player_index, int square_index, int dice_roll) {
  if(game->board[square_index].data.utility.is_mortgaged == 1) {
    printf("No rent, utility is mortgaged\n");
    return;
  }
  
  if (game->board[square_index].data.utility.closed_rounds_left > 0) {
    printf("No rent, utility is closed due to Political Rally!\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  int owned = count_owned_utilities(game, owner_id);
  double rent = 0;
  if (owned > 0 && owned <= 2) {
    double multiplier = game->utility_rent_base[owned - 1];
    rent = dice_roll * multiplier;
  }
  
  if (game->active_regional_card == CARD_ELECTRICITY_TARIFF) {
    rent *= 1.25; // Utility rent +25%
  }
  
  if (game->active_regional_card == CARD_WATER_SHORTAGE && square_index == 28) {
    rent *= 1.20; // Water utility revenue +20%
  }
  
  if (game->active_regulation == REGULATION_ELECTRICITY_TARIFF) {
    rent *= 1.20; // Utility rent +20%
  }

  printf("%s owes LKR %.0lf utility rent to %s (dice was %d, %s owns %d utility(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, dice_roll,
         game->players[owner_index].name, owned);

  if (game->players[player_index].money >= rent) {
    game->players[player_index].money -= rent;
    game->players[owner_index].money += rent;
  } else {
    printf("%s cannot afford the rent! Attempting to raise funds...\n", game->players[player_index].name);
    if (attempt_raise_funds(game, game->players[player_index].id, rent)) {
      game->players[player_index].money -= rent;
      game->players[owner_index].money += rent;
      printf("%s successfully raised funds and paid the rent.\n", game->players[player_index].name);
    } else {
      declare_bankruptcy(game, game->players[player_index].id);
    }
  }
}

// ---BANKING & LOANS---

double get_dynamic_interest_rate(GameState *game) {
  double rate = game->current_interest_rate;
  if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
    rate = 0.15; // Recession
  } else if (game->active_economic_event == EVENT_STOCK_MARKET_BOOM) {
    rate = 0.05; // Economic Boom
  }
  
  if (game->active_regulation == REGULATION_REDUCE_LOAN_INTEREST) {
    rate -= 0.02; // Absolute -2%
  }
  
  if (rate < 0.0) rate = 0.0;
  return rate;
}

double calculate_total_collateral(GameState *game, int player_id) {
  double total = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id != player_id) continue;

    if (game->board[i].type == SQUARE_PROPERTY) {
      if (!game->board[i].data.property.is_mortgaged &&
          !game->board[i].data.property.is_loan_locked) {
        total += get_dynamic_mortgage(game, i);
      }
    } else if (game->board[i].type == SQUARE_RAILWAY) {
      if (!game->board[i].data.railway.is_mortgaged &&
          !game->board[i].data.railway.is_loan_locked) {
        total += get_dynamic_mortgage(game, i);
      }
    } else if (game->board[i].type == SQUARE_UTILITY) {
      if (!game->board[i].data.utility.is_mortgaged &&
          !game->board[i].data.utility.is_loan_locked) {
        total += get_dynamic_mortgage(game, i);
      }
    }
  }
  return total;
}

double calculate_max_loan(GameState *game, int player_id) {
  return calculate_total_collateral(game, player_id) * COLLATERAL_RATIO;
}

static double get_mortgage_value(GameState *game, int square_index) {
  return get_dynamic_mortgage(game, square_index);
}

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

static void lock_square(GameState *game, int square_index) {
  if (game->board[square_index].type == SQUARE_PROPERTY) {
    game->board[square_index].data.property.is_loan_locked = 1;
  } else if (game->board[square_index].type == SQUARE_RAILWAY) {
    game->board[square_index].data.railway.is_loan_locked = 1;
  } else if (game->board[square_index].type == SQUARE_UTILITY) {
    game->board[square_index].data.utility.is_loan_locked = 1;
  }
}

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

void obtain_loan(GameState *game, int player_id, double amount) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (game->players[idx].has_loan) {
    printf("%s already has an active loan (LKR %.0lf remaining)\n",
           game->players[idx].name, game->players[idx].loan_amount);
    return;
  }

  double max_loan = calculate_max_loan(game, player_id);
  if (max_loan <= 0) {
    printf("%s has no eligible collateral for a loan\n", game->players[idx].name);
    return;
  }

  if (amount > max_loan) amount = max_loan;
  if (amount <= 0) return;

  game->players[idx].money += amount;
  game->players[idx].has_loan = 1;
  game->players[idx].loan_amount = amount;
  game->players[idx].loan_rounds_left = LOAN_DURATION;
  game->players[idx].loan_start_round = game->current_turn;
  game->players[idx].loan_interest_rate = get_dynamic_interest_rate(game);

  printf("%s obtained a secured loan.\n", game->players[idx].name);
  printf("Loan Amount : LKR %.0lf.\n", amount);

  lock_sufficient_collateral(game, player_id, amount);
}

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
  game->players[idx].has_loan = 0;
  game->players[idx].loan_amount = 0;
  game->players[idx].loan_rounds_left = 0;
  game->players[idx].loan_start_round = 0;

  unlock_all_collateral(game, player_id);

  printf("LOAN REPAID: %s paid off LKR %.0lf - all collateral unlocked\n",
         game->players[idx].name, balance);
}

void repay_loan_partial(GameState *game, int player_id, double amount) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  if (amount > game->players[idx].money) amount = game->players[idx].money;
  if (amount <= 0) return;

  if (amount >= game->players[idx].loan_amount) {
    repay_loan_full(game, player_id);
    return;
  }

  game->players[idx].money -= amount;
  game->players[idx].loan_amount -= amount;

  printf("PARTIAL REPAYMENT: %s paid LKR %.0lf - remaining balance: LKR %.0lf\n",
         game->players[idx].name, amount, game->players[idx].loan_amount);
}

void refinance_loan(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  double old_balance = game->players[idx].loan_amount;
  double new_max = calculate_max_loan(game, player_id);
  double extra = new_max - old_balance;

  if (extra > 0) {
    game->players[idx].loan_rounds_left = LOAN_DURATION;
    game->players[idx].loan_start_round = game->current_turn;
    game->players[idx].money += extra;
    game->players[idx].loan_amount += extra;

    printf("REFINANCE: %s reset loan to %d rounds + borrowed extra LKR %.0lf (new balance: LKR %.0lf)\n",
           game->players[idx].name, LOAN_DURATION, extra, game->players[idx].loan_amount);

    lock_sufficient_collateral(game, player_id, extra);
  } else if (new_max >= old_balance) {
    game->players[idx].loan_rounds_left = LOAN_DURATION;
    game->players[idx].loan_start_round = game->current_turn;
    printf("REFINANCE: %s reset loan timer to %d rounds (balance unchanged: LKR %.0lf)\n",
           game->players[idx].name, LOAN_DURATION, old_balance);
  } else {
    printf("REFINANCE DENIED: %s cannot secure existing LKR %.0lf debt (max collateral is LKR %.0lf)\n",
           game->players[idx].name, old_balance, new_max);
  }
}

void extend_loan(GameState *game, int player_id, int extra_rounds) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  game->players[idx].loan_rounds_left += extra_rounds;

  printf("LOAN EXTENDED: %s got %d extra rounds - now %d rounds remaining\n",
         game->players[idx].name, extra_rounds, game->players[idx].loan_rounds_left);
}

void increase_loan(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  if (!game->players[idx].has_loan) return;

  double current_balance = game->players[idx].loan_amount;
  double new_max = calculate_max_loan(game, player_id);
  double extra = new_max - current_balance;

  if (extra <= 0) {
    printf("%s has no new collateral to increase the loan\n", game->players[idx].name);
    return;
  }

  game->players[idx].money += extra;
  game->players[idx].loan_amount += extra;

  printf("LOAN INCREASE: %s borrowed extra LKR %.0lf (new balance: LKR %.0lf)\n",
         game->players[idx].name, extra, game->players[idx].loan_amount);

  lock_sufficient_collateral(game, player_id, extra);
}

void foreclosure(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return;

  printf("\n*** FORECLOSURE: %s has defaulted on their loan! ***\n", game->players[idx].name);
  printf("All pledged assets are being seized by the Bank of Ceylon...\n");

  int seized_indices[TOTAL_SQUARES];
  int seized_count = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id != player_id) continue;

    if (game->board[i].type == SQUARE_PROPERTY && game->board[i].data.property.is_loan_locked) {
      printf("  Seized: %s", game->board[i].name);
      if (game->board[i].data.property.has_hotel) {
        printf(" (hotel demolished)");
      } else if (game->board[i].data.property.num_houses > 0) {
        printf(" (%d house(s) demolished)", game->board[i].data.property.num_houses);
      }
      printf("\n");

      game->board[i].data.property.num_houses = 0;
      game->board[i].data.property.has_hotel = 0;
      game->board[i].data.property.is_insured = 0;
      game->board[i].data.property.is_loan_locked = 0;
      game->board[i].data.property.is_mortgaged = 0;
      game->board[i].data.property.owner_id = -1;
      game->board[i].owner_id = -1;

      seized_indices[seized_count++] = i;

    } else if (game->board[i].type == SQUARE_RAILWAY && game->board[i].data.railway.is_loan_locked) {
      printf("  Seized: %s\n", game->board[i].name);

      game->board[i].data.railway.is_loan_locked = 0;
      game->board[i].data.railway.is_mortgaged = 0;
      game->board[i].data.railway.owner_id = -1;
      game->board[i].owner_id = -1;

      seized_indices[seized_count++] = i;

    } else if (game->board[i].type == SQUARE_UTILITY && game->board[i].data.utility.is_loan_locked) {
      printf("  Seized: %s\n", game->board[i].name);

      game->board[i].data.utility.is_loan_locked = 0;
      game->board[i].data.utility.is_mortgaged = 0;
      game->board[i].data.utility.owner_id = -1;
      game->board[i].owner_id = -1;

      seized_indices[seized_count++] = i;
    }
  }

  game->players[idx].has_loan = 0;
  game->players[idx].loan_amount = 0;
  game->players[idx].loan_rounds_left = 0;
  game->players[idx].loan_start_round = 0;

  printf("*** %s's debt has been cleared - assets forfeited ***\n\n", game->players[idx].name);

  for (int s = 0; s < seized_count; s++) {
    run_auction(game, seized_indices[s]);
  }

  int remaining_assets = 0;
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id == player_id) remaining_assets++;
  }
  
  if (remaining_assets == 0 && game->players[idx].money <= 0) {
    declare_bankruptcy(game, player_id);
  }
}

void apply_interest_all(GameState *game) {
  for (int i = 0; i < game->num_players; i++) {
    if (!game->players[i].has_loan) continue;

    double old_balance = game->players[i].loan_amount;
    double rate = game->players[i].loan_interest_rate;
    if (game->players[i].active_national_card == NATIONAL_CARD_INTEREST_RATE_CUT) rate -= 0.02;
    if (game->players[i].active_national_card == NATIONAL_CARD_INTEREST_RATE_INCREASE) rate += 0.02;
    if (rate < 0) rate = 0;
    
    // Divide the term interest rate by LOAN_DURATION for the per-round rate
    double per_round_rate = rate / LOAN_DURATION;
    double interest = old_balance * per_round_rate;

    game->players[i].loan_amount = old_balance + interest;
    game->players[i].loan_rounds_left--;

    printf("INTEREST: %s - LKR %.0lf + %.0lf interest = LKR %.0lf (%d rounds left)\n",
           game->players[i].name, old_balance, interest,
           game->players[i].loan_amount, game->players[i].loan_rounds_left);
  }
}

void check_loan_defaults(GameState *game) {
  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].has_loan && game->players[i].loan_rounds_left <= 0) {
      foreclosure(game, game->players[i].id);
    }
  }
}

static int player_has_any_monopoly(GameState *game, int player_id) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == player_id) {
      if (has_monopoly(game, player_id, i)) {
        return 1;
      }
    }
  }
  return 0;
}

static int needs_building_funds(GameState *game, int player_id) {
  int idx = find_player_index(game, player_id);
  if (idx == -1) return 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == player_id) {
      if (has_monopoly(game, player_id, i)) {
        if (game->players[idx].money < game->board[i].data.property.house_cost) {
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

  // Player 1 - Aggressive Investor
  if (player_id == 1) {
    if (game->players[idx].has_loan) {
      if (game->players[idx].money > 2 * game->players[idx].loan_amount) {
        printf("Aggressive Investor Cash is 2x debt - repaying in full\n");
        repay_loan_full(game, player_id);
      } else {
        double new_max = calculate_max_loan(game, player_id);
        if (new_max > game->players[idx].loan_amount) {
          printf("Aggressive Investor New collateral available - increasing loan\n");
          increase_loan(game, player_id);
        } else {
          printf("Aggressive Investor Extending loan by 5 rounds\n");
          extend_loan(game, player_id, 5);
        }
      }
    } else {
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan <= 0) {
        printf("Aggressive Investor No collateral available - cannot take a loan\n");
        return;
      }
      if (player_has_any_monopoly(game, player_id) && needs_building_funds(game, player_id)) {
        printf("Aggressive Investor Has monopoly but needs building funds - taking max loan\n");
        obtain_loan(game, player_id, max_loan);
      } else {
        double strategic_amount = max_loan * 0.5;
        printf("Aggressive Investor Taking strategic loan for future expansion\n");
        obtain_loan(game, player_id, strategic_amount);
      }
    }
  }
  // Player 2 - Conservative Banker
  else if (player_id == 2) {
    if (game->players[idx].has_loan) {
      if (game->players[idx].money >= game->players[idx].loan_amount) {
        printf("Conservative Banker Can afford full repayment - paying off loan\n");
        repay_loan_full(game, player_id);
      } else {
        double payment = game->players[idx].money * 0.5;
        if (payment > 0) {
          printf("Conservative Banker Making partial repayment of 50%% cash\n");
          repay_loan_partial(game, player_id, payment);
        }
      }
    } else {
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan <= 0) {
        printf("Conservative Banker No collateral available - cannot take a loan\n");
        return;
      }
      if (game->players[idx].money < 500) {
        double loan_amount = 2000;
        if (loan_amount > max_loan) loan_amount = max_loan;
        printf("Conservative Banker Dangerously low cash (LKR %.0lf) - emergency loan\n",
               game->players[idx].money);
        obtain_loan(game, player_id, loan_amount);
      } else {
        double min_loan = 1000;
        if (min_loan > max_loan) min_loan = max_loan;
        printf("Conservative Banker taking minimum loan (bank requires action)\n");
        obtain_loan(game, player_id, min_loan);
      }
    }
  }
  // Player 3 - Risk Taker
  else if (player_id == 3) {
    if (game->players[idx].has_loan) {
      printf("Risk Taker Refinancing to buy more time\n");
      refinance_loan(game, player_id);

      double new_max = calculate_max_loan(game, player_id);
      if (new_max > game->players[idx].loan_amount) {
        printf("Risk Taker More collateral available - increasing loan\n");
        increase_loan(game, player_id);
      }
    } else {
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan > 0) {
        printf("Risk Taker Leveraging everything - taking max loan\n");
        obtain_loan(game, player_id, max_loan);
      } else {
        printf("Risk Taker No collateral to leverage yet\n");
      }
    }
  }
  // Player 4 - Opportunistic Trader
  else if (player_id == 4) {
    if (game->players[idx].has_loan) {
      if (game->players[idx].loan_rounds_left <= 5 &&
          game->players[idx].money >= game->players[idx].loan_amount) {
        printf("Opportunistic Trader Deadline approaching - repaying loan\n");
        repay_loan_full(game, player_id);
      } else if (game->players[idx].loan_rounds_left <= 5) {
        double payment = game->players[idx].money * 0.7;
        printf("Opportunistic TraderDeadline close (%d rounds) - making partial repayment\n",
               game->players[idx].loan_rounds_left);
        repay_loan_partial(game, player_id, payment);
      } else {
        double payment = game->players[idx].money * 0.2;
        printf("Opportunistic Trader Making strategic partial repayment to reduce interest\n");
        repay_loan_partial(game, player_id, payment);
      }
    } else {
      double max_loan = calculate_max_loan(game, player_id);
      if (max_loan <= 0) {
        printf("Opportunistic Trader No collateral available - cannot take a loan\n");
        return;
      }

      double borrowing_cost = max_loan * 3.66;
      double potential_rent_gain = 0;
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].type == SQUARE_PROPERTY &&
            game->board[i].owner_id == player_id &&
            has_monopoly(game, player_id, i)) {
          potential_rent_gain += game->board[i].data.property.base_rent * 4 * LOAN_DURATION;
        }
      }

      if (potential_rent_gain > borrowing_cost && potential_rent_gain > 0) {
        double build_cost = 0;
        for (int i = 0; i < TOTAL_SQUARES; i++) {
          if (game->board[i].type == SQUARE_PROPERTY &&
              game->board[i].owner_id == player_id &&
              has_monopoly(game, player_id, i)) {
            build_cost += game->board[i].data.property.house_cost * 4;
          }
        }

        double needed = build_cost - game->players[idx].money;
        if (needed <= 0) needed = 1000;
        if (needed > max_loan) needed = max_loan;
        printf("Opportunistic Trader ROI is positive - borrowing LKR %.0lf to build\n", needed);
        obtain_loan(game, player_id, needed);
      } else {
        double min_loan = 1000;
        if (min_loan > max_loan) min_loan = max_loan;
        printf("Opportunistic Trader Taking small strategic loan (bank requires action)\n");
        obtain_loan(game, player_id, min_loan);
      }
    }
  }
}

// --------------INSURANCE---------------

const char* get_insurance_name(int tier) {
  switch (tier) {
    case 1: return "Basic Property Insurance";
    case 2: return "Comprehensive Insurance";
    case 3: return "Business Interruption Insurance";
    default: return "Uninsured";
  }
}

void update_insurance_durations(GameState *game) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY) {
      PropertyData *prop = &game->board[i].data.property;
      if (prop->is_insured && prop->insurance_rounds_left > 0) {
        prop->insurance_rounds_left--;

        if (prop->insurance_rounds_left == 3) {
          int owner_idx = find_player_index(game, prop->owner_id);
          if (owner_idx != -1) {
            printf("[INSURANCE EXPIRING] %s's policy on %s expires in 3 rounds!\n",
                   game->players[owner_idx].name, game->board[i].name);
          }
        } else if (prop->insurance_rounds_left == 0) {
          prop->is_insured = 0;
          prop->insurance_tier = 0;
        }
      }
    }
  }
}

static void buy_insurance(GameState *game, int player_index, int square_index, int tier) {
  double value = get_dynamic_price(game, square_index);
  double premium = 0;
  
  if (tier == 1) premium = value * 0.05;
  else if (tier == 2) premium = value * 0.10;
  else if (tier == 3) premium = value * 0.15;
  
  if (game->active_economic_event == EVENT_HEAVY_MONSOON) {
    premium *= 1.50;
  }

  if (game->players[player_index].active_national_card == NATIONAL_CARD_INSURANCE_DISCOUNT) {
    premium *= 0.80;
  }

  if (game->active_regulation == REGULATION_INSURANCE_REGULATION) {
    premium *= 0.85;
  }

  if (game->players[player_index].money >= premium) {
    game->players[player_index].money -= premium;
    game->board[square_index].data.property.is_insured = 1;
    game->board[square_index].data.property.insurance_tier = tier;
    game->board[square_index].data.property.insurance_rounds_left = 20;

    printf("--INSURANCE PURCHASE-- %s bought %s for %s (Premium: LKR %.0lf)\n",
           game->players[player_index].name, get_insurance_name(tier), 
           game->board[square_index].name, premium);
  }
}

void handle_insurance_landing(GameState *game, int player_index) {

  int player_id = game->players[player_index].id;
  printf("%s visited the Insurance Company.\n", game->players[player_index].name);

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == player_id) {
      PropertyData *prop = &game->board[i].data.property;
      
      if (prop->is_insured && prop->insurance_rounds_left > 0) continue;
      if (prop->num_houses == 0 && !prop->has_hotel) continue;

      double val = get_dynamic_price(game, i);

      // Aggressive Investor Purchases only Basic Insurance for houses and Comprehensive Insurance for hotels.
      if (player_id == 1) {
        if (prop->has_hotel) buy_insurance(game, player_index, i, 2);
        else buy_insurance(game, player_index, i, 1);
      } 
      // Conservative banker Always purchases Comprehensive Insurance for every developed property
      else if (player_id == 2) {
        buy_insurance(game, player_index, i, 2);
      }
      // Risk TakerPurchases insurance only after experiencing a financial loss
      else if (player_id == 3) {
        if (prop->is_damaged) {
          buy_insurance(game, player_index, i, 1);
        }
      }
      // Opportunistic Trader Buys insurance if the property value is high.
      else if (player_id == 4) {
        if (val >= 5000) {
          buy_insurance(game, player_index, i, 2);
        }
      }
    }
  }
}

// -------DEPRECIATION, RENOVATION & MAINTENANCE-------

void apply_property_depreciation(GameState *game) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type != SQUARE_PROPERTY) continue;
    if (game->board[i].owner_id == -1) continue;

    PropertyData *prop = &game->board[i].data.property;
    prop->prop_age++;

    if (prop->prop_age > 50) {
      int rounds_past_50 = prop->prop_age - 50;
      if (rounds_past_50 % 5 == 0 && prop->depreciation_pct < 30.0) {
        prop->depreciation_pct += 1.0;
        int owner_idx = find_player_index(game, game->board[i].owner_id);
        printf("[DEPRECIATION] %s's property %s has depreciated. Total value lost: %.1lf%%\n",
               game->players[owner_idx].name, game->board[i].name, prop->depreciation_pct);
      }
    }
  }
}

void apply_building_depreciation(GameState *game) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type != SQUARE_PROPERTY) continue;
    if (game->board[i].owner_id == -1) continue;

    PropertyData *prop = &game->board[i].data.property;
    if (prop->is_damaged) continue;
    
    if (prop->num_houses > 0 || prop->has_hotel) {
      prop->building_condition -= 2.0;
      if (prop->building_condition < 0.0) prop->building_condition = 0.0;
      
      prop->rounds_without_maint++;

      if (prop->rounds_without_maint > 20 && prop->has_structural_damage == 0) {
        prop->has_structural_damage = 1;
        prop->price -= prop->price * 0.15;
        prop->base_rent -= prop->base_rent * 0.25;
        prop->house_cost *= 1.5;
        prop->hotel_cost *= 1.5;

        int owner_idx = find_player_index(game, game->board[i].owner_id);
        printf("[STRUCTURAL DAMAGE] %s's buildings on %s have taken structural damage!\n",
               game->players[owner_idx].name, game->board[i].name);
      }
    }
  }
}

void renovate_property(GameState *game, int square_index, int player_index) {
  PropertyData *prop = &game->board[square_index].data.property;
  double cost = prop->price * 0.10;
  
  if (game->players[player_index].money >= cost) {
    game->players[player_index].money -= cost;
    prop->depreciation_pct = 0.0;
    prop->prop_age = 0;
    prop->base_rent = prop->base_rent_original;
    printf("RENOVATION %s renovated property %s for LKR %.0lf\n",
           game->players[player_index].name, game->board[square_index].name, cost);
  }
}

void do_building_maintenance(GameState *game, int square_index, int player_index) {
  PropertyData *prop = &game->board[square_index].data.property;
  double cost = 0.0;

  if (prop->has_hotel) {
    cost = prop->hotel_cost * 0.08;
  } else if (prop->num_houses > 0) {
    cost = (prop->house_cost * 0.05) * prop->num_houses;
  } else {
    return;
  }

  if (game->players[player_index].money >= cost) {
    game->players[player_index].money -= cost;
    prop->building_condition = 100.0;
    prop->rounds_without_maint = 0;
    printf("MAINTENANCE %s performed maintenance on %s for LKR %.0lf\n",
           game->players[player_index].name, game->board[square_index].name, cost);
  }
}

void renovate_building(GameState *game, int square_index, int player_index) {
  PropertyData *prop = &game->board[square_index].data.property;
  if (prop->has_structural_damage == 0) return;

  double cost = 0.0;
  if (prop->has_hotel) {
    cost = prop->hotel_cost * 0.25;
  } else if (prop->num_houses > 0) {
    cost = prop->house_cost * 0.25;
  }

  if (game->players[player_index].money >= cost) {
    game->players[player_index].money -= cost;
    
    prop->price = prop->price / 0.85; 
    prop->base_rent = prop->base_rent / 0.75;
    
    prop->has_structural_damage = 0;
    prop->building_condition = 100.0;
    prop->rounds_without_maint = 0;
    printf("BUILDING RENOVATION %s repaired structural damage on %s for LKR %.0lf\n",
           game->players[player_index].name, game->board[square_index].name, cost);
  }
}

double get_condition_multiplier(double condition) {
  if (condition >= 90.0) return 1.0;
  if (condition >= 75.0) return 0.90;
  if (condition >= 50.0) return 0.75;
  if (condition >= 25.0) return 0.50;
  return 0.0;
}

void player_maintenance_decision(GameState *game, int player_id) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type != SQUARE_PROPERTY) continue;
    if (game->board[i].owner_id != player_id) continue;

    PropertyData *prop = &game->board[i].data.property;

    // 1. Structural Damage Renovation (Priority 1)
    if (prop->has_structural_damage) {
      if (player_id == 1 || player_id == 2) {
        // Aggressive Investor and Conservative Banker always renovate immediately
        renovate_building(game, i, player_index);
      } else if (player_id == 3) {
        // Risk taker waits until absolutely forced (structural damage has happened, so forced to fix now to get rent back)
        renovate_building(game, i, player_index);
      } else if (player_id == 4) {
        // Opportunistic Trader renovates if reserve is kept
        double cost = prop->has_hotel ? prop->hotel_cost * 0.25 : prop->house_cost * 0.25;
        if (game->players[player_index].money - cost >= 500) {
          renovate_building(game, i, player_index);
        }
      }
    }

    // 2. Routine Maintenance
    if (prop->building_condition < 100.0 && !prop->has_structural_damage && (prop->num_houses > 0 || prop->has_hotel)) {
      if (player_id == 1 || player_id == 2) {
        // Always maintains if possible
        do_building_maintenance(game, i, player_index);
      } else if (player_id == 3) {
        // Risk taker completely ignores maintenance
      } else if (player_id == 4) {
        // Maintains if reserve > 500
        double cost = prop->has_hotel ? prop->hotel_cost * 0.08 : (prop->house_cost * 0.05) * prop->num_houses;
        if (game->players[player_index].money - cost >= 500) {
          do_building_maintenance(game, i, player_index);
        }
      }
    }

    // 3. Property Renovation (Land)
    if (prop->depreciation_pct > 0.0) {
      if (player_id == 1) {
        // Ignores land depreciation
      } else if (player_id == 2 && prop->depreciation_pct >= 10.0) {
        renovate_property(game, i, player_index);
      } else if (player_id == 3 && prop->depreciation_pct >= 30.0) {
        renovate_property(game, i, player_index);
      } else if (player_id == 4 && prop->depreciation_pct >= 15.0) {
        renovate_property(game, i, player_index);
      }
    }
  }
}

// NET WORTH, ASSET LIQUIDATION & BANKRUPTCY

double calculate_net_worth(GameState *game, int player_id) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return 0;

  double cash = game->players[player_index].money;
  double property_value = 0;
  double building_value = 0;
  double railway_value = 0;
  double utility_value = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id == player_id) {
      if (game->board[i].type == SQUARE_PROPERTY) {
        property_value += get_dynamic_property_value(game, i);
        
        int houses = game->board[i].data.property.num_houses;
        int hotels = game->board[i].data.property.has_hotel;
        
        double current_house_cost = get_dynamic_build_cost(game, i, 0);
        double current_hotel_cost = get_dynamic_build_cost(game, i, 1);
        
        double raw_building_val = (houses * current_house_cost) + (hotels * current_hotel_cost);
        
        double condition_pct = game->board[i].data.property.building_condition / 100.0;
        building_value += (raw_building_val * condition_pct);
        
      } else if (game->board[i].type == SQUARE_RAILWAY) {
        railway_value += game->board[i].data.railway.price;
      } else if (game->board[i].type == SQUARE_UTILITY) {
        utility_value += game->board[i].data.utility.price;
      }
    }
  }

  double insurance_receivable = game->players[player_index].insurance_claims_receivable;
  double outstanding_loans = game->players[player_index].loan_amount;
  double accrued_interest = game->players[player_index].loan_amount * get_dynamic_interest_rate(game);
  double taxes_due = game->players[player_index].taxes_due;

  double net_worth = cash + property_value + building_value + railway_value + utility_value + 
                     insurance_receivable - outstanding_loans - accrued_interest - taxes_due;

  return net_worth;
}

void declare_bankruptcy(GameState *game, int player_id) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return;

  printf("\n--BANKRUPTCY-- %s has been declared BANKRUPT!\n", game->players[player_index].name);
  game->players[player_index].is_bankrupt = 1;

  int seized_indices[TOTAL_SQUARES];
  int seized_count = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id == player_id) {
      game->board[i].owner_id = -1;
      if (game->board[i].type == SQUARE_PROPERTY) {
        game->board[i].data.property.owner_id = -1;
        game->board[i].data.property.num_houses = 0;
        game->board[i].data.property.has_hotel = 0;
        game->board[i].data.property.is_mortgaged = 0;
        game->board[i].data.property.is_insured = 0;
        game->board[i].data.property.insurance_rounds_left = 0;
        game->board[i].data.property.is_loan_locked = 0;
        game->board[i].data.property.forced_development_rounds_left = 0;
      } else if (game->board[i].type == SQUARE_RAILWAY) {
        game->board[i].data.railway.owner_id = -1;
        game->board[i].data.railway.is_mortgaged = 0;
        game->board[i].data.railway.is_loan_locked = 0;
      } else if (game->board[i].type == SQUARE_UTILITY) {
        game->board[i].data.utility.owner_id = -1;
        game->board[i].data.utility.is_mortgaged = 0;
        game->board[i].data.utility.is_loan_locked = 0;
      }
      seized_indices[seized_count++] = i;
    }
  }

  for (int s = 0; s < seized_count; s++) {
    run_auction(game, seized_indices[s]);
  }
}

void sell_asset_to_bank(GameState *game, int player_id, int square_index) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return;
  if (game->board[square_index].owner_id != player_id) return;

  double sell_value = 0;
  if (game->board[square_index].type == SQUARE_PROPERTY) {
    sell_value = game->board[square_index].data.property.price * 0.5;
    game->board[square_index].data.property.owner_id = -1;
    game->board[square_index].data.property.num_houses = 0;
    game->board[square_index].data.property.has_hotel = 0;
    game->board[square_index].data.property.is_mortgaged = 0;
    game->board[square_index].data.property.is_insured = 0;
    game->board[square_index].data.property.insurance_rounds_left = 0;
    game->board[square_index].data.property.is_loan_locked = 0;
  } else if (game->board[square_index].type == SQUARE_RAILWAY) {
    sell_value = game->board[square_index].data.railway.price * 0.5;
    game->board[square_index].data.railway.owner_id = -1;
    game->board[square_index].data.railway.is_mortgaged = 0;
    game->board[square_index].data.railway.is_loan_locked = 0;
  } else if (game->board[square_index].type == SQUARE_UTILITY) {
    sell_value = game->board[square_index].data.utility.price * 0.5;
    game->board[square_index].data.utility.owner_id = -1;
    game->board[square_index].data.utility.is_mortgaged = 0;
    game->board[square_index].data.utility.is_loan_locked = 0;
  }

  game->players[player_index].money += sell_value;
  game->board[square_index].owner_id = -1;
  printf("[ASSET LIQUIDATION] %s sold %s to the Bank for LKR %.0lf\n", 
         game->players[player_index].name, game->board[square_index].name, sell_value);
}

int attempt_raise_funds(GameState *game, int player_id, double amount_needed) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->players[player_index].money >= amount_needed) {
      return 1;
    }
    if (game->board[i].owner_id == player_id) {

      int is_locked = 0;
      if (game->board[i].type == SQUARE_PROPERTY) {
        is_locked = game->board[i].data.property.is_loan_locked;
      } else if (game->board[i].type == SQUARE_RAILWAY){
         is_locked = game->board[i].data.railway.is_loan_locked;
      }else if (game->board[i].type == SQUARE_UTILITY) {
        is_locked = game->board[i].data.utility.is_loan_locked;
      }

      if (!is_locked) {
        sell_asset_to_bank(game, player_id, i);
      }
    }
  }

  if (game->players[player_index].money >= amount_needed) {
    return 1;
  }
  return 0;
}

// ---------------INCOME TAX-------------------

void handle_tax_landing(GameState *game, int player_id) {
  int visitor_index = find_player_index(game, player_id);
  if (visitor_index == -1) return;

  printf("Landed on Tax\n");
  double net_worth = calculate_net_worth(game, player_id);
  double rate = 0.15;
  if (game->active_regulation == REGULATION_INCREASE_PROPERTY_TAX) {
    rate = 0.225;
    printf("--REGULATION ACTIVE-- Income Tax increased to 22.5%%!\n");
  }
  double tax_amount = net_worth * rate;
  if (tax_amount < 0) tax_amount = 0;
  printf("%s's Net Worth is LKR %.0lf. Income Tax is LKR %.0lf (%.1lf%%)\n", game->players[visitor_index].name, net_worth, tax_amount, rate * 100);

  if (game->players[visitor_index].money >= tax_amount) {
    game->players[visitor_index].money -= tax_amount;
    printf("%s paid the tax.\n", game->players[visitor_index].name);
  } else {
    printf("%s cannot afford the tax! Attempting to raise funds...\n", game->players[visitor_index].name);
    if (attempt_raise_funds(game, player_id, tax_amount)) {
      game->players[visitor_index].money -= tax_amount;
      printf("%s successfully raised funds and paid the tax.\n", game->players[visitor_index].name);
    } else {
      declare_bankruptcy(game, player_id);
    }
  }
}

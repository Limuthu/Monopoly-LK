#include "types.h"
#include <stdio.h>

double get_dynamic_price(GameState *game, int square_index);
double get_dynamic_rent(GameState *game, int square_index);

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

int find_player_index(GameState *game, int player_id);
int count_owned_railways(GameState *game, int owner_id);
int count_owned_utilities(GameState *game, int owner_id);
int has_monopoly(GameState *game, int player_id, int square_index);

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

  // Rent multiplier (Table 6)
  // No buildings: 1x | 1 house: 2x | 2 houses: 3x | 3 houses: 5x | 4 houses: 7x | Hotel: 10x

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
  double get_condition_multiplier(double condition);
  double condition_mult = get_condition_multiplier(game->board[square_index].data.property.building_condition);
  rent = rent * condition_mult;

  if (condition_mult == 0.0) {
    printf("No rent, building is closed due to poor condition!\n");
    return;
  }

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  // Display what multiplier was applied
  if (hotel) {
    printf("%s paid LKR %.0lf rent to %s for %s (HOTEL - 10x)\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name);
  } else if (num_houses > 0) {
    printf("%s paid LKR %.0lf rent to %s for %s (%d house(s))\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name,
           num_houses);
  } else {
    printf("%s paid LKR %.0lf rent to %s for %s\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name);
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

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %.0lf railway rent to %s (%s owns %d station(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, game->players[owner_index].name, owned);
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

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %.0lf utility rent to %s (dice was %d, %s owns %d utility(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, dice_roll,
         game->players[owner_index].name, owned);
}

// =============================================================
// INCOME TAX SUPPORT FUNCTIONS
// =============================================================

double get_dynamic_property_value(GameState *game, int square_index);
double get_dynamic_build_cost(GameState *game, int square_index, int is_hotel);
double get_dynamic_interest_rate(GameState *game);
void run_auction(GameState *game, int square_index, int triggering_player_id);

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
        
        // Apply depreciation based on physical condition
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

  printf("\n[BANKRUPTCY] %s has been declared BANKRUPT!\n", game->players[player_index].name);
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

  // Auction all liquidated assets
  for (int s = 0; s < seized_count; s++) {
    run_auction(game, seized_indices[s], player_id);
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

  // Keep selling assets until we have enough money, or we run out of assets
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->players[player_index].money >= amount_needed) {
      return 1; // Successfully raised enough
    }
    if (game->board[i].owner_id == player_id) {
      // Don't sell properties that are locked as collateral for loans
      int is_locked = 0;
      if (game->board[i].type == SQUARE_PROPERTY) is_locked = game->board[i].data.property.is_loan_locked;
      else if (game->board[i].type == SQUARE_RAILWAY) is_locked = game->board[i].data.railway.is_loan_locked;
      else if (game->board[i].type == SQUARE_UTILITY) is_locked = game->board[i].data.utility.is_loan_locked;
      
      if (!is_locked) {
        sell_asset_to_bank(game, player_id, i);
      }
    }
  }

  // Check one last time
  if (game->players[player_index].money >= amount_needed) {
    return 1;
  }
  return 0; // Failed to raise enough funds
}

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

// External function declarations
double get_dynamic_rent(GameState *game, int square_index);
double get_dynamic_price(GameState *game, int square_index);

// Helper to get disaster name
const char* get_disaster_name(DisasterType type) {
  switch (type) {
    case DISASTER_FIRE: return "Fire";
    case DISASTER_FLOOD: return "Flood";
    case DISASTER_RIOT: return "Riot";
    case DISASTER_BUILDING_COLLAPSE: return "Building Collapse (Earthquake)";
    case DISASTER_ELECTRICAL_FAILURE: return "Electrical Failure (Vandalism)";
    default: return "Unknown";
  }
}

// Helper to get insurance tier name
const char* get_insurance_name(int tier) {
  switch (tier) {
    case 1: return "Basic Property Insurance";
    case 2: return "Comprehensive Insurance";
    case 3: return "Business Interruption Insurance";
    default: return "Uninsured";
  }
}

// Attempt to pay off disaster repair costs if player has cash
void attempt_disaster_repairs(GameState *game, int player_id) {
  int player_index = -1;
  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].id == player_id) {
      player_index = i;
      break;
    }
  }
  if (player_index == -1) return;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && 
        game->board[i].owner_id == player_id &&
        game->board[i].data.property.is_damaged) {
      
      double cost = game->board[i].data.property.pending_repair_cost;
      double payout = game->board[i].data.property.pending_insurance_payout;
      
      if (game->players[player_index].money + payout >= cost) {
        game->players[player_index].money += payout;
        game->players[player_index].money -= cost;
        game->players[player_index].insurance_claims_receivable -= payout;
        game->board[i].data.property.pending_insurance_payout = 0;

        game->board[i].data.property.is_damaged = 0;
        game->board[i].data.property.pending_repair_cost = 0;
        
        // Reset maintenance state (Disaster repair acts as full renovation)
        if (game->board[i].data.property.has_structural_damage) {
          game->board[i].data.property.price /= 0.85;
          game->board[i].data.property.base_rent /= 0.75;
          game->board[i].data.property.has_structural_damage = 0;
        }
        game->board[i].data.property.building_condition = 100.0;
        game->board[i].data.property.rounds_without_maint = 0;
        
        printf("[REPAIR] %s finally paid LKR %.0lf to repair %s. It can collect rent again!\n",
               game->players[player_index].name, cost, game->board[i].name);
      }
    }
  }
}

// ----------------------------------------------------------------------------
// DISASTER LOGIC
// ----------------------------------------------------------------------------

void trigger_disaster(GameState *game) {
  // Find all developed properties (has at least 1 house or hotel)
  int developed_squares[TOTAL_SQUARES];
  int count = 0;

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY) {
      if (game->board[i].data.property.num_houses > 0 || game->board[i].data.property.has_hotel) {
        developed_squares[count++] = i;
      }
    }
  }

  if (count == 0) {
    printf("\n[DISASTER] A natural disaster struck, but no properties are developed yet!\n\n");
    return;
  }

  // Pick random property and disaster
  int target_index = developed_squares[rand() % count];
  
  // Disaster generation logic
  DisasterType type;
  int r = rand() % 100;
  
  if (game->active_economic_event == EVENT_HEAVY_MONSOON) {
    // Flood becomes 50% chance
    if (r < 50) type = DISASTER_FLOOD;
    else type = (DisasterType)(rand() % 5); 
  } else if (game->active_economic_event == EVENT_POLITICAL_UNREST) {
    // Riot probability doubles (usually 20%, so make it 40%)
    if (r < 40) type = DISASTER_RIOT;
    else type = (DisasterType)(rand() % 5);
  } else {
    // Normal 20% each
    type = (DisasterType)(rand() % 5);
  }
  
  PropertyData *prop = &game->board[target_index].data.property;
  int owner_idx = -1;
  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].id == prop->owner_id) {
      owner_idx = i;
      break;
    }
  }

  if (owner_idx == -1) return;

  // Calculate repair cost (50% of total built value)
  double built_value = (prop->house_cost * prop->num_houses) + (prop->hotel_cost * prop->has_hotel);
  double repair_cost = built_value * 0.50;

  printf("\n======================================================\n");
  printf("[DISASTER] A %s has struck %s!\n", get_disaster_name(type), game->board[target_index].name);
  printf("Total Repair Cost: LKR %.0lf\n", repair_cost);

  // Determine coverage
  double compensation = 0;
  int is_covered = 0;

  if (prop->is_insured && prop->insurance_rounds_left > 0) {
    if (prop->insurance_tier == 1) {
      // Basic covers Fire, Flood
      if (type == DISASTER_FIRE || type == DISASTER_FLOOD) {
        is_covered = 1;
        compensation = repair_cost * 0.80; // 80% payout
      }
    } else if (prop->insurance_tier == 2) {
      // Comprehensive covers all
      is_covered = 1;
      compensation = repair_cost * 1.00; // 100% payout
    } else if (prop->insurance_tier == 3) {
      // Bus Interruption covers all + rent
      is_covered = 1;
      int rent_rounds = 5;
      if (game->active_economic_event == EVENT_POLITICAL_UNREST) {
        rent_rounds = 10; // Claims increase
      }
      compensation = repair_cost * 1.00 + (get_dynamic_rent(game, target_index) * rent_rounds);
    }
  }

  if (is_covered) {
    printf("Insurance (%s) covers the disaster!\n", get_insurance_name(prop->insurance_tier));
    printf("Payout credited to %s: LKR %.0lf\n", game->players[owner_idx].name, compensation);
    
    // Set pending payout to act as receivable
    prop->pending_insurance_payout = compensation;
    game->players[owner_idx].insurance_claims_receivable += compensation;
  } else {
    if (prop->is_insured) {
      printf("Unfortunately, %s does not cover this type of disaster!\n", get_insurance_name(prop->insurance_tier));
    } else {
      printf("The property is uninsured!\n");
    }
  }

  // Deduct repair cost if affordable
  if (game->players[owner_idx].money >= repair_cost) {
    game->players[owner_idx].money -= repair_cost;
    
    // Reset maintenance state
    if (prop->has_structural_damage) {
      prop->price /= 0.85;
      prop->base_rent /= 0.75;
      prop->has_structural_damage = 0;
    }
    prop->building_condition = 100.0;
    prop->rounds_without_maint = 0;

    printf("%s paid the repair cost. The property remains fully functional.\n", game->players[owner_idx].name);
  } else {
    // Cannot afford!
    prop->is_damaged = 1;
    prop->pending_repair_cost = repair_cost;
    printf(">> WARNING: %s cannot afford the repair! %s is damaged and cannot collect rent until repaired.\n",
           game->players[owner_idx].name, game->board[target_index].name);
  }
  printf("======================================================\n\n");
}

// ----------------------------------------------------------------------------
// INSURANCE LOGIC
// ----------------------------------------------------------------------------

void update_insurance_durations(GameState *game) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY) {
      PropertyData *prop = &game->board[i].data.property;
      if (prop->is_insured && prop->insurance_rounds_left > 0) {
        prop->insurance_rounds_left--;

        if (prop->insurance_rounds_left == 3) {
          int owner_idx = -1;
          for (int p = 0; p < game->num_players; p++) {
            if (game->players[p].id == prop->owner_id) {
              owner_idx = p;
              break;
            }
          }
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

// Helper to buy policy
static void buy_insurance(GameState *game, int player_index, int square_index, int tier) {
  double value = get_dynamic_price(game, square_index);
  double premium = 0;
  
  if (tier == 1) premium = value * 0.05;
  else if (tier == 2) premium = value * 0.10;
  else if (tier == 3) premium = value * 0.15;
  
  if (game->active_economic_event == EVENT_HEAVY_MONSOON) {
    premium *= 1.50; // Global Insurance Premiums +50%
  }

  if (game->players[player_index].active_national_card == NATIONAL_CARD_INSURANCE_DISCOUNT) {
    premium *= 0.80; // 20% discount
  }

  if (game->active_regulation == REGULATION_INSURANCE_REGULATION) {
    premium *= 0.85; // 15% discount
  }

  if (game->players[player_index].money >= premium) {
    game->players[player_index].money -= premium;
    game->board[square_index].data.property.is_insured = 1;
    game->board[square_index].data.property.insurance_tier = tier;
    game->board[square_index].data.property.insurance_rounds_left = 20;

    printf("[INSURANCE PURCHASE] %s bought %s for %s (Premium: LKR %.0lf)\n",
           game->players[player_index].name, get_insurance_name(tier), 
           game->board[square_index].name, premium);
  }
}

// Triggered when landing on square 17 or 33
void handle_insurance_landing(GameState *game, int player_index) {
  int player_id = game->players[player_index].id;
  printf("%s visited the Insurance Company.\n", game->players[player_index].name);

  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == player_id) {
      PropertyData *prop = &game->board[i].data.property;
      
      // Skip if already insured
      if (prop->is_insured && prop->insurance_rounds_left > 0) continue;
      
      // Skip undeveloped
      if (prop->num_houses == 0 && !prop->has_hotel) continue;

      double val = get_dynamic_price(game, i);

      // AI Logic
      if (player_id == 1) { // Aggressive Investor
        if (prop->has_hotel) buy_insurance(game, player_index, i, 2); // Buys Comp for hotels
        else buy_insurance(game, player_index, i, 1);                 // Buys Basic for houses
      } 
      else if (player_id == 2) { // Conservative Banker
        buy_insurance(game, player_index, i, 2); // Highly cautious, buys Comp for all
      }
      else if (player_id == 3) { // Risk Taker
        // Refuses to buy insurance until after suffered a financial loss
        if (prop->is_damaged) {
          buy_insurance(game, player_index, i, 1);
        }
      }
      else if (player_id == 4) { // Opportunistic Trader
        // Only buys Comp for high value (> 5000)
        if (val >= 5000) {
          buy_insurance(game, player_index, i, 2);
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------
// NATIONAL EVENT CARDS LOGIC
// ----------------------------------------------------------------------------

void init_national_deck(GameState *game) {
  NationalEventCardType temp_deck[20] = {
    NATIONAL_CARD_TOURISM_HYPE,
    NATIONAL_CARD_FUEL_SHORTAGE,
    NATIONAL_CARD_HEAVY_FLOODS,
    NATIONAL_CARD_POLITICAL_RALLY,
    NATIONAL_CARD_STOCK_MARKET_RISE,
    NATIONAL_CARD_ECONOMIC_DOWNTURN,
    NATIONAL_CARD_HOUSING_SUBSIDY,
    NATIONAL_CARD_INTEREST_RATE_CUT,
    NATIONAL_CARD_INTEREST_RATE_INCREASE,
    NATIONAL_CARD_TAX_AMNESTY,
    NATIONAL_CARD_POWER_FAILURE,
    NATIONAL_CARD_FOREIGN_FUNDING,
    NATIONAL_CARD_PORT_EXPANSION,
    NATIONAL_CARD_FESTIVAL_SEASON,
    NATIONAL_CARD_LABOUR_STRIKE,
    NATIONAL_CARD_INSURANCE_DISCOUNT,
    NATIONAL_CARD_PROPERTY_REVALUATION,
    NATIONAL_CARD_CURRENCY_DEPRECIATION,
    NATIONAL_CARD_GOVERNMENT_GRANT,
    NATIONAL_CARD_NATIONAL_DISASTER
  };
  
  // Shuffle the deck deterministically for the game instance
  for (int i = 19; i > 0; i--) {
    int j = rand() % (i + 1);
    NationalEventCardType temp = temp_deck[i];
    temp_deck[i] = temp_deck[j];
    temp_deck[j] = temp;
  }
  
  for (int i = 0; i < 20; i++) {
    game->national_deck[i] = temp_deck[i];
  }
  game->national_deck_index = 0;
}

const char* get_national_card_name(NationalEventCardType type) {
  switch (type) {
    case NATIONAL_CARD_TOURISM_HYPE: return "Tourism Hype";
    case NATIONAL_CARD_FUEL_SHORTAGE: return "Fuel Shortage";
    case NATIONAL_CARD_HEAVY_FLOODS: return "Heavy Floods";
    case NATIONAL_CARD_POLITICAL_RALLY: return "Political Rally";
    case NATIONAL_CARD_STOCK_MARKET_RISE: return "Stock Market Rise";
    case NATIONAL_CARD_ECONOMIC_DOWNTURN: return "Economic Downturn";
    case NATIONAL_CARD_HOUSING_SUBSIDY: return "Housing Subsidy";
    case NATIONAL_CARD_INTEREST_RATE_CUT: return "Interest Rate Cut";
    case NATIONAL_CARD_INTEREST_RATE_INCREASE: return "Interest Rate Increase";
    case NATIONAL_CARD_TAX_AMNESTY: return "Tax Amnesty";
    case NATIONAL_CARD_POWER_FAILURE: return "Power Failure";
    case NATIONAL_CARD_FOREIGN_FUNDING: return "Foreign Funding";
    case NATIONAL_CARD_PORT_EXPANSION: return "Port Expansion";
    case NATIONAL_CARD_FESTIVAL_SEASON: return "Festival Season";
    case NATIONAL_CARD_LABOUR_STRIKE: return "Labour Strike";
    case NATIONAL_CARD_INSURANCE_DISCOUNT: return "Insurance Discount";
    case NATIONAL_CARD_PROPERTY_REVALUATION: return "Property Revaluation";
    case NATIONAL_CARD_CURRENCY_DEPRECIATION: return "Currency Depreciation";
    case NATIONAL_CARD_GOVERNMENT_GRANT: return "Government Grant";
    case NATIONAL_CARD_NATIONAL_DISASTER: return "National Disaster";
    default: return "None";
  }
}

void draw_national_card(GameState *game, int player_index) {
  Player *p = &game->players[player_index];
  NationalEventCardType card = game->national_deck[game->national_deck_index];
  
  printf("\n--- %s draws a National Event Card! ---\n", p->name);
  printf("Card: %s\n", get_national_card_name(card));
  
  // Advance deck index
  game->national_deck_index = (game->national_deck_index + 1) % 20;
  
  int is_instant = 0;
  
  switch (card) {
    case NATIONAL_CARD_HEAVY_FLOODS: {
      is_instant = 1;
      int coastal_props[40];
      int count = 0;
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == p->id) {
           PropertyGroup grp = game->board[i].data.property.group;
           if (grp == GROUP_YELLOW || grp == GROUP_LIGHT_BLUE || grp == GROUP_ORANGE) {
             coastal_props[count++] = i;
           }
        }
      }
      if (count > 0) {
        int target = coastal_props[rand() % count];
        PropertyData *prop = &game->board[target].data.property;
        double repair_cost = ((prop->house_cost * prop->num_houses) + (prop->hotel_cost * prop->has_hotel)) * 0.50;
        if (repair_cost == 0) repair_cost = prop->price * 0.20; // fallback if undeveloped
        prop->is_damaged = 1;
        prop->pending_repair_cost = repair_cost;
        printf("[EVENT] Heavy Floods! %s is damaged. Repair cost: LKR %.0lf\n", game->board[target].name, repair_cost);
      } else {
        printf("[EVENT] Heavy Floods! But %s owns no coastal properties.\n", p->name);
      }
      break;
    }
    case NATIONAL_CARD_POLITICAL_RALLY: {
      p->active_national_card = card;
      p->national_card_rounds_left = 2;
      int owned_props[40];
      int count = 0;
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if ((game->board[i].type == SQUARE_PROPERTY || game->board[i].type == SQUARE_RAILWAY || game->board[i].type == SQUARE_UTILITY) && game->board[i].owner_id == p->id) {
           owned_props[count++] = i;
        }
      }
      if (count > 0) {
        int target = owned_props[rand() % count];
        if (game->board[target].type == SQUARE_PROPERTY) {
            game->board[target].data.property.closed_rounds_left = 2;
        } else if (game->board[target].type == SQUARE_RAILWAY) {
            game->board[target].data.railway.closed_rounds_left = 2;
        } else if (game->board[target].type == SQUARE_UTILITY) {
            game->board[target].data.utility.closed_rounds_left = 2;
        }
        printf("[EVENT] Political Rally! %s is closed for 2 rounds.\n", game->board[target].name);
      } else {
        printf("[EVENT] Political Rally! But %s owns no properties.\n", p->name);
      }
      break;
    }
    case NATIONAL_CARD_TAX_AMNESTY: {
      is_instant = 1;
      printf("[EVENT] Tax Amnesty! Each player receives LKR 2,000.\n");
      for (int i = 0; i < game->num_players; i++) {
        if (!game->players[i].is_bankrupt) {
          game->players[i].money += 2000;
        }
      }
      break;
    }
    case NATIONAL_CARD_GOVERNMENT_GRANT: {
      is_instant = 1;
      int count_active = 0;
      for (int i = 0; i < game->num_players; i++) {
        if (!game->players[i].is_bankrupt) count_active++;
      }
      if (count_active > 0) {
          int target = -1;
          int r = rand() % count_active;
          int curr = 0;
          for (int i = 0; i < game->num_players; i++) {
            if (!game->players[i].is_bankrupt) {
                if (curr == r) { target = i; break; }
                curr++;
            }
          }
          if (target != -1) {
              game->players[target].money += 5000;
              printf("[EVENT] Government Grant! %s receives LKR 5,000.\n", game->players[target].name);
          }
      }
      break;
    }
    case NATIONAL_CARD_NATIONAL_DISASTER: {
      is_instant = 1;
      int dev_props[40];
      int count = 0;
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == p->id) {
           if (game->board[i].data.property.num_houses > 0 || game->board[i].data.property.has_hotel) {
             dev_props[count++] = i;
           }
        }
      }
      if (count > 0) {
        int target = dev_props[rand() % count];
        PropertyData *prop = &game->board[target].data.property;
        double repair_cost = ((prop->house_cost * prop->num_houses) + (prop->hotel_cost * prop->has_hotel)) * 0.50;
        prop->is_damaged = 1;
        prop->pending_repair_cost = repair_cost;
        printf("[EVENT] National Disaster! %s is damaged. Repair cost: LKR %.0lf\n", game->board[target].name, repair_cost);
      } else {
        printf("[EVENT] National Disaster! But %s has no developed properties.\n", p->name);
      }
      break;
    }
    case NATIONAL_CARD_PROPERTY_REVALUATION: {
      p->active_national_card = card;
      p->national_card_rounds_left = 15;
      
      PropertyGroup owned_groups[9];
      int count = 0;
      int has_group[9] = {0};
      
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].type == SQUARE_PROPERTY && game->board[i].owner_id == p->id) {
            PropertyGroup grp = game->board[i].data.property.group;
            if (!has_group[grp]) {
                has_group[grp] = 1;
                owned_groups[count++] = grp;
            }
        }
      }
      if (count > 0) {
          p->revalued_group = owned_groups[rand() % count];
          printf("[EVENT] Property Revaluation! One of %s's property groups appreciates by 15%% for 15 rounds.\n", p->name);
      } else {
          p->revalued_group = GROUP_NONE;
          printf("[EVENT] Property Revaluation! But %s owns no properties.\n", p->name);
      }
      break;
    }
    default: {
      p->active_national_card = card;
      if (card == NATIONAL_CARD_TOURISM_HYPE || card == NATIONAL_CARD_FUEL_SHORTAGE) {
          p->national_card_rounds_left = 5;
      } else if (card == NATIONAL_CARD_POWER_FAILURE) {
          p->national_card_rounds_left = 3;
      } else if (card == NATIONAL_CARD_LABOUR_STRIKE) {
          p->national_card_rounds_left = 2;
          p->construction_suspended = 1;
      } else {
          p->national_card_rounds_left = 15;
      }
      printf("[EVENT] %s modifier applies for %d rounds.\n", get_national_card_name(card), p->national_card_rounds_left);
      break;
    }
  }
  printf("--------------------------------------------\n");
}

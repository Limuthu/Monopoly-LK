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
      
      if (game->players[player_index].money >= cost) {
        game->players[player_index].money -= cost;
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
    
    // Credit compensation
    game->players[owner_idx].money += compensation;
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

#include "types.h"
#include <stdio.h>

int find_player_index(GameState *game, int player_id);

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
    
    // Skip properties currently completely destroyed by a disaster
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
    printf("[RENOVATION] %s renovated property %s for LKR %.0lf\n",
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
    printf("[MAINTENANCE] %s performed maintenance on %s for LKR %.0lf\n",
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
    
    // Restore the penalty values
    prop->price = prop->price / 0.85; 
    prop->base_rent = prop->base_rent / 0.75;
    
    prop->has_structural_damage = 0;
    prop->building_condition = 100.0;
    prop->rounds_without_maint = 0;
    printf("[BUILDING RENOVATION] %s repaired structural damage on %s for LKR %.0lf\n",
           game->players[player_index].name, game->board[square_index].name, cost);
  }
}

double get_condition_multiplier(double condition) {
  if (condition >= 90.0) return 1.0;
  if (condition >= 75.0) return 0.90;
  if (condition >= 50.0) return 0.75;
  if (condition >= 25.0) return 0.50;
  return 0.0; // Below 25% condition -> closed
}

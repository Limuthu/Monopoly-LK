#include "types.h"
#include <stdio.h>

int has_monopoly(GameState *game, int player_id, int square_index);
int min_houses_in_group(GameState *game, int square_index);
int find_player_index(GameState *game, int player_id);
double get_dynamic_build_cost(GameState *game, int square_index, int is_hotel);
double get_dynamic_price(GameState *game, int square_index);


int can_build_house(GameState *game, int square_index) {

  // Must be a property
  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;

  // Must have an owner
  int owner_id = game->board[square_index].owner_id;
  if (owner_id == -1) return 0;

  // Must not be mortgaged
  if (game->board[square_index].data.property.is_mortgaged) return 0;

  // Owner must have monopoly on this colour group (Rule 8)
  if (!has_monopoly(game, owner_id, square_index)) return 0;

  // Max 4 houses per property (Rule 9)
  if (game->board[square_index].data.property.num_houses >= 4) return 0;

  // Cannot have houses if hotel exists (Rule 10)
  if (game->board[square_index].data.property.has_hotel) return 0;

  // Even-building rule (Rule 9):
  // Can only add a house if this property's count equals the group minimum
  int min = min_houses_in_group(game, square_index);
  if (game->board[square_index].data.property.num_houses > min) return 0;

  // Owner must be able to afford the house cost
  int player_index = find_player_index(game, owner_id);
  if (player_index == -1) return 0;
  if (game->players[player_index].money <
      get_dynamic_build_cost(game, square_index, 0))
    return 0;

  return 1;
}

void build_house(GameState *game, int square_index) {
  int owner_id = game->board[square_index].owner_id;
  int player_index = find_player_index(game, owner_id);

  double cost = get_dynamic_build_cost(game, square_index, 0);
  game->players[player_index].money -= cost;
  game->board[square_index].data.property.num_houses++;

  printf("%s built house %d on %s for LKR %.0lf\n",
         game->players[player_index].name,
         game->board[square_index].data.property.num_houses,
         game->board[square_index].name,
         cost);
}

int can_build_hotel(GameState *game, int square_index) {

  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;

  int owner_id = game->board[square_index].owner_id;
  if (owner_id == -1) return 0;

  if (game->board[square_index].data.property.is_mortgaged) return 0;

  // Must have exactly 4 houses to upgrade (Rule 10)
  if (game->board[square_index].data.property.num_houses != 4) return 0;

  // Must not already have a hotel
  if (game->board[square_index].data.property.has_hotel) return 0;

  // Owner must afford it
  int player_index = find_player_index(game, owner_id);
  if (player_index == -1) return 0;
  if (game->players[player_index].money <
      get_dynamic_build_cost(game, square_index, 1))
    return 0;

  return 1;
}

void build_hotel(GameState *game, int square_index) {
  int owner_id = game->board[square_index].owner_id;
  int player_index = find_player_index(game, owner_id);

  double cost = get_dynamic_build_cost(game, square_index, 1);
  game->players[player_index].money -= cost;
  game->board[square_index].data.property.num_houses = 0; // Hotel replaces 4 houses
  game->board[square_index].data.property.has_hotel = 1;

  printf("%s built a HOTEL on %s for LKR %.0lf\n",
         game->players[player_index].name, game->board[square_index].name,
         cost);
}

int has_any_mortgage(GameState *game, int player_id) {
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].owner_id != player_id) continue;

    if (game->board[i].type == SQUARE_PROPERTY &&
        game->board[i].data.property.is_mortgaged)
      return 1;
    if (game->board[i].type == SQUARE_RAILWAY &&
        game->board[i].data.railway.is_mortgaged)
      return 1;
    if (game->board[i].type == SQUARE_UTILITY &&
        game->board[i].data.utility.is_mortgaged)
      return 1;
  }
  return 0;
}

void player_build_decision(GameState *game, int player_id) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return;

  if (game->players[player_index].construction_suspended) {
    printf("%s Cannot construct due to Labour Strike!\n", game->players[player_index].name);
    return;
  }

  // Handle Anti-Speculation Act 
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY && 
        game->board[i].owner_id == player_id &&
        game->board[i].data.property.forced_development_rounds_left > 0) {
      if (can_build_house(game, i)) {
        double cost = get_dynamic_build_cost(game, i, 0);
        if (game->players[player_index].money >= cost) {
          printf("%s Forced to build on %s to comply with Anti-Speculation Act!\n", 
                 game->players[player_index].name, game->board[i].name);
          build_house(game, i);
        }
      }
    }
  }

  // Player 1 - Aggressive Investor
  // Builds max houses immediately after monopoly.
  // Upgrades to hotels as soon as legally possible.

  if (player_id == 1) {

    int built = 1;
    while (built) {
      built = 0;

      // Build houses evenly across all monopolised groups
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].owner_id != player_id) continue;
        if (can_build_house(game, i)) {
          build_house(game, i);
          built = 1;
        }
      }

      // Upgrade to hotels wherever possible
      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].owner_id != player_id) continue;
        if (can_build_hotel(game, i)) {
          build_hotel(game, i);
          built = 1;
        }
      }
    }
  }

  // Player 2 - Conservative Banker
  // never builds hotels until all outstanding mortgages are paid.
  // Maintains 50% cash reserve (consistent with buying behaviour).

  else if (player_id == 2) {
    if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
      printf("Conservative Banker Refusing to build during Economic Recession!\n");
      return;
    }

    int mortgages_exist = has_any_mortgage(game, player_id);
    int has_loan = game->players[player_index].has_loan;

    int built = 1;
    while (built) {
      built = 0;

      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].owner_id != player_id) continue;

        // Build houses only if 50% of current cash remains after purchase
        if (can_build_house(game, i)) {
          double cost = get_dynamic_build_cost(game, i, 0);
          if (cost <= game->players[player_index].money * 0.5) {
            build_house(game, i);
            built = 1;
          }
        }

        // Hotels only allowed when zero mortgages AND no active loan AND 50% cash reserve
        if (!mortgages_exist && !has_loan && can_build_hotel(game, i)) {
          double cost = get_dynamic_build_cost(game, i, 1);
          if (cost <= game->players[player_index].money * 0.5) {
            build_hotel(game, i);
            built = 1;
          }
        }
      }
    }
  }

  // Player 3 - Risk Taker
  // Hotels as early as possible.
  // Prioritises expensive property groups over balanced portfolios.
  // No cash reserve - will build even if it drains almost all money.
  // Sells lower-value properties to finance premium developments.

  else if (player_id == 3) {
    void sell_asset_to_bank(GameState *game, int player_id, int square_index);
    int p_idx = find_player_index(game, player_id);

    // Iterate groups from most expensive to cheapest
    PropertyGroup priority[] = {GROUP_DARK_BLUE, GROUP_GREEN,  GROUP_YELLOW,
                                GROUP_RED,       GROUP_ORANGE, GROUP_PINK,
                                GROUP_LIGHT_BLUE, GROUP_BROWN};

    int built = 1;
    while (built) {
      built = 0;

      for (int p = 0; p < 8; p++) {
        for (int i = 0; i < TOTAL_SQUARES; i++) {
          if (game->board[i].type != SQUARE_PROPERTY) continue;
          if (game->board[i].owner_id != player_id) continue;
          if (game->board[i].data.property.group != priority[p]) continue;

          // Build houses aggressively
          if (can_build_house(game, i)) {
            build_house(game, i);
            built = 1;
          } else {
            // Check if it's a premium property and only failing due to money
            if (priority[p] == GROUP_DARK_BLUE || priority[p] == GROUP_GREEN || priority[p] == GROUP_YELLOW) {
              game->players[p_idx].money += 10000;
              int could_build = can_build_house(game, i);
              game->players[p_idx].money -= 10000;
              
              if (could_build) {
                double cost = get_dynamic_build_cost(game, i, 0);
                PropertyGroup cheap_groups[] = {GROUP_BROWN, GROUP_LIGHT_BLUE, GROUP_PINK, GROUP_ORANGE};
                for (int c = 0; c < 4; c++) {
                  for (int j = 0; j < TOTAL_SQUARES; j++) {
                    if (game->players[p_idx].money >= cost) break;
                    if (game->board[j].type == SQUARE_PROPERTY && game->board[j].owner_id == player_id &&
                        game->board[j].data.property.group == cheap_groups[c] &&
                        game->board[j].data.property.num_houses == 0 &&
                        !game->board[j].data.property.has_hotel &&
                        !game->board[j].data.property.is_loan_locked) {
                      sell_asset_to_bank(game, player_id, j);
                    }
                  }
                }
                if (game->players[p_idx].money >= cost) {
                  build_house(game, i);
                  built = 1;
                }
              }
            }
          }

          // Upgrade to hotel immediately when possible
          if (can_build_hotel(game, i)) {
            build_hotel(game, i);
            built = 1;
          } else {
            if (priority[p] == GROUP_DARK_BLUE || priority[p] == GROUP_GREEN || priority[p] == GROUP_YELLOW) {
              game->players[p_idx].money += 10000;
              int could_build = can_build_hotel(game, i);
              game->players[p_idx].money -= 10000;
              
              if (could_build) {
                double cost = get_dynamic_build_cost(game, i, 1);
                PropertyGroup cheap_groups[] = {GROUP_BROWN, GROUP_LIGHT_BLUE, GROUP_PINK, GROUP_ORANGE};
                for (int c = 0; c < 4; c++) {
                  for (int j = 0; j < TOTAL_SQUARES; j++) {
                    if (game->players[p_idx].money >= cost) break;
                    if (game->board[j].type == SQUARE_PROPERTY && game->board[j].owner_id == player_id &&
                        game->board[j].data.property.group == cheap_groups[c] &&
                        game->board[j].data.property.num_houses == 0 &&
                        !game->board[j].data.property.has_hotel &&
                        !game->board[j].data.property.is_loan_locked) {
                      sell_asset_to_bank(game, player_id, j);
                    }
                  }
                }
                if (game->players[p_idx].money >= cost) {
                  build_hotel(game, i);
                  built = 1;
                }
              }
            }
          }
        }
      }
    }
  }

  // Player 4 - Opportunistic Trader
  // Delays construction during inflation.
  // Accelerates construction if Housing Subsidy is active.
  // Maintains LKR 500 cash reserve 

  else if (player_id == 4) {

    if (game->current_inflation_rate > 0.0) {
      printf("Opportunistic Trader Delaying construction due to inflation (%.0lf%%)\n", 
             game->current_inflation_rate * 100);
      return;
    }

    // Sell properties in a declining group, coastal properties during Heavy Monsoon, or properties hit by regional debuffs
    for (int i = 0; i < TOTAL_SQUARES; i++) {
      if (game->board[i].owner_id == player_id && game->board[i].type == SQUARE_PROPERTY) {
        PropertyGroup grp = game->board[i].data.property.group;
        int is_coastal = (grp == GROUP_YELLOW || grp == GROUP_LIGHT_BLUE || grp == GROUP_ORANGE);
        
        int has_market_decline = (grp == game->market_decline_group);
        int has_monsoon_decline = (game->active_economic_event == EVENT_HEAVY_MONSOON && is_coastal);
        int has_regional_decline = 0;
        
        if (game->active_regional_card == CARD_BEACH_POLLUTION && grp == GROUP_YELLOW) has_regional_decline = 1;
        if (game->active_regional_card == CARD_FLOOD_DAMAGE && is_coastal) has_regional_decline = 1;
        if (game->active_regional_card == CARD_WATER_SHORTAGE && (i == 26 || i == 27 || i == 29)) has_regional_decline = 1;

        if (has_market_decline || has_monsoon_decline || has_regional_decline) {
          // Sell property back to the bank for half its base price
          double sell_price = game->board[i].data.property.price * 0.5;
          game->players[player_index].money += sell_price;
          game->board[i].owner_id = -1;
          game->board[i].data.property.owner_id = -1;
          game->board[i].data.property.num_houses = 0;
          game->board[i].data.property.has_hotel = 0;
          
          if (has_regional_decline) {
            printf("Opportunistic Trader Sold %s back to the bank for LKR %.0lf to avoid Regional Development debuff.\n", 
                   game->board[i].name, sell_price);
          } else if (has_monsoon_decline) {
            printf("Opportunistic Trader Sold %s back to the bank for LKR %.0lf to avoid Heavy Monsoon damage.\n", 
                   game->board[i].name, sell_price);
          } else {
            printf("Opportunistic Trader Sold %s back to the bank for LKR %.0lf to avoid Market Decline.\n", 
                   game->board[i].name, sell_price);
          }
        }
      }
    }

    int built = 1;
    while (built) {
      built = 0;

      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].owner_id != player_id) continue;

        if (can_build_house(game, i)) {
          double cost = get_dynamic_build_cost(game, i, 0);
          int reserve_needed = (game->active_economic_event == EVENT_GOV_HOUSING || 
                                game->active_regulation == REGULATION_HOUSING_SUBSIDY) ? 0 : 500;
          
          if (game->players[player_index].money - cost >= reserve_needed) {
            build_house(game, i);
            built = 1;
          }
        }

        // Build hotels with same reserve check
        if (can_build_hotel(game, i)) {
          double cost = get_dynamic_build_cost(game, i, 1);
          if (game->players[player_index].money - cost >= 500) {
            build_hotel(game, i);
            built = 1;
          }
        }
      }
    }
  }
}




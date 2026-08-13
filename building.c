#include "types.h"
#include <stdio.h>

int has_monopoly(GameState *game, int player_id, int square_index);
int min_houses_in_group(GameState *game, int square_index);
int find_player_index(GameState *game, int player_id);

// =============================================
// CORE BUILDING FUNCTIONS
// =============================================

// CAN BUILD HOUSE
// Checks all prerequisites: monopoly, even-building, limits, affordability

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
      game->board[square_index].data.property.house_cost)
    return 0;

  return 1;
}

// BUILD HOUSE
// Deducts house_cost from owner and increments num_houses

void build_house(GameState *game, int square_index) {
  int owner_id = game->board[square_index].owner_id;
  int player_index = find_player_index(game, owner_id);

  game->players[player_index].money -=
      game->board[square_index].data.property.house_cost;
  game->board[square_index].data.property.num_houses++;

  printf("%s built house %d on %s for LKR %.0lf\n",
         game->players[player_index].name,
         game->board[square_index].data.property.num_houses,
         game->board[square_index].name,
         game->board[square_index].data.property.house_cost);
}

// CAN BUILD HOTEL
// Property must have exactly 4 houses, owner must afford hotel_cost

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
      game->board[square_index].data.property.hotel_cost)
    return 0;

  return 1;
}

// BUILD HOTEL
// Deducts hotel_cost, sets has_hotel = 1, resets num_houses = 0

void build_hotel(GameState *game, int square_index) {
  int owner_id = game->board[square_index].owner_id;
  int player_index = find_player_index(game, owner_id);

  game->players[player_index].money -=
      game->board[square_index].data.property.hotel_cost;
  game->board[square_index].data.property.num_houses = 0; // Hotel replaces 4 houses
  game->board[square_index].data.property.has_hotel = 1;

  printf("%s built a HOTEL on %s for LKR %.0lf\n",
         game->players[player_index].name, game->board[square_index].name,
         game->board[square_index].data.property.hotel_cost);
}

// =============================================
// HELPER: CHECK FOR OUTSTANDING MORTGAGES
// =============================================
// Used by Conservative Banker — returns 1 if ANY owned property/railway/utility
// is mortgaged

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

// =============================================
// PLAYER BUILDING STRATEGIES
// =============================================

void player_build_decision(GameState *game, int player_id) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return;

  // ---------------------------------------------------------
  // Player 1 — Aggressive Investor
  // Builds max houses immediately after monopoly.
  // Upgrades to hotels as soon as legally possible.
  // ---------------------------------------------------------

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

  // ---------------------------------------------------------
  // Player 2 — Conservative Banker
  // NEVER builds hotels until all outstanding mortgages are paid.
  // Maintains 50% cash reserve (consistent with buying behaviour).
  // ---------------------------------------------------------

  else if (player_id == 2) {

    int mortgages_exist = has_any_mortgage(game, player_id);
    int has_loan = game->players[player_index].has_loan;

    int built = 1;
    while (built) {
      built = 0;

      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].owner_id != player_id) continue;

        // Build houses only if 50% of current cash remains after purchase
        if (can_build_house(game, i)) {
          double cost = game->board[i].data.property.house_cost;
          if (cost <= game->players[player_index].money * 0.5) {
            build_house(game, i);
            built = 1;
          }
        }

        // Hotels only allowed when zero mortgages AND no active loan AND 50% cash reserve
        if (!mortgages_exist && !has_loan && can_build_hotel(game, i)) {
          double cost = game->board[i].data.property.hotel_cost;
          if (cost <= game->players[player_index].money * 0.5) {
            build_hotel(game, i);
            built = 1;
          }
        }
      }
    }
  }

  // ---------------------------------------------------------
  // Player 3 — Risk Taker
  // Hotels as early as possible.
  // Prioritises expensive property groups over balanced portfolios.
  // No cash reserve — will build even if it drains almost all money.
  // ---------------------------------------------------------

  else if (player_id == 3) {

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
          }

          // Upgrade to hotel immediately when possible
          if (can_build_hotel(game, i)) {
            build_hotel(game, i);
            built = 1;
          }
        }
      }
    }
  }

  // ---------------------------------------------------------
  // Player 4 — Opportunistic Trader
  // Delays construction during inflation.
  // Accelerates construction if Housing Subsidy is active.
  // Maintains LKR 500 cash reserve (consistent with buying behaviour).
  // ---------------------------------------------------------

  else if (player_id == 4) {

    // TODO: When inflation/events system is implemented, add checks:
    //   - Skip building if inflation rate is high
    //   - Build aggressively if Housing Subsidy is active (30% cost reduction)

    int built = 1;
    while (built) {
      built = 0;

      for (int i = 0; i < TOTAL_SQUARES; i++) {
        if (game->board[i].owner_id != player_id) continue;

        // Build houses only if LKR 500 reserve maintained
        if (can_build_house(game, i)) {
          double cost = game->board[i].data.property.house_cost;
          if (game->players[player_index].money - cost >= 500) {
            build_house(game, i);
            built = 1;
          }
        }

        // Build hotels with same reserve check
        if (can_build_hotel(game, i)) {
          double cost = game->board[i].data.property.hotel_cost;
          if (game->players[player_index].money - cost >= 500) {
            build_hotel(game, i);
            built = 1;
          }
        }
      }
    }
  }
}

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

// Helper to get group name as string
const char* get_group_name(PropertyGroup group) {
  switch (group) {
    case GROUP_BROWN: return "Brown";
    case GROUP_LIGHT_BLUE: return "Light Blue";
    case GROUP_PINK: return "Pink";
    case GROUP_ORANGE: return "Orange";
    case GROUP_RED: return "Red";
    case GROUP_YELLOW: return "Yellow";
    case GROUP_GREEN: return "Green";
    case GROUP_DARK_BLUE: return "Dark Blue";
    default: return "None";
  }
}

// Update the property market every 10 rounds
void update_property_market(GameState *game) {
  // 1. Decrement cooldowns
  for (int i = 1; i <= 8; i++) {
    if (game->group_cooldowns[i] > 0) {
      game->group_cooldowns[i] -= 10;
    }
  }

  // 2. Find eligible groups (cooldown <= 0)
  int eligible_groups[8];
  int eligible_count = 0;
  for (int i = 1; i <= 8; i++) {
    if (game->group_cooldowns[i] <= 0) {
      eligible_groups[eligible_count++] = i;
    }
  }

  // Ensure we have at least 2 eligible groups to pick from
  if (eligible_count < 2) {
    printf("[MARKET UPDATE] Not enough eligible groups for market shift.\n");
    return;
  }

  // 3. Randomly select boom and decline groups
  int boom_idx = rand() % eligible_count;
  game->market_boom_group = (PropertyGroup)eligible_groups[boom_idx];

  int decline_idx;
  do {
    decline_idx = rand() % eligible_count;
  } while (decline_idx == boom_idx);
  
  game->market_decline_group = (PropertyGroup)eligible_groups[decline_idx];

  // 4. Set cooldowns to 30
  game->group_cooldowns[game->market_boom_group] = 30;
  game->group_cooldowns[game->market_decline_group] = 30;
  game->market_rounds_left = 10;

  // 5. Print announcement
  printf("\n======================================================\n");
  printf("[MARKET UPDATE] A major shift in the Property Market!\n");
  printf("MARKET BOOM: %s Group\n", get_group_name(game->market_boom_group));
  printf("  -> Prices +15%%, Rent +25%%, Mortgage +15%%, Build Cost +10%%\n");
  printf("MARKET DECLINE: %s Group\n", get_group_name(game->market_decline_group));
  printf("  -> Values -15%%, Rent -20%%, Mortgage -10%%, Auction -25%%\n");
  printf("======================================================\n\n");
}

// ----------------------------------------------------------------------------
// GETTER FUNCTIONS (Dynamically apply market buffs/debuffs)
// ----------------------------------------------------------------------------

double get_dynamic_price(GameState *game, int square_index) {
  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;
  
  double base = game->board[square_index].data.property.price;
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_boom_group) {
    return base * 1.15; // Boom: +15% Purchase Price
  }
  return base;
}

double get_dynamic_property_value(GameState *game, int square_index) {
  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;
  
  double base = game->board[square_index].data.property.price;
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_boom_group) {
    return base * 1.20; // Boom: +20% Property Value (for net worth/insurance)
  } else if (group == game->market_decline_group) {
    return base * 0.85; // Decline: -15% Property Value
  }
  return base;
}

double get_dynamic_rent(GameState *game, int square_index) {
  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;
  
  double base = game->board[square_index].data.property.base_rent;
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_boom_group) {
    return base * 1.25; // Boom: +25% Rental Income
  } else if (group == game->market_decline_group) {
    return base * 0.80; // Decline: -20% Rental Income
  }
  return base;
}

double get_dynamic_mortgage(GameState *game, int square_index) {
  if (game->board[square_index].type != SQUARE_PROPERTY) {
    // Railway and Utility mortgages are unaffected by property market
    if (game->board[square_index].type == SQUARE_RAILWAY) {
      return game->board[square_index].data.railway.mortgage_value;
    }
    if (game->board[square_index].type == SQUARE_UTILITY) {
      return game->board[square_index].data.utility.mortgage_value;
    }
    return 0;
  }
  
  double base = game->board[square_index].data.property.mortgage_value;
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_boom_group) {
    return base * 1.15; // Boom: +15% Mortgage Value
  } else if (group == game->market_decline_group) {
    return base * 0.90; // Decline: -10% Mortgage Value
  }
  return base;
}

double get_dynamic_build_cost(GameState *game, int square_index, int is_hotel) {
  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;
  
  double base;
  if (is_hotel) {
    base = game->board[square_index].data.property.hotel_cost;
  } else {
    base = game->board[square_index].data.property.house_cost;
  }
  
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_boom_group) {
    return base * 1.10; // Boom: +10% Construction Cost
  }
  return base;
}

double get_dynamic_auction_start(GameState *game, int square_index) {
  if (game->board[square_index].type != SQUARE_PROPERTY) {
    if (game->board[square_index].type == SQUARE_RAILWAY) {
      return game->board[square_index].data.railway.price * 0.5;
    }
    if (game->board[square_index].type == SQUARE_UTILITY) {
      return game->board[square_index].data.utility.price * 0.5;
    }
    return 0;
  }

  double base_price = game->board[square_index].data.property.price;
  double start_bid = base_price * 0.5; // Normal starting bid is 50% of price
  
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_decline_group) {
    return start_bid * 0.75; // Decline: -25% Auction starting price
  }
  return start_bid;
}

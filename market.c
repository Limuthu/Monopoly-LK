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

// Helpers for Economic Events
int is_coastal(PropertyGroup group) {
  return (group == GROUP_YELLOW || group == GROUP_LIGHT_BLUE || group == GROUP_ORANGE);
}

int is_commercial(int square_index) {
  // Pettah (1) and Maradana (3)
  return (square_index == 1 || square_index == 3);
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
    base *= 1.15; // Boom: +15% Purchase Price
  }
  
  // Apply Economic Event Modifiers
  if (game->active_economic_event == EVENT_TOURISM_BOOM && group == GROUP_YELLOW) {
    base *= 1.15;
  } else if (game->active_economic_event == EVENT_HEAVY_MONSOON && is_coastal(group)) {
    base *= 0.90;
  } else if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
    base *= 0.85;
  } else if (game->active_economic_event == EVENT_STOCK_MARKET_BOOM) {
    base *= 1.10;
  } else if (game->active_economic_event == EVENT_FOREIGN_INVESTMENT && is_commercial(square_index)) {
    base *= 1.20;
  }
  
  // Apply National Event Cards
  int owner_id = game->board[square_index].data.property.owner_id;
  if (owner_id != -1) {
    for (int i = 0; i < game->num_players; i++) {
      if (game->players[i].id == owner_id) {
        NationalEventCardType card = game->players[i].active_national_card;
        if (card == NATIONAL_CARD_STOCK_MARKET_RISE) base *= 1.10;
        if (card == NATIONAL_CARD_ECONOMIC_DOWNTURN) base *= 0.85;
        if (card == NATIONAL_CARD_FOREIGN_FUNDING && is_commercial(square_index)) base *= 1.15;
        if (card == NATIONAL_CARD_PROPERTY_REVALUATION && group == game->players[i].revalued_group) base *= 1.15;
        break;
      }
    }
  }
  
  // 3. Apply Regional Development Modifiers
  if (game->active_regional_card == CARD_PORT_CITY_EXPANSION && (square_index == 1 || square_index == 3)) { // Port City Expansion: Pettah and Maradana (Values +25%)
    base *= 1.25;
  } else if (game->active_regional_card == CARD_IT_INDUSTRY_GROWTH && group == GROUP_PINK) {
    base *= 1.20;
  } else if (game->active_regional_card == CARD_NORTHERN_DEV_PROGRAMME && group == GROUP_GREEN) {
    base *= 1.30;
  } else if (game->active_regional_card == CARD_TEA_EXPORT_BOOM && square_index == 37) { // Nuwara Eliya
    base *= 1.35;
  } else if (game->active_regional_card == CARD_UNIVERSITY_CITY_GROWTH && (square_index == 21 || square_index == 23)) { // Kandy City, Peradeniya
    base *= 1.20;
  } else if (game->active_regional_card == CARD_FLOOD_DAMAGE && is_coastal(group)) {
    base *= 0.80;
  } else if (game->active_regional_card == CARD_WATER_SHORTAGE && (square_index == 26 || square_index == 27 || square_index == 29)) { // Surrounding Water Board
    base *= 0.90;
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
  if (game->board[square_index].data.property.closed_rounds_left > 0) return 0;
  
  double base = game->board[square_index].data.property.base_rent;
  PropertyGroup group = game->board[square_index].data.property.group;

  if (group == game->market_boom_group) {
    base *= 1.25; // Boom: +25% Rental Income
  } else if (group == game->market_decline_group) {
    base *= 0.80; // Decline: -20% Rental Income
  }
  
  // Apply Economic Event Modifiers
  if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
    base *= 0.90;
  } else if (game->active_economic_event == EVENT_TOURISM_BOOM) {
    if (game->board[square_index].data.property.has_hotel) {
      base *= 2.0; // Hotel rent doubles
    }
  } else if (game->active_economic_event == EVENT_POLITICAL_UNREST) {
    if (game->board[square_index].data.property.has_hotel) {
      base *= 0.5; // Hotel rent drops 50%
    }
  }
  
  // Apply National Event Cards
  int owner_id = game->board[square_index].data.property.owner_id;
  if (owner_id != -1) {
    for (int i = 0; i < game->num_players; i++) {
      if (game->players[i].id == owner_id) {
        NationalEventCardType card = game->players[i].active_national_card;
        if (card == NATIONAL_CARD_TOURISM_HYPE && game->board[square_index].data.property.has_hotel) base *= 2.0;
        if (card == NATIONAL_CARD_FESTIVAL_SEASON && game->board[square_index].data.property.has_hotel) base *= 1.5;
        break;
      }
    }
  }
  
  // 3. Apply Regional Development Modifiers
  if (game->active_regional_card == CARD_SOUTHERN_TOURISM_BOOM && group == GROUP_YELLOW) {
    base *= 1.40;
  } else if (game->active_regional_card == CARD_AIRPORT_EXPANSION && group == GROUP_ORANGE) {
    base *= 1.30;
  } else if (game->active_regional_card == CARD_BEACH_POLLUTION && group == GROUP_YELLOW) {
    base *= 0.70;
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
    base *= 1.10; // Boom: +10% Construction Cost
  }
  
  // Apply Economic Event Modifiers
  if (game->active_economic_event == EVENT_FUEL_CRISIS) {
    base *= 1.20; // +20% development cost
  } else if (game->active_economic_event == EVENT_GOV_HOUSING && !is_hotel) {
    base *= 0.75; // -25% house cost
  }
  
  // Apply National Event Cards
  int owner_id = game->board[square_index].data.property.owner_id;
  if (owner_id != -1) {
    for (int i = 0; i < game->num_players; i++) {
      if (game->players[i].id == owner_id) {
        NationalEventCardType card = game->players[i].active_national_card;
        if (card == NATIONAL_CARD_HOUSING_SUBSIDY && !is_hotel) base *= 0.70;
        if (card == NATIONAL_CARD_CURRENCY_DEPRECIATION) base *= 1.10;
        break;
      }
    }
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

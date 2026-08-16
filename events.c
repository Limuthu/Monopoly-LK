#include "types.h"
#include <stdio.h>
#include <stdlib.h>

// External function declarations
double get_dynamic_rent(GameState *game, int square_index);
double get_dynamic_price(GameState *game, int square_index);
double get_dynamic_property_value(GameState *game, int square_index);
int find_player_index(GameState *game, int player_id);
const char* get_insurance_name(int tier);

// ----NATURAL DISASTERS & REPAIRS----

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

void attempt_disaster_repairs(GameState *game, int player_id) {
  int player_index = find_player_index(game, player_id);
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
    if (r < 50) type = DISASTER_FLOOD;
    else type = (DisasterType)(rand() % 5); 
  } else if (game->active_economic_event == EVENT_POLITICAL_UNREST) {
    if (r < 40) type = DISASTER_RIOT;
    else type = (DisasterType)(rand() % 5);
  } else {
    type = (DisasterType)(rand() % 5);
  }
  
  PropertyData *prop = &game->board[target_index].data.property;
  int owner_idx = find_player_index(game, prop->owner_id);
  if (owner_idx == -1) return;

  // Calculate repair cost (50% of total built value)
  double built_value = (prop->house_cost * prop->num_houses) + (prop->hotel_cost * prop->has_hotel);
  double repair_cost = built_value * 0.50;

  printf("\n======================================================\n");
  printf("DISASTER A %s has struck %s!\n", get_disaster_name(type), game->board[target_index].name);
  printf("Total Repair Cost: LKR %.0lf\n", repair_cost);

  // Determine coverage
  double compensation = 0;
  int is_covered = 0;

  if (prop->is_insured && prop->insurance_rounds_left > 0) {
    if (prop->insurance_tier == 1) {
      if (type == DISASTER_FIRE || type == DISASTER_FLOOD) {
        is_covered = 1;
        compensation = repair_cost * 0.80;
      }
    } else if (prop->insurance_tier == 2) {
      is_covered = 1;
      compensation = repair_cost * 1.00;
    } else if (prop->insurance_tier == 3) {
      is_covered = 1;
      int rent_rounds = 5;
      if (game->active_economic_event == EVENT_POLITICAL_UNREST) {
        rent_rounds = 10;
      }
      compensation = repair_cost * 1.00 + (get_dynamic_rent(game, target_index) * rent_rounds);
    }
  }

  if (is_covered) {
    printf("Insurance (%s) covers the disaster!\n", get_insurance_name(prop->insurance_tier));
    printf("Payout credited to %s: LKR %.0lf\n", game->players[owner_idx].name, compensation);
    
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
    
    if (prop->has_structural_damage) {
      prop->price /= 0.85;
      prop->base_rent /= 0.75;
      prop->has_structural_damage = 0;
    }
    prop->building_condition = 100.0;
    prop->rounds_without_maint = 0;

    printf("%s paid the repair cost. The property remains fully functional.\n", game->players[owner_idx].name);
  } else {
    prop->is_damaged = 1;
    prop->pending_repair_cost = repair_cost;
    printf(">> WARNING: %s cannot afford the repair! %s is damaged and cannot collect rent until repaired.\n",
           game->players[owner_idx].name, game->board[target_index].name);
  }
  printf("======================================================\n\n");
}

// ECONOMIC EVENTS

const char* get_economic_event_name(EconomicEventType type) {
  switch (type) {
    case EVENT_TOURISM_BOOM: return "Tourism Boom";
    case EVENT_FUEL_CRISIS: return "Fuel Crisis";
    case EVENT_HEAVY_MONSOON: return "Heavy Monsoon";
    case EVENT_ECONOMIC_RECESSION: return "Economic Recession";
    case EVENT_STOCK_MARKET_BOOM: return "Stock Market Boom";
    case EVENT_GOV_HOUSING: return "Government Housing Programme";
    case EVENT_FOREIGN_INVESTMENT: return "Foreign Investment";
    case EVENT_POLITICAL_UNREST: return "Political Unrest";
    default: return "None";
  }
}

void trigger_economic_event(GameState *game) {
  game->active_economic_event = (EconomicEventType)((rand() % 8) + 1);
  game->economic_event_rounds_left = 15;

  printf("\n======================================================\n");
  printf("ECONOMIC EVENT A new national economic phase has begun!\n");
  printf("ACTIVE EVENT: %s\n", get_economic_event_name(game->active_economic_event));
  
  switch (game->active_economic_event) {
    case EVENT_TOURISM_BOOM:
      printf("Hotel rent x2\n");
      printf("Southern Coastal Properties (Yellow) Value +15%%\n");
      break;
    case EVENT_FUEL_CRISIS:
      printf("Railway rent x2\n");
      printf("Property Development Costs +20%%\n");
      break;
    case EVENT_HEAVY_MONSOON:
      printf("Flood Disaster probability increased\n");
      printf("Global Insurance Premiums +50%%\n");
      printf("Coastal Properties Value -10%%\n");
      break;
    case EVENT_ECONOMIC_RECESSION:
      printf("Global Property Values -15%%\n");
      printf("Global Rental Incomes -10%%\n");
      printf("Prevailing Loan Interest Rate +15%%\n");
      break;
    case EVENT_STOCK_MARKET_BOOM:
      printf("Global Property Values +10%%\n");
      printf("Prevailing Loan Interest Rate -10%%\n");
      break;
    case EVENT_GOV_HOUSING:
      printf("House Construction Costs -25%%\n");
      break;
    case EVENT_FOREIGN_INVESTMENT:
      printf("Commercial Properties Value +20%%\n");
      break;
    case EVENT_POLITICAL_UNREST:
      printf("Riot Disaster probability x2\n");
      printf("Hotel rent drops 50%%\n");
      printf("Business Interruption claims increased\n");
      break;
    default:
      break;
  }
  printf("======================================================\n\n");
}

// 3. GOVERNMENT REGULATIONS

const char* get_government_regulation_name(GovernmentRegulationType type) {
  switch (type) {
    case REGULATION_INCREASE_PROPERTY_TAX: return "Increase Property Tax";
    case REGULATION_REDUCE_LOAN_INTEREST: return "Reduce Loan Interest";
    case REGULATION_HOUSING_SUBSIDY: return "Housing Subsidy";
    case REGULATION_LUXURY_PROPERTY_TAX: return "Luxury Property Tax";
    case REGULATION_RAILWAY_MODERNIZATION: return "Railway Modernization";
    case REGULATION_ELECTRICITY_TARIFF: return "Electricity Tariff Revision";
    case REGULATION_INSURANCE_REGULATION: return "Insurance Regulation";
    case REGULATION_ANTI_SPECULATION: return "Anti-Speculation Act";
    default: return "None";
  }
}

void trigger_government_regulation(GameState *game) {
  game->active_regulation = (GovernmentRegulationType)((rand() % 8) + 1);
  game->regulation_rounds_left = 20;

  printf("\n======================================================\n");
  printf("GOVERNMENT REGULATION The Sri Lankan Government has announced a new regulation!\n");
  printf("New Regulation: %s\n", get_government_regulation_name(game->active_regulation));

  // Handle immediate effects
  if (game->active_regulation == REGULATION_LUXURY_PROPERTY_TAX) {
    printf("  -> Immediate 25%% tax on all luxury properties (hotels)!\n");
    for (int i = 0; i < TOTAL_SQUARES; i++) {
      if (game->board[i].type == SQUARE_PROPERTY && game->board[i].data.property.has_hotel) {
        int owner_id = game->board[i].data.property.owner_id;
        if (owner_id != -1) {
          int owner_index = find_player_index(game, owner_id);
          if (owner_index != -1) {
            double property_value = get_dynamic_property_value(game, i);
            double tax = property_value * 0.25;
            game->players[owner_index].money -= tax;
            printf("  -> %s paid LKR %.0lf tax for hotel on %s.\n", game->players[owner_index].name, tax, game->board[i].name);
          }
        }
      }
    }
  }

  printf("======================================================\n\n");
}

// REGIONAL DEVELOPMENT CARDS

const char* get_regional_card_name(RegionalCardType type) {
  switch (type) {
    case CARD_SOUTHERN_TOURISM_BOOM: return "Southern Tourism Boom";
    case CARD_PORT_CITY_EXPANSION: return "Port City Expansion";
    case CARD_IT_INDUSTRY_GROWTH: return "IT Industry Growth";
    case CARD_NORTHERN_DEV_PROGRAMME: return "Northern Development Programme";
    case CARD_TEA_EXPORT_BOOM: return "Tea Export Boom";
    case CARD_AIRPORT_EXPANSION: return "Airport Expansion";
    case CARD_UNIVERSITY_CITY_GROWTH: return "University City Growth";
    case CARD_BEACH_POLLUTION: return "Beach Pollution";
    case CARD_FLOOD_DAMAGE: return "Flood Damage";
    case CARD_TRANSPORT_STRIKE: return "Transport Strike";
    case CARD_ELECTRICITY_TARIFF: return "Electricity Tariff Increase";
    case CARD_WATER_SHORTAGE: return "Water Shortage";
    default: return "None";
  }
}

void trigger_regional_development(GameState *game) {
  game->active_regional_card = (RegionalCardType)((rand() % 12) + 1);
  game->regional_card_rounds_left = 15;

  printf("\n======================================================\n");
  printf("REGIONAL DEVELOPMENT A new regional development card has been drawn!\n");
  printf("ACTIVE CARD: %s\n", get_regional_card_name(game->active_regional_card));
  
  switch (game->active_regional_card) {
    case CARD_SOUTHERN_TOURISM_BOOM:
      printf("Galle Fort, Unawatuna and Hikkaduwa rental income +40%%\n");
      break;
    case CARD_PORT_CITY_EXPANSION:
      printf("Pettah, Maradana and Colombo Fort Station values +25%%\n");
      break;
    case CARD_IT_INDUSTRY_GROWTH:
      printf("Maharagama, Nugegoda and Kottawa values +20%%\n");
      break;
    case CARD_NORTHERN_DEV_PROGRAMME:
      printf("Jaffna Town, Nallur and Trincomalee values +30%%\n");
      break;
    case CARD_TEA_EXPORT_BOOM:
      printf("Nuwara Eliya value +35%%\n");
      break;
    case CARD_AIRPORT_EXPANSION:
      printf("Negombo, Katunayake and Ja-Ela rents +30%%\n");
      break;
    case CARD_UNIVERSITY_CITY_GROWTH:
      printf("Peradeniya and Kandy City values +20%%\n");
      break;
    case CARD_BEACH_POLLUTION:
      printf("Southern coastal rents -30%%\n");
      break;
    case CARD_FLOOD_DAMAGE:
      printf("Low-lying coastal properties lose 20%% value\n");
      break;
    case CARD_TRANSPORT_STRIKE:
      printf("Railway revenue reduced by 40%%\n");
      break;
    case CARD_ELECTRICITY_TARIFF:
      printf("Utility rent +25%%\n");
      break;
    case CARD_WATER_SHORTAGE:
      printf("Water utility revenue +20%%; surrounding properties -10%%\n");
      break;
    default:
      break;
  }
  printf("======================================================\n\n");
}

// --- NATIONAL EVENT CARDS ----

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
  
  game->national_deck_index = (game->national_deck_index + 1) % 20;
  
  switch (card) {
    case NATIONAL_CARD_HEAVY_FLOODS: {
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
        if (repair_cost == 0) repair_cost = prop->price * 0.20; 
        prop->is_damaged = 1;
        prop->pending_repair_cost = repair_cost;
        printf("EVENT Heavy Floods! %s is damaged. Repair cost: LKR %.0lf\n", game->board[target].name, repair_cost);
      } else {
        printf("EVENT Heavy Floods! But %s owns no coastal properties.\n", p->name);
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
        printf("EVENT Political Rally! %s is closed for 2 rounds.\n", game->board[target].name);
      } else {
        printf("EVENT Political Rally! But %s owns no properties.\n", p->name);
      }
      break;
    }
    case NATIONAL_CARD_TAX_AMNESTY: {
      printf("EVENT Tax Amnesty! Each player receives LKR 2,000.\n");
      for (int i = 0; i < game->num_players; i++) {
        if (!game->players[i].is_bankrupt) {
          game->players[i].money += 2000;
        }
      }
      break;
    }
    case NATIONAL_CARD_GOVERNMENT_GRANT: {
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
              printf("EVENT Government Grant! %s receives LKR 5,000.\n", game->players[target].name);
          }
      }
      break;
    }
    case NATIONAL_CARD_NATIONAL_DISASTER: {
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
        printf("EVENT National Disaster! %s is damaged. Repair cost: LKR %.0lf\n", game->board[target].name, repair_cost);
      } else {
        printf("EVENT National Disaster! But %s has no developed properties.\n", p->name);
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
          printf("EVENT Property Revaluation! One of %s's property groups appreciates by 15%% for 15 rounds.\n", p->name);
      } else {
          p->revalued_group = GROUP_NONE;
          printf("EVENT Property Revaluation! But %s owns no properties.\n", p->name);
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
      printf("EVENT %s modifier applies for %d rounds.\n", get_national_card_name(card), p->national_card_rounds_left);
      break;
    }
  }
  printf("--------------------------------------------\n");
}

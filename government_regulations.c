#include "types.h"
#include <stdio.h>
#include <stdlib.h>

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

int find_player_index(GameState *game, int player_id);
double get_dynamic_property_value(GameState *game, int square_index);

void trigger_government_regulation(GameState *game) {
  // Pick random regulation 1 to 8
  game->active_regulation = (GovernmentRegulationType)((rand() % 8) + 1);
  game->regulation_rounds_left = 20;

  printf("\n======================================================\n");
  printf("[GOVERNMENT REGULATION] The Sri Lankan Government has announced a new regulation!\n");
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

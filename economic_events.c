#include "types.h"
#include <stdio.h>
#include <stdlib.h>

// Helper
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

// ----------------------------------------------------------------------------
// ECONOMIC EVENTS LOGIC
// ----------------------------------------------------------------------------

void trigger_economic_event(GameState *game) {
  // Generate random event 1-8
  game->active_economic_event = (EconomicEventType)((rand() % 8) + 1);
  game->economic_event_rounds_left = 15;

  printf("\n======================================================\n");
  printf("[ECONOMIC EVENT] A new national economic phase has begun!\n");
  printf("ACTIVE EVENT: %s\n", get_economic_event_name(game->active_economic_event));
  
  switch (game->active_economic_event) {
    case EVENT_TOURISM_BOOM:
      printf("  -> Hotel rent x2\n");
      printf("  -> Southern Coastal Properties (Yellow) Value +15%%\n");
      break;
    case EVENT_FUEL_CRISIS:
      printf("  -> Railway rent x2\n");
      printf("  -> Property Development Costs +20%%\n");
      break;
    case EVENT_HEAVY_MONSOON:
      printf("  -> Flood Disaster probability increased\n");
      printf("  -> Global Insurance Premiums +50%%\n");
      printf("  -> Coastal Properties Value -10%%\n");
      break;
    case EVENT_ECONOMIC_RECESSION:
      printf("  -> Global Property Values -15%%\n");
      printf("  -> Global Rental Incomes -10%%\n");
      printf("  -> Prevailing Loan Interest Rate +15%%\n");
      break;
    case EVENT_STOCK_MARKET_BOOM:
      printf("  -> Global Property Values +10%%\n");
      printf("  -> Prevailing Loan Interest Rate -10%%\n");
      break;
    case EVENT_GOV_HOUSING:
      printf("  -> House Construction Costs -25%%\n");
      break;
    case EVENT_FOREIGN_INVESTMENT:
      printf("  -> Commercial Properties Value +20%%\n");
      break;
    case EVENT_POLITICAL_UNREST:
      printf("  -> Riot Disaster probability x2\n");
      printf("  -> Hotel rent drops 50%%\n");
      printf("  -> Business Interruption claims increased\n");
      break;
    default:
      break;
  }
  printf("======================================================\n\n");
}

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

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
  // Generate random card 1-12
  game->active_regional_card = (RegionalCardType)((rand() % 12) + 1);
  game->regional_card_rounds_left = 15;

  printf("\n======================================================\n");
  printf("[REGIONAL DEVELOPMENT] A new regional development card has been drawn!\n");
  printf("ACTIVE CARD: %s\n", get_regional_card_name(game->active_regional_card));
  
  switch (game->active_regional_card) {
    case CARD_SOUTHERN_TOURISM_BOOM:
      printf("  -> Galle Fort, Unawatuna and Hikkaduwa rental income +40%%\n");
      break;
    case CARD_PORT_CITY_EXPANSION:
      printf("  -> Pettah, Maradana and Colombo Fort Station values +25%%\n");
      break;
    case CARD_IT_INDUSTRY_GROWTH:
      printf("  -> Maharagama, Nugegoda and Kottawa values +20%%\n");
      break;
    case CARD_NORTHERN_DEV_PROGRAMME:
      printf("  -> Jaffna Town, Nallur and Trincomalee values +30%%\n");
      break;
    case CARD_TEA_EXPORT_BOOM:
      printf("  -> Nuwara Eliya value +35%%\n");
      break;
    case CARD_AIRPORT_EXPANSION:
      printf("  -> Negombo, Katunayake and Ja-Ela rents +30%%\n");
      break;
    case CARD_UNIVERSITY_CITY_GROWTH:
      printf("  -> Peradeniya and Kandy City values +20%%\n");
      break;
    case CARD_BEACH_POLLUTION:
      printf("  -> Southern coastal rents -30%%\n");
      break;
    case CARD_FLOOD_DAMAGE:
      printf("  -> Low-lying coastal properties lose 20%% value\n");
      break;
    case CARD_TRANSPORT_STRIKE:
      printf("  -> Railway revenue reduced by 40%%\n");
      break;
    case CARD_ELECTRICITY_TARIFF:
      printf("  -> Utility rent +25%%\n");
      break;
    case CARD_WATER_SHORTAGE:
      printf("  -> Water utility revenue +20%%; surrounding properties -10%%\n");
      break;
    default:
      break;
  }
  printf("======================================================\n\n");
}

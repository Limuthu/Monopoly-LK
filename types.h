#ifndef TYPES_H
#define TYPES_H

#define TOTAL_SQUARES 40
#define MAX_PLAYERS 4

// Square types

typedef enum {
  SQUARE_START,
  SQUARE_PROPERTY,
  SQUARE_RAILWAY,
  SQUARE_UTILITY,
  SQUARE_TAX,
  SQUARE_EVENT,
  SQUARE_JAIL,
  SQUARE_SPECIAL,
  SQUARE_BANK,
  SQUARE_INSURANCE
} SquareType;

// Disaster types
typedef enum {
  DISASTER_FIRE,
  DISASTER_FLOOD,
  DISASTER_RIOT,
  DISASTER_BUILDING_COLLAPSE, 
  DISASTER_ELECTRICAL_FAILURE 
} DisasterType;

// Economic Event types
typedef enum {
  EVENT_NONE,
  EVENT_TOURISM_BOOM,
  EVENT_FUEL_CRISIS,
  EVENT_HEAVY_MONSOON,
  EVENT_ECONOMIC_RECESSION,
  EVENT_STOCK_MARKET_BOOM,
  EVENT_GOV_HOUSING,
  EVENT_FOREIGN_INVESTMENT,
  EVENT_POLITICAL_UNREST
} EconomicEventType;

// Regional Development Cards
typedef enum {
  CARD_NONE,
  CARD_SOUTHERN_TOURISM_BOOM,
  CARD_PORT_CITY_EXPANSION,
  CARD_IT_INDUSTRY_GROWTH,
  CARD_NORTHERN_DEV_PROGRAMME,
  CARD_TEA_EXPORT_BOOM,
  CARD_AIRPORT_EXPANSION,
  CARD_UNIVERSITY_CITY_GROWTH,
  CARD_BEACH_POLLUTION,
  CARD_FLOOD_DAMAGE,
  CARD_TRANSPORT_STRIKE,
  CARD_ELECTRICITY_TARIFF,
  CARD_WATER_SHORTAGE
} RegionalCardType;

// National Event Cards
typedef enum {
  NATIONAL_CARD_NONE,
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
} NationalEventCardType;

// Government Regulations
typedef enum {
  REGULATION_NONE,
  REGULATION_INCREASE_PROPERTY_TAX,
  REGULATION_REDUCE_LOAN_INTEREST,
  REGULATION_HOUSING_SUBSIDY,
  REGULATION_LUXURY_PROPERTY_TAX,
  REGULATION_RAILWAY_MODERNIZATION,
  REGULATION_ELECTRICITY_TARIFF,
  REGULATION_INSURANCE_REGULATION,
  REGULATION_ANTI_SPECULATION
} GovernmentRegulationType;

// Group types for properties

typedef enum {
  GROUP_NONE,
  GROUP_BROWN,
  GROUP_LIGHT_BLUE,
  GROUP_PINK,
  GROUP_ORANGE,
  GROUP_RED,
  GROUP_YELLOW,
  GROUP_GREEN,
  GROUP_DARK_BLUE
} PropertyGroup;

// Property data structure

typedef struct {
  double price;
  double mortgage_value;
  double base_rent;
  double house_cost;
  double hotel_cost;
  int num_houses;
  int has_hotel;
  int owner_id;

  int is_mortgaged;
  int is_insured;
  int insurance_tier;          
  int insurance_rounds_left;   // Starts at 20 when purchased
  int is_damaged;              
  double pending_repair_cost;  
  double pending_insurance_payout; 
  int is_loan_locked;
  PropertyGroup group;

  // Property Depreciation 
  int    prop_age;               // Rounds since last renovation
  double depreciation_pct;      
  double base_rent_original;     

  // Building Depreciation 
  double building_condition;     // Drops 2 perround.
  int    rounds_without_maint;   // > 20 make structural damage.
  int    has_structural_damage;  
  int    closed_rounds_left;     // National Event: Political Rally
  int    forced_development_rounds_left; // Anti-Speculation Act deadline
} PropertyData;

// Railway data structure

typedef struct {
  double price;
  double mortgage_value;   
  int owner_id;
  int is_mortgaged;
  int is_loan_locked;
  int closed_rounds_left;
} RailwayData;

// utility data structure

typedef struct {
  double price;
  double mortgage_value;   
  int owner_id;
  int is_mortgaged;
  int is_loan_locked;
  int closed_rounds_left;
} UtilityData;

// Square data structure

typedef struct {
  int index;
  char name[60];
  SquareType type;
  int owner_id;
  union {
    PropertyData property;
    RailwayData railway;
    UtilityData utility;
  } data;
} Square;

typedef struct {
  int id;
  char name[30];
  double money;
  int position;
  int in_jail;
  int jail_turns;
  int roll_result;

  int has_loan;            
  double loan_amount;      
  int loan_rounds_left;   
  int loan_start_round;
  double loan_interest_rate;
  int is_bankrupt;
  int passed_go_this_round;
  double insurance_claims_receivable;
  double taxes_due;

  // National Event Tracking
  NationalEventCardType active_national_card;
  int national_card_rounds_left;
  PropertyGroup revalued_group;
  int construction_suspended;
} Player;

typedef struct {
  Square board[TOTAL_SQUARES];
  Player players[MAX_PLAYERS];
  int num_players;
  int current_turn;
  double bank_money;

  // Inflation
  double current_inflation_rate;
  double current_interest_rate;
  double railway_rent_base[4];
  double utility_rent_base[2];

  // Dynamic Property Market
  PropertyGroup market_boom_group;
  PropertyGroup market_decline_group;
  int market_rounds_left;
  int group_cooldowns[9]; 

  // Economic Events
  EconomicEventType active_economic_event;
  int economic_event_rounds_left;

  // Regional Development
  RegionalCardType active_regional_card;
  int regional_card_rounds_left;

  // National Event Deck
  NationalEventCardType national_deck[20];
  int national_deck_index;
  
  // Government Regulations
  GovernmentRegulationType active_regulation;
  int regulation_rounds_left;
} GameState;

#endif

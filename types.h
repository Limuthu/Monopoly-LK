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
  PropertyGroup group;
} PropertyData;

// Railway data structure

typedef struct {
  double price;
  int owner_id;
  int is_mortgaged;
} RailwayData;

// utility data structure

typedef struct {
  double price;
  int owner_id;
  int is_mortgaged;
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
} Player;

typedef struct {
  Square board[TOTAL_SQUARES];
  Player players[MAX_PLAYERS];
  int num_players;
  int current_turn;
  double bank_money;
} GameState;

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
    #define CLEAR_SCREEN() system("cls")
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
    #define ENABLE_ANSI() SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    #define CLEAR_SCREEN() system("clear")
    #define ENABLE_ANSI()
#endif

#define COLOR_RESET   "\x1B[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"
#define COLOR_BOLD    "\x1B[1m"

#define MAX_NAME_LENGTH 50
#define MAX_PLAYERS 8
#define WINNING_SCORE 100
#define AI_MIN_BANK 20
#define INPUT_BUFFER_SIZE 256

typedef struct {
    char name[MAX_NAME_LENGTH];
    uint16_t bankScore;
    uint8_t isAI;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    uint8_t playerCount;
    uint8_t currentPlayer;
    uint16_t turnScore;
    uint8_t gameOver;
} GameState;

uint8_t rollDice(void);
void clearInputBuffer(void);
char* getSecureInput(char* buffer, size_t size);
void addAIPlayer(GameState* game, uint8_t number);
void displayScoreboard(const GameState* game);
char getPlayerChoice(void);
void nextPlayer(GameState* game);
uint8_t checkWinner(GameState* game);
void displayTurnStatus(const GameState* game);
void playAITurn(GameState* game);
void playHumanTurn(GameState* game);
void playTurn(GameState* game);
void setupPlayers(GameState* game);
void initializeGame(GameState* game, int argc, char* argv[]);

/*
    {
        FILE *urandom = fopen("/dev/urandom", "rb");
        unsigned int seed;
        if (urandom && fread(&seed, sizeof(seed), 1, urandom) == 1) {
            srand(seed);
            fclose(urandom);
        } else {
            seed = (unsigned int)time(NULL) ^ (unsigned int)clock() ^ (unsigned int)getpid();
            srand(seed);
        }
    }
*/

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));
    
    ENABLE_ANSI();
    
    printf(COLOR_BOLD COLOR_CYAN "Bienvenu dans le jeux du cochon\n " COLOR_RESET);

    printf("\n" COLOR_BOLD COLOR_YELLOW "RULES:" COLOR_RESET "\n");
    printf("- Each turn, a player rolls a die until either a 1 is rolled or they decide to 'bank'.\n");
    printf("- If the player rolls a 1, they score nothing and it becomes the next player's turn.\n");
    printf("- If the player rolls any other number, it is added to their turn score.\n");
    printf("- If a player chooses to 'bank', their turn score is added to their total score.\n");
    printf("- The first player to reach %d points wins the game.\n\n", WINNING_SCORE);
    
    GameState game;
    initializeGame(&game, argc, argv);
    
    while (!game.gameOver) {
        displayScoreboard(&game);
        printf(COLOR_CYAN "\nPlayer %s, it's your turn!" COLOR_RESET "\n", game.players[game.currentPlayer].name);
        playTurn(&game);
        if (!checkWinner(&game)) {
            nextPlayer(&game);
        }
    }
    
    printf(COLOR_BOLD COLOR_GREEN "\n*** %s wins the game with %d points! ***" COLOR_RESET "\n",
        game.players[game.currentPlayer].name, 
        game.players[game.currentPlayer].bankScore);
    
    return 0;
}

uint8_t rollDice(void) {
    return (rand() % 6) + 1;
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char* getSecureInput(char* buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        return NULL;
    }
    
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        clearInputBuffer();
    }
    
    return buffer;
}

void addAIPlayer(GameState* game, uint8_t number) {
    if (game->playerCount >= MAX_PLAYERS) {
        return;
    }
    
    Player* newPlayer = &game->players[game->playerCount];
    snprintf(newPlayer->name, MAX_NAME_LENGTH, "Computer %d", number);
    newPlayer->bankScore = 0;
    newPlayer->isAI = 1;
    game->playerCount++;
}

void displayScoreboard(const GameState* game) {
    printf("\n" COLOR_BOLD COLOR_MAGENTA "=== SCORES ===" COLOR_RESET "\n");
    for (uint8_t i = 0; i < game->playerCount; i++) {
        if (i == game->currentPlayer) {
            printf(COLOR_GREEN "> %s: %d" COLOR_RESET, game->players[i].name, game->players[i].bankScore);
        } else {
            printf("  %s: %d", game->players[i].name, game->players[i].bankScore);
        }
        
        if (game->players[i].isAI) {
            printf(COLOR_CYAN " [AI]" COLOR_RESET);
        }
        printf("\n");
    }
}

char getPlayerChoice(void) {
    char choice;
    char inputBuffer[INPUT_BUFFER_SIZE];
    
    do {
        printf(COLOR_CYAN "    Continue [r]olling, or [b]ank points? " COLOR_RESET);
        if (getSecureInput(inputBuffer, INPUT_BUFFER_SIZE) == NULL) {
            continue;
        }
        
        if (strlen(inputBuffer) > 0) {
            choice = tolower((unsigned char)inputBuffer[0]);
        } else {
            choice = 0;
        }
    } while (choice != 'r' && choice != 'b');
    
    return choice;
}

void nextPlayer(GameState* game) {
    game->currentPlayer = (game->currentPlayer + 1) % game->playerCount;
}

uint8_t checkWinner(GameState* game) {
    game->gameOver = (game->players[game->currentPlayer].bankScore >= WINNING_SCORE);
    return game->gameOver;
}

void displayTurnStatus(const GameState* game) {
    printf(COLOR_YELLOW "    You have %d points in bank, and %d points for this turn." COLOR_RESET "\n",
        game->players[game->currentPlayer].bankScore,
        game->turnScore);
}

void playAITurn(GameState* game) {
    uint8_t diceValue;
    uint16_t targetScore = AI_MIN_BANK;
    uint16_t remainingScore = WINNING_SCORE - game->players[game->currentPlayer].bankScore;
    
    if (remainingScore < AI_MIN_BANK) {
        targetScore = remainingScore;
    }
    
    for (uint8_t i = 0; i < game->playerCount; i++) {
        if (i != game->currentPlayer && 
            game->players[i].bankScore > WINNING_SCORE - 25) {
            targetScore = 25;
            break;
        }
    }
    
    game->turnScore = 0;
    
    while (1) {
        SLEEP_MS(1000);
        
        diceValue = rollDice();
        printf(COLOR_CYAN "    %s rolled a %d" COLOR_RESET "\n", game->players[game->currentPlayer].name, diceValue);
        
        if (diceValue == 1) {
            printf(COLOR_RED "    No luck :( :(" COLOR_RESET "\n");
            printf("    %s has 0 points for this turn.\n", game->players[game->currentPlayer].name);
            game->turnScore = 0;
            break;
        }
        
        game->turnScore += diceValue;
        
        printf(COLOR_YELLOW "    %s has %d points in bank, and %d points for this turn." COLOR_RESET "\n",
            game->players[game->currentPlayer].name,
            game->players[game->currentPlayer].bankScore,
            game->turnScore);
        
        if (game->turnScore >= targetScore || 
            game->players[game->currentPlayer].bankScore + game->turnScore >= WINNING_SCORE) {
            printf(COLOR_GREEN "    %s banks %d points." COLOR_RESET "\n", 
                game->players[game->currentPlayer].name, game->turnScore);
            game->players[game->currentPlayer].bankScore += game->turnScore;
            break;
        } else {
            printf(COLOR_CYAN "    %s continues rolling." COLOR_RESET "\n", game->players[game->currentPlayer].name);
        }
    }
}

void playHumanTurn(GameState* game) {
    uint8_t diceValue;
    char choice;
    
    game->turnScore = 0;
    
    while (1) {
        diceValue = rollDice();
        printf(COLOR_CYAN "    You rolled a %d" COLOR_RESET "\n", diceValue);
        
        if (diceValue == 1) {
            printf(COLOR_RED "    No luck :( :(" COLOR_RESET "\n");
            printf(COLOR_YELLOW "    You have %d points in bank." COLOR_RESET "\n", 
                game->players[game->currentPlayer].bankScore);
            game->turnScore = 0;
            break;
        }
        
        game->turnScore += diceValue;
        
        displayTurnStatus(game);
        
        choice = getPlayerChoice();
        
        if (choice == 'b') {
            game->players[game->currentPlayer].bankScore += game->turnScore;
            printf(COLOR_GREEN "    You have %d points in bank." COLOR_RESET "\n", 
                game->players[game->currentPlayer].bankScore);
            break;
        }
    }
}

void playTurn(GameState* game) {
    if (game->players[game->currentPlayer].isAI) {
        playAITurn(game);
    } else {
        playHumanTurn(game);
    }
}

void setupPlayers(GameState* game) {
    char buffer[INPUT_BUFFER_SIZE];
    int humanPlayerCount = 0;
    int aiPlayerCount = 0;
    
    do {
        printf("Number of human players (1-%d): ", MAX_PLAYERS);
        if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL) {
            continue;
        }
        humanPlayerCount = atoi(buffer);
    } while (humanPlayerCount < 1 || humanPlayerCount > MAX_PLAYERS);
    
    if (humanPlayerCount == 1) {
        do {
            printf("Number of computer players (1-%d): ", MAX_PLAYERS - humanPlayerCount);
            if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL) {
                continue;
            }
            aiPlayerCount = atoi(buffer);
        } while (aiPlayerCount < 1 || aiPlayerCount > MAX_PLAYERS - humanPlayerCount);
    }
    
    game->playerCount = 0;
    
    for (int i = 0; i < humanPlayerCount; i++) {
        printf("Player %d name: ", i + 1);
        if (getSecureInput(game->players[game->playerCount].name, MAX_NAME_LENGTH) == NULL) {
            strncpy(game->players[game->playerCount].name, "Player", MAX_NAME_LENGTH - 1);
        }
        game->players[game->playerCount].bankScore = 0;
        game->players[game->playerCount].isAI = 0;
        game->playerCount++;
    }
    
    for (int i = 0; i < aiPlayerCount; i++) {
        addAIPlayer(game, i + 1);
    }
}


void initializeGame(GameState* game, int argc, char* argv[]) {
    memset(game, 0, sizeof(GameState));
    
    if (argc > 1) {
        game->playerCount = 0;
        
        for (int i = 1; i < argc && game->playerCount < MAX_PLAYERS; i++) {
            strncpy(game->players[game->playerCount].name, argv[i], MAX_NAME_LENGTH - 1);
            game->players[game->playerCount].name[MAX_NAME_LENGTH - 1] = '\0';
            game->players[game->playerCount].bankScore = 0;
            game->players[game->playerCount].isAI = 0;
            game->playerCount++;
        }
        
        if (game->playerCount == 1) {
            addAIPlayer(game, 1);
        }
    } else {
        setupPlayers(game);
    }
    
    game->currentPlayer = 0;
    game->turnScore = 0;
    game->gameOver = 0;
}
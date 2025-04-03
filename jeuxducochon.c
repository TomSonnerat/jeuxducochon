#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <locale.h>

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

typedef enum {
    LANG_EN,
    LANG_FR
} Language;

Language currentLanguage = LANG_EN;

const char* translate(const char* key) {
    if (currentLanguage == LANG_EN) {
        return key;
    }
    
    if (strcmp(key, "Error: invalid game pointer\n") == 0) return "Erreur : pointeur de jeu invalide\n";
    if (strcmp(key, "         PIG GAME              \n") == 0) return "         JEU DU COCHON              \n";
    if (strcmp(key, "Number of human players (0-%d): ") == 0) return "Nombre de joueurs humains (0-%d) : ";
    if (strcmp(key, "Number of computer players (0-%d): ") == 0) return "Nombre de joueurs ordinateur (0-%d) : ";
    if (strcmp(key, "Spectator mode activated with 2 computers\n") == 0) return "Mode spectateur activé avec 2 ordinateurs\n";
    if (strcmp(key, "Player %d name: ") == 0) return "Nom du joueur %d : ";
    if (strcmp(key, "Player %d") == 0) return "Joueur %d";
    if (strcmp(key, "Computer %d") == 0) return "Ordinateur %d";
    if (strcmp(key, "\nPlayers configured. Press Enter to continue...") == 0) return "\nJoueurs configurés. Appuyez sur Entrée pour continuer...";
    if (strcmp(key, "Error: invalid parameters\n") == 0) return "Erreur : paramètres invalides\n";
    if (strcmp(key, "Select strategy for %s:\n") == 0) return "Sélectionnez la stratégie pour %s :\n";
    if (strcmp(key, "1. Random\n") == 0) return "1. Aléatoire\n";
    if (strcmp(key, "2. Conservative (stops at 15)\n") == 0) return "2. Conservateur (s'arrête à 15)\n";
    if (strcmp(key, "3. Aggressive (stops at 25)\n") == 0) return "3. Agressif (s'arrête à 25)\n";
    if (strcmp(key, "4. Optimal (based on game state)\n") == 0) return "4. Optimal (basé sur l'état du jeu)\n";
    if (strcmp(key, "Choice: ") == 0) return "Choix : ";
    if (strcmp(key, "GAME MODE SELECTION:") == 0) return "SÉLECTION DU MODE DE JEU :";
    if (strcmp(key, "GAME RULES:") == 0) return "RÈGLES DU JEU :";
    if (strcmp(key, "CURRENT SCORES:") == 0) return "SCORES ACTUELS :";
    if (strcmp(key, " (AI)") == 0) return " (IA)";
    if (strcmp(key, " WINNER!") == 0) return " VAINQUEUR !";
    if (strcmp(key, "CONGRATULATIONS! %s wins with %d points!\n") == 0) return "FÉLICITATIONS ! %s gagne avec %d points !\n";
    if (strcmp(key, "\nThanks for playing Pig Game!\n") == 0) return "\nMerci d'avoir joué au Jeu du Cochon !\n";
    if (strcmp(key, "Turn of %s") == 0) return "Tour de %s";
    if (strcmp(key, "You rolled a 1! Bad luck, you lose your turn points.\n") == 0) return "Vous avez fait 1 ! Pas de chance, vous perdez les points du tour.\n";
    if (strcmp(key, "the side! Bad luck, you lose your turn points.\n") == 0) return "sur le côté ! Pas de chance, vous perdez les points du tour.\n";
    if (strcmp(key, "Piggyback! Game over!\n") == 0) return "Piggyback ! Fin de partie !\n";
    if (strcmp(key, "Makin' Bacon! You lose your turn points.\n") == 0) return "Makin' Bacon ! Vous perdez les points du tour.\n";
    if (strcmp(key, "You have enough points to win! Your points are automatically banked.\n") == 0) return "Vous avez assez de points pour gagner ! Vos points sont automatiquement sauvegardés.\n";
    if (strcmp(key, "You banked %d points! Your total score is now %d.\n") == 0) return "Vous avez sauvegardé %d points ! Votre score total est maintenant de %d.\n";
    if (strcmp(key, "Turn of %s (AI)") == 0) return "Tour de %s (IA)";
    if (strcmp(key, "AI rolled a 1! It loses its turn points.\n") == 0) return "L'IA a fait 1 ! Elle perd les points du tour.\n";
    if (strcmp(key, "the side! It loses its turn points.\n") == 0) return "sur le côté ! Elle perd les points du tour.\n";
    if (strcmp(key, "Makin' Bacon! AI loses its turn points.\n") == 0) return "Makin' Bacon ! L'IA perd les points du tour.\n";
    if (strcmp(key, "AI has enough points to win! Its points are automatically banked.\n") == 0) return "L'IA a assez de points pour gagner ! Ses points sont automatiquement sauvegardés.\n";
    if (strcmp(key, "AI banks %d points! Its total score is now %d.\n") == 0) return "L'IA sauvegarde %d points ! Son score total est maintenant de %d.\n";
    
    return key;
}

#define _(String) translate(String)

typedef enum {
    AI_STRATEGY_RANDOM,
    AI_STRATEGY_CONSERVATIVE,
    AI_STRATEGY_AGGRESSIVE,
    AI_STRATEGY_OPTIMAL
} AIStrategy;

typedef enum {
    GAME_MODE_STANDARD,
    GAME_MODE_ONE_PIG,
    GAME_MODE_TWO_PIGS
} GameMode;

typedef struct {
    char name[MAX_NAME_LENGTH];
    uint16_t bankScore;
    uint8_t isAI;
    AIStrategy strategy;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    uint8_t playerCount;
    uint8_t currentPlayer;
    uint16_t turnScore;
    uint8_t gameOver;
    GameMode mode;
} GameState;

void initGame(GameState* game);
void setupPlayers(GameState* game);
void selectGameMode(GameState* game);
void explainRules(GameMode mode);
void playGame(GameState* game);
void playTurn(GameState* game);
void playAITurn(GameState* game);
uint8_t rollDice(void);
uint8_t rollOnePig(void);
void rollTwoPigs(uint8_t* score, uint8_t* turnEnds, uint8_t* gameEnds);
void displayDice(uint8_t value);
void showLoadingAnimation(const char* message, int duration);
char* getSecureInput(char* buffer, size_t bufferSize);
void selectAIStrategy(GameState* game, uint8_t playerIndex);
uint16_t findHighestScore(const GameState* game);
void clearInputBuffer(void);
void selectLanguage(void);

int main(void) {
    srand((unsigned int)time(NULL));
    
    ENABLE_ANSI();
    
    selectLanguage();
    
    GameState game;
    initGame(&game);
    setupPlayers(&game);
    selectGameMode(&game);
    playGame(&game);
    
    return 0;
}

void selectLanguage(void) {
    char buffer[INPUT_BUFFER_SIZE];
    
    CLEAR_SCREEN();
    printf("%s%s%s\n", COLOR_BOLD, "LANGUAGE SELECTION / SÉLECTION DE LA LANGUE", COLOR_RESET);
    printf("1. English\n");
    printf("2. Français\n");
    printf("Choice / Choix: ");
    
    if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL || buffer[0] == '\0') {
        currentLanguage = LANG_EN;
    } else {
        int choice = atoi(buffer);
        if (choice == 2) {
            currentLanguage = LANG_FR;
        } else {
            currentLanguage = LANG_EN;
        }
    }
    
    printf("\n");
}

void initGame(GameState* game) {
    if (game == NULL) {
        fprintf(stderr, _("Error: invalid game pointer\n"));
        exit(EXIT_FAILURE);
    }
    
    memset(game, 0, sizeof(GameState));
    game->gameOver = 0;
    game->currentPlayer = 0;
    game->turnScore = 0;
    game->mode = GAME_MODE_STANDARD;
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

void setupPlayers(GameState* game) {
    char buffer[INPUT_BUFFER_SIZE];
    int humanPlayerCount = 0;
    int aiPlayerCount = 0;
    
    if (game == NULL) {
        fprintf(stderr, _("Error: invalid game pointer\n"));
        exit(EXIT_FAILURE);
    }
    
    printf(_("Number of human players (0-%d): "), MAX_PLAYERS);
    if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL) {
        humanPlayerCount = 1;
    } else {
        humanPlayerCount = atoi(buffer);
        if (humanPlayerCount < 0 || humanPlayerCount > MAX_PLAYERS) {
            humanPlayerCount = 1;
        }
    }
    
    if (humanPlayerCount < MAX_PLAYERS) {
        printf(_("Number of computer players (0-%d): "), MAX_PLAYERS - humanPlayerCount);
        if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL) {
            aiPlayerCount = 1;
        } else {
            aiPlayerCount = atoi(buffer);
            if (aiPlayerCount < 0 || aiPlayerCount > (MAX_PLAYERS - humanPlayerCount)) {
                aiPlayerCount = 1;
            }
        }
    }
    
    if (humanPlayerCount == 0 && aiPlayerCount == 0) {
        aiPlayerCount = 2;
        printf(_("Spectator mode activated with 2 computers\n"));
    }
    
    game->playerCount = (uint8_t)(humanPlayerCount + aiPlayerCount);
    
    for (uint8_t i = 0; i < humanPlayerCount; i++) {
        printf(_("Player %d name: "), i + 1);
        if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL || buffer[0] == '\0') {
            snprintf(game->players[i].name, MAX_NAME_LENGTH, _("Player %d"), i + 1);
        } else {
            strncpy(game->players[i].name, buffer, MAX_NAME_LENGTH - 1);
            game->players[i].name[MAX_NAME_LENGTH - 1] = '\0';
        }
        game->players[i].bankScore = 0;
        game->players[i].isAI = 0;
    }
    
    for (uint8_t i = 0; i < aiPlayerCount; i++) {
        uint8_t playerIndex = humanPlayerCount + i;
        snprintf(game->players[playerIndex].name, MAX_NAME_LENGTH, _("Computer %d"), i + 1);
        game->players[playerIndex].bankScore = 0;
        game->players[playerIndex].isAI = 1;
        selectAIStrategy(game, playerIndex);
    }
    
    printf(_("\nPlayers configured. Press Enter to continue..."));
    getSecureInput(buffer, INPUT_BUFFER_SIZE);
}

void selectAIStrategy(GameState* game, uint8_t playerIndex) {
    char buffer[INPUT_BUFFER_SIZE];
    int strategyChoice;
    
    if (game == NULL || playerIndex >= game->playerCount) {
        fprintf(stderr, _("Error: invalid parameters\n"));
        return;
    }
    
    printf(_("Select strategy for %s:\n"), game->players[playerIndex].name);
    printf(_("1. Random\n"));
    printf(_("2. Conservative (stops at 15)\n"));
    printf(_("3. Aggressive (stops at 25)\n"));
    printf(_("4. Optimal (based on game state)\n"));
    printf(_("Choice: "));
    
    if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL) {
        strategyChoice = 3;
    } else {
        strategyChoice = atoi(buffer);
        if (strategyChoice < 1 || strategyChoice > 4) {
            strategyChoice = 3;
        }
    }
    
    switch(strategyChoice) {
        case 1: game->players[playerIndex].strategy = AI_STRATEGY_RANDOM; break;
        case 2: game->players[playerIndex].strategy = AI_STRATEGY_CONSERVATIVE; break;
        case 3: game->players[playerIndex].strategy = AI_STRATEGY_AGGRESSIVE; break;
        case 4: game->players[playerIndex].strategy = AI_STRATEGY_OPTIMAL; break;
        default: game->players[playerIndex].strategy = AI_STRATEGY_AGGRESSIVE;
    }
}

void selectGameMode(GameState* game) {
    char buffer[INPUT_BUFFER_SIZE];
    int modeChoice;
    
    if (game == NULL) {
        fprintf(stderr, _("Error: invalid game pointer\n"));
        return;
    }
    
    printf("%s%s%s%s\n", COLOR_BOLD, COLOR_YELLOW, _("GAME MODE SELECTION:"), COLOR_RESET);
    printf("1. Standard Pig (one die, 1 = lose turn)\n");
    printf("2. One Pig variant (special positions)\n");
    printf("3. Pass the Pigs (two pigs with special positions)\n");
    printf("Choice: ");
    
    if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL) {
        modeChoice = 1;
    } else {
        modeChoice = atoi(buffer);
        if (modeChoice < 1 || modeChoice > 3) {
            modeChoice = 1;
        }
    }
    
    switch(modeChoice) {
        case 2: game->mode = GAME_MODE_ONE_PIG; break;
        case 3: game->mode = GAME_MODE_TWO_PIGS; break;
        default: game->mode = GAME_MODE_STANDARD;
    }
    
    explainRules(game->mode);
}

void explainRules(GameMode mode) {
    char buffer[INPUT_BUFFER_SIZE];
    
    printf("%s%s%s%s\n\n", COLOR_BOLD, COLOR_YELLOW, _("GAME RULES:"), COLOR_RESET);
    
    switch(mode) {
        case GAME_MODE_STANDARD:
            printf("Standard mode:\n");
            printf("- Each turn, roll a 6-sided die\n");
            printf("- If you roll 2-6, add that number to your turn score\n");
            printf("- If you roll 1, you lose all points for this turn\n");
            printf("- You can choose to continue rolling or bank your points\n");
            printf("- First player to reach %d points wins\n", WINNING_SCORE);
            break;
            
        case GAME_MODE_ONE_PIG:
            printf("One Pig mode:\n");
            printf("- Each turn, roll a pig that can land in different positions\n");
            printf("- Side (35%%): 0 points\n");
            printf("- Back (30%%): 5 points\n");
            printf("- Standing (20%%): 5 points\n");
            printf("- Snout (10%%): 10 points\n");
            printf("- Ear (5%%): 15 points\n");
            printf("- If you get a side, you lose all points for this turn\n");
            break;
            
        case GAME_MODE_TWO_PIGS:
            printf("Two Pigs mode:\n");
            printf("- Each turn, roll two pigs\n");
            printf("- Points depend on the combination of positions\n");
            printf("- Two pigs on side: 1 point (Sider) or 0 points (Pig Out)\n");
            printf("- One pig on side: sum of position points\n");
            printf("- Same positions: points x4\n");
            printf("- Different positions: sum of points\n");
            printf("- Pigs touching: Makin' Bacon (lose turn and points)\n");
            printf("- Pigs stacked: Piggyback (game over)\n");
            break;
    }
    
    printf("\nPress Enter to start the game...");
    getSecureInput(buffer, INPUT_BUFFER_SIZE);
}

void playGame(GameState* game) {
    if (game == NULL) {
        fprintf(stderr, _("Error: invalid game pointer\n"));
        return;
    }
    
    while (!game->gameOver) {
        
        printf("%s%s%s\n", COLOR_BOLD, _("CURRENT SCORES:"), COLOR_RESET);
        for (uint8_t i = 0; i < game->playerCount; i++) {
            const char* currentMarker = (i == game->currentPlayer) ? "-> " : "   ";
            const char* aiMarker = game->players[i].isAI ? _(" (AI)") : "";
            printf("%s%s%s: %d%s\n", 
                   currentMarker, 
                   game->players[i].name, 
                   aiMarker,
                   game->players[i].bankScore,
                   (game->players[i].bankScore >= WINNING_SCORE) ? _(" WINNER!") : "");
        }
        printf("\n");
        
        game->turnScore = 0;
        if (game->players[game->currentPlayer].isAI) {
            playAITurn(game);
        } else {
            playTurn(game);
        }
        
        if (game->players[game->currentPlayer].bankScore >= WINNING_SCORE) {
            printf("%s%s%s %s wins with %d points!\n%s", 
                   COLOR_BOLD, COLOR_GREEN, _("CONGRATULATIONS!"), 
                   game->players[game->currentPlayer].name,
                   game->players[game->currentPlayer].bankScore, COLOR_RESET);
            game->gameOver = 1;
            break;
        }
        
        game->currentPlayer = (game->currentPlayer + 1) % game->playerCount;
    }
    
    printf(_("\nThanks for playing Pig Game!\n"));
}

void playTurn(GameState* game) {
    char buffer[INPUT_BUFFER_SIZE];
    char choice;
    uint8_t diceValue;
    uint8_t turnEnds = 0;
    uint8_t gameEnds = 0;
    
    if (game == NULL) {
        fprintf(stderr, _("Error: invalid game pointer\n"));
        return;
    }
    
    Player* currentPlayer = &game->players[game->currentPlayer];
    
    printf("%s%s%s%s\n", COLOR_BOLD, COLOR_CYAN, _("Turn of %s"), COLOR_RESET);
    
    do {
        printf("You have %d points in bank, and %d points for this turn.\n", 
               currentPlayer->bankScore, game->turnScore);
        printf("Press Enter to roll...");
        getSecureInput(buffer, INPUT_BUFFER_SIZE);
        
        showLoadingAnimation("Rolling", 5);
        
        switch(game->mode) {
            case GAME_MODE_STANDARD:
                diceValue = rollDice();
                displayDice(diceValue);
                
                if (diceValue == 1) {
                    printf("%s%s%s\n", COLOR_RED, _("You rolled a 1! Bad luck, you lose your turn points."), COLOR_RESET);
                    game->turnScore = 0;
                    turnEnds = 1;
                } else {
                    game->turnScore += diceValue;
                    printf("You rolled %d! Your score for this turn is now %d.\n", 
                           diceValue, game->turnScore);
                }
                break;
                
            case GAME_MODE_ONE_PIG:
                diceValue = rollOnePig();
                printf("The pig lands on ");
                
                if (diceValue == 0) {
                    printf("%s%s%s\n", COLOR_RED, _("the side! Bad luck, you lose your turn points."), COLOR_RESET);
                    game->turnScore = 0;
                    turnEnds = 1;
                } else if (diceValue == 5) {
                    printf("the back or standing! +5 points.\n");
                    game->turnScore += diceValue;
                } else if (diceValue == 10) {
                    printf("the snout! +10 points.\n");
                    game->turnScore += diceValue;
                } else if (diceValue == 15) {
                    printf("the ear! +15 points.\n");
                    game->turnScore += diceValue;
                }
                break;
                
            case GAME_MODE_TWO_PIGS:
                rollTwoPigs(&diceValue, &turnEnds, &gameEnds);
                
                if (gameEnds) {
                    printf("%s%s%s\n", COLOR_RED, _("Piggyback! Game over!"), COLOR_RESET);
                    game->gameOver = 1;
                    return;
                }
                
                if (turnEnds) {
                    printf("%s%s%s\n", COLOR_RED, _("Makin' Bacon! You lose your turn points."), COLOR_RESET);
                    game->turnScore = 0;
                } else {
                    game->turnScore += diceValue;
                    printf("You got %d points! Your score for this turn is now %d.\n", 
                           diceValue, game->turnScore);
                }
                break;
        }
        
        if (!turnEnds && (currentPlayer->bankScore + game->turnScore) >= WINNING_SCORE) {
            printf("%s%s%s\n", COLOR_GREEN, _("You have enough points to win! Your points are automatically banked."), COLOR_RESET);
            currentPlayer->bankScore += game->turnScore;
            return;
        }
        
        if (!turnEnds) {
            printf("\nContinue [r]olling or [b]ank points? ");
            if (getSecureInput(buffer, INPUT_BUFFER_SIZE) == NULL || buffer[0] == '\0') {
                choice = 'r';
            } else {
                choice = tolower((unsigned char)buffer[0]);
            }
            
            if (choice == 'b') {
                currentPlayer->bankScore += game->turnScore;
                printf("%s%s %d points! Your total score is now %d.%s\n", 
                       COLOR_GREEN, _("You banked"), game->turnScore, currentPlayer->bankScore, COLOR_RESET);
                turnEnds = 1;
            }
        }
        
    } while (!turnEnds);
    
    printf("\nEnd of your turn. Press Enter to continue...");
    getSecureInput(buffer, INPUT_BUFFER_SIZE);
}

void playAITurn(GameState* game) {
    uint8_t diceValue;
    uint16_t targetScore;
    uint8_t turnEnds = 0;
    uint8_t gameEnds = 0;
    char buffer[INPUT_BUFFER_SIZE];
    
    if (game == NULL) {
        fprintf(stderr, _("Error: invalid game pointer\n"));
        return;
    }
    
    Player* currentPlayer = &game->players[game->currentPlayer];
    
    printf("%s%s%s%s\n", COLOR_BOLD, COLOR_CYAN, _("Turn of %s (AI)"), COLOR_RESET);
    
    switch(currentPlayer->strategy) {
        case AI_STRATEGY_RANDOM:
            targetScore = (uint16_t)((rand() % 30) + 5);
            break;
        case AI_STRATEGY_CONSERVATIVE:
            targetScore = 15;
            break;
        case AI_STRATEGY_AGGRESSIVE:
            targetScore = 25;
            break;
        case AI_STRATEGY_OPTIMAL:
            if (currentPlayer->bankScore >= 71 || 
                findHighestScore(game) >= 71) {
                targetScore = (uint16_t)(WINNING_SCORE - currentPlayer->bankScore);
            } else {
                targetScore = (uint16_t)(21 + (findHighestScore(game) - currentPlayer->bankScore) / 8);
            }
            break;
        default:
            targetScore = 20;
    }
    
    do {
        printf("AI has %d points in bank, and %d points for this turn.\n", 
               currentPlayer->bankScore, game->turnScore);
        printf("AI decides to roll...\n");
        
        SLEEP_MS(1000);
        showLoadingAnimation("AI thinking", 3);
        
        switch(game->mode) {
            case GAME_MODE_STANDARD:
                diceValue = rollDice();
                displayDice(diceValue);
                
                if (diceValue == 1) {
                    printf("%s%s%s\n", COLOR_RED, _("AI rolled a 1! It loses its turn points."), COLOR_RESET);
                    game->turnScore = 0;
                    turnEnds = 1;
                } else {
                    game->turnScore += diceValue;
                    printf("AI rolled %d! Its score for this turn is now %d.\n", 
                           diceValue, game->turnScore);
                }
                break;
                
            case GAME_MODE_ONE_PIG:
                diceValue = rollOnePig();
                printf("AI's pig lands on ");
                
                if (diceValue == 0) {
                    printf("%s%s%s\n", COLOR_RED, _("the side! It loses its turn points."), COLOR_RESET);
                    game->turnScore = 0;
                    turnEnds = 1;
                } else if (diceValue == 5) {
                    printf("the back or standing! +5 points.\n");
                    game->turnScore += diceValue;
                } else if (diceValue == 10) {
                    printf("the snout! +10 points.\n");
                    game->turnScore += diceValue;
                } else if (diceValue == 15) {
                    printf("the ear! +15 points.\n");
                    game->turnScore += diceValue;
                }
                break;
                
            case GAME_MODE_TWO_PIGS:
                rollTwoPigs(&diceValue, &turnEnds, &gameEnds);
                
                if (gameEnds) {
                    printf("%s%s%s\n", COLOR_RED, _("Piggyback! Game over!"), COLOR_RESET);
                    game->gameOver = 1;
                    return;
                }
                
                if (turnEnds) {
                    printf("%s%s%s\n", COLOR_RED, _("Makin' Bacon! AI loses its turn points."), COLOR_RESET);
                    game->turnScore = 0;
                } else {
                    game->turnScore += diceValue;
                    printf("AI got %d points! Its score for this turn is now %d.\n", 
                           diceValue, game->turnScore);
                }
                break;
        }
        
        if (!turnEnds && (currentPlayer->bankScore + game->turnScore) >= WINNING_SCORE) {
            printf("%s%s%s\n", COLOR_GREEN, _("AI has enough points to win! Its points are automatically banked."), COLOR_RESET);
            currentPlayer->bankScore += game->turnScore;
            return;
        }
        
        if (!turnEnds) {
            if (game->turnScore >= targetScore) {
                currentPlayer->bankScore += game->turnScore;
                printf("%s%s %d points! Its total score is now %d.%s\n", 
                       COLOR_GREEN, _("AI banks"), game->turnScore, currentPlayer->bankScore, COLOR_RESET);
                turnEnds = 1;
            } else {
                printf("AI decides to continue rolling (target: %d points).\n", targetScore);
                SLEEP_MS(1000);
            }
        }
        
    } while (!turnEnds);
    
    printf("\nEnd of AI's turn. Press Enter to continue...");
    getSecureInput(buffer, INPUT_BUFFER_SIZE);
}

uint8_t rollDice(void) {
    return (uint8_t)((rand() % 6) + 1);
}

uint8_t rollOnePig(void) {
    uint8_t position = (uint8_t)((rand() % 100) + 1);
    
    if (position <= 35) return 0;
    else if (position <= 65) return 5;
    else if (position <= 85) return 5;
    else if (position <= 95) return 10;
    else return 15;
}

void rollTwoPigs(uint8_t* score, uint8_t* turnEnds, uint8_t* gameEnds) {
    uint8_t pig1 = rollOnePig();
    uint8_t pig2 = rollOnePig();
    
    *turnEnds = 0;
    *gameEnds = 0;
    
    if (pig1 == 0 && pig2 == 0) {
        if ((rand() % 2) == 0) {
            *score = 1;
        } else {
            *score = 0;
            *turnEnds = 1;
        }
    } else if (pig1 == 0 || pig2 == 0) {
        *score = (pig1 + pig2);
    } else {
        if ((rand() % 10) == 0) {
            if ((rand() % 10) == 0) {
                *gameEnds = 1;
            } else {
                *score = 0;
                *turnEnds = 1;
                *gameEnds = 1;
            }
        } else {
            if (pig1 == pig2) {
                *score = (uint8_t)(pig1 * 4);
            } else {
                *score = (uint8_t)(pig1 + pig2);
            }
        }
    }
}

void displayDice(uint8_t value) {
    printf("\n");
    printf("  +-------+\n");
    switch(value) {
        case 1:
            printf("  |       |\n");
            printf("  |   o   |\n");
            printf("  |       |\n");
            break;
        case 2:
            printf("  | o     |\n");
            printf("  |       |\n");
            printf("  |     o |\n");
            break;
        case 3:
            printf("  | o     |\n");
            printf("  |   o   |\n");
            printf("  |     o |\n");
            break;
        case 4:
            printf("  | o   o |\n");
            printf("  |       |\n");
            printf("  | o   o |\n");
            break;
        case 5:
            printf("  | o   o |\n");
            printf("  |   o   |\n");
            printf("  | o   o |\n");
            break;
        case 6:
            printf("  | o   o |\n");
            printf("  | o   o |\n");
            printf("  | o   o |\n");
            break;
    }
    printf("  +-------+\n\n");
}

void showLoadingAnimation(const char* message, int duration) {
    const char spinner[] = "|/-\\";
    int i;
    
    printf("%s ", message);
    for (i = 0; i < duration; i++) {
        printf("%c\b", spinner[i % 4]);
        fflush(stdout);
        SLEEP_MS(100);
    }
    printf(" Done!\n");
}

uint16_t findHighestScore(const GameState* game) {
    uint16_t highestScore = 0;
    
    if (game == NULL) {
        return 0;
    }
    
    for (uint8_t i = 0; i < game->playerCount; i++) {
        if (game->players[i].bankScore > highestScore) {
            highestScore = game->players[i].bankScore;
        }
    }
    
    return highestScore;
}

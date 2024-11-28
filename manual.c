#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

//Typedef declarations
typedef struct coordinates {
    int x, y, z;
} coordinates;

typedef struct {
    coordinates *stack_array;
    int top;
    int capacity;
} Stack;

// Variable declarations
int mode;//auto or manual
int m, n, o; // Grid dimensions
int no_of_monsters; // Number of monsters
coordinates Mario_coordinates, Princess_coordinates; // Mario and princess coordinates
coordinates *Monsters_coordinates; // Array to store monsters' coordinates
Stack *visited_mario; // Stack for Mario's visited cells
Stack **visited_monsters; // Array of stacks, one for each monster
bool instructions_printed = false;

// Function declarations

void sleep_microseconds(long microseconds);
int gamemode(int gamemode_entered);
void get_grid_dimensions_and_monsters(int gamemode_entered);
void generate_coords(int gamemode_entered);
bool is_within_grid(int x, int y, int z);
void get_input_for_coords(const char *entity_name, coordinates *coord);
int calculate_manhattan_distance(coordinates a, coordinates b);
Stack *create_stack(int capacity);
void push(Stack *stack, coordinates coord);
coordinates pop(Stack *stack);
bool is_visited(Stack *stack, coordinates coord);
bool is_valid_move(coordinates new_position, Stack *visited);
void move_entity(coordinates *entity, coordinates target, Stack *visited, const char *entity_name);
void move_monsters(coordinates *monsters, int num_monsters, coordinates mario, coordinates princess);
void manual_move(coordinates *mario, Stack *visited);
void choose_mode();
void freememory();

// Function to log the game results when Mario wins
void log_game_result(const char *filename, int moves) {
    FILE *file = fopen(filename, "a"); // Open the file in append mode
    if (file == NULL) {
        perror("Error opening results file");
        return;
    }

    // Write the result to the file when Mario wins
    fprintf(file, "Winner: Mario\n");
    fprintf(file, "Total Moves: %d\n\n", moves);

    fclose(file);
}

int main(void) {
    srand(time(NULL)); // Seed the random number generator
    int moves = 0;  // Track number of moves
    const char *winner = "Mario";
    int round_number = 1;

    // Ask user for mode selection
    choose_mode();
    // Get game settings
    int gamemode_entered = gamemode(0);
    get_grid_dimensions_and_monsters(gamemode_entered);
    generate_coords(gamemode_entered);

    // Initialize stack for Mario
    visited_mario = create_stack(m * n * o);
    push(visited_mario, Mario_coordinates); // Push Mario's initial position

    // Initialize stacks for monsters
    visited_monsters = malloc(no_of_monsters * sizeof(Stack *));
    for (int i = 0; i < no_of_monsters; i++) {
        visited_monsters[i] = create_stack(m * n * o);
        push(visited_monsters[i], Monsters_coordinates[i]); // Push each monster's initial position
    }

    // Game loop
    bool game_running = true;
    while (game_running) {
        printf("\n========== Round %d ==========\n", round_number);

        // Mario's turn
        printf("\nMario's Turn:\n");
        if (mode == 1) { // Manual mode
            
            manual_move(&Mario_coordinates, visited_mario);
        } else { // Automatic mode
            move_entity(&Mario_coordinates, Princess_coordinates, visited_mario, "Mario");
        }
        
        // Now, move the monsters after Mario's move
        move_monsters(Monsters_coordinates, no_of_monsters, Mario_coordinates, Princess_coordinates);
        // Check if Mario reached the princess
        if (Mario_coordinates.x == Princess_coordinates.x &&
            Mario_coordinates.y == Princess_coordinates.y &&
            Mario_coordinates.z == Princess_coordinates.z) {
            printf("\nMario has reached the Princess! You Win!\n");
            log_game_result("game_results.txt", moves);
            game_running = false;
            break;
        }

        // Monsters' turn
        printf("\nMonsters' Turn:\n");
        for (int i = 0; i < no_of_monsters; i++) {
            move_entity(&Monsters_coordinates[i],
                        calculate_manhattan_distance(Monsters_coordinates[i], Mario_coordinates) <
                        calculate_manhattan_distance(Monsters_coordinates[i], Princess_coordinates)
                        ? Mario_coordinates : Princess_coordinates,
                        visited_monsters[i], "Monster");

            // Check if any monster captured the princess
            if (Monsters_coordinates[i].x == Princess_coordinates.x &&
                Monsters_coordinates[i].y == Princess_coordinates.y &&
                Monsters_coordinates[i].z == Princess_coordinates.z) {
                printf("\nA Monster has captured the Princess! Mario loses!\n");
                game_running = false;
                break;
            }
        }

        if (!game_running) {
            break;
        }

        round_number++;
        moves++;
        // Add a delay between rounds
        printf("\nEnd of Round. Preparing for the next round...\n");
        sleep_microseconds(1000000); 
    }

    printf("Game finished!\n");
    printf("Winner: %s\n", winner);
    printf("Total Moves: %d\n", moves);
    // Clean up
    free(visited_mario->stack_array);
    free(visited_mario);

    for (int i = 0; i < no_of_monsters; i++) {
        free(visited_monsters[i]->stack_array);
        free(visited_monsters[i]);
    }
    free(visited_monsters);
    return 0;
}

/**int main(void) {
    srand(time(NULL)); // Seed the random number generator

    int round_number = 1;

    printf("Press Enter to start...\n");
    while (getchar() != '\n');

    // Loading animation
    printf("Loading: [");
    fflush(stdout);
    for (int i = 0; i <= 50; i++) {
        if (i % 5 == 0) {
            printf("#");
            fflush(stdout);
        }
        sleep_microseconds(50000);
    }
    printf("] 100%%\n");

    // Ask user for mode selection
    choose_mode();

    // Get game difficulty settings
    int gamemode_entered = gamemode(0);
    get_grid_dimensions_and_monsters(gamemode_entered);

    // Generate coordinates for Mario, Princess, and Monsters
    generate_coords(gamemode_entered);

    // Initialize stack for Mario
    visited_mario = create_stack(m * n * o);
    push(visited_mario, Mario_coordinates); // Push Mario's initial position

    // Initialize stacks for monsters
    visited_monsters = malloc(no_of_monsters * sizeof(Stack *));
    for (int i = 0; i < no_of_monsters; i++) {
        visited_monsters[i] = create_stack(m * n * o);
        push(visited_monsters[i], Monsters_coordinates[i]); // Push each monster's initial position
    }

    // Game loop
    bool game_running = true;
    while (game_running) {
        printf("\n========== Round %d ==========\n", round_number);

        // Mario's turn
        printf("\nMario's Turn:\n");
        if (mode == 1) { // Manual mode
            manual_move(&Mario_coordinates, visited_mario);
        } else { // Automatic mode
            move_entity(&Mario_coordinates, Princess_coordinates, visited_mario, "Mario");
        }

        // Check if Mario reached the princess
        if (Mario_coordinates.x == Princess_coordinates.x &&
            Mario_coordinates.y == Princess_coordinates.y &&
            Mario_coordinates.z == Princess_coordinates.z) {
            printf("\nMario has reached the Princess! You Win!\n");
            game_running = false;
            break;
        }

        // Monsters' turn
        printf("\nMonsters' Turn:\n");
        for (int i = 0; i < no_of_monsters; i++) {
            move_entity(&Monsters_coordinates[i],
                        calculate_manhattan_distance(Monsters_coordinates[i], Mario_coordinates) <
                        calculate_manhattan_distance(Monsters_coordinates[i], Princess_coordinates)
                        ? Mario_coordinates : Princess_coordinates,
                        visited_monsters[i], "Monster");

            // Check if any monster captured the princess
            if (Monsters_coordinates[i].x == Princess_coordinates.x &&
                Monsters_coordinates[i].y == Princess_coordinates.y &&
                Monsters_coordinates[i].z == Princess_coordinates.z) {
                printf("\nA Monster has captured the Princess! Mario loses!\n");
                game_running = false;
                break;
            }
        }

        if (!game_running) {
            break;
        }

        round_number++;

        // Add a delay between rounds
        printf("\nEnd of Round. Preparing for the next round...\n");
        sleep_microseconds(2000000); // 2 seconds delay for clarity
    }

    // Clean up
    free(visited_mario->stack_array);
    free(visited_mario);

    for (int i = 0; i < no_of_monsters; i++) {
        free(visited_monsters[i]->stack_array);
        free(visited_monsters[i]);
    }
    free(visited_monsters);

    return 0;
}
*/
//for 50ms delay, we use this function
void sleep_microseconds(long microseconds) {
    struct timespec ts;             //declaring timespec 
    ts.tv_sec = microseconds / 1000000;           // Convert microseconds to seconds
    ts.tv_nsec = (microseconds % 1000000) * 1000; // Remaining microseconds to nanoseconds
    nanosleep(&ts, NULL);
}

// Asking user for gamemode
int gamemode(int gamemode_entered) {
    printf("Please Select The Game Mode:\nEasy(1)\nMedium(2)\nHard(3)\nCustom(4)\n");
    scanf("%d", &gamemode_entered);
    return gamemode_entered;
}

// Asking user for dimensions and number of monsters
void get_grid_dimensions_and_monsters(int gamemode_entered) {
    switch (gamemode_entered) {
        case 1: // Easy
            m = 5; n = 5; o = 5;
            no_of_monsters = 2;
            break;
        case 2: // Medium
            m = 8; n = 8; o = 8;
            no_of_monsters = rand() % 2 + 4; // Random number between 4 and 5
            break;
        case 3: // Hard
            m = 12; n = 12; o = 12;
            no_of_monsters = rand() % 2 + 7; // Random number between 7 and 8
            break;
        case 4: // Custom
            while (true) { // Loop until valid input is provided
                printf("Enter custom grid dimensions (x y z): ");
                scanf("%d %d %d", &m, &n, &o);

                if (m > 0 && n > 0 && o > 0) { // Ensure dimensions are positive
                    break; // Exit loop when valid dimensions are entered
                }

                printf("Invalid grid dimensions. Please enter valid dimensions greater than 0.\n");
            }

            while (true) { // Loop for the number of monsters
                printf("Enter the number of monsters: ");
                scanf("%d", &no_of_monsters);

                if (no_of_monsters > 0) { // Ensure the number of monsters is positive
                    break; // Exit loop when valid input is provided
                }

                printf("Invalid number of monsters. Please enter a positive number.\n");
            }
            break;
        default:
            printf("Invalid mode selected. Defaulting to Easy.\n");
            m = 5; n = 5; o = 5;
            no_of_monsters = 2;
    }

    printf("Grid size: %dx%dx%d, Number of monsters: %d\n", m, n, o, no_of_monsters);
}

// Check if coordinates are within the grid
bool is_within_grid(int x, int y, int z) {
    return (x >= 0 && x < m && y >= 0 && y < n && z >= 0 && z < o);
}

// Push a coordinate onto the stack
void push(Stack *stack, coordinates coord) {
    if (stack->top < stack->capacity - 1) {
        stack->stack_array[++stack->top] = coord;
    } else {
        printf("Stack overflow! Increase capacity.\n");
    }
}

// Pop a coordinate from the stack
coordinates pop(Stack *stack) {
    if (stack->top >= 0) {
        return stack->stack_array[stack->top--];
    } else {
        printf("Stack underflow!\n");
        return (coordinates){-1, -1, -1}; // Invalid coordinate
    }
}

// Check if a coordinate is already visited
bool is_visited(Stack *stack, coordinates coord) {
    for (int i = 0; i <= stack->top; i++) {
        if (stack->stack_array[i].x == coord.x &&
            stack->stack_array[i].y == coord.y &&
            stack->stack_array[i].z == coord.z) {
            return true;
        }
    }
    return false;
}

// Generate coordinates for Mario, princess, and monsters
void generate_coords(int gamemode_entered) {
    Monsters_coordinates = (coordinates *)malloc(no_of_monsters * sizeof(coordinates));
    if (Monsters_coordinates == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    if (gamemode_entered == 4) { // Custom Mode
        printf("Enter Mario's coordinates (x y z): ");
        while (true) {
            scanf("%d %d %d", &Mario_coordinates.x, &Mario_coordinates.y, &Mario_coordinates.z);
            if (is_within_grid(Mario_coordinates.x, Mario_coordinates.y, Mario_coordinates.z)) {
                break;
            }
            printf("Invalid coordinates. Please enter coordinates within the grid dimensions.\n");
        }

        // Get user input for Princess's coordinates
        printf("Enter Princess's coordinates (x y z): ");
        while (true) {
            scanf("%d %d %d", &Princess_coordinates.x, &Princess_coordinates.y, &Princess_coordinates.z);
            if (is_within_grid(Princess_coordinates.x, Princess_coordinates.y, Princess_coordinates.z)) {
                break;
            }
            printf("Invalid coordinates. Please enter coordinates within the grid dimensions.\n");
        }

        // Get user input for Monsters' coordinates
        for (int i = 0; i < no_of_monsters; i++) {
            printf("Enter Monster %d's coordinates (x y z): ", i + 1);
            while (true) {
                scanf("%d %d %d", &Monsters_coordinates[i].x, &Monsters_coordinates[i].y, &Monsters_coordinates[i].z);
                 if (is_within_grid(Monsters_coordinates[i].x, Monsters_coordinates[i].y, Monsters_coordinates[i].z) &&
                    !(Monsters_coordinates[i].x == Mario_coordinates.x &&
                      Monsters_coordinates[i].y == Mario_coordinates.y &&
                      Monsters_coordinates[i].z == Mario_coordinates.z) &&
                    !(Monsters_coordinates[i].x == Princess_coordinates.x &&
                      Monsters_coordinates[i].y == Princess_coordinates.y &&
                      Monsters_coordinates[i].z == Princess_coordinates.z)) {
                    break;
                }
                printf("Invalid coordinates. Please enter coordinates within the grid dimensions.\n");
            }
        }
    } else { // Randomized Coordinates for other modes
        do {
            Mario_coordinates.x = rand() % m;
            Mario_coordinates.y = rand() % n;
            Mario_coordinates.z = rand() % o;
        } while (!is_within_grid(Mario_coordinates.x, Mario_coordinates.y, Mario_coordinates.z));

        do {
            Princess_coordinates.x = rand() % m;
            Princess_coordinates.y = rand() % n;
            Princess_coordinates.z = rand() % o;
        } while (!is_within_grid(Princess_coordinates.x, Princess_coordinates.y, Princess_coordinates.z));

         for (int i = 0; i < no_of_monsters; i++) {
            do {
                Monsters_coordinates[i].x = rand() % m;
                Monsters_coordinates[i].y = rand() % n;
                Monsters_coordinates[i].z = rand() % o;
            } while (!is_within_grid(Monsters_coordinates[i].x, Monsters_coordinates[i].y, Monsters_coordinates[i].z) ||
                     (Monsters_coordinates[i].x == Mario_coordinates.x &&
                      Monsters_coordinates[i].y == Mario_coordinates.y &&
                      Monsters_coordinates[i].z == Mario_coordinates.z) ||
                     (Monsters_coordinates[i].x == Princess_coordinates.x &&
                      Monsters_coordinates[i].y == Princess_coordinates.y &&
                      Monsters_coordinates[i].z == Princess_coordinates.z));
        }
    }

    // Print coordinates
    printf("Mario coordinates: (%d, %d, %d)\n", Mario_coordinates.x, Mario_coordinates.y, Mario_coordinates.z);
    printf("Princess coordinates: (%d, %d, %d)\n", Princess_coordinates.x, Princess_coordinates.y, Princess_coordinates.z);
    for (int i = 0; i < no_of_monsters; i++) {
        printf("Monster %d coordinates: (%d, %d, %d)\n", i + 1, Monsters_coordinates[i].x, Monsters_coordinates[i].y, Monsters_coordinates[i].z);
    }
}


// Move entity logic
void move_entity(coordinates *entity, coordinates target, Stack *visited, const char *entity_name) {
    coordinates best_move = *entity;
    int min_distance = calculate_manhattan_distance(*entity, target);

    coordinates possible_moves[6] = {
        {entity->x + 1, entity->y, entity->z},
        {entity->x - 1, entity->y, entity->z},
        {entity->x, entity->y + 1, entity->z},
        {entity->x, entity->y - 1, entity->z},
        {entity->x, entity->y, entity->z + 1},
        {entity->x, entity->y, entity->z - 1}
    };

    for (int i = 0; i < 6; i++) {
        if (is_valid_move(possible_moves[i], visited)) {
            int distance = calculate_manhattan_distance(possible_moves[i], target);
            if (distance < min_distance) {
                best_move = possible_moves[i];
                min_distance = distance;
            }
        }
    }
    *entity = best_move;
    push(visited, best_move);
    printf("%s moved to (%d, %d, %d)\n", entity_name, entity->x, entity->y, entity->z);
}


// Calculate Manhattan distance between two points
int calculate_manhattan_distance(coordinates a, coordinates b) {
    return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
}

bool is_valid_move(coordinates new_position, Stack *visited) {
    return is_within_grid(new_position.x, new_position.y, new_position.z) &&
           !is_visited(visited, new_position);
}

Stack *create_stack(int capacity) {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->stack_array = (coordinates *)malloc(capacity * sizeof(coordinates));
    stack->top = -1; // Initialize top to -1 (empty stack)
    stack->capacity = capacity;
    return stack;
}

// Prompt user for mode selection
void choose_mode() {
    printf("Select the mode for Mario:\n");
    printf("1. Manual\n");
    printf("2. Automatic\n");
    while (true) {
        scanf("%d", &mode);
        if (mode == 1 || mode == 2) {
            break;
        }
        printf("Invalid choice. Please enter 1 for Manual or 2 for Automatic.\n");
    }
}
//instruction manual
void printInstructions() {
    printf("Press w to move one block front\n");
    printf("Press s to move one block back\n");
    printf("Press a to move one block to the left\n");
    printf("Press d to move one block to the right\n");
    printf("Press q to move one block above\n");
    printf("Press e to move one block below\n");
}
// Manual movement function for Mario
void manual_move(coordinates *mario, Stack *visited) {
    char move;
    if (!instructions_printed) {
        printInstructions();
        instructions_printed = true; // Set the flag to true after printing
    }
    printf("Enter Mario's move (w/a/s/d/q/e): ");
    while (true) {
        scanf(" %c", &move); // Space before %c to consume newline/whitespace
        
        coordinates new_position = *mario;
        switch (move) {
            case 'w': new_position.y += 1; break; // Move +y
            case 's': new_position.y -= 1; break; // Move -y
            case 'a': new_position.x -= 1; break; // Move -x
            case 'd': new_position.x += 1; break; // Move +x
            case 'q': new_position.z += 1; break; // Move +z
            case 'e': new_position.z -= 1; break; // Move -z
            default:
                printf("Invalid move. Use w/a/s/d/q/e for printing directions.\n");
                continue;
        }

        if (is_valid_move(new_position, visited)) {
            *mario = new_position;
            push(visited, new_position);
            printf("Mario moved to (%d, %d, %d)\n", mario->x, mario->y, mario->z);
            break;
        } else {
            printf("Invalid move or position already visited. Try again.\n");
        }
    }
}
void move_monsters(coordinates *monsters, int num_monsters, coordinates mario, coordinates princess) {
    for (int i = 0; i < num_monsters; i++) {
        coordinates target;
        
        // Determine which target (Mario or Princess) is closer
        if (calculate_manhattan_distance(monsters[i], mario) < calculate_manhattan_distance(monsters[i], princess)) {
            target = mario; // Move towards Mario
        } else {
            target = princess; // Move towards Princess
        }

        // Move the monster towards the target
        move_entity(&monsters[i], target, visited_monsters[i], "Monster");
    }
}
void freememory() {
    free(visited_mario->stack_array);
    free(visited_mario);

    for (int i = 0; i < no_of_monsters; i++) {
        free(visited_monsters[i]->stack_array);
        free(visited_monsters[i]);
    }
    free(visited_monsters);
    free(Monsters_coordinates);
}
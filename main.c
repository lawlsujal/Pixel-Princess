#include "raylib.h"             //install raylib alag se warna code won't run
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Constants screen define karne ke liye
#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 1200

// Enums
typedef enum
{
    STATE_SPLASH,
    STATE_MENU,
    STATE_GAME,
    STATE_WIN,
    STATE_LOSE
} GameState;

// Typedef declarations
typedef struct coordinates
{
    int x, y, z;
} coordinates;

typedef struct
{
    coordinates *stack_array;
    int top;            //top and capacity are used to control stack overflow
    int capacity;
} Stack;

// Global variables GUI WALE
static GameState currentState = STATE_SPLASH;           
static int selectedOption = 0;                          
static Model modelMario, modelPrincess, modelMonster;   
static bool isPaused = true;                            
static float pathOpacity = 2.0f;                        
static float moveTimer = 0.0f;                              
static const float MOVE_DELAY = 1.0f;                   // Delay between moves in seconds
static float splashTimer = 0.0f;
static float loadingProgress = 0.0f;
static const float SPLASH_DURATION = 10.0f;

typedef struct
{
    Vector3 position;
    Color color;
} PathPoint;

typedef struct
{
    PathPoint *points;
    int count;
    int capacity;
} Path;

static Path marioPath;
static Path *monsterPaths;
// CODE WALE VARIABLES
int mode;
int m, n, o; // Grid dimensions
int no_of_monsters;
coordinates Mario_coordinates, Princess_coordinates; // Mario and princess coordinates defined under the structure coordinates line 22 pe define kar rakkha hai
coordinates *Monsters_coordinates;                   // Array to store monsters' coordinates
Stack *visited_mario;                                // Stack for Mario's visited cells;jo bhi visit karega use store kar dega taaki dobara visit na kare
Stack **visited_monsters;                            // Array of stacks, one for each monster

// Function declarations

void draw_menu(void);
void draw_game(void);
void draw_grid(void);
void draw_winscreen(void);
void draw_losescreen(void);
void draw_splash_screen(void);
void initializePaths(void);
void addPathPoint(Path *path, Vector3 position, Color color);
void drawPaths(void);
void cleanupPaths(void);
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

// function to initialize paths
void initializePaths(void)
{
    marioPath.capacity = m * n * o;
    marioPath.points = (PathPoint *)malloc(marioPath.capacity * sizeof(PathPoint));
    marioPath.count = 0;

    monsterPaths = (Path *)malloc(no_of_monsters * sizeof(Path));
    for (int i = 0; i < no_of_monsters; i++)
    {
        monsterPaths[i].capacity = m * n * o;
        monsterPaths[i].points = (PathPoint *)malloc(monsterPaths[i].capacity * sizeof(PathPoint));
        monsterPaths[i].count = 0;
    }
}

// function to add points to paths
void addPathPoint(Path *path, Vector3 position, Color color)
{
    if (path->count < path->capacity)
    {
        path->points[path->count].position = position;
        path->points[path->count].color = color;
        path->count++;
    }
}
// function to draw paths
void drawPaths(void)
{
    // draw mario's path
    for (int i = 0; i < marioPath.count - 1; i++)
    {
        DrawLine3D(marioPath.points[i].position, marioPath.points[i + 1].position,
                   ColorAlpha(RED, pathOpacity));
        DrawSphere(marioPath.points[i].position, 0.2f, ColorAlpha(RED, pathOpacity));
    }

    // Draw monsters' paths
    for (int m = 0; m < no_of_monsters; m++)
    {
        for (int i = 0; i < monsterPaths[m].count - 1; i++)
        {
            DrawLine3D(monsterPaths[m].points[i].position,
                       monsterPaths[m].points[i + 1].position,
                       ColorAlpha(PURPLE, pathOpacity));
            DrawSphere(monsterPaths[m].points[i].position, 0.2f, ColorAlpha(PURPLE, pathOpacity));
        }
    }
}
// path cleanup
void cleanupPaths(void)
{
    // Check if marioPath.points is not NULL before freeing
    if (marioPath.points != NULL)
    {
        free(marioPath.points);
        marioPath.points = NULL;
    }

    // Check if monsterPaths is not NULL before freeing
    if (monsterPaths != NULL)
    {
        for (int i = 0; i < no_of_monsters; i++)
        {
            if (monsterPaths[i].points != NULL)
            {
                free(monsterPaths[i].points);
                monsterPaths[i].points = NULL;
            }
        }
        free(monsterPaths);
        monsterPaths = NULL;
    }
}
// Main function
int main(void)
{
    srand(time(NULL));

    // Initialize window and 3D
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pixel Princess!");
    SetTargetFPS(60);

    // Initialize camera with default values (will be updated later)
    Camera3D camera = {0};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load models
    modelMario = LoadModel("./mario.glb");
    modelMonster = LoadModel("./bowser.glb");
    modelPrincess = LoadModel("./princess.glb");

    // Game loop
    while (!WindowShouldClose())
    {
        if (currentState == STATE_SPLASH)
        {
            splashTimer += GetFrameTime();
            loadingProgress = splashTimer / SPLASH_DURATION;

            // Load models during splash screen
            if (splashTimer >= SPLASH_DURATION / 3.0f && modelMario.meshCount == 0)
                modelMario = LoadModel("./mario.glb");

            if (splashTimer >= (SPLASH_DURATION * 2.0f) / 3.0f && modelPrincess.meshCount == 0)
                modelPrincess = LoadModel("./princess.glb");

            if (splashTimer >= SPLASH_DURATION && modelMonster.meshCount == 0)
                modelMonster = LoadModel("./bowser.glb");

            if (splashTimer >= SPLASH_DURATION)
            {
                currentState = STATE_MENU;
            }
        }
        else if (currentState == STATE_MENU)
        {
            // Menu controls
            if (IsKeyPressed(KEY_UP))
                selectedOption = (selectedOption - 1 + 3) % 3;
            if (IsKeyPressed(KEY_DOWN))
                selectedOption = (selectedOption + 1) % 3;

            if (IsKeyPressed(KEY_ENTER))
            {
                int gamemode_entered = selectedOption + 1;
                get_grid_dimensions_and_monsters(gamemode_entered);
                generate_coords(gamemode_entered);

                // Initialize game state
                float centerX = (float)m / 2.0f;
                float centerY = (float)n / 2.0f;
                float centerZ = (float)o / 2.0f;

                camera.position = (Vector3){centerX * 2.0f + 10.0f, centerY * 2.0f + 10.0f, centerZ * 2.0f + 10.0f};
                camera.target = (Vector3){centerX * 2.0f, centerY * 2.0f, centerZ * 2.0f};

                initializePaths();

                // Initialize stacks
                visited_mario = create_stack(m * n * o);
                push(visited_mario, Mario_coordinates);

                visited_monsters = malloc(no_of_monsters * sizeof(Stack *));
                for (int i = 0; i < no_of_monsters; i++)
                {
                    visited_monsters[i] = create_stack(m * n * o);
                    push(visited_monsters[i], Monsters_coordinates[i]);
                }

                currentState = STATE_GAME;
            }
        }
        else if (currentState == STATE_GAME)
        {
            UpdateCamera(&camera, CAMERA_FREE);

            if (IsKeyDown(KEY_ENTER))
            {
                isPaused = false;
            }
            else
            {
                isPaused = true;
            }

            if (!isPaused)
            {
                moveTimer += GetFrameTime(); // Add time since last frame

                if (moveTimer >= MOVE_DELAY)
                { // Only move when enough time has passed
                    // Add current positions to paths
                    Vector3 marioPos = {Mario_coordinates.x * 2.0f, Mario_coordinates.y * 2.0f, Mario_coordinates.z * 2.0f};
                    addPathPoint(&marioPath, marioPos, RED);

                    for (int i = 0; i < no_of_monsters; i++)
                    {
                        Vector3 monsterPos = {Monsters_coordinates[i].x * 2.0f,
                                              Monsters_coordinates[i].y * 2.0f,
                                              Monsters_coordinates[i].z * 2.0f};
                        addPathPoint(&monsterPaths[i], monsterPos, PURPLE);
                    }

                    // Move entities
                    move_entity(&Mario_coordinates, Princess_coordinates, visited_mario, "Mario");

                    for (int i = 0; i < no_of_monsters; i++)
                    {
                        move_entity(&Monsters_coordinates[i],
                                    calculate_manhattan_distance(Monsters_coordinates[i], Mario_coordinates) <
                                            calculate_manhattan_distance(Monsters_coordinates[i], Princess_coordinates)
                                        ? Mario_coordinates
                                        : Princess_coordinates,
                                    visited_monsters[i], "Monster");

                        // Check if monster caught Mario
                        if (Monsters_coordinates[i].x == Mario_coordinates.x &&
                            Monsters_coordinates[i].y == Mario_coordinates.y &&
                            Monsters_coordinates[i].z == Mario_coordinates.z)
                        {
                            currentState = STATE_LOSE;
                            cleanupPaths();
                            break;
                        }
                        if (Monsters_coordinates[i].x == Princess_coordinates.x &&
                            Monsters_coordinates[i].y == Princess_coordinates.y &&
                            Monsters_coordinates[i].z == Princess_coordinates.z)
                        {
                            currentState = STATE_LOSE;
                            cleanupPaths();
                            break;
                        }
                    }

                    // Check win condition
                    if (Mario_coordinates.x == Princess_coordinates.x &&
                        Mario_coordinates.y == Princess_coordinates.y &&
                        Mario_coordinates.z == Princess_coordinates.z)
                    {
                        currentState = STATE_WIN;
                        cleanupPaths();
                    }

                    moveTimer = 0.0f; // Reset timer after moving
                }
            }
        }
        else if (currentState == STATE_WIN || currentState == STATE_LOSE)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                currentState = STATE_MENU;

                // Cleanup game state
                if (visited_mario != NULL)
                {
                    free(visited_mario->stack_array);
                    free(visited_mario);
                }

                if (visited_monsters != NULL)
                {
                    for (int i = 0; i < no_of_monsters; i++)
                    {
                        free(visited_monsters[i]->stack_array);
                        free(visited_monsters[i]);
                    }
                    free(visited_monsters);
                }

                if (Monsters_coordinates != NULL)
                {
                    free(Monsters_coordinates);
                    Monsters_coordinates = NULL;
                }
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground((Color){30, 144, 255, 255}); // Deep Sky Blue
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                               (Color){135, 206, 235, 255}, // Sky Blue (top)
                               (Color){0, 191, 255, 255});  // Deep Sky Blue (bottom)

        if (currentState == STATE_SPLASH)
        {
            draw_splash_screen();
        }
        else if (currentState == STATE_MENU)
        {
            draw_menu();
        }
        else if (currentState == STATE_GAME)
        {
            BeginMode3D(camera);
            draw_game();
            drawPaths();
            EndMode3D();

            // Draw UI elements
            DrawText("Press ESC to exit", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Grid Size: %dx%dx%d", m, n, o), 10, 40, 20, DARKGRAY);
            DrawText(TextFormat("Monsters: %d", no_of_monsters), 10, 70, 20, DARKGRAY);
            DrawText("Hold ENTER to animate movement", 10, 100, 20, DARKGRAY);
            DrawText(isPaused ? "PAUSED" : "RUNNING", 10, 130, 20, isPaused ? RED : GREEN);
        }
        else if (currentState == STATE_WIN)
        {
            draw_winscreen();
        }
        else if (currentState == STATE_LOSE)
        {
            draw_losescreen();
        }

        EndDrawing();

        // Return to menu if ESC is pressed during game
        if (currentState == STATE_GAME && IsKeyPressed(KEY_ESCAPE))
        {
            currentState = STATE_MENU;
            cleanupPaths();

            // Cleanup game state
            if (visited_mario != NULL)
            {
                free(visited_mario->stack_array);
                free(visited_mario);
            }

            if (visited_monsters != NULL)
            {
                for (int i = 0; i < no_of_monsters; i++)
                {
                    free(visited_monsters[i]->stack_array);
                    free(visited_monsters[i]);
                }
                free(visited_monsters);
            }

            if (Monsters_coordinates != NULL)
            {
                free(Monsters_coordinates);
                Monsters_coordinates = NULL;
            }
        }
    }

    // Final cleanup
    UnloadModel(modelMario);
    UnloadModel(modelMonster);
    UnloadModel(modelPrincess);
    CloseWindow();

    return 0;
}
/*************************************************************************************** */

// GUI SECTION BEGINS
void draw_menu(void)
{
    // Background gradient
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           (Color){70, 130, 180, 255}, // Steel Blue
                           (Color){30, 144, 255, 255}  // Deep Sky Blue
    );

    const char *options[] = {"Easy", "Medium", "Hard"};
    int startY = SCREEN_HEIGHT / 2 - 100;

    // Game title with shadow effect
    DrawText("Pixel Princess!",
             SCREEN_WIDTH / 2 - MeasureText("Pixel Princess!", 40) / 2,
             startY - 60, 40, RED);
    DrawText("Pixel Princess!",
             SCREEN_WIDTH / 2 - MeasureText("Pixel Princess!", 40) / 2 + 2,
             startY - 58, 40, (Color){139, 0, 0, 200}); // Dark Red shadow

    // Menu options with hover effect
    for (int i = 0; i < 3; i++)
    {
        Color baseColor = (Color){255, 255, 255, 220}; // Off-white
        Color hoverColor = (Color){255, 69, 0, 255};   // Bright Red-Orange
        Color textColor = (i == selectedOption) ? hoverColor : baseColor;

        int textWidth = MeasureText(options[i], 30);
        DrawText(options[i],
                 SCREEN_WIDTH / 2 - textWidth / 2,
                 startY + i * 50,
                 30,
                 textColor);
    }

    // Instruction text with softer color
    DrawText("Use UP/DOWN arrows to select and ENTER to confirm",
             SCREEN_WIDTH / 2 - MeasureText("Use UP/DOWN arrows to select and ENTER to confirm", 20) / 2,
             SCREEN_HEIGHT - 50,
             20,
             (Color){255, 255, 255, 180}); // Translucent white
}

void draw_splash_screen(void)
{
    // Background gradient
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           (Color){70, 130, 180, 255}, // Steel Blue
                           (Color){30, 144, 255, 255}  // Deep Sky Blue
    );
    // Game title
    DrawText("Pixel Princess!",
             SCREEN_WIDTH / 2 - MeasureText("Pixel Princess!", 50) / 2,
             SCREEN_HEIGHT / 2 - 100,
             50, RED);

    int textFontSize = 30;
    const char *storyText1 = "In the Pixel World, everything was peaceful and";
    const char *storyText2 = "above them all showering the entire world with love";
    const char *storyText3 = "was Pixel Princess, loved and cherished by all.";
    const char *storyText4 = "Until one day hordes of vile monsters arrived from";
    const char *storyText5 = "a far away place to kidnap the Pixel Princess,";
    const char *storyText6 = "and to her rescue came Mario, the great pixel hero.";
    const char *storyText7 = "Here unfolds the greatest legend the pixel world has ever seen...";

    // Draw story text
    DrawText(storyText1, 
             SCREEN_WIDTH / 2 - MeasureText(storyText1, textFontSize) / 2, 
             SCREEN_HEIGHT / 2, 
             textFontSize, WHITE);
    DrawText(storyText2, 
             SCREEN_WIDTH / 2 - MeasureText(storyText2, textFontSize) / 2, 
             SCREEN_HEIGHT / 2 + 30, 
             textFontSize, WHITE);
    DrawText(storyText3, 
             SCREEN_WIDTH / 2 - MeasureText(storyText3, textFontSize) / 2, 
             SCREEN_HEIGHT / 2 + 60, 
             textFontSize, WHITE);
    DrawText(storyText4, 
             SCREEN_WIDTH / 2 - MeasureText(storyText4, textFontSize) / 2, 
             SCREEN_HEIGHT / 2 + 90, 
             textFontSize, WHITE);
    DrawText(storyText5, 
             SCREEN_WIDTH / 2 - MeasureText(storyText5, textFontSize) / 2, 
             SCREEN_HEIGHT / 2 + 120, 
             textFontSize, WHITE);
    DrawText(storyText6, 
             SCREEN_WIDTH / 2 - MeasureText(storyText6, textFontSize) / 2, 
             SCREEN_HEIGHT / 2 + 150, 
             textFontSize, WHITE);
    DrawText(storyText7, 
             SCREEN_WIDTH / 2 - MeasureText(storyText7, textFontSize) / 2, 
             SCREEN_HEIGHT / 2 + 180, 
             textFontSize, WHITE);

    // Loading bar
    int barWidth = 600;
    int barHeight = 30;
    int barX = SCREEN_WIDTH / 2 - barWidth / 2;
    int barY = SCREEN_HEIGHT / 2 + 250;

    // Background of loading bar
    DrawRectangle(barX, barY, barWidth, barHeight, LIGHTGRAY);

    // Progress of loading bar
    DrawRectangle(barX, barY, (int)(barWidth * loadingProgress), barHeight, GREEN);

    // Loading text
    DrawText("Loading...",
             SCREEN_WIDTH / 2 - MeasureText("Loading...", 20) / 2,
             barY + barHeight + 20,
             20,
             WHITE);
}

void draw_game(void)
{
    // Draw grid
    draw_grid();

    // Draw Mario
    DrawModelEx(modelMario,
                (Vector3){Mario_coordinates.x * 2.0f, Mario_coordinates.y * 2.0f, Mario_coordinates.z * 2.0f},
                (Vector3){1, 0, 0}, 0.0f, (Vector3){0.16f, 0.16f, 0.16f}, ColorBrightness(RED, 0.5f));

    // Draw Princess
    DrawModelEx(modelPrincess,
                (Vector3){Princess_coordinates.x * 2.0f, Princess_coordinates.y * 2.0f, Princess_coordinates.z * 2.0f},
                (Vector3){1, 0, 0}, 0.0f, (Vector3){0.8f, 0.8f, 0.8f}, ColorBrightness(PINK, 0.5f));

    // Draw Monsters
    for (int i = 0; i < no_of_monsters; i++)
    {
        DrawModelEx(modelMonster,
                    (Vector3){Monsters_coordinates[i].x * 2.0f, Monsters_coordinates[i].y * 2.0f, Monsters_coordinates[i].z * 2.0f},
                    (Vector3){1, 0, 0}, 0.0f, (Vector3){0.003f, 0.003f, 0.003f}, ColorBrightness(GREEN, 0.5f));
    }
    // Draw UI text for Enter control
    DrawText("Hold Enter to animate movement", 10, 100, 25, WHITE);
    DrawText(isPaused ? "PAUSED" : "RUNNING", 10, 130, 25,
             isPaused ? ColorBrightness(RED, 0.3f) : ColorBrightness(GREEN, 0.3f));
}

void draw_grid(void)
{
    // Draw grid lines
    for (int i = 0; i <= m; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            DrawLine3D((Vector3){i * 2.0f, 0, j * 2.0f},
                       (Vector3){i * 2.0f, o * 2.0f, j * 2.0f},
                       Fade((Color){70, 130, 180, 128}, 0.5f)); // Steel Blue with transparency
        }
    }
    for (int i = 0; i <= m; i++)
    {
        for (int k = 0; k <= o; k++)
        {
            DrawLine3D((Vector3){i * 2.0f, k * 2.0f, 0},
                       (Vector3){i * 2.0f, k * 2.0f, n * 2.0f},
                       Fade((Color){255, 160, 122, 255}, 0.7f)); // Light Salmon
        }
    }

    for (int j = 0; j <= n; j++)
    {
        for (int k = 0; k <= o; k++)
        {
            DrawLine3D((Vector3){0, k * 2.0f, j * 2.0f},
                       (Vector3){m * 2.0f, k * 2.0f, j * 2.0f},
                       Fade((Color){255, 160, 122, 255}, 0.7f)); // Light Salmon
        }
    }
}

void draw_winscreen(void)
{
    const char *winText = "MARIO WINS!";
    const char *subText = "Princess has been rescued!";
    const char *instructionText = "Press ENTER to return to menu";

    int winTextWidth = MeasureText(winText, 60);
    int subTextWidth = MeasureText(subText, 30);
    int instructionWidth = MeasureText(instructionText, 20);

    DrawText(winText,
             SCREEN_WIDTH / 2 - winTextWidth / 2,
             SCREEN_HEIGHT / 2 - 60,
             60, GREEN);

    DrawText(subText,
             SCREEN_WIDTH / 2 - subTextWidth / 2,
             SCREEN_HEIGHT / 2,
             30, GREEN);

    DrawText(instructionText,
             SCREEN_WIDTH / 2 - instructionWidth / 2,
             SCREEN_HEIGHT / 2 + 60,
             20, DARKGRAY);
}

void draw_losescreen(void)
{
    const char *loseText = "GAME OVER";
    const char *subText = "Mario LOST!";
    const char *instructionText = "Press ENTER to return to menu";

    int loseTextWidth = MeasureText(loseText, 60);
    int subTextWidth = MeasureText(subText, 30);
    int instructionWidth = MeasureText(instructionText, 20);

    DrawText(loseText,
             SCREEN_WIDTH / 2 - loseTextWidth / 2,
             SCREEN_HEIGHT / 2 - 60,
             60, RED);

    DrawText(subText,
             SCREEN_WIDTH / 2 - subTextWidth / 2,
             SCREEN_HEIGHT / 2,
             30, RED);

    DrawText(instructionText,
             SCREEN_WIDTH / 2 - instructionWidth / 2,
             SCREEN_HEIGHT / 2 + 60,
             20, DARKGRAY);
}

// GUI WALA PART BANDH

//*************************************************************************************/

// for 50ms delay, we use this function
/*void sleep_microseconds(long microseconds)
{
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;           // Convert microseconds to seconds
    ts.tv_nsec = (microseconds % 1000000) * 1000; // Remaining microseconds to nanoseconds
    nanosleep(&ts, NULL);
} **/ 
//yupar wali funciton is not needed kyuki gui bana diya h but rehne do cuz maine bohot mehnat kari h iske liye T-T


// Asking user for gamemode
int gamemode(int gamemode_entered)
{
    printf("Please Select The Game Mode:\nEasy(1)\nMedium(2)\nHard(3)\nCustom(4)\n");
    scanf("%d", &gamemode_entered);
    return gamemode_entered;
}

// Asking user for dimensions and number of monsters
void get_grid_dimensions_and_monsters(int gamemode_entered)
{
    switch (gamemode_entered)
    {
    case 1: // Easy
        m = 4;
        n = 4;
        o = 4;
        no_of_monsters = 2;
        break;
    case 2: // Medium
        m = 8;
        n = 8;
        o = 8;
        no_of_monsters = rand() % 2 + 4; // Random number between 4 and 5
        break;
    case 3: // Hard
        m = 12;
        n = 12;
        o = 12;
        no_of_monsters = rand() % 2 + 7; // Random number between 7 and 8
        break;
    case 4: // Custom but GUI ke saath integrate nahi ho saka don't delete
        while (true)
        { // Loop until valid input is provided
            printf("Enter custom grid dimensions (x y z): ");
            scanf("%d %d %d", &m, &n, &o);

            if (m > 0 && n > 0 && o > 0)
            {          // Ensure dimensions are positive
                break; // Exit loop when valid dimensions are entered
            }

            printf("Invalid grid dimensions. Please enter valid dimensions greater than 0.\n");
        }

        while (true)
        { // Loop for the number of monsters
            printf("Enter the number of monsters: ");
            scanf("%d", &no_of_monsters);

            if (no_of_monsters > 0)
            {          // Ensure the number of monsters is positive
                break; // Exit loop when valid input is provided
            }

            printf("Invalid number of monsters. Please enter a positive number.\n");
        }
        break;
    default:
        printf("Invalid mode selected. Defaulting to Easy.\n");
        m = 5;
        n = 5;
        o = 5;
        no_of_monsters = 2;
    }

    printf("Grid size: %dx%dx%d, Number of monsters: %d\n", m, n, o, no_of_monsters);
}

// Check if coordinates are within the grid
bool is_within_grid(int x, int y, int z)
{
    return (x >= 0 && x < m && y >= 0 && y < n && z >= 0 && z < o);
}

// Push a coordinate onto the stack
void push(Stack *stack, coordinates coord)
{
    if (stack->top < stack->capacity - 1)
    {
        stack->stack_array[++stack->top] = coord;
    }
    else
    {
        printf("Stack overflow! Increase capacity.\n");
    }
}
//IGNORE START
// Pop a coordinate from the stack
//iska zarurat nahi pada 
coordinates pop(Stack *stack)
{
    if (stack->top >= 0)
    {
        return stack->stack_array[stack->top--];
    }
    else
    {
        printf("Stack underflow!\n");
        return (coordinates){-1, -1, -1}; // Invalid coordinate
    }
}
//IGNORE STOP

// Check if a coordinate is already visited
bool is_visited(Stack *stack, coordinates coord)
{
    for (int i = 0; i <= stack->top; i++)
    {
        if (stack->stack_array[i].x == coord.x &&
            stack->stack_array[i].y == coord.y &&
            stack->stack_array[i].z == coord.z)
        {
            return true;
        }
    }
    return false;
}

// Generate coordinates for Mario, princess, and monsters
void generate_coords(int gamemode_entered)
{
    Monsters_coordinates = (coordinates *)malloc(no_of_monsters * sizeof(coordinates));
    if (Monsters_coordinates == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    //IGNORE START
    if (gamemode_entered == 4)
    { // Custom Mode 
        // Get user input for Mario's coordinates
        printf("Enter Mario's coordinates (x y z): ");
        while (true)
        {
            scanf("%d %d %d", &Mario_coordinates.x, &Mario_coordinates.y, &Mario_coordinates.z);
            if (is_within_grid(Mario_coordinates.x, Mario_coordinates.y, Mario_coordinates.z))
            {
                break;
            }
            printf("Invalid coordinates. Please enter coordinates within the grid dimensions.\n");
        }

        // Get user input for Princess's coordinates
        printf("Enter Princess's coordinates (x y z): ");
        while (true)
        {
            scanf("%d %d %d", &Princess_coordinates.x, &Princess_coordinates.y, &Princess_coordinates.z);
            if (is_within_grid(Princess_coordinates.x, Princess_coordinates.y, Princess_coordinates.z))
            {
                break;
            }
            printf("Invalid coordinates. Please enter coordinates within the grid dimensions.\n");
        }

        // Get user input for Monsters' coordinates
        for (int i = 0; i < no_of_monsters; i++)
        {
            printf("Enter Monster %d's coordinates (x y z): ", i + 1);
            while (true)
            {
                scanf("%d %d %d", &Monsters_coordinates[i].x, &Monsters_coordinates[i].y, &Monsters_coordinates[i].z);
                if (is_within_grid(Monsters_coordinates[i].x, Monsters_coordinates[i].y, Monsters_coordinates[i].z))
                {
                    break;
                }
                printf("Invalid coordinates. Please enter coordinates within the grid dimensions.\n");
            }
        }
    }

    //IGNORE STOP

    else
    { // Randomized Coordinates for other modes
        do
        {
            Mario_coordinates.x = rand() % m;
            Mario_coordinates.y = rand() % n;
            Mario_coordinates.z = rand() % o;
        } while (!is_within_grid(Mario_coordinates.x, Mario_coordinates.y, Mario_coordinates.z));

        do
        {
            Princess_coordinates.x = rand() % m;
            Princess_coordinates.y = rand() % n;
            Princess_coordinates.z = rand() % o;
        } while (!is_within_grid(Princess_coordinates.x, Princess_coordinates.y, Princess_coordinates.z));

        for (int i = 0; i < no_of_monsters; i++)
        {
            do
            {
                Monsters_coordinates[i].x = rand() % m;
                Monsters_coordinates[i].y = rand() % n;
                Monsters_coordinates[i].z = rand() % o;
            } while (!is_within_grid(Monsters_coordinates[i].x, Monsters_coordinates[i].y, Monsters_coordinates[i].z));
        }
    }

    // Print coordinates
    printf("Mario coordinates: (%d, %d, %d)\n", Mario_coordinates.x, Mario_coordinates.y, Mario_coordinates.z);
    printf("Princess coordinates: (%d, %d, %d)\n", Princess_coordinates.x, Princess_coordinates.y, Princess_coordinates.z);
    for (int i = 0; i < no_of_monsters; i++)
    {
        printf("Monster %d coordinates: (%d, %d, %d)\n", i + 1, Monsters_coordinates[i].x, Monsters_coordinates[i].y, Monsters_coordinates[i].z);
    }
}

// MOVE KARANE KE LIYE SAARE ENTITIES KO
void move_entity(coordinates *entity, coordinates target, Stack *visited, const char *entity_name)
{
    coordinates best_move = *entity;
    int min_distance = calculate_manhattan_distance(*entity, target);

    coordinates possible_moves[6] = {
        {entity->x + 1, entity->y, entity->z},
        {entity->x - 1, entity->y, entity->z},
        {entity->x, entity->y + 1, entity->z},
        {entity->x, entity->y - 1, entity->z},
        {entity->x, entity->y, entity->z + 1},
        {entity->x, entity->y, entity->z - 1}};

    for (int i = 0; i < 6; i++)
    {
        if (is_valid_move(possible_moves[i], visited))
        {
            int distance = calculate_manhattan_distance(possible_moves[i], target);
            if (distance < min_distance)
            {
                best_move = possible_moves[i];
                min_distance = distance;
            }
        }
    }
    *entity = best_move;
    push(visited, best_move);
    printf("%s moved to (%d, %d, %d)\n", entity_name, entity->x, entity->y, entity->z);
}

// Manhattan distance calculate kar rha h between two
int calculate_manhattan_distance(coordinates a, coordinates b)
{
    return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
}

bool is_valid_move(coordinates new_position, Stack *visited)
{
    return is_within_grid(new_position.x, new_position.y, new_position.z) &&
           !is_visited(visited, new_position);
}

// I mean the name says it all kya hi explain karu alag se
Stack *create_stack(int capacity)
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->stack_array = (coordinates *)malloc(capacity * sizeof(coordinates));
    stack->top = -1; // Initialize top to -1 (empty stack)
    stack->capacity = capacity;
    return stack;
}
void move_monsters(coordinates *monsters, int num_monsters, coordinates mario, coordinates princess)
{
    for (int i = 0; i < num_monsters; i++)
    {
        coordinates target;

        // Determine which target (Mario or Princess) is closer
        if (calculate_manhattan_distance(monsters[i], mario) < calculate_manhattan_distance(monsters[i], princess))
        {
            target = mario; // Move towards Mario
        }
        else
        {
            target = princess; // Move towards Princess
        }

        // Move the monster towards the target
        move_entity(&monsters[i], target, visited_monsters[i], "Monster");
    }
}
//this was not really necessary but good for efficint usage of memory
void freememory()
{
    free(visited_mario->stack_array);
    free(visited_mario);

    for (int i = 0; i < no_of_monsters; i++)
    {
        free(visited_monsters[i]->stack_array);
        free(visited_monsters[i]);
    }
    free(visited_monsters);
    free(Monsters_coordinates);
}
// *************** Libraries ***************

#include <time.h> 
#include <stdio.h>
#include <stdlib.h>


// *************** Global ***************

char screen_tab[25][80];
char display[25*80+1]= {0};
int height = 10000;
char far *video_memory = (unsigned char far *)0xB8000000;


// *************** Game Assets ***************

int game_running = 1;
int police = 1;
int lives = 3;
int score = 0;               // Player's score
int streak = 0;
char obstacle_char = '#';    // Character representing obstacles
char point_char = '$';       // Character representing collectible points
char slide_char = '^';
int runway_length = 3, pas_length = 4, horizon_length = 18, visible_dist = 2;
int state;
int ball_section, ball_position, ball_in_sideways_motion, ball_level;


// *************** Color Sets ***************

#define BLUE_BG_WHITE_TEXT 0x1F // Blue background, white text
#define RED_BG_YELLOW_TEXT 0x4E // Red background, yellow text
#define RED_BG_WHITE_TEXT 0x4F
#define GREEN_BG_BLACK_TEXT 0x20 // Green background, black text
#define GREEN_BG_YELLOW_TEXT 0x2E // Green background, black text
#define CYAN_BG_MAGENTA_TEXT 0xD3 // Cyan background, magenta text
#define BROWN_BG_WHITE_TEXT 0x6F
#define WHITE_BG_BLACK_TEXT 0xF0
#define WHITE_BG_BLULE_TEXT 0xF1
#define DARK_GRAY_BG_LIGHT_GRAY_TEXT 0x70 // Dark gray background, light gray text
#define BLACK_BG_WHITE_TEXT 0x0F // Black background, white text
#define BLACK_BG_MAGENTA_TEXT 0x0D
#define BLACK_BG_BLUE_TEXT 0x01
#define BLACK_BG_GREEN_TEXT 0x02
#define BLACK_BG_CYAN_TEXT 0x03
#define BLACK_BG_RED_TEXT 0x04

int background_color = BLUE_BG_WHITE_TEXT; // Default streak background color
int street_color = BLACK_BG_MAGENTA_TEXT; // Default streak street color


// *************** Obstacle Entities ***************

#define MAX_ENTITIES 1 // Maximum number of obstacles/points
#define LANE_1 (40 - 6) // Left lane
#define LANE_2 40       // Middle lane
#define LANE_3 (40 + 6) // Right lane

typedef struct Entity 
{
    int row;        // Row position
    int col;        // Column position
    char type;      // '#' for obstacle, '$' for point, '^' for slide
    int lane;       // Lane (1, 2, or 3)
} Entity;

Entity entities[MAX_ENTITIES];


// *************** Game Implementation ***************

// Function to set a frequency and play the beep
void playCollisionSound(unsigned char lowFreqByte, unsigned char highFreqByte, int duration)
{
    unsigned char status;

    // Set frequency for the beep
    outportb(0x43, 0xB6);  // Set the command to generate square wave
    outportb(0x42, lowFreqByte);  // Low byte of frequency
    outportb(0x42, highFreqByte);  // High byte of frequency

    // Enable the speaker
    status = inportb(0x61);
    outportb(0x61, status | 0x03); // Turn on the speaker

    // Wait for the beep duration
    delay(duration);  // Duration in milliseconds

    // Turn off the speaker
    outportb(0x61, status);  // Disable the speaker
}

void playGameStartMusic()
{
    // Play a short melody for 2 seconds
    playCollisionSound(0x6C, 0x0D, 300);  // C5, 300 ms
    playCollisionSound(0x4C, 0x0D, 300);  // G4, 300 ms
    playCollisionSound(0xE1, 0x1A, 300);  // D4, 300 ms
    playCollisionSound(0x6F, 0x1D, 300);  // C4, 300 ms
    playCollisionSound(0x3C, 0x22, 500);  // A3, 500 ms
}

void playGameOverMusic()
{
    // Play a descending melody for 3 seconds
    playCollisionSound(0xE4, 0x2C, 500);  // E3, 500 ms
    playCollisionSound(0x9A, 0x31, 500);  // D3, 500 ms
    playCollisionSound(0x9E, 0x38, 500);  // C3, 500 ms
    playCollisionSound(0x6F, 0x1D, 500);  // C4, 500 ms
    playCollisionSound(0x3C, 0x22, 500);  // A3, 500 ms
    playCollisionSound(0x6C, 0x0D, 500);  // G3, 500 ms
}

void playSlideSound()
{
    // Start from a high frequency and decrease to a lower frequency
    unsigned char lowFreq, highFreq;
    int freq;
    for (freq = 1200; freq >= 600; freq -= 100) {
        int divisor = 1193182 / freq;
        lowFreq = divisor & 0xFF;
        highFreq = (divisor >> 8) & 0xFF;
        playCollisionSound(lowFreq, highFreq, 50);  // 50 ms per tone
    }
}

void update_streak()
{
    // Change colors every 25 points
    if (score % 20 == 0 && score != 0) {
        // Change background color (cycle through a set of colors)
        if (background_color == BLUE_BG_WHITE_TEXT)
            background_color = GREEN_BG_BLACK_TEXT;
        else if (background_color == GREEN_BG_BLACK_TEXT)
            background_color = CYAN_BG_MAGENTA_TEXT;
        else if(background_color == CYAN_BG_MAGENTA_TEXT)
            background_color = BLACK_BG_MAGENTA_TEXT;

        // Change street color
        if (street_color == BLACK_BG_MAGENTA_TEXT)
            street_color = BLACK_BG_WHITE_TEXT;
        else if (street_color == BLACK_BG_WHITE_TEXT)
            street_color = BLACK_BG_BLUE_TEXT;
        else if (street_color == BLACK_BG_BLUE_TEXT)
            street_color = WHITE_BG_BLULE_TEXT;
    }
}

void write_string_to_video_memory(int row, int col, char *text, char attribute)
{
    int i = 0;
    unsigned char far *vmem = video_memory + (row * 80 + col) * 2;

    while (text[i] != '\0')
    {
        vmem[i * 2] = text[i];          // Write character
        vmem[i * 2 + 1] = attribute;    // Write attribute (color)
        i++;
    }
}

void display_game_over()
{
    int i, j;
    char game_over_text[] = "GAME OVER! FINAL SCORE:";
    char score_text[20];
    sprintf(score_text, "%d", score); // Convert score to string

    // Clear screen
    for (i = 0; i < 25; i++)
    {
        for (j = 0; j < 80; j++)
        {
            video_memory[(i * 80 + j) * 2] = ' ';
            video_memory[(i * 80 + j) * 2 + 1] = BLACK_BG_WHITE_TEXT;
        }
    }

    // Write "GAME OVER!" message
    write_string_to_video_memory(12, 28, game_over_text, RED_BG_YELLOW_TEXT);
    write_string_to_video_memory(13, 37, score_text, RED_BG_YELLOW_TEXT);
}

void display_score()
{
    char score_text[20];
    sprintf(score_text, "Score: %d", score); // Convert score to string
    write_string_to_video_memory(22, 35, score_text, BLACK_BG_WHITE_TEXT); // Display at row 24, column 0
}

void display_lives()
{
    char lives_text[20];
    sprintf(lives_text, "Lives: %d", lives); // Convert score to string
    write_string_to_video_memory(24, 0, lives_text, BLACK_BG_RED_TEXT); // Display at row 24, column 0
}

void initialize_entities()
{
    int i;
    srand(time(NULL)); // Seed random number generator
    for (i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i].row = 1; // start from top

        // Assign a random lane
        entities[i].lane = (rand() % 3) + 1;

        // Assign column based on the lane
        if (entities[i].lane == 1)
            entities[i].col = LANE_1;
        else if (entities[i].lane == 2)
            entities[i].col = LANE_2;
        else
            entities[i].col = LANE_3;

        // Assign random type
        if(rand() % 3 == 0)
        {
            entities[i].type = obstacle_char;
        }
        else if(rand() % 2 == 0)
        {
            entities[i].type = point_char;
        }
        else
        {
            entities[i].type = slide_char;
        }
    }
}

void update_entities()
{
   int i;
    for (i = 0; i < MAX_ENTITIES; i++)
    {
        // If entity reaches the bottom, reset it to the top and reassign its lane
        if (entities[i].row >= horizon_length)
        {
            initialize_entities();
        }
        // Move entity down by one row
        entities[i].row++;
    }
}

void draw_point()
{
    int i, r, e, pos_a = 0, pos_b = 0, state1 = 0, state2 = 0;

    for (e = 0; e < MAX_ENTITIES; e++)
    {
        // Determine the boundaries of the lane
        pos_a = -1; // Left boundary
        pos_b = -1; // Right boundary
        state1 = 0;
        state2 = 0;

        for (i = 0; i < 80; i++)
        {
            if ((screen_tab[entities[e].row][i] == '/') || (screen_tab[entities[e].row][i] == '\\'))
            {
                state1++;
                if (state1 != state2)
                {
                    if (state1 == entities[e].lane)
                        pos_a = i; // Left boundary
                    else if (state1 == (entities[e].lane + 1))
                    {
                        pos_b = i; // Right boundary
                        break;
                    }
                    state2 = state1;
                }
            }
        }

        if (entities[e].row != 1)
        {
            // Update the horizontal position within the lane
            entities[e].col = (pos_a + pos_b) / 2;
        }

        // Render the point as a 2x2 square
        if (pos_a != -1 && pos_b != -1)
        {
            for (r = 0; r < 2; r++) // 2 rows
            {
                int current_row = entities[e].row + r;
                if (current_row < 25) // Ensure within bounds
                {
                    for (i = -1; i <= 0; i++) // 2 columns centered at col
                    {
                        int current_col = entities[e].col + i;
                        if (current_col >= pos_a + 1 && current_col < pos_b) // Ensure within lane boundaries
                        {
                            screen_tab[current_row][current_col] = '$';
                        }
                    }
                }
            }
        }
    }
}

void draw_slide()
{
    int i, r, e, pos_a = -1, pos_b = -1, state1 = 0, state2 = 0, slide_width, slide_height, start_col;

    for (e = 0; e < MAX_ENTITIES; e++)
    {
        // Determine the boundaries of the lane based on the entity's lane
        state1 = 0;
        state2 = 0;

        for (i = 0; i < 80; i++)
        {
            if ((screen_tab[entities[e].row][i] == '/') || (screen_tab[entities[e].row][i] == '\\'))
            {
                state1++;
                if (state1 != state2)
                {
                    if (state1 == entities[e].lane) // Match the entity's lane
                        pos_a = i; // Left boundary
                    else if (state1 == (entities[e].lane + 1))
                    {
                        pos_b = i; // Right boundary
                        break;
                    }
                    state2 = state1;
                }
            }
        }

        // Update the horizontal position within the selected lane
        if (pos_a != -1 && pos_b != -1)
        {
            entities[e].col = (pos_a + pos_b) / 2; // Center the slide column within the lane

            // Render the slide as a 3x4 rectangle, centered and adjusted to lane boundaries
            slide_width = 4; // Fixed width of 4 columns
            slide_height = 3; // Fixed height of 3 rows
            start_col = entities[e].col - (slide_width / 2); // Center the slide horizontally

            for (r = 0; r < slide_height; r++) // 3 rows
            {
                int current_row = entities[e].row + r; // Rows below the entity's row
                if (current_row < 25) // Ensure within bounds
                {
                    for (i = 0; i < slide_width; i++) // Fixed width
                    {
                        int current_col = start_col + i; // Columns centered at `entities[e].col`
                        if (current_col >= pos_a && current_col < pos_b) // Ensure within lane boundaries
                        {
                            screen_tab[current_row][current_col] = '^'; // Render slide
                        }
                    }
                }
            }
        }
    }
}

void draw_obstacle()
{
    int i, r, e, pos_a = 0, pos_b = 0, state1 = 0, state2 = 0;

    for (e = 0; e < MAX_ENTITIES; e++)
    {
        // Determine the boundaries of the lane
        pos_a = -1; // Left boundary
        pos_b = -1; // Right boundary
        state1 = 0;
        state2 = 0;

        for (i = 0; i < 80; i++)
        {
            if ((screen_tab[entities[e].row][i] == '/') || (screen_tab[entities[e].row][i] == '\\'))
            {
                state1++;
                if (state1 != state2)
                {
                    if (state1 == entities[e].lane)
                        pos_a = i; // Left boundary
                    else if (state1 == (entities[e].lane + 1))
                    {
                        pos_b = i; // Right boundary
                        break;
                    }
                    state2 = state1;
                }
            }
        }

        if (entities[e].row != 1)
        {
            // Update the horizontal position within the lane
            entities[e].col = (pos_a + pos_b) / 2;
        }

        // Render a fixed-size obstacle (3 rows x 5 columns)
        if (pos_a != -1 && pos_b != -1)
        {
            int obstacle_width = 6; // Fixed width
            int obstacle_height = 3; // Fixed height
            int start_col = entities[e].col - (obstacle_width / 2); // Center the obstacle
            int start_row = entities[e].row; // Start from the entity's row

            for (r = 0; r < obstacle_height; r++) // For 3 rows
            {
                int current_row = start_row - r; // Rows above the entity's position
                if (current_row < 25 && current_row >= 0) // Ensure within bounds
                {
                    for (i = start_col; i < start_col + obstacle_width; i++) // Fixed width
                    {
                        if (i >= pos_a && i < pos_b) // Ensure within lane boundaries
                        {
                            screen_tab[current_row][i] = '#'; // Render obstacle
                        }
                    }
                }
            }
        }
    }
}

void draw_void()
{
    int i, r, e;

    for (e = 0; e < MAX_ENTITIES; e++)
    {
        // Render the void across all 3 lanes
        for (r = 0; r < 3; r++) // For 3 rows
        {
            int current_row = entities[e].row - r;

            // Ensure within bounds and restrict void drawing to above row 21
            if (current_row < 13 && current_row >= 0) // Rows 21-24 are untouched
            {
                for (i = 0; i < 80; i++) // Replace the entire row
                {
                    if (screen_tab[current_row][i] == '_' || screen_tab[current_row][i] == '/' || screen_tab[current_row][i] == '\\')
                    {
                        screen_tab[current_row][i] = ' '; // Delete the road, including boundaries
                    }
                }
            }
        }
    }
}

void draw_entity()
{
    int e;
    for(e = 0; e < MAX_ENTITIES; e++)
    {
        if(entities[e].type == obstacle_char)
        {
            draw_obstacle();
        }
        else if(entities[e].type == point_char)
        {
            draw_point();
        }
        else if (entities[e].type == slide_char)
        {
            draw_slide();
            draw_void();
        }
    }
}

void draw_runway()
{
    int i, j, e, k = 2, p = 0, state = 0;

    for (i = 0; i < horizon_length; i++)
    {
        state = 0;
        if (i >= visible_dist)
        {
            for (j = 0; j < pas_length - 3 * k; j++)
            {
                screen_tab[i][40 - runway_length + k] = '/';
                screen_tab[i][40 + runway_length - k - 2] = '\\';
                screen_tab[i][40 - 3 * runway_length + 2 * k - p] = '/';
                screen_tab[i][40 + 3 * runway_length - 2 * k + p] = '\\';
            }

            for (j = 0; j < 80; j++)
            {
                if ((screen_tab[i][j] == '/') || (screen_tab[i][j] == '\\'))
                {
                    state++;
                }
                else if ((state >= 1) && (state <= 3))
                {
                    screen_tab[i][j] = '_';
                }
            }
        }
        k--;
    }
    p++;
}

void draw_car()
{
    int i, e, pos_a = -1, pos_b = -1, state1 = 0, state2 = 0;

    // Determine the lane boundaries for the car
    for (i = 0; i < 80; i++)
    {
        if ((screen_tab[ball_level][i] == '/') || (screen_tab[ball_level][i] == '\\'))
        {
            state1++;
            if (state1 != state2)
            {
                if (state1 == ball_section)
                {
                    pos_a = i;
                }
                else if (state1 == (ball_section + 1))
                {
                    pos_b = i;
                }
                state2 = state1;
            }
        }
    }

    // Adjust car movement
    if ((ball_in_sideways_motion == 1) && (ball_section > 1))
    {
        pos_b = pos_a;
        ball_section--;
        ball_in_sideways_motion = 0;
    }

    else if ((ball_in_sideways_motion == 2) && (ball_section < 3))
    {
        pos_a = pos_b;
        ball_section++;
        ball_in_sideways_motion = 0;
    }

    ball_position = (pos_a + pos_b) / 2;

    // Check collision with entities
    for (e = 0; e < MAX_ENTITIES; e++)
    {
        // Check if any part of the car is on a void (spaces ' ')
        if (screen_tab[ball_level][ball_position] == ' ')       // Void detection
        {
            display_game_over(); // Display game-over message
            playGameOverMusic();
            exit(0); // End game on collision with void
        }

        // Check collision for all parts of the car
        if ((entities[e].row == ball_level ||            // Center
             entities[e].row == ball_level - 1 ||        // Top
             entities[e].row == ball_level - 2) &&       // Bottom
            (entities[e].col == ball_position ||         // Center column
             entities[e].col == ball_position - 3 ||
             entities[e].col == ball_position - 2 ||
             entities[e].col == ball_position - 1 ||     // Left edge
             entities[e].col == ball_position + 3 ||
             entities[e].col == ball_position + 2 ||
             entities[e].col == ball_position + 1))      // Right edge
        {
            if (entities[e].type == obstacle_char)
            {
                   
                lives--;
                if (lives == 0)
                {
                    display_game_over(); // Display game-over message
                    playGameOverMusic(); // Play game over music
                    exit(0); // End game on collision with obstacle
                }
                else
                {
                    playCollisionSound(0x9A, 0x31, 200);  // Trigger sound for obstacle collision
                    // Remove the obstacle from screen_tab
                    screen_tab[entities[e].row][entities[e].col] = '_';
                    initialize_entities();
                    break;
                }
            }
            else if (entities[e].type == slide_char)
            { 
                playSlideSound(); // Play the sound for jumping on a slide

                // Slide behavior: Advance 4 rows up
                ball_level -= 4;
                if (ball_level < 1) // Prevent moving out of bounds
                    ball_level = 1;

                // Adjust position for lanes 1, 2, and 3
                if (entities[e].lane == 1)  // Left lane
                {
                    ball_position += 3; // Adjust right
                }
                else if (entities[e].lane == 3)  // Right lane
                {
                    ball_position -= 3; // Adjust left
                }

                score += 5; // Add score for hitting the slide
                update_streak();
                break;
            }
            else if (entities[e].type == point_char)
            {
                playCollisionSound(0x4C, 0x05, 200);  // Play the sound for collecting a point
                score += 10; // Add points for collectible
                update_streak();            // Check and update streak colors

                // Remove the point from screen_tab
                screen_tab[entities[e].row][entities[e].col] = '_';
                initialize_entities();
                break;
            }
        }
    }

    // Represent the car as a fuller spinning shape
    screen_tab[ball_level][ball_position] = 'V';          // Center of the car
    screen_tab[ball_level][ball_position - 1] = '!';      // Left edge
    screen_tab[ball_level][ball_position + 1] = '!';      // Right edge
    screen_tab[ball_level - 1][ball_position] = '*';      // Top
    screen_tab[ball_level + 1][ball_position] = '-';      // Bottom
    screen_tab[ball_level - 1][ball_position - 1] = 'O';  // Top-left
    screen_tab[ball_level - 1][ball_position + 1] = 'O';  // Top-right
    screen_tab[ball_level + 1][ball_position - 1] = 'O';  // Bottom-left
    screen_tab[ball_level + 1][ball_position + 1] = 'O';  // Bottom-right

    // Move the car one level down
    ball_level--;

    if (ball_level <= 1)
    {
        ball_level = 17;
    }
}

void erase_screen_tab()
{
   int i, j;
   for(i=0; i < 25; i++)
   {
      for(j=0; j < 80; j++)
      {
         screen_tab[i][j] = ' ';
      }
   }
} // erase_screen_tab()

void copy_to_display()
{
    int i, j;
    char byte, color;

    for (i = 0; i < 25; i++)
    {
        for (j = 0; j < 80; j++)
        {
            byte = screen_tab[i][j];

            // Assign color based on the character
            switch (byte)
            {
                case ' ':  // Empty space
                    color = background_color;
                    break;

                case 'V':  // Ball center
                    if(police == 1){
                        color = BLUE_BG_WHITE_TEXT;
                        police = 0;
                        break;
                    }
                    if(police == 0){
                        color = RED_BG_WHITE_TEXT;
                        police = 1;
                        break;
                    }
                case '!':  // Ball edges
                case '-':
                case '*':  // Ball top/bottom
                    color = DARK_GRAY_BG_LIGHT_GRAY_TEXT;
                    break;
                case 'O':
                    color = BROWN_BG_WHITE_TEXT;
                    break;
                case '#':  // Obstacle
                    color = RED_BG_YELLOW_TEXT;
                    break;
                case '$':  // Collectible point
                    color = GREEN_BG_YELLOW_TEXT;
                    break;
                case '^':
                    color = 0x30;
                    break;
                default:   // Default case for unhandled characters
                    color = street_color ;
                    break;
            }

            // Write character and attribute to video memory
            video_memory[(i * 80 + j) * 2] = byte;
            video_memory[(i * 80 + j) * 2 + 1] = color;
        }
    }
}

int main() {
    char scan_code;
    int i;

    system("cls");

    // Initialize car starting position
    ball_section = 2;         // Middle lane
    ball_position = 39;       // Default horizontal position
    ball_in_sideways_motion = 0;
    ball_level = 17;          // Start near the bottom

    // Draw initial runway and entities
    erase_screen_tab();
    draw_runway();
    initialize_entities();
    copy_to_display();  // Copy the initial display to video memory

    // Display score and lives
    display_score();
    display_lives();

    // Now play the game start sound
    playGameStartMusic();

    while (game_running) {
        scan_code = 0;
        asm {
            MOV AH, 1
            INT 16h
            JZ Skip
            MOV AH, 0
            INT 16h
            MOV scan_code, AH
        }
        Skip:

        if (scan_code == 1) {
            asm {
                MOV AH, 4Ch
                INT 21h
            }
        }

        if (scan_code == 75) {
            ball_in_sideways_motion = 1;
        }

        if (scan_code == 77) {
            ball_in_sideways_motion = 2;
        }

        erase_screen_tab();
        draw_runway();       // Draw the runway
        draw_entity();       // Draw all entities (voids, slides, points, etc.)
        draw_car();          // Render the car
        update_entities();   // Move entities downward
        copy_to_display();   // Copy everything to video memory

        display_score();     // Display score
        display_lives();     // Display lives

        delay(500);          // Add delay for smoother gameplay
    }
    return 0;
}
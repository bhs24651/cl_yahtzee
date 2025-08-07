// CL_Yahtzee v1.0
// Command Line Yahtzee Game, Developed by Jason Wu

#include <iostream>
#include <windows.h>

// --- Console VT (ANSI) enable + fallback ---
static bool VT_ENABLED = false;
static HANDLE HOUT = GetStdHandle(STD_OUTPUT_HANDLE);
static WORD DEFAULT_ATTRS = 0;

bool enable_vt()
{
    DWORD mode = 0;
    if (!GetConsoleMode(HOUT, &mode)) return false;
    // Save default attributes
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (GetConsoleScreenBufferInfo(HOUT, &info)) DEFAULT_ATTRS = info.wAttributes;
    // Try to enable VT
    DWORD newMode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(HOUT, newMode)) return false;
    VT_ENABLED = true;
    return true;
}

void clr_screen()
{
    if (VT_ENABLED) {
        std::cout << "\033[2J\033[H";
    } else {
        system("cls"); // legacy fallback
    }
}

void move_to(short r, short c) {
    COORD pos{ static_cast<SHORT>(c - 1), static_cast<SHORT>(r - 1) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

enum Color { NORMAL, YELLOW, RED };
void set_color(Color c)
{
    if (VT_ENABLED) {
        if (c == YELLOW) std::cout << "\033[33m";
        else if (c == RED) std::cout << "\033[31m";
        else std::cout << "\033[0m";
    } else {
        WORD attr = DEFAULT_ATTRS;
        if (c == YELLOW) attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        if (c == RED)    attr = FOREGROUND_RED | FOREGROUND_INTENSITY;
        SetConsoleTextAttribute(HOUT, attr);
    }
}
void reset_color() { set_color(NORMAL); }

#include <random>
#include <conio.h>
#include <algorithm>
#include <set>
#include <thread>
#include <chrono>

using namespace std;

// common escape sequences
const std::string CLEAR_SCREEN = "\033[2J";
const std::string CURSOR_HOME = "\033[H";
const string MOVE_TO = "\033[3;4H";
const string CLEAR_LINE = "\033[2K";
const string HIDE_CURSOR = "\033[?25l";
const string SHOW_CURSOR = "\033[?25h";

// initialize scorecard values
// upper section
int ones = 0; bool ones_final = false; // final score for each section
int twos = 0; bool twos_final = false;
int threes = 0; bool threes_final = false;
int fours = 0; bool fours_final = false;
int fives = 0; bool fives_final = false;
int sixes = 0; bool sixes_final = false;
int upper_section_bonus = 0; int upper_section_bonus_final = false;
int upper_total = 0;
// lower section
int three_of_a_kind = 0; bool three_of_a_kind_final = false; // final score for each section
int four_of_a_kind = 0; bool four_of_a_kind_final = false;
int full_house = 0; bool full_house_final = false;
int sml_straight = 0; bool sml_straight_final = false;
int lrg_straight = 0; bool lrg_straight_final = false;
int yahtzee = 0; bool yahtzee_final = false;
int chance = 0; bool chance_final = false;
// int yahtzee_bonus = 0; (to be implemented soon)
int lower_total = 0;

int grand_total = 0;

// initialize dice values
int d1 = 0; bool d1h = false; // hold status for each die
int d2 = 0; bool d2h = false;
int d3 = 0; bool d3h = false;
int d4 = 0; bool d4h = false;
int d5 = 0; bool d5h = false;
int rolls = 3;

// Forward declarations for potential score helper functions
int count_dice_with_value(int val);
int potential_three_of_a_kind();
int potential_four_of_a_kind();
bool is_full_house();
bool is_small_straight();
bool is_large_straight();
bool is_yahtzee();

// seed rng
std::random_device rd;
std::mt19937 rng(rd());

void clear_screen() {
    cout << CLEAR_SCREEN << CURSOR_HOME;
}

void flush_output_buffer() {
    cout << flush;
}

// finalization helper functions
bool all_upper_final() {
    return ones_final && twos_final && threes_final && fours_final && fives_final && sixes_final;
}
bool all_lower_final() {
    return three_of_a_kind_final && four_of_a_kind_final && full_house_final &&
           sml_straight_final && lrg_straight_final && yahtzee_final && chance_final;
}

void draw_scorecard() {
    cout << "\033[1;1H" << "CL_Yahtzee v1.0 | Developed by Jason Wu" << endl << endl;

    cout << "Scorecard" << endl;
    cout << "-------------------" << endl;
    
    // Upper Section
    cout << "UPPER SECTION" << endl;
    cout << "1s: " << (ones_final ? "\033[33m" : "") << ones << "\033[0m";
    if (!ones_final && rolls < 3) cout << "\b(" << count_dice_with_value(1) * 1 << ")"; // applied backspace chars
    cout << endl;
    cout << "2s: " << (twos_final ? "\033[33m" : "") << twos << "\033[0m";
    if (!twos_final && rolls < 3) cout << "\b(" << count_dice_with_value(2) * 2 << ")";
    cout << endl;
    cout << "3s: " << (threes_final ? "\033[33m" : "") << threes << "\033[0m";
    if (!threes_final && rolls < 3) cout << "\b(" << count_dice_with_value(3) * 3 << ")";
    cout << endl;
    cout << "4s: " << (fours_final ? "\033[33m" : "") << fours << "\033[0m";
    if (!fours_final && rolls < 3) cout << "\b(" << count_dice_with_value(4) * 4 << ")";
    cout << endl;
    cout << "5s: " << (fives_final ? "\033[33m" : "") << fives << "\033[0m";
    if (!fives_final && rolls < 3) cout << "\b(" << count_dice_with_value(5) * 5 << ")";
    cout << endl;
    cout << "6s: " << (sixes_final ? "\033[33m" : "") << sixes << "\033[0m";
    if (!sixes_final && rolls < 3) cout << "\b(" << count_dice_with_value(6) * 6 << ")";
    cout << endl;
    upper_total = ones + twos + threes + fours + fives + sixes;
    if (upper_total >= 63) {
        upper_section_bonus = 35; // bonus for scoring 63 or more
        if (all_upper_final) upper_section_bonus_final = true; // finalizing upper section bonus
    } else {
        upper_section_bonus = 0;
        if (all_upper_final) upper_section_bonus_final = true;
    }
    cout << "Upper Section Bonus: ";
    if (all_upper_final()) cout << "\033[33m";
    cout << upper_section_bonus << "\033[0m" << endl; // changed formatting of all finalized totals
    cout << "Upper Total: ";
    if (all_upper_final()) cout << "\033[33m";
    cout << upper_total + upper_section_bonus << "\033[0m" << endl;

    // Lower Section
    cout << "\033[5;26H" << "| LOWER SECTION" << endl;
    cout << "\033[6;26H" << "| 3 of a Kind: " << (three_of_a_kind_final ? "\033[33m" : "") << three_of_a_kind << "\033[0m";
    if (!three_of_a_kind_final && rolls < 3) cout << "\b(" << potential_three_of_a_kind() << ")";
    cout << endl;
    cout << "\033[7;26H" << "| 4 of a Kind: " << (four_of_a_kind_final ? "\033[33m" : "") << four_of_a_kind << "\033[0m";
    if (!four_of_a_kind_final && rolls < 3) cout << "\b(" << potential_four_of_a_kind() << ")";
    cout << endl;
    cout << "\033[8;26H" << "| Full House: " << (full_house_final ? "\033[33m" : "") << full_house << "\033[0m";
    if (!full_house_final && rolls < 3) cout << "\b(" << (is_full_house() ? 25 : 0) << ")";
    cout << endl;
    cout << "\033[9;26H" << "| Small Straight: " << (sml_straight_final ? "\033[33m" : "") << sml_straight << "\033[0m";
    if (!sml_straight_final && rolls < 3) cout << "\b(" << (is_small_straight() ? 30 : 0) << ")";
    cout << endl;
    cout << "\033[10;26H" << "| Large Straight: " << (lrg_straight_final ? "\033[33m" : "") << lrg_straight << "\033[0m";
    if (!lrg_straight_final && rolls < 3) cout << "\b(" << (is_large_straight() ? 40 : 0) << ")";
    cout << endl;
    cout << "\033[11;26H" << "| Yahtzee: " << (yahtzee_final ? "\033[33m" : "") << yahtzee << "\033[0m";
    if (!yahtzee_final && rolls < 3) cout << "\b(" << (is_yahtzee() ? 50 : 0) << ")";
    cout << endl;
    cout << "\033[12;26H" << "| Chance: " << (chance_final ? "\033[33m" : "") << chance << "\033[0m";
    if (!chance_final && rolls < 3) cout << "\b(" << (d1 + d2 + d3 + d4 + d5) << ")";
    cout << endl;

    lower_total = three_of_a_kind + four_of_a_kind + full_house + sml_straight + lrg_straight + yahtzee + chance;
    grand_total = upper_total + upper_section_bonus + lower_total;
    if (all_lower_final())
        cout;
    cout << "\033[13;26H" << "| Lower Total: "; 
    if (all_lower_final()) cout << "\033[33m";
    cout << lower_total << "\033[0m" << endl;
    cout << "-------------------" << endl; //    ...including these two as well.
    cout << "Grand Total: ";
    if (all_upper_final() && all_lower_final()) cout << "\033[33m";
    cout << grand_total << "\033[0m" << endl;
    cout << "-------------------" << endl << endl;
    flush_output_buffer();
}

void draw_dice() {
    cout << "\033[18;1H" << "Dice: ";
    if (rolls == 0) cout << "\033[31m"; // RED if no rolls left
    cout << rolls << " ROLL(S) LEFT";
    if (rolls == 0) cout << "\033[0m";
    cout << endl << endl;
    cout << "   " << d1 << "   " << d2 << "   " << d3 << "   " << d4 << "   " << d5 << endl;
    cout << "   " << (d1h ? "H" : " ") << "   " << (d2h ? "H" : " ") << "   " << (d3h ? "H" : " ") << "   " << (d4h ? "H" : " ") << "   " << (d5h ? "H" : " ") << endl;
    cout << "-------------------" << endl;
}


void draw_commands_before_first_roll() {
    cout << "\033[23;1H" << "[Space] : Roll all dice" << endl << endl;
}

void draw_commands_after_dice_roll() {
    cout << "\033[23;1H";
    cout << "[1] Hold D1    [4] Hold D4" << endl;
    cout << "[2] Hold D2    [5] Hold D5" << endl;
    cout << "[3] Hold D3    [0] Submit" << endl;
    cout << "[Space] Reroll all unheld dice" << endl << endl;
}


void draw_commands_select_section(int r) {
    cout << "\033[23;1H" << "Select Section:" << endl;
    cout << "[1] Upper Section    [2] Lower Section" << endl;
    // cout << (r != 0 ? "[0] Return to previous menu" : "") << endl << endl;
    if (r != 0) {
        cout << "[0] Return to previous menu" << endl << endl;
    } else {
        cout << endl << endl;
    }
}


void draw_commands_select_upper_section() {
    cout << "\033[23;1H" << "Select slot from Upper Section:" << endl;
    if (!ones_final || !fours_final) // fixed indenting issues
        cout << ( !ones_final ? "[1] 1s" : "      ") << "         " << ( !fours_final ? "[4] 4s" : "" ) << endl;
    if (!twos_final || !fives_final)
        cout << ( !twos_final ? "[2] 2s" : "      ") << "         " << ( !fives_final ? "[5] 5s" : "" ) << endl;
    if (!threes_final || !sixes_final)
        cout << ( !threes_final ? "[3] 3s" : "      ") << "         " << ( !sixes_final ? "[6] 6s" : "" ) << endl;
    cout << "[0] Return to previous menu" << endl << endl;
}

void draw_commands_select_lower_section() {
    cout << "\033[23;1H" << "Select slot from Lower Section:" << endl;
    // First column: 3K, FH, LS, Chance. Second: 4K, SS, Yahtzee
    if (!three_of_a_kind_final || !four_of_a_kind_final) // fixed indenting issues
        cout << ( !three_of_a_kind_final ? "[1] 3 of a Kind   " : "                  ")
             << "   " << ( !four_of_a_kind_final ? "[2] 4 of a Kind" : "" ) << endl;
    if (!full_house_final || !sml_straight_final)
        cout << ( !full_house_final ? "[3] Full House    " : "                  ")
             << "   " << ( !sml_straight_final ? "[4] Small Straight" : "" ) << endl;
    if (!lrg_straight_final || !yahtzee_final)
        cout << ( !lrg_straight_final ? "[5] Large Straight" : "                  ")
             << "   " << ( !yahtzee_final ? "[6] Yahtzee" : "" ) << endl;
    if (!chance_final)
        cout << "[7] Chance" << endl;
    cout << "[0] Return to previous menu" << endl << endl;
}

// Dice rolling animation
void animate_dice_roll() {
    int temp_d1 = d1, temp_d2 = d2, temp_d3 = d3, temp_d4 = d4, temp_d5 = d5;
    for (int i = 0; i < 30; ++i) {
        if (!d1h) temp_d1 = std::uniform_int_distribution<int>(1, 6)(rng);
        if (!d2h) temp_d2 = std::uniform_int_distribution<int>(1, 6)(rng);
        if (!d3h) temp_d3 = std::uniform_int_distribution<int>(1, 6)(rng);
        if (!d4h) temp_d4 = std::uniform_int_distribution<int>(1, 6)(rng);
        if (!d5h) temp_d5 = std::uniform_int_distribution<int>(1, 6)(rng);

        // Draw animation frame

        // temporarily override global values for animation only
        int old_d1 = d1, old_d2 = d2, old_d3 = d3, old_d4 = d4, old_d5 = d5;
        d1 = temp_d1; d2 = temp_d2; d3 = temp_d3; d4 = temp_d4; d5 = temp_d5;
        draw_dice();
        d1 = old_d1; d2 = old_d2; d3 = old_d3; d4 = old_d4; d5 = old_d5;

        flush_output_buffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void show_cursor() {
    cout << SHOW_CURSOR;
}

void hide_cursor() {
    cout << HIDE_CURSOR;
}

// Helper functions for potential scores
int count_dice_with_value(int val) {
    int arr[5] = {d1, d2, d3, d4, d5};
    int cnt = 0;
    for (int i = 0; i < 5; ++i) if (arr[i] == val) cnt++;
    return cnt;
}

int potential_three_of_a_kind() {
    int arr[6] = {0};
    int dice[5] = {d1, d2, d3, d4, d5};
    for (int i = 0; i < 5; ++i) arr[dice[i]-1]++;
    for (int i = 0; i < 6; ++i) if (arr[i] >= 3) return d1 + d2 + d3 + d4 + d5;
    return 0;
}

int potential_four_of_a_kind() {
    int arr[6] = {0};
    int dice[5] = {d1, d2, d3, d4, d5};
    for (int i = 0; i < 5; ++i) arr[dice[i]-1]++;
    for (int i = 0; i < 6; ++i) if (arr[i] >= 4) return d1 + d2 + d3 + d4 + d5;
    return 0;
}

bool is_full_house() {
    int arr[6] = {0};
    int dice[5] = {d1, d2, d3, d4, d5};
    for (int i = 0; i < 5; ++i) arr[dice[i]-1]++;
    bool has3 = false, has2 = false;
    for (int i = 0; i < 6; ++i) {
        if (arr[i] == 3) has3 = true;
        if (arr[i] == 2) has2 = true;
    }
    return has3 && has2;
}

bool is_small_straight() {
    int arr[6] = {0};
    int dice[5] = {d1, d2, d3, d4, d5};
    for (int i = 0; i < 5; ++i) arr[dice[i]-1] = 1;
    // check for 4 consecutive
    for (int i = 0; i < 3; ++i) {
        if (arr[i] && arr[i+1] && arr[i+2] && arr[i+3]) return true;
    }
    return false;
}

bool is_large_straight() {
    int arr[6] = {0};
    int dice[5] = {d1, d2, d3, d4, d5};
    for (int i = 0; i < 5; ++i) arr[dice[i]-1] = 1;
    // check for 5 consecutive
    if (arr[0] && arr[1] && arr[2] && arr[3] && arr[4]) return true;
    if (arr[1] && arr[2] && arr[3] && arr[4] && arr[5]) return true;
    return false;
}

bool is_yahtzee() {
    return (d1 == d2 && d2 == d3 && d3 == d4 && d4 == d5);
}

// Resets dice, holds and rolls for a new turn
void reset_dice_and_holds() {
    d1 = d2 = d3 = d4 = d5 = 0;
    d1h = d2h = d3h = d4h = d5h = false;
    rolls = 3;
}

int main() {
    enable_vt(); // try VT; if it fails we'll use the Win32 fallback helpers

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // prevent starting screen from being drawn twice
    // clear_screen();
    // draw_scorecard();
    // draw_dice();
    // draw_commands_before_first_roll();
    // flush_output_buffer();

    char cmd = ' ';
    bool can_roll = true;

    for (int turn = 0; turn < 13; ++turn) {
        bool turn_complete = false;
        reset_dice_and_holds();

        while (!turn_complete) {
            // ROLLING PHASE
            bool submitted = false;
            can_roll = true;
            while (can_roll && !submitted) {
                clear_screen();
                draw_scorecard();
                draw_dice();
                if (rolls == 3) draw_commands_before_first_roll();
                else draw_commands_after_dice_roll();
                flush_output_buffer();

                cmd = _getch();
                if (cmd == ' ' && rolls > 0) {
                    animate_dice_roll();
                    if (!d1h) { d1 = std::uniform_int_distribution<int>(1, 6)(rng); }
                    if (!d2h) { d2 = std::uniform_int_distribution<int>(1, 6)(rng); }
                    if (!d3h) { d3 = std::uniform_int_distribution<int>(1, 6)(rng); }
                    if (!d4h) { d4 = std::uniform_int_distribution<int>(1, 6)(rng); }
                    if (!d5h) { d5 = std::uniform_int_distribution<int>(1, 6)(rng); }
                    rolls--;
                    if (rolls == 0) can_roll = false;
                } else if (cmd >= '1' && cmd <= '5' && rolls < 3) {
                    int die_index = cmd - '1';
                    switch (die_index) {
                        case 0: d1h = !d1h; break;
                        case 1: d2h = !d2h; break;
                        case 2: d3h = !d3h; break;
                        case 3: d4h = !d4h; break;
                        case 4: d5h = !d5h; break;
                    }
                } else if (cmd == '0' && rolls < 3) {
                    // replace with break for bugfix
                    break;
                } else if (cmd == 27 || cmd == 3) {
                    return 0;
                }
            }

            // SCORING PHASE
            while (true) {
                clear_screen();
                draw_scorecard();
                draw_dice();
                draw_commands_select_section(rolls);
                flush_output_buffer();
                cmd = _getch();

                if (cmd == 27 || cmd == 3) return 0;
                if (cmd == '0' && rolls > 0) break; // Go back to dice rolling with same dice/rolls (if rolls greater than 0)

                bool scored = false;
                if (cmd == '1') {
                    // UPPER SECTION SELECTION
                    while (true) {
                        clear_screen();
                        draw_scorecard();
                        draw_dice();
                        draw_commands_select_upper_section();
                        flush_output_buffer();
                        cmd = _getch();
                        bool valid = false;
                        int points = 0;
                        string slot_name;
                        switch (cmd) {
                            case '1':
                                if (!ones_final)   { points = 1 * count_dice_with_value(1); slot_name = "1s"; }
                                break;
                            case '2':
                                if (!twos_final)   { points = 2 * count_dice_with_value(2); slot_name = "2s"; }
                                break;
                            case '3':
                                if (!threes_final) { points = 3 * count_dice_with_value(3); slot_name = "3s"; }
                                break;
                            case '4':
                                if (!fours_final)  { points = 4 * count_dice_with_value(4); slot_name = "4s"; }
                                break;
                            case '5':
                                if (!fives_final)  { points = 5 * count_dice_with_value(5); slot_name = "5s"; }
                                break;
                            case '6':
                                if (!sixes_final)  { points = 6 * count_dice_with_value(6); slot_name = "6s"; }
                                break;
                            case '0':
                                break;
                            case 27: case 3:
                                return 0;
                        }
                        if (cmd == '0') break;

                        if (!slot_name.empty()) {
                            // redraw commands section before drawing confirmation message
                            clear_screen();
                            draw_scorecard();
                            draw_dice();
                            cout << "Confirm submission of " << points << " points to " << slot_name << "?\n";
                            cout << "[1] Yes   [0] No" << endl;
                            flush_output_buffer();
                            char confirm = _getch();
                            cout << confirm << endl;
                            if (confirm == '1') {
                                switch (cmd) {
                                    case '1': ones = points; ones_final = true; break;
                                    case '2': twos = points; twos_final = true; break;
                                    case '3': threes = points; threes_final = true; break;
                                    case '4': fours = points; fours_final = true; break;
                                    case '5': fives = points; fives_final = true; break;
                                    case '6': sixes = points; sixes_final = true; break;
                                }
                                valid = true;
                            }
                        }
                        if (valid) { scored = true; break; }
                    }
                } else if (cmd == '2') {
                    // LOWER SECTION SELECTION
                    while (true) {
                        clear_screen();
                        draw_scorecard();
                        draw_dice();
                        draw_commands_select_lower_section();
                        flush_output_buffer();
                        cmd = _getch();
                        bool valid = false;
                        int points = 0;
                        string slot_name;
                        switch (cmd) {
                            case '1':
                                if (!three_of_a_kind_final)   { points = potential_three_of_a_kind(); slot_name = "3 of a Kind"; }
                                break;
                            case '2':
                                if (!four_of_a_kind_final)    { points = potential_four_of_a_kind(); slot_name = "4 of a Kind"; }
                                break;
                            case '3':
                                if (!full_house_final)        { points = is_full_house() ? 25 : 0; slot_name = "Full House"; }
                                break;
                            case '4':
                                if (!sml_straight_final)      { points = is_small_straight() ? 30 : 0; slot_name = "Small Straight"; }
                                break;
                            case '5':
                                if (!lrg_straight_final)      { points = is_large_straight() ? 40 : 0; slot_name = "Large Straight"; }
                                break;
                            case '6':
                                if (!yahtzee_final)           { points = is_yahtzee() ? 50 : 0; slot_name = "Yahtzee"; }
                                break;
                            case '7':
                                if (!chance_final)            { points = d1 + d2 + d3 + d4 + d5; slot_name = "Chance"; }
                                break;
                            case '0':
                                break;
                            case 27: case 3:
                                return 0;
                        }
                        if (cmd == '0') break;

                        if (!slot_name.empty()) {
                            // redraw commands section before drawing confirmation message
                            clear_screen();
                            draw_scorecard();
                            draw_dice();
                            cout << "Confirm submission of " << points << " points to " << slot_name << "?\n";
                            cout << "[1] Yes   [0] No" << endl;
                            flush_output_buffer();
                            char confirm = _getch();
                            cout << confirm << endl;
                            if (confirm == '1') {
                                switch (cmd) {
                                    case '1': three_of_a_kind = points; three_of_a_kind_final = true; break;
                                    case '2': four_of_a_kind = points; four_of_a_kind_final = true; break;
                                    case '3': full_house = points; full_house_final = true; break;
                                    case '4': sml_straight = points; sml_straight_final = true; break;
                                    case '5': lrg_straight = points; lrg_straight_final = true; break;
                                    case '6': yahtzee = points; yahtzee_final = true; break;
                                    case '7': chance = points; chance_final = true; break;
                                }
                                valid = true;
                            }
                        }
                        if (valid) { scored = true; break; }
                    }
                }
                if (scored) { turn_complete = true; break; }
            } // end scoring phase

        } // end while(!turn_complete)
    } // end for(turn)

    clear_screen();
    draw_scorecard();
    cout << "Game over! Press any key to exit..." << endl;
    _getch();
}

// why did i make this
// to build: g++ -O2 -static -static-libstdc++ -static-libgcc cl_yahtzee.cpp -o cl_yahtzee.exe -lwinpthread
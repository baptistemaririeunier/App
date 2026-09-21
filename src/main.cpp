#include <extisp.h>
#include <kandinsky.h>
#include <ion.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Dimensions de l'ecran NumWorks : 320x240 pixels
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Palette de couleurs (Format KDColor)
const KDColor COLOR_BG = KDColor::RGB24(0x18, 0x18, 0x18);
const KDColor COLOR_TEXT = KDColor::RGB24(0xFF, 0xFF, 0xFF);
const KDColor COLOR_GREEN = KDColor::RGB24(0x2E, 0xCC, 0x71);
const KDColor COLOR_RED = KDColor::RGB24(0xE7, 0x4C, 0x3C);
const KDColor COLOR_BLUE = KDColor::RGB24(0x34, 0x98, 0xDB);
const KDColor COLOR_GOLD = KDColor::RGB24(0xF1, 0xC4, 0x0F);

enum State {
    MENU_MAIN,
    CHOICE_CLASS,
    VILLAGE,
    COMBAT,
    INVENTORY,
    QUESTS,
    GAME_OVER
};

struct Player {
    char name[16];
    char classe[10];
    int pv;
    int pv_max;
    int mana;
    int mana_max;
    int degats;
    int gold;
    int xp;
    int xp_max;
    int level;
    int acces;
    int potions;
    int weapon_tier;
};

struct Monster {
    char name[20];
    int pv;
    int pv_max;
    int degats;
    int gold;
    int xp;
};

Player player;
Monster current_monster;
State current_state = MENU_MAIN;
int current_quest = 1;
int quest_progress = 0;
int quest_goal = 15;
int selected_menu_option = 0;

void clear_screen() {
    KDRect rect = KDRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    KDFillRect(rect, COLOR_BG);
}

void draw_header(const char* title) {
    KDFillRect(KDRect(0, 0, SCREEN_WIDTH, 25), COLOR_BLUE);
    KDFillRect(KDRect(0, 23, SCREEN_WIDTH, 2), COLOR_TEXT);
    KDDrawString(title, KDPoint(10, 5), COLOR_TEXT, COLOR_BLUE, KDFont::Size::Large);
}

void wait_key_unpressed() {
    while (Ion::Keyboard::scan().keyDown(Ion::Keyboard::Key::OK) || 
           Ion::Keyboard::scan().keyDown(Ion::Keyboard::Key::EXE)) {
        Ion::Timing::msleep(20);
    }
}

void init_player(const char* name, int is_mage) {
    strncpy(player.name, name, 15);
    player.gold = 20;
    player.xp = 0;
    player.xp_max = 50;
    player.level = 1;
    player.acces = 1;
    player.potions = 1;
    player.weapon_tier = 1;

    if (is_mage) {
        strcpy(player.classe, "Mage");
        player.pv = player.pv_max = 35;
        player.mana = player.mana_max = 50;
        player.degats = 12;
    } else {
        strcpy(player.classe, "Guerrier");
        player.pv = player.pv_max = 50;
        player.mana = player.mana_max = 30;
        player.degats = 12;
    }
}

void start_combat() {
    current_state = COMBAT;
    int r = rand() % 100;
    if (player.acces == 1) {
        if (r < 50) { strcpy(current_monster.name, "Rat Geant"); current_monster.pv = current_monster.pv_max = 20; current_monster.degats = 4; current_monster.gold = 5; current_monster.xp = 5; }
        else { strcpy(current_monster.name, "Gobelin"); current_monster.pv = current_monster.pv_max = 40; current_monster.degats = 8; current_monster.gold = 10; current_monster.xp = 10; }
    } else if (player.acces == 2) {
        if (r < 50) { strcpy(current_monster.name, "Squelette"); current_monster.pv = current_monster.pv_max = 70; current_monster.degats = 12; current_monster.gold = 20; current_monster.xp = 20; }
        else { strcpy(current_monster.name, "Zombie"); current_monster.pv = current_monster.pv_max = 100; current_monster.degats = 10; current_monster.gold = 25; current_monster.xp = 25; }
    } else {
        strcpy(current_monster.name, "Demon"); current_monster.pv = current_monster.pv_max = 180; current_monster.degats = 22; current_monster.gold = 50; current_monster.xp = 50;
    }
}

void draw_village() {
    clear_screen();
    draw_header("ARCADIA - Village");

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s (%s) Lv.%d", player.name, player.classe, player.level);
    KDDrawString(buffer, KDPoint(10, 35), COLOR_TEXT, COLOR_BG, KDFont::Size::Large);

    snprintf(buffer, sizeof(buffer), "PV: %d/%d  MP: %d/%d  Or: %d", player.pv, player.pv_max, player.mana, player.mana_max, player.gold);
    KDDrawString(buffer, KDPoint(10, 55), COLOR_GOLD, COLOR_BG, KDFont::Size::Small);

    const char* options[] = {
        "1. Explorer (Combat)",
        "2. Marchand",
        "3. Inventaire / Potions",
        "4. Auberge (Soin total: 20G)",
        "5. Quetes"
    };

    for (int i = 0; i < 5; i++) {
        KDColor fg = (selected_menu_option == i) ? COLOR_GOLD : COLOR_TEXT;
        KDDrawString(options[i], KDPoint(20, 90 + i * 25), fg, COLOR_BG, KDFont::Size::Large);
    }
}

void draw_combat() {
    clear_screen();
    draw_header("COMBAT");

    char buffer[64];
    // Joueur
    snprintf(buffer, sizeof(buffer), "%s - PV: %d/%d MP: %d/%d", player.name, player.pv, player.pv_max, player.mana, player.mana_max);
    KDDrawString(buffer, KDPoint(10, 35), COLOR_GREEN, COLOR_BG, KDFont::Size::Small);

    // Monstre
    snprintf(buffer, sizeof(buffer), "%s - PV: %d/%d", current_monster.name, current_monster.pv, current_monster.pv_max);
    KDDrawString(buffer, KDPoint(10, 65), COLOR_RED, COLOR_BG, KDFont::Size::Large);

    // Barres de vie
    int bar_p = (player.pv * 100) / player.pv_max;
    int bar_m = (current_monster.pv * 100) / current_monster.pv_max;
    KDFillRect(KDRect(10, 50, bar_p, 8), COLOR_GREEN);
    KDFillRect(KDRect(10, 85, bar_m, 8), COLOR_RED);

    const char* actions[] = {
        "1. Attaque Physique",
        "2. Attaque Magique (20 MP)",
        "3. Utiliser Potion",
        "4. Fuir"
    };

    for (int i = 0; i < 4; i++) {
        KDColor fg = (selected_menu_option == i) ? COLOR_GOLD : COLOR_TEXT;
        KDDrawString(actions[i], KDPoint(20, 110 + i * 25), fg, COLOR_BG, KDFont::Size::Large);
    }
}

extern "C" void ion_app_main() {
    Ion::Timing::msleep(100);
    clear_screen();

    bool running = true;
    while (running) {
        Ion::Keyboard::State key_state = Ion::Keyboard::scan();

        if (key_state.keyDown(Ion::Keyboard::Key::Back)) {
            if (current_state == VILLAGE) running = false;
            else { current_state = VILLAGE; selected_menu_option = 0; }
            wait_key_unpressed();
        }

        switch (current_state) {
            case MENU_MAIN:
                clear_screen();
                draw_header("ARCADIA RPG - NWA Native");
                KDDrawString("Appuyez sur OK pour jouer", KDPoint(30, 120), COLOR_GOLD, COLOR_BG, KDFont::Size::Large);
                if (key_state.keyDown(Ion::Keyboard::Key::OK) || key_state.keyDown(Ion::Keyboard::Key::EXE)) {
                    current_state = CHOICE_CLASS;
                    selected_menu_option = 0;
                    wait_key_unpressed();
                }
                break;

            case CHOICE_CLASS:
                clear_screen();
                draw_header("Choisissez votre classe");
                KDDrawString("1. Mage (Moins de PV, Fort Mana)", KDPoint(10, 80), (selected_menu_option == 0) ? COLOR_GOLD : COLOR_TEXT, COLOR_BG, KDFont::Size::Small);
                KDDrawString("2. Guerrier (Plus de PV, Attaque)", KDPoint(10, 120), (selected_menu_option == 1) ? COLOR_GOLD : COLOR_TEXT, COLOR_BG, KDFont::Size::Small);

                if (key_state.keyDown(Ion::Keyboard::Key::Up)) selected_menu_option = 0;
                if (key_state.keyDown(Ion::Keyboard::Key::Down)) selected_menu_option = 1;

                if (key_state.keyDown(Ion::Keyboard::Key::OK) || key_state.keyDown(Ion::Keyboard::Key::EXE)) {
                    init_player("Heros", selected_menu_option == 0);
                    current_state = VILLAGE;
                    selected_menu_option = 0;
                    wait_key_unpressed();
                }
                break;

            case VILLAGE:
                draw_village();
                if (key_state.keyDown(Ion::Keyboard::Key::Up)) { selected_menu_option = (selected_menu_option + 4) % 5; wait_key_unpressed(); }
                if (key_state.keyDown(Ion::Keyboard::Key::Down)) { selected_menu_option = (selected_menu_option + 1) % 5; wait_key_unpressed(); }

                if (key_state.keyDown(Ion::Keyboard::Key::OK) || key_state.keyDown(Ion::Keyboard::Key::EXE)) {
                    wait_key_unpressed();
                    if (selected_menu_option == 0) { start_combat(); selected_menu_option = 0; }
                    else if (selected_menu_option == 3) {
                        if (player.gold >= 20) {
                            player.gold -= 20;
                            player.pv = player.pv_max;
                            player.mana = player.mana_max;
                        }
                    }
                }
                break;

            case COMBAT:
                draw_combat();
                if (key_state.keyDown(Ion::Keyboard::Key::Up)) { selected_menu_option = (selected_menu_option + 3) % 4; wait_key_unpressed(); }
                if (key_state.keyDown(Ion::Keyboard::Key::Down)) { selected_menu_option = (selected_menu_option + 1) % 4; wait_key_unpressed(); }

                if (key_state.keyDown(Ion::Keyboard::Key::OK) || key_state.keyDown(Ion::Keyboard::Key::EXE)) {
                    wait_key_unpressed();
                    int dmg = 0;
                    if (selected_menu_option == 0) dmg = player.degats;
                    else if (selected_menu_option == 1 && player.mana >= 20) { dmg = player.degats * 2; player.mana -= 20; }
                    else if (selected_menu_option == 2 && player.potions > 0) { player.pv = player.pv_max; player.potions--; }
                    else if (selected_menu_option == 3) { current_state = VILLAGE; selected_menu_option = 0; break; }

                    if (dmg > 0) {
                        current_monster.pv -= dmg;
                        if (current_monster.pv <= 0) {
                            player.gold += current_monster.gold;
                            player.xp += current_monster.xp;
                            quest_progress++;
                            if (quest_progress >= quest_goal) {
                                player.acces = 2; // Debloque les ruines
                            }
                            current_state = VILLAGE;
                            selected_menu_option = 0;
                            break;
                        }
                    }

                    // Tour du monstre
                    player.pv -= current_monster.degats;
                    player.mana = (player.mana + 3 > player.mana_max) ? player.mana_max : player.mana + 3;

                    if (player.pv <= 0) {
                        current_state = GAME_OVER;
                    }
                }
                break;

            case GAME_OVER:
                clear_screen();
                draw_header("PARTIE TERMINEE");
                KDDrawString("Vous etes mort...", KDPoint(80, 100), COLOR_RED, COLOR_BG, KDFont::Size::Large);
                KDDrawString("Appuyez sur OK pour revenir", KDPoint(30, 140), COLOR_TEXT, COLOR_BG, KDFont::Size::Small);
                if (key_state.keyDown(Ion::Keyboard::Key::OK) || key_state.keyDown(Ion::Keyboard::Key::EXE)) {
                    current_state = MENU_MAIN;
                    wait_key_unpressed();
                }
                break;

            default:
                break;
        }

        Ion::Timing::msleep(20);
    }
}

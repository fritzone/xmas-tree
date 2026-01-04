#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>


// DATA TYPES
typedef struct {
    int r, g, b;
    int bg_r, bg_g, bg_b; 
    int is_empty;       
    char* symbol;       
    int is_wide;        
} Pixel;

typedef struct {
    int active;
    int style;     
    float freq;
    float phase;
    float amp;
    int r, g, b;
} Garland;

// CONFIGURATION
const int VIRTUAL_HEIGHT = 80; 
const int WIDTH_MAX = 30;     
const int CANVAS_WIDTH = 80;  
const int CENTER_X = 40;      

// 5x5 BOLD BITMAP FONT
const int FONT_M[5][5] = {
    {1,0,0,0,1}, {1,1,0,1,1}, {1,0,1,0,1}, {1,0,0,0,1}, {1,0,0,0,1}
};
const int FONT_E[5][5] = {
    {1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,0}, {1,0,0,0,0}, {1,1,1,1,1}
};
const int FONT_R[5][5] = {
    {1,1,1,1,0}, {1,0,0,0,1}, {1,1,1,1,0}, {1,0,1,0,0}, {1,0,0,1,1}
};
const int FONT_Y[5][5] = {
    {1,0,0,0,1}, {0,1,0,1,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}
};
const int FONT_X[5][5] = {
    {1,0,0,0,1}, {0,1,0,1,0}, {0,0,1,0,0}, {0,1,0,1,0}, {1,0,0,0,1}
};
const int FONT_DASH[5][5] = {
    {0,0,0,0,0}, {0,0,0,0,0}, {1,1,1,1,1}, {0,0,0,0,0}, {0,0,0,0,0}
};
const int FONT_A[5][5] = {
    {0,1,1,1,0}, {1,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,1}, {1,0,0,0,1}
};
const int FONT_S[5][5] = {
    {0,0,1,1,1}, {1,0,0,0,0}, {0,1,1,1,0}, {0,0,0,0,1}, {1,1,1,1,0}
};

int get_font_pixel(const int letter[5][5], int px, int py) {
    if (px < 0 || px >= 5 || py < 0 || py >= 5) return 0;
    return letter[py][px];
}

// STAR COLLECTION
const char* STARS[] = {
    "★", "☆", "✶", "✴", "✭", "✫", "✪", "✰", "✧", "✦", "✥", "✣", "✤", "✱", "✲"
};
const int NUM_STARS = 15;

// UTILS
float rand_float() { return (float)rand() / (float)RAND_MAX; }

float noise_2d(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

void blend_color(int* r, int* g, int* b, int tr, int tg, int tb, float alpha) {
    if (alpha <= 0.0f) return;
    if (alpha >= 1.0f) { *r = tr; *g = tg; *b = tb; return; }
    *r = (int)(*r * (1.0f - alpha) + tr * alpha);
    *g = (int)(*g * (1.0f - alpha) + tg * alpha);
    *b = (int)(*b * (1.0f - alpha) + tb * alpha);
}

Pixel get_pixel_at(int x, int y, Garland* garlands, int num_garlands) {
    Pixel p = {0, 0, 0, 0, 0, 0, 1, NULL, 0}; 

    // 0. TEXT RENDERING
    // Left Side: "MERRY" (x=2)
    // Right Side: "X-MAS" (x=73)
    
    const int(*curr_font)[5] = NULL;
    int letter_y_start = 0;
    int start_y = 15;
    
    // LEFT SIDE: MERRY (x: 2-6)
    if (x >= 2 && x < 7) {
        if      (y >= start_y && y < start_y+5)       { curr_font = FONT_M; letter_y_start = start_y; }
        else if (y >= start_y+9 && y < start_y+14)    { curr_font = FONT_E; letter_y_start = start_y+9; }
        else if (y >= start_y+18 && y < start_y+23)   { curr_font = FONT_R; letter_y_start = start_y+18; }
        else if (y >= start_y+27 && y < start_y+32)   { curr_font = FONT_R; letter_y_start = start_y+27; }
        else if (y >= start_y+36 && y < start_y+41)   { curr_font = FONT_Y; letter_y_start = start_y+36; }
    }
    // RIGHT SIDE: X-MAS (x: 73-77)
    else if (x >= 73 && x < 78) {
        if      (y >= start_y && y < start_y+5)       { curr_font = FONT_X; letter_y_start = start_y; }
        else if (y >= start_y+9 && y < start_y+14)    { curr_font = FONT_DASH; letter_y_start = start_y+9; }
        else if (y >= start_y+18 && y < start_y+23)   { curr_font = FONT_M; letter_y_start = start_y+18; }
        else if (y >= start_y+27 && y < start_y+32)   { curr_font = FONT_A; letter_y_start = start_y+27; }
        else if (y >= start_y+36 && y < start_y+41)   { curr_font = FONT_S; letter_y_start = start_y+36; }
    }

    if (curr_font != NULL) {
        int ly = y - letter_y_start;
        int lx = (x < 40) ? (x - 2) : (x - 73); 
        
        if (get_font_pixel(curr_font, lx, ly)) { 
             // Solid Gold for Readability
             p.r=255; p.g=215; p.b=0; 
             // Optional: Subtle shine
             if (y % 5 == 0) { p.r=255; p.g=255; p.b=200; }
             p.is_empty = 0;
             return p; 
        }
    }

    // 1. TREE GEOMETRY
    float slope = (float)y / (float)VIRTUAL_HEIGHT;
    float max_radius = slope * WIDTH_MAX;
    if (y < 12) max_radius *= 0.6; 

    int dist_i = abs(x - CENTER_X);
    float dist = (float)dist_i;
    float rel_x = (float)(x - CENTER_X);

    // Smooth Top
    float noise = (float)(rand()%3 - 1);
    float smooth_factor = (y < 5) ? (float)y / 5.0 : 1.0;
    float edge_limit = max_radius + (noise * smooth_factor);

    // EDGE CANDLES LOGIC
    int is_edge_candle = 0;
    // Place candles on the tips every 10 vertical pixels
    if (y > 10 && (y % 10 == 0)) {
        int candle_pos = (int)max_radius;
        // Check if we are at the exact edge
        if (dist_i == candle_pos) {
            is_edge_candle = 1;
        }
    }

    // If outside the tree limit AND NOT a candle, return empty
    if (dist > edge_limit && !is_edge_candle) return p;

    int is_top = (y < 4 && dist_i <= 1);

    // 2. FOLIAGE
    float layer_freq = 0.4;
    float droop = 0.30;
    float depth = dist / (max_radius + 0.1);
    float wave = cos((y * layer_freq) - (dist * droop));
    float layer_density = (wave + 1.0) / 2.0;
    float core_bias = (1.0 - depth) * 0.85;
    float final_density = layer_density + core_bias + 0.15;
    if (is_top) final_density = 1.2;

    int f_r, f_g, f_b;
    if (final_density < 0.45 && !is_top) {
        f_r = 5; f_g = 15; f_b = 5; 
    } else {
        f_r = 15; f_g = 50; f_b = 20; 
        if (depth > 0.8) { f_g += 50; f_r += 20; }
        else if (depth > 0.4) { f_g += 20; f_r += 10; }
        int tex = (rand() % 30) - 15; f_g += tex; f_r += tex/2;
        if (y < 4) { f_r+=30; f_g+=30; } 
    }
    p.r = f_r; p.g = f_g; p.b = f_b;
    p.bg_r = f_r; p.bg_g = f_g; p.bg_b = f_b;
    p.is_empty = 0;

    // 3. GARLANDS (STARS)
    // Don't draw garlands over candles
    if (y > 4 && final_density > 0.30 && !is_edge_candle) { 
        for(int i=0; i<3; i++) {
            if (!garlands[i].active) continue;
            float target_x = sin(y * (garlands[i].freq/2.0) + garlands[i].phase) * (max_radius * garlands[i].amp);
            
            if (fabs(rel_x - target_x) < 1.0) {
                int hash = (x * 31 + y * 17) % NUM_STARS;
                p.symbol = (char*)STARS[hash];
                p.r = garlands[i].r;
                p.g = garlands[i].g;
                p.b = garlands[i].b;
                if ((x+y)%5 == 0) { p.r=255; p.g=255; p.b=255; }
                p.is_wide = 0; 
                return p;
            }
        }
    }

    // 4. DECORATIONS
    
    // RENDER EDGE CANDLE
    if (is_edge_candle) {
        p.symbol = "\U0001F56F"; // 🕯
        p.r = 255; p.g = 140; p.b = 0; // Orange/Gold
        p.is_wide = 1; 
        
        // If the candle is sticking out into the void, set background to black
        if (dist > edge_limit) {
            p.bg_r = 0; p.bg_g = 0; p.bg_b = 0;
            // Ensure is_empty is false so it gets drawn
            p.is_empty = 0; 
        }
        return p;
    }

    float deco_noise = noise_2d(x, y + CENTER_X);
    int can_decorate = (deco_noise > 0.35);
    int has_support = (layer_density > 0.6 && depth > 0.35);

    if (y > 10 && has_support && can_decorate && rand_float() > 0.92) {
        int type = rand() % 200;
        if (type < 6) {
             p.symbol = "\U0001F56F"; p.r=255; p.g=140; p.b=0; p.is_wide=1;
        } else if (type < 18) {
             int sn = rand()%3;
             if(sn==0) p.symbol="\u2744"; else if(sn==1) p.symbol="\u2745"; else p.symbol="\u2746";
             p.r=200; p.g=240; p.b=255; p.is_wide=1;
        } else if (type < 40) {
             int st = rand()%5;
             if(st==0) p.symbol="\u2728"; else if(st==1) p.symbol="\u272F"; 
             else if(st==2) p.symbol="\u2735"; else if(st==3) p.symbol="\u2747"; else p.symbol="\u274B";
             p.r=255; p.g=225; p.b=50; p.is_wide=1;
        } else if (type < 65) {
             int fl = rand()%4;
             if(fl==0) p.symbol="\u273F"; else if(fl==1) p.symbol="\u2741"; 
             else if(fl==2) p.symbol="\u2743"; else p.symbol="\u2749";
             p.r=255; p.g=60; p.b=120; p.is_wide=1;
        } else if (type < 90) {
             int bl = rand()%4;
             if(bl==0) p.symbol="\u272A"; else if(bl==1) p.symbol="\u2734"; 
             else if(bl==2) p.symbol="\u2739"; else p.symbol="\u2742";
             p.r=0; p.g=210; p.b=255; p.is_wide=1;
        }
    }

    // Clamp
    if(p.r<0)p.r=0; if(p.g<0)p.g=0; if(p.b<0)p.b=0;
    if(p.r>255)p.r=255; if(p.g>255)p.g=255; if(p.b>255)p.b=255;
    
    return p;
}

int main(void) {
    srand(time(NULL));

    // SETUP GARLANDS
    int num_garlands = 2 + (rand() % 2); 
    Garland garlands[3];
    for(int i=0; i<3; i++) {
        if (i < num_garlands) {
            garlands[i].active = 1;
            garlands[i].style = rand() % 2; 
            garlands[i].freq = 0.12 + (rand_float() * 0.1); 
            garlands[i].amp = 0.95 + (rand_float() * 0.15); 
            garlands[i].phase = rand_float() * 10.0;
            if (i==0) { garlands[i].r=255; garlands[i].g=215; garlands[i].b=0; } 
            else if (i==1) { garlands[i].r=220; garlands[i].g=220; garlands[i].b=255; } 
            else { garlands[i].r=255; garlands[i].g=40; garlands[i].b=60; } 
        } else garlands[i].active = 0;
    }

    printf("\n");

    /* STAR */
    for(int i=0; i < CENTER_X - 1; i++) printf(" "); 
    printf("\033[38;2;255;255;200m\U0001F31F\033[0m\n"); 

    /* RENDER LOOP */
    for (int real_y = 0; real_y < VIRTUAL_HEIGHT / 2; ++real_y) {
        
        int y_top = real_y * 2;
        int y_bot = real_y * 2 + 1;

        for (int x = 0; x < CANVAS_WIDTH; ++x) {
            
            Pixel p1 = get_pixel_at(x, y_top, garlands, num_garlands);
            Pixel p2 = get_pixel_at(x, y_bot, garlands, num_garlands);

            /* SYMBOL RENDER */
            if (p1.symbol != NULL) {
                printf("\033[38;2;%d;%d;%dm", p1.r, p1.g, p1.b); // FG
                printf("\033[48;2;%d;%d;%dm", p1.bg_r, p1.bg_g, p1.bg_b); // BG
                printf("%s\033[0m", p1.symbol);
                if (p1.is_wide) x++; 
                continue;
            }
            if (p2.symbol != NULL) {
                printf("\033[38;2;%d;%d;%dm", p2.r, p2.g, p2.b); // FG
                printf("\033[48;2;%d;%d;%dm", p2.bg_r, p2.bg_g, p2.bg_b); // BG
                printf("%s\033[0m", p2.symbol);
                if (p2.is_wide) x++; 
                continue;
            }

            /* HALF-BLOCK RENDER */
            if (p1.is_empty && p2.is_empty) {
                printf("\033[0m ");
                continue;
            }

            if (p1.is_empty) {
                printf("\033[38;2;%d;%d;%dm\033[49m▄", p2.r, p2.g, p2.b);
            } else {
                printf("\033[38;2;%d;%d;%dm", p1.r, p1.g, p1.b); 
                if (p2.is_empty) {
                    printf("\033[49m▀");
                } else {
                    printf("\033[48;2;%d;%d;%dm▀", p2.r, p2.g, p2.b); 
                }
            }
            printf("\033[0m");
        }
        printf("\n");
    }

    /* TRUNK */
    for(int i=0; i<2; i++) {
        for(int x=0; x<CANVAS_WIDTH; ++x) {
            if (x == CENTER_X || x == CENTER_X - 1) {
                printf("\033[38;2;60;40;20m█");
            } else if (i==1 && abs(x-CENTER_X) < 14 && rand()%6==0) {
                 printf("\033[38;2;230;230;250m.");
            } else {
                printf("\033[0m ");
            }
        }
        printf("\n");
    }
    printf("\033[0m\n");

    return 0;
}
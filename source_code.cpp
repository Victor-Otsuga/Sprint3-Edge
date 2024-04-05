#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Variaveis 
int width = 128, height = 64;
bool playing = false;
int pins[4] = {2,3,4,5};
long notes[4][4] = {{0},{0},{0},{0}};
unsigned long last_note = 0;
int score = 0;
int btn_states[4] = {0};
int sizes[2] = {15,11};

// Parte das seta vazias

const unsigned char epd_bitmap_down_arrow [] PROGMEM = {
	0xf8, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 
	0x88, 0x00, 0x8f, 0x07, 0x06, 0x03, 0x8c, 0x01, 0xd8, 0x00, 0x70, 0x00, 0x20, 0x00
};
const unsigned char epd_bitmap_up_arrow [] PROGMEM = {
	0x20, 0x00, 0x70, 0x00, 0xd8, 0x00, 0x8c, 0x01, 0x06, 0x03, 0x8f, 0x07, 0x88, 0x00, 0x88, 0x00, 
	0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0xf8, 0x00
};
const unsigned char epd_bitmap_left_arrow [] PROGMEM = {
	0x20, 0x00, 0x30, 0x00, 0x38, 0x00, 0xec, 0x7f, 0x06, 0x40, 0x03, 0x40, 0x06, 0x40, 0xec, 0x7f, 
	0x38, 0x00, 0x30, 0x00, 0x20, 0x00
};
const unsigned char epd_bitmap_right_arrow [] PROGMEM = {
	0x00, 0x02, 0x00, 0x06, 0x00, 0x0e, 0xff, 0x1b, 0x01, 0x30, 0x01, 0x60, 0x01, 0x30, 0xff, 0x1b, 
	0x00, 0x0e, 0x00, 0x06, 0x00, 0x02
};

//Parte do desenho das setas que sobem
const unsigned char epd_bitmap_down_arrow_filled [] PROGMEM = {
	0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 
	0xf8, 0x00, 0xff, 0x07, 0xfe, 0x03, 0xfc, 0x01, 0xf8, 0x00, 0x70, 0x00, 0x20, 0x00
};
const unsigned char epd_bitmap_up_arrow_filled [] PROGMEM = {
	0x20, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xfc, 0x01, 0xfe, 0x03, 0xff, 0x07, 0xf8, 0x00, 0xf8, 0x00, 
	0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00
};
const unsigned char epd_bitmap_left_arrow_filled [] PROGMEM = {
	0x20, 0x00, 0x30, 0x00, 0x38, 0x00, 0xfc, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xfe, 0x7f, 0xfc, 0x7f, 
	0x38, 0x00, 0x30, 0x00, 0x20, 0x00
};
const unsigned char epd_bitmap_right_arrow_filled [] PROGMEM = {
	0x00, 0x02, 0x00, 0x06, 0x00, 0x0e, 0xff, 0x1f, 0xff, 0x3f, 0xff, 0x7f, 0xff, 0x3f, 0xff, 0x1f, 
	0x00, 0x0e, 0x00, 0x06, 0x00, 0x02
};

//definição das setas dentro de uma lista 

const int epd_bitmap_allArray_LEN = 8;
const unsigned char* epd_bitmap_allArray[8] = {
  epd_bitmap_left_arrow,
  epd_bitmap_down_arrow,
  epd_bitmap_up_arrow,
  epd_bitmap_right_arrow,
  epd_bitmap_left_arrow_filled,
  epd_bitmap_down_arrow_filled,
  epd_bitmap_up_arrow_filled,
  epd_bitmap_right_arrow_filled,
};

void setup(void) {
  u8g2.begin();
  for (int pin : pins) {pinMode(pin, INPUT_PULLUP);}
  // definação do boundrate
  Serial.begin(115200);
}

bool check_active() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(pins[i]) == 0) {return true;}
  }
  return false;
}

//criação da seta
void draw_arrow(int i, int y, int add) {
  int ind = int(ceil(float(i)/2))%2;
  int w = sizes[ind], h = sizes[(ind+1)%2];
  int x = ((width-75)/2)+(i*20);
  u8g2.drawXBMP(x, y-(h/2), w, h, epd_bitmap_allArray[i+add]);
}

void loop(void) {
  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_squeezed_b7_tr);
    //Começo do jogo
    if (playing) {
      unsigned long time = millis();
      if (time - last_note > 700) {
        last_note = time;
        int nidx = random(0,3);
        notes[nidx][notes[nidx][0]] = last_note;
        notes[nidx][0] = 1 + (notes[nidx][0]+1)%3;
      }

      for (int i = 0; i < 4; i++) {
        // input dos botoes
        int pval = digitalRead(pins[i]);
        int add = (pval == 0) ? 4 : 0;
        int h = sizes[int(ceil(float(i+1)/2))%2];

        // Chamando desenho das setas
        draw_arrow(i, 8, add);
        for (int z = 1; z < 4; z++) {
          long elapsed = time - notes[i][z];
          int y = 64 - (elapsed/25);
          int target_y = 8-(h/2);
          // calculo da pontuação baseado na proximidade da seta com seu espaço vazio
          if (notes[i][z] == 0 || elapsed > 1800) {continue;}
          else if (btn_states[i] != HIGH && abs(y-target_y) <= 5 && pval == HIGH) {
            score += 10 * (5-abs(y-target_y));
            notes[i][z] = 0;
            continue;
          }
          draw_arrow(i, y, 4);
        }
        btn_states[i] = pval;
        char cstr[5];
        itoa(score, cstr, 10);
        u8g2.drawStr(0,9,cstr);
      }
    // caso o jogo ainda n tenha começado este sera o menu mostrado no display
    } else {
      playing = check_active();
      char out[24] = "Aperte para comecar";
      int w = u8g2.getStrWidth(out);
      u8g2.drawStr((width-w)/2,(height+9)/2,out);
    }
  } while (u8g2.nextPage());
}

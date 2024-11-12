#define ARROW_UP            0
#define ARROW_DOWN          1
#define ARROW_UP_PARTIAL    2
#define ARROW_DOWN_PARTIAL  3
#define BELL                4
#define DEGREE              5

byte arrow_up_icon[] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

byte arrow_up_partial_icon[] = {
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100
};

byte arrow_down_icon[] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};

byte arrow_down_partial_icon[] = {
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte bell_icon[] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100
};

byte degree_sym[] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
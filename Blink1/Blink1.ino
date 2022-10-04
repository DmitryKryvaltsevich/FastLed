#include <FastLED.h>
#define l_count_d 16 
#define l_count_t 22 // число светодиодов в кольце/ленте
constexpr int LED_DT_d = 13;
#define LED_DT_t 12  // пин, куда подключен DIN ленты

class p_leds
{
public:
    p_leds(int L_count, int Led_dt, int Idex, int Ihue, CRGB ledss[])
        :led_dt(Led_dt), l_count(L_count), idex(Idex), ihue(Ihue), leds(ledss)
    {}
    int l_count;
    int const led_dt;
    int idex;
    int ihue;
    CRGB* leds; 
    //int** ledsX = new int** [l_count][3];;
};
int max_bright = 120;          // максимальна€ €ркость (0 - 255)
boolean adapt_light = 0;       // адаптивна€ подсветка (1 - включить, 0 - выключить)
byte fav_modes[] = { 30, 27, 23 };  // список "любимых" режимов
//byte fav_modes[] = { 2, 3, 8, 10, 11, 18, 19, 37,12,13,14,15,16,17,18,19,20 };  // список "любимых" режимов
//byte fav_modes[] = { 1, };  // список "любимых" режимов
byte num_modes = sizeof(fav_modes);         // получить количество "любимых" режимов (они все по 1 байту..)
unsigned long change_time, last_change, last_bright;
int new_bright;

int ledMode = 1;

/*
  —тартовый режим
  0 - все выключены
  1 - все включены
  3 - кольцева€ радуга
  888 - демо-режим
*/

// цвета м€чиков дл€ режима
byte ballColors[3][3] = {
  {0xff, 0, 0},
  {0xff, 0xff, 0xff},
  {0   , 0   , 0xff}
};

// ---------------—Ћ”∆≈ЅЌџ≈ ѕ≈–≈ћ≈ЌЌџ≈-----------------
int BOTTOM_INDEX = 0;        // светодиод начала отсчЄта
struct CRGB leds_d[l_count_d];
struct CRGB leds_t[l_count_t];
int ledsX_d[l_count_d][3];     //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)
int ledsX_t[l_count_t][3];     //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)



int thisdelay = 20;          //-FX LOOPS DELAY VAR
int thisstep = 10;           //-FX LOOPS DELAY VAR
int thishue = 0;             //-FX LOOPS DELAY VAR
int thissat = 255;           //-FX LOOPS DELAY VAR

int idex_t = 0;                //-LED INDEX (0 to LED_COUNT-1
int idex_d = 0;                //-LED INDEX (0 to LED_COUNT-1
int ihue_t = 0;
int ihue_d = 0;                  //-HUE (0-255)
int ibright = 0;             //-BRIGHTNESS (0-255)
int isat = 0;                //-SATURATION (0-255)
int bouncedirection = 0;     //-SWITCH FOR COLOR BOUNCE (0-1)
float tcount = 0.0;          //-INC VAR FOR SIN LOOPS
int lcount = 0;              //-ANOTHER COUNTING VAR
p_leds leds[]{ p_leds(l_count_t, LED_DT_d, idex_t, ihue_t,leds_t) ,p_leds(l_count_d, LED_DT_d, idex_d, ihue_d,leds_d) };
// ---------------—Ћ”∆≈ЅЌџ≈ ѕ≈–≈ћ≈ЌЌџ≈-----------------


void setup()
{
    Serial.begin(9600);              // открыть порт дл€ св€зи
    LEDS.setBrightness(max_bright);  // ограничить максимальную €ркость
    //for (int lc = 0; lc < sizeof(leds); lc++)
    //{
    //    LEDS.addLeds<WS2811, 13, GRB>(leds[lc].leds, leds[lc].l_count);  // настрйоки дл€ нашей ленты (ленты на WS2811, WS2812, WS2812B)
    //}
    LEDS.addLeds<WS2811, LED_DT_t, GRB>(leds[0].leds, l_count_t);  // настрйоки дл€ нашей ленты (ленты на WS2811, WS2812, WS2812B)
    LEDS.addLeds<WS2811, LED_DT_d, GRB>(leds_d, l_count_d);  // настрйоки дл€ нашей ленты (ленты на WS2811, WS2812, WS2812B)
    one_color_all(0, 0, 0);          // погасить все светодиоды
    LEDS.show();
    randomSeed(analogRead(0));  
}

void one_color_all(int cred, int cgrn, int cblu) {       //-SET ALL LEDS TO ONE COLOR
    for (int i = 0; i < l_count_d; i++) {
        leds_d[i].setRGB(cred, cgrn, cblu);
    }
    for (int i = 0; i < l_count_t; i++) {
        leds_t[i].setRGB(cred, cgrn, cblu);
    }
}

void loop() {
    if (ledMode >= 33)
    //if (ledMode >= sizeof(fav_modes))
    {
        ledMode = 1;
    }

    if (adapt_light) {                        // если включена адаптивна€ €ркость
        if (millis() - last_bright > 500) {     // каждые полсекунды
            last_bright = millis();               // сброить таймер
            new_bright = map(analogRead(6), 1, 1000, 3, max_bright);   // считать показани€ с фоторезистора, перевести диапазон
            LEDS.setBrightness(new_bright);        // установить новую €ркость
        }
    }

    if (millis() - last_change > change_time) {
        change_time = 5000; // получаем новое случайное врем€ до следующей смены режима
        ledMode++;   // получаем новый случайный номер следующего режима
        change_mode(ledMode); // мен€ем режим через change_mode (там дл€ каждого режима сто€т цвета и задержки)
        //change_mode(fav_modes[ledMode]); // мен€ем режим через change_mode (там дл€ каждого режима сто€т цвета и задержки)
        last_change = millis();
    }
    /*
      if (Serial.available() > 0) {     // если что то прислали
        ledMode = Serial.parseInt();    // парсим в тип данных int
        change_mode(ledMode);           // мен€ем режим через change_mode (там дл€ каждого режима сто€т цвета и задержки)
      }
    */
    switch (ledMode) {
    case 999: break;                           // пазуа
    case  1: { 
        red_fade(l_count_t, leds_t, ihue_t);
        red_fade(l_count_d, leds_d, ihue_d);}; break;                // плавна€ смена цветов всей ленты
    case  2: {
        rainbow_fade(l_count_t, leds_t, ihue_t);
        rainbow_fade(l_count_d, leds_d, ihue_d); }; break;            // плавна€ смена цветов всей ленты
    case  3: {
        rainbow_loop(l_count_t, leds_t, idex_t, ihue_t);
        rainbow_loop(l_count_d, leds_d, idex_d, ihue_d);} break;            // крут€ща€с€ радуга
    case  4: {
        random_burst(l_count_t, leds_t, idex_t, ihue_t);
        random_burst(l_count_d, leds_d, idex_d, ihue_d);} break;           // случайна€ смена цветов
    case  5: {
        color_bounce(l_count_t, leds_t, idex_t);
        color_bounce(l_count_d, leds_d, idex_d);} break;            // бегающий светодиод
    case  6: {
        color_bounceFADE(l_count_t, leds_t, idex_t);
        color_bounceFADE(l_count_d, leds_d, idex_d);} break;        // бегающий паровозик светодиодов
    case  7: {
        ems_lightsONE(l_count_t, leds_t, idex_t);
        ems_lightsONE(l_count_d, leds_d, idex_d);} break;          // вращаютс€ красный и синий
    case  8: {
        ems_lightsALL(l_count_t, leds_t, idex_t);
        ems_lightsALL(l_count_d, leds_d, idex_d); } break;           // вращаетс€ половина красных и половина синих
    case  9: {
        flicker(l_count_t, leds_t);
        flicker(l_count_d, leds_d); } break;                 // случайный стробоскоп
    case 10: {
        pulse_one_color_all(190,l_count_t, leds_t);
        pulse_one_color_all(190, l_count_d, leds_d); } break;    // пульсаци€ одним цветом
    case 11: {
        pulse_one_color_all_rev(l_count_t, leds_t);
        pulse_one_color_all_rev(l_count_d, leds_d);} break; // пульсаци€ со сменой цветов
    case 12: {
        fade_vertical(l_count_t, leds_t, idex_t);
        fade_vertical(l_count_d, leds_d, idex_d); } break;           // плавна€ смена €ркости по вертикали (дл€ кольца)
    case 13: {
        rule30(l_count_t, leds_t, ledsX_t);
        rule30(l_count_d, leds_d, ledsX_d); } break;                 // безумие красных светодиодов
    case 14: {
        random_march(l_count_t, leds_t, ledsX_t);
        random_march(l_count_d, leds_d, ledsX_d); } break;            // безумие случайных цветов
    case 15: {
        rwb_march(l_count_t, leds_t, ledsX_t, idex_t);
        rwb_march(l_count_d, leds_d, ledsX_d, idex_d); } break;              // белый синий красный бегут по кругу (ѕј“–»ќ“»«ћ!)
    case 16: {
        radiation(l_count_t, leds_t);
        radiation(l_count_d, leds_d); } break;               // пульсирует значок радиации
    case 17: {
        color_loop_vardelay(l_count_t, leds_t, idex_t);
        color_loop_vardelay(l_count_d, leds_d, idex_d); } break;    // красный светодиод бегает по кругу
    case 18: {
        white_temps(l_count_t, leds_t);
        white_temps(l_count_d, leds_d); } break;;             // бело синий градиент (?)
    case 19: {
        sin_bright_wave(l_count_t, leds_t);
        sin_bright_wave(l_count_d, leds_d); } break;         // тоже хрень кака€ то
    case 20: {
        pop_horizontal(l_count_t, leds_t, idex_t);
        pop_horizontal(l_count_d, leds_d, idex_d); } break;;          // красные вспышки спускаютс€ вниз
    case 21: {
        quad_bright_curve(l_count_t, leds_t);
        quad_bright_curve(l_count_d, leds_d); } break;;       // полумес€ц
    case 22: {
        flame(l_count_t, leds_t);
        flame(l_count_d, leds_d); } break;;                   // эффект пламени
    case 23: {
        rainbow_vertical(l_count_t, leds_t, idex_t, ihue_t);
        rainbow_vertical(l_count_d, leds_d, idex_d, ihue_d); } break;;         // радуга в вертикаьной плоскости (кольцо)
    case 24: {
        pacman(l_count_t, leds_t);
        pacman(l_count_d, leds_d); } break;;                  // пакман
    case 25: {
        random_color_pop(l_count_t, leds_t, idex_t, ihue_t);
        random_color_pop(l_count_d, leds_d, idex_d, ihue_d); } break;;        // безумие случайных вспышек
    case 26: {
        ems_lightsSTROBE(l_count_t, leds_t);
        ems_lightsSTROBE(l_count_d, leds_d); } break;;        // полицейска€ мигалка
    case 27: {
        rgb_propeller(l_count_t, leds_t, idex_t);
        rgb_propeller(l_count_d, leds_d, idex_d); } break;;           // RGB пропеллер
    case 28: {
        kitt(l_count_t, leds_t);
        kitt(l_count_d, leds_d); } break;;                    // случайные вспышки красного в вертикаьной плоскости
    case 29: {
        matrix(l_count_t, leds_t, ledsX_t);
        matrix(l_count_d, leds_d, ledsX_d); } break;;                  // зелЄненькие бегают по кругу случайно
    case 30: {
        new_rainbow_loop(l_count_t, leds_t, ihue_t);
        new_rainbow_loop(l_count_d, leds_d, ihue_d); } break;;         // крута€ плавна€ вращающа€с€ радуга
    case 31: {
        strip_march_ccw(l_count_t, leds_t, ledsX_t);
        strip_march_ccw(l_count_d, leds_d, ledsX_d); } break;;         // чЄт сломалось
    case 32: {
        strip_march_cw(l_count_t, leds_t, ledsX_t);
        strip_march_cw(l_count_d, leds_d, ledsX_d); } break;;          // чЄт сломалось
    //case 33: colorWipe(0x00, 0xff, 0x00, thisdelay);
    //    colorWipe(0x00, 0x00, 0x00, thisdelay); break;                                // плавное заполнение цветом
    //case 34: CylonBounce(0xff, 0, 0, 4, 10, thisdelay); break;                      // бегающие светодиоды
    //case 35: Fire(55, 120, thisdelay); break;                                       // линейный огонь
    //case 36: NewKITT(0xff, 0, 0, 8, 10, thisdelay); break;                          // беготн€ секторов круга (не работает)
    //case 37: rainbowCycle(thisdelay); break;                                        // очень плавна€ вращающа€с€ радуга
    //case 38: TwinkleRandom(20, thisdelay, 1); break;                                // случайные разноцветные включени€ (1 - танцуют все, 0 - случайный 1 диод)
    //case 39: RunningLights(0xff, 0xff, 0x00, thisdelay); break;                     // бегущие огни
    //case 40: Sparkle(0xff, 0xff, 0xff, thisdelay); break;                           // случайные вспышки белого цвета
    //case 41: SnowSparkle(0x10, 0x10, 0x10, thisdelay, random(100, 1000)); break;    // случайные вспышки белого цвета на белом фоне
    //case 42: theaterChase(0xff, 0, 0, thisdelay); break;                            // бегущие каждые 3 („»—Ћќ —¬≈“ќƒ»ќƒќ¬ ƒќЋ∆Ќќ Ѕџ“№  –ј“Ќќ 3)
    //case 43: theaterChaseRainbow(thisdelay); break;                                 // бегущие каждые 3 радуга („»—Ћќ —¬≈“ќƒ»ќƒќ¬ ƒќЋ∆Ќќ Ѕџ“№  –ј“Ќќ 3)
    //case 44: Strobe(0xff, 0xff, 0xff, 10, thisdelay, 1000); break;                  // стробоскоп
    //case 45: BouncingBalls(0xff, 0, 0, 3); break;                                   // прыгающие м€чики
    //case 46: BouncingColoredBalls(3, ballColors); break;                            // прыгающие м€чики цветные
    }
}

void change_mode(int newmode) {
    thissat = 255;
    switch (newmode) {
    case 0: one_color_all(0, 0, 0); LEDS.show(); break; //---ALL OFF
    case 1: one_color_all(255, 255, 255); LEDS.show(); break; //---ALL ON
    case 2: thisdelay = 60; break;                      //---STRIP RAINBOW FADE
    case 3: thisdelay = 20; thisstep = 10; break;       //---RAINBOW LOOP
    case 4: thisdelay = 20; break;                      //---RANDOM BURST
    case 5: thisdelay = 20; thishue = 0; break;         //---CYLON v1
    case 6: thisdelay = 80; thishue = 0; break;         //---CYLON v2
    case 7: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SINGLE
    case 8: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SOLID
    case 9: thishue = 160; thissat = 50; break;         //---STRIP FLICKER
    case 10: thishue = 100; thisdelay = 15; thishue = 0; break;        //---PULSE COLOR BRIGHTNESS
    case 11: thisdelay = 30; thishue = 0; break;        //---PULSE COLOR SATURATION
    case 12: thisdelay = 60; thishue = 180; break;      //---VERTICAL SOMETHING
    case 13: thisdelay = 100; break;                    //---CELL AUTO - RULE 30 (RED)
    case 14: thisdelay = 80; break;                     //---MARCH RANDOM COLORS
    case 15: thisdelay = 80; break;                     //---MARCH RWB COLORS
    case 16: thisdelay = 60; thishue = 95; break;       //---RADIATION SYMBOL
        //---PLACEHOLDER FOR COLOR LOOP VAR DELAY VARS
    case 19: thisdelay = 80; thishue = 180; break;      //---SIN WAVE BRIGHTNESS
    case 20: thisdelay = 100; thishue = 0; break;       //---POP LEFT/RIGHT
    case 21: thisdelay = 100; thishue = 180; break;     //---QUADRATIC BRIGHTNESS CURVE
        //---PLACEHOLDER FOR FLAME VARS
    case 23: thisdelay = 50; thisstep = 15; break;      //---VERITCAL RAINBOW
    case 24: thisdelay = 50; break;                     //---PACMAN
    case 25: thisdelay = 35; break;                     //---RANDOM COLOR POP
    case 26: thisdelay = 25; thishue = 0; break;        //---EMERGECNY STROBE
    case 27: thisdelay = 100; thishue = 0; break;        //---RGB PROPELLER
    case 28: thisdelay = 100; thishue = 0; break;       //---KITT
    case 29: thisdelay = 100; thishue = 95; break;       //---MATRIX RAIN
    case 30: thisdelay = 15; break;                      //---NEW RAINBOW LOOP
    case 31: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW
    case 32: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW
    case 33: thisdelay = 50; break;                     // colorWipe
    case 34: thisdelay = 50; break;                     // CylonBounce
    case 35: thisdelay = 15; break;                     // Fire
    case 36: thisdelay = 50; break;                     // NewKITT
    case 37: thisdelay = 20; break;                     // rainbowCycle
    case 38: thisdelay = 10; break;                     // rainbowTwinkle
    case 39: thisdelay = 50; break;                     // RunningLights
    case 40: thisdelay = 0; break;                      // Sparkle
    case 41: thisdelay = 30; break;                     // SnowSparkle
    case 42: thisdelay = 50; break;                     // theaterChase
    case 43: thisdelay = 50; break;                     // theaterChaseRainbow
    case 44: thisdelay = 100; break;                    // Strobe

    case 101: one_color_all(122, 40, 255); LEDS.show(); break; //---ALL RED
    case 102: one_color_all(0, 255, 0); LEDS.show(); break; //---ALL GREEN
    case 103: one_color_all(0, 0, 255); LEDS.show(); break; //---ALL BLUE
    case 104: one_color_all(255, 255, 0); LEDS.show(); break; //---ALL COLOR X
    case 105: one_color_all(0, 255, 255); LEDS.show(); break; //---ALL COLOR Y
    case 106: one_color_all(255, 0, 255); LEDS.show(); break; //---ALL COLOR Z
    }
    bouncedirection = 0;
    one_color_all(0, 0, 0);
    ledMode = newmode;
}


//------------------------LED EFFECT FUNCTIONS------------------------


void one_color_allHSV(int ahue, int LED_COUNT, struct CRGB leds[]) {    //-SET ALL LEDS TO ONE COLOR (HSV)
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = CHSV(ahue, thissat, 255);
    }
}

void red_fade(int LED_COUNT, struct CRGB leds[], int& ihue) {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
    ihue++;
    if (ihue > 255) {
        ihue = 0;
    }
    for (int idex = 0; idex < LED_COUNT; idex++) {
        leds[idex] = CHSV(190, thissat, 0);
    }
    LEDS.show();
    delay(thisdelay);

}
void rainbow_fade(int LED_COUNT, struct CRGB leds[], int& ihue) {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
    ihue++;
    if (ihue > 255) {
        ihue = 0;
    }
    for (int idex = 0; idex < LED_COUNT; idex++) {
        leds[idex] = CHSV(ihue, thissat, 255);
    }
    LEDS.show();
    delay(thisdelay);
}


void rainbow_loop(int LED_COUNT, struct CRGB leds[],int& idex, int& ihue) {                        //-m3-LOOP HSV RAINBOW
    idex++;
    ihue = ihue + thisstep;
    if (idex >= LED_COUNT) {
        idex = 0;
    }
    if (ihue > 255) {
        ihue = 0;
    }
    leds[idex] = CHSV(ihue, thissat, 255);
    LEDS.show();
    delay(thisdelay);
}

void random_burst(int LED_COUNT, struct CRGB leds[], int& idex, int& ihue) {                         //-m4-RANDOM INDEX/COLOR
    idex = random(0, LED_COUNT);
    ihue = random(0, 255);
    leds[idex] = CHSV(ihue, thissat, 255);
    LEDS.show();
    delay(thisdelay);
}

void color_bounce(int LED_COUNT, struct CRGB leds[], int& idex) {                        //-m5-BOUNCE COLOR (SINGLE LED)
    if (bouncedirection == 0) {
        idex = idex + 1;
        if (idex == LED_COUNT) {
            bouncedirection = 1;
            idex = idex - 1;
        }
    }
    if (bouncedirection == 1) {
        idex = idex - 1;
        if (idex == 0) {
            bouncedirection = 0;
        }
    }
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == idex) {
            leds[i] = CHSV(thishue, thissat, 255);
        }
        else {
            leds[i] = CHSV(0, 0, 0);
        }
    }
    LEDS.show();
    delay(thisdelay);
}

void color_bounceFADE(int LED_COUNT, struct CRGB leds[], int& idex) {                    //-m6-BOUNCE COLOR (SIMPLE MULTI-LED FADE)
    if (bouncedirection == 0) {
        idex = idex + 1;
        if (idex == LED_COUNT) {
            bouncedirection = 1;
            idex = idex - 1;
        }
    }
    if (bouncedirection == 1) {
        idex = idex - 1;
        if (idex == 0) {
            bouncedirection = 0;
        }
    }
    int iL1 = adjacent_cw(idex, LED_COUNT, leds);
    int iL2 = adjacent_cw(iL1, LED_COUNT, leds);
    int iL3 = adjacent_cw(iL2, LED_COUNT, leds);
    int iR1 = adjacent_ccw(idex, LED_COUNT, leds);
    int iR2 = adjacent_ccw(iR1, LED_COUNT, leds);
    int iR3 = adjacent_ccw(iR2, LED_COUNT, leds);
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == idex) {
            leds[i] = CHSV(thishue, thissat, 255);
        }
        else if (i == iL1) {
            leds[i] = CHSV(thishue, thissat, 150);
        }
        else if (i == iL2) {
            leds[i] = CHSV(thishue, thissat, 80);
        }
        else if (i == iL3) {
            leds[i] = CHSV(thishue, thissat, 20);
        }
        else if (i == iR1) {
            leds[i] = CHSV(thishue, thissat, 150);
        }
        else if (i == iR2) {
            leds[i] = CHSV(thishue, thissat, 80);
        }
        else if (i == iR3) {
            leds[i] = CHSV(thishue, thissat, 20);
        }
        else {
            leds[i] = CHSV(0, 0, 0);
        }
    }
    LEDS.show();
    delay(thisdelay);
}

void ems_lightsONE(int LED_COUNT, struct CRGB leds[], int& idex) {                    //-m7-EMERGENCY LIGHTS (TWO COLOR SINGLE LED)
    idex++;
    if (idex >= LED_COUNT) {
        idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR, LED_COUNT, leds);
    int thathue = (thishue + 160) % 255;
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == idexR) {
            leds[i] = CHSV(thishue, thissat, 255);
        }
        else if (i == idexB) {
            leds[i] = CHSV(thathue, thissat, 255);
        }
        else {
            leds[i] = CHSV(0, 0, 0);
        }
    }
    LEDS.show();
    delay(thisdelay);
}

void ems_lightsALL(int LED_COUNT, struct CRGB leds[], int &idex) {                  //-m8-EMERGENCY LIGHTS (TWO COLOR SOLID)
    idex++;
    if (idex >= LED_COUNT) {
        idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR, LED_COUNT, leds);
    int thathue = (thishue + 160) % 255;
    leds[idexR] = CHSV(thishue, thissat, 255);
    leds[idexB] = CHSV(thathue, thissat, 255);
    LEDS.show();
    delay(thisdelay);
}

void flicker(int LED_COUNT, struct CRGB leds[]) {                          //-m9-FLICKER EFFECT
    int random_bright = random(0, 255);
    int random_delay = random(10, 100);
    int random_bool = random(0, random_bright);
    if (random_bool < 10) {
        for (int i = 0; i < LED_COUNT; i++) {
            leds[i] = CHSV(thishue, thissat, random_bright);
        }
        LEDS.show();
        delay(random_delay);
    }
}

void pulse_one_color_all(int col, int LED_COUNT, struct CRGB leds[]) {              //-m10-PULSE BRIGHTNESS ON ALL LEDS TO ONE COLOR
    if (bouncedirection == 0) {
        ibright++;
        if (ibright >= 255) {
            bouncedirection = 1;
        }
    }
    if (bouncedirection == 1) {
        ibright = ibright - 1;
        if (ibright <= 1) {
            bouncedirection = 0;
        }
    }
    for (int idex = 0; idex < LED_COUNT; idex++) {
        leds[idex] = CHSV(col, thissat, ibright);
    }
    LEDS.show();
    delay(thisdelay);
}

void pulse_one_color_all_rev(int LED_COUNT, struct CRGB leds[]) {           //-m11-PULSE SATURATION ON ALL LEDS TO ONE COLOR
    if (bouncedirection == 0) {
        isat++;
        if (isat >= 255) {
            bouncedirection = 1;
        }
    }
    if (bouncedirection == 1) {
        isat = isat - 1;
        if (isat <= 1) {
            bouncedirection = 0;
        }
    }
    for (int idex = 0; idex < LED_COUNT; idex++) {
        leds[idex] = CHSV(thishue, isat, 255);
    }
    LEDS.show();
    delay(thisdelay);
}

void fade_vertical(int LED_COUNT, struct CRGB leds[], int& idex) {                    //-m12-FADE 'UP' THE LOOP
    idex++;
    if (idex > int(LED_COUNT / 2)) {
        idex = 0;
    }
    int idexA = idex;
    int idexB = horizontal_index(idexA, LED_COUNT, leds);
    ibright = ibright + 10;
    if (ibright > 255) {
        ibright = 0;
    }
    leds[idexA] = CHSV(thishue, thissat, ibright);
    leds[idexB] = CHSV(thishue, thissat, ibright);
    LEDS.show();
    delay(thisdelay);
}

void random_red(int LED_COUNT, struct CRGB leds[]) {                       //QUICK 'N DIRTY RANDOMIZE TO GET CELL AUTOMATA STARTED
    int temprand;
    for (int i = 0; i < LED_COUNT; i++) {
        temprand = random(0, 100);
        if (temprand > 50) {
            leds[i].r = 255;
        }
        if (temprand <= 50) {
            leds[i].r = 0;
        }
        leds[i].b = 0; leds[i].g = 0;
    }
    LEDS.show();
}

void rule30(int LED_COUNT, struct CRGB leds[], int ledsX[][3]) {                          //-m13-1D CELLULAR AUTOMATA - RULE 30 (RED FOR NOW)
    if (bouncedirection == 0) {
        random_red(LED_COUNT, leds);
        bouncedirection = 1;
    }
    
    copy_led_array(LED_COUNT, leds, ledsX);
    int iCW;
    int iCCW;
    int y = 100;
    for (int i = 0; i < LED_COUNT; i++) {
        iCW = adjacent_cw(i, LED_COUNT, leds);
        iCCW = adjacent_ccw(i, LED_COUNT, leds);
        if (ledsX[iCCW][0] > y && ledsX[i][0] > y && ledsX[iCW][0] > y) {
            leds[i].r = 0;
        }
        if (ledsX[iCCW][0] > y && ledsX[i][0] > y && ledsX[iCW][0] <= y) {
            leds[i].r = 0;
        }
        if (ledsX[iCCW][0] > y && ledsX[i][0] <= y && ledsX[iCW][0] > y) {
            leds[i].r = 0;
        }
        if (ledsX[iCCW][0] > y && ledsX[i][0] <= y && ledsX[iCW][0] <= y) {
            leds[i].r = 255;
        }
        if (ledsX[iCCW][0] <= y && ledsX[i][0] > y && ledsX[iCW][0] > y) {
            leds[i].r = 255;
        }
        if (ledsX[iCCW][0] <= y && ledsX[i][0] > y && ledsX[iCW][0] <= y) {
            leds[i].r = 255;
        }
        if (ledsX[iCCW][0] <= y && ledsX[i][0] <= y && ledsX[iCW][0] > y) {
            leds[i].r = 255;
        }
        if (ledsX[iCCW][0] <= y && ledsX[i][0] <= y && ledsX[iCW][0] <= y) {
            leds[i].r = 0;
        }
    }
    LEDS.show();
    delay(thisdelay);
}

void random_march(int LED_COUNT, struct CRGB leds[], int ledsX[][3]) {                   //-m14-RANDOM MARCH CCW
    copy_led_array(LED_COUNT, leds, ledsX);
    int iCCW;
    leds[0] = CHSV(random(0, 255), 255, 255);
    for (int idex = 1; idex < LED_COUNT; idex++) {
        iCCW = adjacent_ccw(idex, LED_COUNT, leds);
        leds[idex].r = ledsX[iCCW][0];
        leds[idex].g = ledsX[iCCW][1];
        leds[idex].b = ledsX[iCCW][2];
    }
    LEDS.show();
    delay(thisdelay);
}

void rwb_march(int LED_COUNT, struct CRGB leds[], int ledsX[][3], int& idex) {                    //-m15-R,W,B MARCH CCW
    copy_led_array(LED_COUNT, leds, ledsX);
    int iCCW;
    idex++;
    if (idex > 2) {
        idex = 0;
    }
    switch (idex) {
    case 0:
        leds[0].r = 255;
        leds[0].g = 0;
        leds[0].b = 0;
        break;
    case 1:
        leds[0].r = 255;
        leds[0].g = 255;
        leds[0].b = 255;
        break;
    case 2:
        leds[0].r = 0;
        leds[0].g = 0;
        leds[0].b = 255;
        break;
    }
    for (int i = 1; i < LED_COUNT; i++) {
        iCCW = adjacent_ccw(i, LED_COUNT, leds);
        leds[i].r = ledsX[iCCW][0];
        leds[i].g = ledsX[iCCW][1];
        leds[i].b = ledsX[iCCW][2];
    }
    LEDS.show();
    delay(thisdelay);
}

void radiation(int LED_COUNT, struct CRGB leds[]) {                   //-m16-SORT OF RADIATION SYMBOLISH-
    int N3 = int(LED_COUNT / 3);
    int N6 = int(LED_COUNT / 6);
    int N12 = int(LED_COUNT / 12);
    for (int i = 0; i < N6; i++) {    //-HACKY, I KNOW...
        tcount = tcount + .02;
        if (tcount > 3.14) {
            tcount = 0.0;
        }
        ibright = int(sin(tcount) * 255);
        int j0 = (i + LED_COUNT - N12) % LED_COUNT;
        int j1 = (j0 + N3) % LED_COUNT;
        int j2 = (j1 + N3) % LED_COUNT;
        leds[j0] = CHSV(thishue, thissat, ibright);
        leds[j1] = CHSV(thishue, thissat, ibright);
        leds[j2] = CHSV(thishue, thissat, ibright);
    }
    LEDS.show();
    delay(thisdelay);
}

void color_loop_vardelay(int LED_COUNT, struct CRGB leds[], int& idex) {                    //-m17-COLOR LOOP (SINGLE LED) w/ VARIABLE DELAY
    idex++;
    if (idex > LED_COUNT) {
        idex = 0;
    }
    int di = abs(int(LED_COUNT / 2) - idex);
    int t = constrain((10 / di) * 10, 10, 500);
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == idex) {
            leds[i] = CHSV(0, thissat, 255);
        }
        else {
            leds[i].r = 0; leds[i].g = 0; leds[i].b = 0;
        }
    }
    LEDS.show();
    delay(t);
}

void white_temps(int LED_COUNT, struct CRGB leds[]) {                            //-m18-SHOW A SAMPLE OF BLACK BODY RADIATION COLOR TEMPERATURES
    int N9 = int(LED_COUNT / 9);
    for (int i = 0; i < LED_COUNT; i++) {
        if (i >= 0 && i < N9) {
            leds[i].r = 64;  //-CANDLE - 1900
            leds[i].g = 156;
            leds[i].b = 255;
        }
        if (i >= N9 && i < N9 * 2) {
            leds[i].r = 64;  //-40W TUNG - 2600
            leds[i].g = 156;
            leds[i].b = 255;
        }
        if (i >= N9 * 2 && i < N9 * 3) {
            leds[i].r = 255;  //-100W TUNG - 2850
            leds[i].g = 255;
            leds[i].b = 251;
        }
        if (i >= N9 * 3 && i < N9 * 4) {
            leds[i].r = 255;  //-HALOGEN - 3200
            leds[i].g = 241;
            leds[i].b = 224;
        }
        if (i >= N9 * 4 && i < N9 * 5) {
            leds[i].r = 255;  //-CARBON ARC - 5200
            leds[i].g = 250;
            leds[i].b = 244;
        }
        if (i >= N9 * 5 && i < N9 * 6) {
            leds[i].r = 255;  //-HIGH NOON SUN - 5400
            leds[i].g = 255;
            leds[i].b = 251;
        }
        if (i >= N9 * 6 && i < N9 * 7) {
            leds[i].r = 255;  //-DIRECT SUN - 6000
            leds[i].g = 255;
            leds[i].b = 255;
        }
        if (i >= N9 * 7 && i < N9 * 8) {
            leds[i].r = 201;  //-OVERCAST SKY - 7000
            leds[i].g = 226;
            leds[i].b = 255;
        }
        if (i >= N9 * 8 && i < LED_COUNT) {
            leds[i].r = 64;  //-CLEAR BLUE SKY - 20000
            leds[i].g = 156;
            leds[i].b = 255;
        }
    }
    LEDS.show();
    delay(100);
}

void sin_bright_wave(int LED_COUNT, struct CRGB leds[]) {        //-m19-BRIGHTNESS SINE WAVE
    for (int i = 0; i < LED_COUNT; i++) {
        tcount = tcount + .1;
        if (tcount > 3.14) {
            tcount = 0.0;
        }
        ibright = int(sin(tcount) * 255);
        leds[i] = CHSV(thishue, thissat, ibright);
        LEDS.show();
        delay(thisdelay);
    }
}

void pop_horizontal(int LED_COUNT, struct CRGB leds[], int& idex) {        //-m20-POP FROM LEFT TO RIGHT UP THE RING
    int ix;
    if (bouncedirection == 0) {
        bouncedirection = 1;
        ix = idex;
    }
    else if (bouncedirection == 1) {
        bouncedirection = 0;
        ix = horizontal_index(idex, LED_COUNT, leds);
        idex++;
        if (idex > int(LED_COUNT / 2)) {
            idex = 0;
        }
    }
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == ix) {
            leds[i] = CHSV(thishue, thissat, 255);
        }
        else {
            leds[i].r = 0; leds[i].g = 0; leds[i].b = 0;
        }
    }
    LEDS.show();
    delay(thisdelay);
}

void quad_bright_curve(int LED_COUNT, struct CRGB leds[]) {      //-m21-QUADRATIC BRIGHTNESS CURVER
    int ax;
    for (int x = 0; x < LED_COUNT; x++) {
        if (x <= int(LED_COUNT / 2)) {
            ax = x;
        }
        else if (x > int(LED_COUNT / 2)) {
            ax = LED_COUNT - x;
        }
        int a = 1; int b = 1; int c = 0;
        int iquad = -(ax * ax * a) + (ax * b) + c; //-ax2+bx+c
        int hquad = -(int(LED_COUNT / 2) * int(LED_COUNT / 2) * a) + (int(LED_COUNT / 2) * b) + c;
        ibright = int((float(iquad) / float(hquad)) * 255);
        leds[x] = CHSV(thishue, thissat, ibright);
    }
    LEDS.show();
    delay(thisdelay);
}

void flame(int LED_COUNT, struct CRGB leds[]) {                                    //-m22-FLAMEISH EFFECT
    int idelay = random(0, 35);
    float hmin = 0.1; float hmax = 45.0;
    float hdif = hmax - hmin;
    int randtemp = random(0, 3);
    float hinc = (hdif / float(int(LED_COUNT / 2))) + randtemp;
    int ihue = hmin;
    for (int i = 0; i <= int(LED_COUNT / 2); i++) {
        ihue = ihue + hinc;
        leds[i] = CHSV(ihue, thissat, 255);
        int ih = horizontal_index(i, LED_COUNT, leds);
        leds[ih] = CHSV(ihue, thissat, 255);
        leds[int(LED_COUNT / 2)].r = 255; leds[int(LED_COUNT / 2)].g = 255; leds[int(LED_COUNT / 2)].b = 255;
        LEDS.show();
        delay(idelay);
    }
}

void rainbow_vertical(int LED_COUNT, struct CRGB leds[], int& idex, int& ihue) {                        //-m23-RAINBOW 'UP' THE LOOP
    idex++;
    if (idex > int(LED_COUNT / 2)) {
        idex = 0;
    }
    ihue = ihue + thisstep;
    if (ihue > 255) {
        ihue = 0;
    }
    int idexA = idex;
    int idexB = horizontal_index(idexA, LED_COUNT, leds);
    leds[idexA] = CHSV(ihue, thissat, 255);
    leds[idexB] = CHSV(ihue, thissat, 255);
    LEDS.show();
    delay(thisdelay);
}

void pacman(int LED_COUNT, struct CRGB leds[]) {                                  //-m24-REALLY TERRIBLE PACMAN CHOMPING EFFECT
    int s = int(LED_COUNT / 4);
    lcount++;
    if (lcount > 5) {
        lcount = 0;
    }
    if (lcount == 0) {
        for (int i = 0; i < LED_COUNT; i++) {
            set_color_led(i, 255, 255, 0, LED_COUNT, leds);
        }
    }
    if (lcount == 1 || lcount == 5) {
        for (int i = 0; i < LED_COUNT; i++) {
            set_color_led(i, 255, 255, 0, LED_COUNT, leds);
        }
        leds[s].r = 0; leds[s].g = 0; leds[s].b = 0;
    }
    if (lcount == 2 || lcount == 4) {
        for (int i = 0; i < LED_COUNT; i++) {
            set_color_led(i, 255, 255, 0, LED_COUNT, leds);
        }
        leds[s - 1].r = 0; leds[s - 1].g = 0; leds[s - 1].b = 0;
        leds[s].r = 0; leds[s].g = 0; leds[s].b = 0;
        leds[s + 1].r = 0; leds[s + 1].g = 0; leds[s + 1].b = 0;
    }
    if (lcount == 3) {
        for (int i = 0; i < LED_COUNT; i++) {
            set_color_led(i, 255, 255, 0, LED_COUNT, leds);
        }
        leds[s - 2].r = 0; leds[s - 2].g = 0; leds[s - 2].b = 0;
        leds[s - 1].r = 0; leds[s - 1].g = 0; leds[s - 1].b = 0;
        leds[s].r = 0; leds[s].g = 0; leds[s].b = 0;
        leds[s + 1].r = 0; leds[s + 1].g = 0; leds[s + 1].b = 0;
        leds[s + 2].r = 0; leds[s + 2].g = 0; leds[s + 2].b = 0;
    }
    LEDS.show();
    delay(thisdelay);
}

void random_color_pop(int LED_COUNT, struct CRGB leds[], int& idex, int& ihue) {                         //-m25-RANDOM COLOR POP
    idex = random(0, LED_COUNT);
    ihue = random(0, 255);
    one_color_all(0, 0, 0);
    leds[idex] = CHSV(ihue, thissat, 255);
    LEDS.show();
    delay(thisdelay);
}

void ems_lightsSTROBE(int LED_COUNT, struct CRGB leds[]) {                  //-m26-EMERGENCY LIGHTS (STROBE LEFT/RIGHT)
    int thishue = 0;
    int thathue = (thishue + 160) % 255;
    for (int x = 0; x < 5; x++) {
        for (int i = 0; i < int(LED_COUNT / 2); i++) {
            leds[i] = CHSV(thishue, thissat, 255);
        }
        LEDS.show(); delay(thisdelay);
        one_color_all(0, 0, 0);
        LEDS.show(); delay(thisdelay);
    }
    for (int x = 0; x < 5; x++) {
        for (int i = int(LED_COUNT / 2); i < LED_COUNT; i++) {
            leds[i] = CHSV(thathue, thissat, 255);
        }
        LEDS.show(); delay(thisdelay);
        one_color_all(0, 0, 0);
        LEDS.show(); delay(thisdelay);
    }
}

void rgb_propeller(int LED_COUNT, struct CRGB leds[], int& idex) {                           //-m27-RGB PROPELLER
    idex++;
    int ghue = (thishue + 80) % 255;
    int bhue = (thishue + 160) % 255;
    int N3 = int(LED_COUNT / 3);
    int N6 = int(LED_COUNT / 6);
    int N12 = int(LED_COUNT / 12);
    for (int i = 0; i < N3; i++) {
        int j0 = (idex + i + LED_COUNT - N12) % LED_COUNT;
        int j1 = (j0 + N3) % LED_COUNT;
        int j2 = (j1 + N3) % LED_COUNT;
        leds[j0] = CHSV(thishue, thissat, 255);
        leds[j1] = CHSV(ghue, thissat, 255);
        leds[j2] = CHSV(bhue, thissat, 255);
    }
    LEDS.show();
    delay(thisdelay);
}

void kitt(int LED_COUNT, struct CRGB leds[]) {                                     //-m28-KNIGHT INDUSTIES 2000
    int rand = random(0, int(LED_COUNT / 2));
    for (int i = 0; i < rand; i++) {
        leds[int(LED_COUNT / 2) + i] = CHSV(thishue, thissat, 255);
        leds[int(LED_COUNT / 2) - i] = CHSV(thishue, thissat, 255);
        LEDS.show();
        delay(thisdelay / rand);
    }
    for (int i = rand; i > 0; i--) {
        leds[int(LED_COUNT / 2) + i] = CHSV(thishue, thissat, 0);
        leds[int(LED_COUNT / 2) - i] = CHSV(thishue, thissat, 0);
        LEDS.show();
        delay(thisdelay / rand);
    }
}

void matrix(int LED_COUNT, struct CRGB leds[], int ledsX[][3]) {                                   //-m29-ONE LINE MATRIX
    int rand = random(0, 100);
    if (rand > 90) {
        leds[0] = CHSV(thishue, thissat, 255);
    }
    else {
        leds[0] = CHSV(thishue, thissat, 0);
    }
    copy_led_array(LED_COUNT, leds, ledsX);
    for (int i = 1; i < LED_COUNT; i++) {
        leds[i].r = ledsX[i - 1][0];
        leds[i].g = ledsX[i - 1][1];
        leds[i].b = ledsX[i - 1][2];
    }
    LEDS.show();
    delay(thisdelay);
}

void strip_march_cw(int LED_COUNT, struct CRGB leds[], int ledsX[][3]) {                        //-m50-MARCH STRIP CW
    copy_led_array(LED_COUNT, leds, ledsX);
    int iCW;
    for (int i = 0; i < LED_COUNT; i++) {
        iCW = adjacent_cw(i, LED_COUNT, leds);
        leds[i].r = ledsX[iCW][0];
        leds[i].g = ledsX[iCW][1];
        leds[i].b = ledsX[iCW][2];
    }
    LEDS.show();
    delay(thisdelay);
}

void strip_march_ccw(int LED_COUNT, struct CRGB leds[], int ledsX[][3]) {                        //-m51-MARCH STRIP CCW
    copy_led_array(LED_COUNT, leds, ledsX);
    int iCCW;
    for (int i = 0; i < LED_COUNT; i++) {
        iCCW = adjacent_ccw(i, LED_COUNT, leds);
        leds[i].r = ledsX[iCCW][0];
        leds[i].g = ledsX[iCCW][1];
        leds[i].b = ledsX[iCCW][2];
    }
    LEDS.show();
    delay(thisdelay);
}

void new_rainbow_loop(int LED_COUNT, struct CRGB leds[], int& ihue) {                      //-m88-RAINBOW FADE FROM FAST_SPI2
    ihue -= 1;
    fill_rainbow(leds, LED_COUNT, ihue);
    LEDS.show();
    delay(thisdelay);
}

//-----------------------------плавное заполнение цветом-----------------------------------------
void colorWipe(byte red, byte green, byte blue, int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        setPixel(i, red, green, blue, LED_COUNT, leds);
        FastLED.show();
        delay(SpeedDelay);
    }
}
//-----------------------------------бегающие светодиоды-----------------------------------
void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, int LED_COUNT, struct CRGB leds[]) {

    for (int i = 0; i < LED_COUNT - EyeSize - 2; i++) {
        setAll(0, 0, 0, LED_COUNT, leds);
        setPixel(i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        FastLED.show();
        delay(SpeedDelay);
    }

    delay(ReturnDelay);

    for (int i = LED_COUNT - EyeSize - 2; i > 0; i--) {
        setAll(0, 0, 0, LED_COUNT, leds);
        setPixel(i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        FastLED.show();
        delay(SpeedDelay);
    }

    delay(ReturnDelay);
}

//---------------------------------линейный огонь-------------------------------------
void Fire(int Cooling, int Sparking, int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    
    byte* heat = new byte[LED_COUNT];
    int cooldown;

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < LED_COUNT; i++) {
        cooldown = random(0, ((Cooling * 10) / LED_COUNT) + 2);

        if (cooldown > heat[i]) {
            heat[i] = 0;
        }
        else {
            heat[i] = heat[i] - cooldown;
        }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = LED_COUNT - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if (random(255) < Sparking) {
        int y = random(7);
        heat[y] = heat[y] + random(160, 255);
        //heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for (int j = 0; j < LED_COUNT; j++) {
        setPixelHeatColor(j, heat[j], LED_COUNT, leds);
    }

    FastLED.show();
    delay(SpeedDelay);
}

void setPixelHeatColor(int Pixel, byte temperature, int LED_COUNT, struct CRGB leds[]) {
    // Scale 'heat' down from 0-255 to 0-191
    byte t192 = round((temperature / 255.0) * 191);

    // calculate ramp up from
    byte heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252

    // figure out which third of the spectrum we're in:
    if (t192 > 0x80) {                    // hottest
        setPixel(Pixel, 255, 255, heatramp, LED_COUNT, leds);
    }
    else if (t192 > 0x40) {            // middle
        setPixel(Pixel, 255, heatramp, 0, LED_COUNT, leds);
    }
    else {                               // coolest
        setPixel(Pixel, heatramp, 0, 0, LED_COUNT, leds);
    }
}

//-------------------------------newKITT---------------------------------------
//void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
//    RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//    CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
//}

void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, int LED_COUNT, struct CRGB leds[]) {
    for (int i = ((LED_COUNT - EyeSize) / 2); i >= 0; i--) {
        setAll(0, 0, 0, LED_COUNT, leds);

        setPixel(i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);

        setPixel(LED_COUNT - i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(LED_COUNT - i - j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(LED_COUNT - i - EyeSize - 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);

        FastLED.show();
        delay(SpeedDelay);
    }
    delay(ReturnDelay);
}

void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, int LED_COUNT, struct CRGB leds[]) {
    for (int i = 0; i <= ((LED_COUNT - EyeSize) / 2); i++) {
        setAll(0, 0, 0, LED_COUNT, leds);

        setPixel(i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);

        setPixel(LED_COUNT - i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(LED_COUNT - i - j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(LED_COUNT - i - EyeSize - 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);

        FastLED.show();
        delay(SpeedDelay);
    }
    delay(ReturnDelay);
}

void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, int LED_COUNT, struct CRGB leds[]) {
    for (int i = 0; i < LED_COUNT - EyeSize - 2; i++) {
        setAll(0, 0, 0, LED_COUNT, leds);
        setPixel(i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        FastLED.show();
        delay(SpeedDelay);
    }
    delay(ReturnDelay);
}

void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, int LED_COUNT, struct CRGB leds[]) {
    for (int i = LED_COUNT - EyeSize - 2; i > 0; i--) {
        setAll(0, 0, 0, LED_COUNT, leds);
        setPixel(i, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue, LED_COUNT, leds);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10, LED_COUNT, leds);
        FastLED.show();
        delay(SpeedDelay);
    }
    delay(ReturnDelay);
}

//-------------------------------newKITT---------------------------------------
void rainbowCycle(int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    byte* c;
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
        for (i = 0; i < LED_COUNT; i++) {
            c = Wheel(((i * 256 / LED_COUNT) + j) & 255);
            setPixel(i, *c, *(c + 1), *(c + 2), LED_COUNT, leds);
        }
        FastLED.show();
        delay(SpeedDelay);
    }
}

byte* Wheel(byte WheelPos) {
    static byte c[3];

    if (WheelPos < 85) {
        c[0] = WheelPos * 3;
        c[1] = 255 - WheelPos * 3;
        c[2] = 0;
    }
    else if (WheelPos < 170) {
        WheelPos -= 85;
        c[0] = 255 - WheelPos * 3;
        c[1] = 0;
        c[2] = WheelPos * 3;
    }
    else {
        WheelPos -= 170;
        c[0] = 0;
        c[1] = WheelPos * 3;
        c[2] = 255 - WheelPos * 3;
    }

    return c;
}

//-------------------------------TwinkleRandom---------------------------------------
void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne, int LED_COUNT, struct CRGB leds[]) {
    setAll(0, 0, 0, LED_COUNT, leds);

    for (int i = 0; i < Count; i++) {
        setPixel(random(LED_COUNT), random(0, 255), random(0, 255), random(0, 255), LED_COUNT, leds);
        FastLED.show();
        delay(SpeedDelay);
        if (OnlyOne) {
            setAll(0, 0, 0, LED_COUNT, leds);
        }
    }

    delay(SpeedDelay);
}

//-------------------------------RunningLights---------------------------------------
void RunningLights(byte red, byte green, byte blue, int WaveDelay, int LED_COUNT, struct CRGB leds[]) {
    int Position = 0;

    for (int i = 0; i < LED_COUNT * 2; i++)
    {
        Position++; // = 0; //Position + Rate;
        for (int i = 0; i < LED_COUNT; i++) {
            // sine wave, 3 offset waves make a rainbow!
            //float level = sin(i+Position) * 127 + 128;
            //setPixel(i,level,0,0);
            //float level = sin(i+Position) * 127 + 128;
            setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                ((sin(i + Position) * 127 + 128) / 255) * green,
                ((sin(i + Position) * 127 + 128) / 255) * blue, LED_COUNT, leds);
        }

        FastLED.show();
        delay(WaveDelay);
    }
}

//-------------------------------Sparkle---------------------------------------
void Sparkle(byte red, byte green, byte blue, int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    int Pixel = random(LED_COUNT);
    setPixel(Pixel, red, green, blue, LED_COUNT, leds);
    FastLED.show();
    delay(SpeedDelay);
    setPixel(Pixel, 0, 0, 0, LED_COUNT, leds);
}

//-------------------------------SnowSparkle---------------------------------------
void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    setAll(red, green, blue, LED_COUNT, leds);

    int Pixel = random(LED_COUNT);
    setPixel(Pixel, 0xff, 0xff, 0xff, LED_COUNT, leds);
    FastLED.show();
    delay(SparkleDelay);
    setPixel(Pixel, red, green, blue, LED_COUNT, leds);
    FastLED.show();
    delay(SpeedDelay);
}

//-------------------------------theaterChase---------------------------------------
void theaterChase(byte red, byte green, byte blue, int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < LED_COUNT; i = i + 3) {
                setPixel(i + q, red, green, blue, LED_COUNT, leds);  //turn every third pixel on
            }
            FastLED.show();
            delay(SpeedDelay);
            for (int i = 0; i < LED_COUNT; i = i + 3) {
                setPixel(i + q, 0, 0, 0, LED_COUNT, leds);    //turn every third pixel off
            }
        }
    }
}

//-------------------------------theaterChaseRainbow---------------------------------------
void theaterChaseRainbow(int SpeedDelay, int LED_COUNT, struct CRGB leds[]) {
    byte* c;

    for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < LED_COUNT; i = i + 3) {
                c = Wheel((i + j) % 255);
                setPixel(i + q, *c, *(c + 1), *(c + 2), LED_COUNT, leds); //turn every third pixel on
            }
            FastLED.show();
            delay(SpeedDelay);
            for (int i = 0; i < LED_COUNT; i = i + 3) {
                setPixel(i + q, 0, 0, 0, LED_COUNT, leds);    //turn every third pixel off
            }
        }
    }
}

//-------------------------------Strobe---------------------------------------
void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause, int LED_COUNT, struct CRGB leds[]) {
    for (int j = 0; j < StrobeCount; j++) {
        setAll(red, green, blue, LED_COUNT, leds);
        FastLED.show();
        delay(FlashDelay);
        setAll(0, 0, 0, LED_COUNT, leds);
        FastLED.show();
        delay(FlashDelay);
    }

    delay(EndPause);
}

//-------------------------------BouncingBalls---------------------------------------
void BouncingBalls(byte red, byte green, byte blue, int BallCount, int LED_COUNT, struct CRGB leds[]) {
    float Gravity = -9.81;
    int StartHeight = 1;
    float Height[3];
    float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
    float ImpactVelocity[3];
    float TimeSinceLastBounce[3];
    int   Position[3];
    long  ClockTimeSinceLastBounce[3];
    float Dampening[3];

    for (int i = 0; i < BallCount; i++) {
        ClockTimeSinceLastBounce[i] = millis();
        Height[i] = StartHeight;
        Position[i] = 0;
        ImpactVelocity[i] = ImpactVelocityStart;
        TimeSinceLastBounce[i] = 0;
        Dampening[i] = 0.90 - float(i) / pow(BallCount, 2);
    }

    while (true) {
        for (int i = 0; i < BallCount; i++) {
            TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
            Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

            if (Height[i] < 0) {
                Height[i] = 0;
                ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
                ClockTimeSinceLastBounce[i] = millis();

                if (ImpactVelocity[i] < 0.01) {
                    ImpactVelocity[i] = ImpactVelocityStart;
                }
            }
            Position[i] = round(Height[i] * (LED_COUNT - 1) / StartHeight);
        }

        for (int i = 0; i < BallCount; i++) {
            setPixel(Position[i], red, green, blue, LED_COUNT, leds);
        }
        FastLED.show();
        setAll(0, 0, 0, LED_COUNT, leds);
    }
}

//-------------------------------BouncingColoredBalls---------------------------------------
void BouncingColoredBalls(int BallCount, byte colors[][3], int LED_COUNT, struct CRGB leds[]) {
    float Gravity = -9.81;
    int StartHeight = 1;

    float Height[3];
    float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
    float ImpactVelocity[3];
    float TimeSinceLastBounce[3];
    int   Position[3];
    long  ClockTimeSinceLastBounce[3];
    float Dampening[3];

    for (int i = 0; i < BallCount; i++) {
        ClockTimeSinceLastBounce[i] = millis();
        Height[i] = StartHeight;
        Position[i] = 0;
        ImpactVelocity[i] = ImpactVelocityStart;
        TimeSinceLastBounce[i] = 0;
        Dampening[i] = 0.90 - float(i) / pow(BallCount, 2);
    }

    while (true) {
        for (int i = 0; i < BallCount; i++) {
            TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
            Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

            if (Height[i] < 0) {
                Height[i] = 0;
                ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
                ClockTimeSinceLastBounce[i] = millis();

                if (ImpactVelocity[i] < 0.01) {
                    ImpactVelocity[i] = ImpactVelocityStart;
                }
            }
            Position[i] = round(Height[i] * (LED_COUNT - 1) / StartHeight);
        }

        for (int i = 0; i < BallCount; i++) {
            setPixel(Position[i], colors[i][0], colors[i][1], colors[i][2], LED_COUNT, leds);
        }
        FastLED.show();
        setAll(0, 0, 0, LED_COUNT, leds);
    }
}

////------------------------------------- UTILITY FXNS --------------------------------------
//---SET THE COLOR OF A SINGLE RGB LED
void set_color_led(int adex, int cred, int cgrn, int cblu, int LED_COUNT, struct CRGB leds[]) {
    leds[adex].setRGB(cred, cgrn, cblu);
}

//---FIND INDEX OF HORIZONAL OPPOSITE LED
int horizontal_index(int i, int LED_COUNT, struct CRGB leds[]) {
    //-ONLY WORKS WITH INDEX < TOPINDEX
    if (i == BOTTOM_INDEX) {
        return BOTTOM_INDEX;
    }
    if (i == int(LED_COUNT / 2) && (LED_COUNT % 2) == 1) {
        return int(LED_COUNT / 2) + 1;
    }
    if (i == int(LED_COUNT / 2) && (LED_COUNT % 2) == 0) {
        return int(LED_COUNT / 2);
    }
    return LED_COUNT - i;
}

//---FIND INDEX OF ANTIPODAL OPPOSITE LED
int antipodal_index(int i, int LED_COUNT, struct CRGB leds[]) {
    int iN = i + int(LED_COUNT / 2);
    if (i >= int(LED_COUNT / 2)) {
        iN = (i + int(LED_COUNT / 2)) % LED_COUNT;
    }
    return iN;
}

//---FIND ADJACENT INDEX CLOCKWISE
int adjacent_cw(int i, int LED_COUNT, struct CRGB leds[]) {
    int r;
    if (i < LED_COUNT - 1) {
        r = i + 1;
    }
    else {
        r = 0;
    }
    return r;
}

//---FIND ADJACENT INDEX COUNTER-CLOCKWISE
int adjacent_ccw(int i, int LED_COUNT, struct CRGB leds[]) {
    int r;
    if (i > 0) {
        r = i - 1;
    }
    else {
        r = LED_COUNT - 1;
    }
    return r;
}

void copy_led_array(int LED_COUNT, struct CRGB leds[], int ledsX[][3]) {
    for (int i = 0; i < LED_COUNT; i++) {
        ledsX[i][0] = leds[i].r;
        ledsX[i][1] = leds[i].g;
        ledsX[i][2] = leds[i].b;
    }
}

void setPixel(int Pixel, byte red, byte green, byte blue, int LED_COUNT, struct CRGB leds[]) {
    leds[Pixel].r = red;
    leds[Pixel].g = green;
    leds[Pixel].b = blue;
}

void setAll(byte red, byte green, byte blue, int LED_COUNT, struct CRGB leds[]) {
    for (int i = 0; i < LED_COUNT; i++) {
        setPixel(i, red, green, blue, LED_COUNT, leds);
    }
    FastLED.show();
}

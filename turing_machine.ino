#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "turing.h"

#define BTN1 3
#define BTN2 5
#define BTN3 6
#define BTN4 9
#define BTN5 10
int counter = 0;

#define DISPLAY_SIZE 16
#define CLICK_INTERVAL 300
#define NUMBERS_OF_PROGRAMS 3
#define TAPE_SIZE 15

int tapePosition = 0;
int selectProgram = 0;

boolean transitionFunction = true;
boolean tapeFunction = false;
boolean programFunction = false;

long lastBtn1, lastBtn2, lastBtn3, lastBtn4, lastBtn5;
char *t = "################";
LiquidCrystal_I2C lcd(0x27, 16, 2);

machine *m;
state *q0, *q1, *q2, *q3, *q4, *q5, *q6, *q7, *qF;
transition *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8, *t9, *t10, *t11, *t12, *t13, *t14, *t15, *t16, *t17, *t18, *t19, *t20;
head *h;

int cont = 1;

void setup()
{
  //m->inputSymbols = "01";
  //m->blank = '#';

  Serial.begin(9600);
  pinMode(BTN1, INPUT);
  digitalWrite(BTN1, HIGH);
  pinMode(BTN2, INPUT);
  digitalWrite(BTN2, HIGH);
  pinMode(BTN3, INPUT);
  digitalWrite(BTN3, HIGH);
  pinMode(BTN4, INPUT);
  digitalWrite(BTN4, HIGH);
  pinMode(BTN5, INPUT);
  digitalWrite(BTN5, HIGH);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  displayProgramsName();
}

void loop()
{
  if ((digitalRead(BTN1) == LOW)  && (digitalRead(BTN2) == HIGH)  && (digitalRead(BTN3) == HIGH)  && (digitalRead(BTN4) == HIGH)  && (digitalRead(BTN5) == HIGH)) {
    if ((millis() - lastBtn1) > CLICK_INTERVAL) {
      if (transitionFunction && !tapeFunction && !programFunction) {
        selectProgramLeft();
      }

      if (!transitionFunction && tapeFunction && !programFunction) {
        tapeMoveLeft();
      }
      lastBtn1 = millis();
    }
  }

  if ((digitalRead(BTN1) == HIGH)  && (digitalRead(BTN2) == LOW)  && (digitalRead(BTN3) == HIGH)  && (digitalRead(BTN4) == HIGH)  && (digitalRead(BTN5) == HIGH)) {
    if ((millis() - lastBtn2) > CLICK_INTERVAL) {
      if (transitionFunction && !tapeFunction && !programFunction) {
        selectProgramRight();
      }

      if (!transitionFunction && tapeFunction && !programFunction) {
        tapeMoveRight();
      }
      lastBtn2 = millis();
    }
  }

  if ((digitalRead(BTN1) == HIGH)  && (digitalRead(BTN2) == HIGH)  && (digitalRead(BTN3) == LOW)  && (digitalRead(BTN4) == HIGH)  && (digitalRead(BTN5) == HIGH)) {
    if ((millis() - lastBtn3) > CLICK_INTERVAL) {
      if (!transitionFunction && tapeFunction && !programFunction) {
        tapeUpChange();
      }
      lastBtn3 = millis();
    }
  }

  if ((digitalRead(BTN1) == HIGH)  && (digitalRead(BTN2) == HIGH)  && (digitalRead(BTN3) == HIGH)  && (digitalRead(BTN4) == LOW)  && (digitalRead(BTN5) == HIGH)) {
    if ((millis() - lastBtn4) > CLICK_INTERVAL) {
      if (!transitionFunction && tapeFunction && !programFunction) {
        tapeDownChange();
      }
      lastBtn4 = millis();
    }
  }

  if ((digitalRead(BTN1) == HIGH)  && (digitalRead(BTN2) == HIGH)  && (digitalRead(BTN3) == HIGH)  && (digitalRead(BTN4) == HIGH)  && (digitalRead(BTN5) == LOW)) {
    if ((millis() - lastBtn5) > CLICK_INTERVAL) {
      if (!transitionFunction && !tapeFunction && programFunction) {
        runTuringMachine();
      }

      if (!transitionFunction && tapeFunction && !programFunction) {
        tapeFunction = false;
        programFunction = true;
        h->tape = (char *)calloc(sizeof(char), strlen(t) + 1);
        strcpy(h->tape, t);
        h->position = 0;
        runTuringMachine();
      }

      if (transitionFunction && !tapeFunction && !programFunction) {
        transitionFunction = false;
        tapeFunction = true;
        selectedProgram();
        createTapeHead();
      }

      lastBtn5 = millis();
    }
  }
}

void runTuringMachine() {
  if (cont) {
    lcd.clear();
    Serial.write(h->current->name);
    lcd.setCursor(0, 0);
    lcd.print(h->current->name);
    Serial.write(": ");
    lcd.print(": ");
    Serial.write(h->tape);
    lcd.print(h->tape);
    Serial.write("\n");

    char *pos = (char *)malloc(sizeof(char) * h->position + 2);
    int i;
    for (i = 0; i < h->position; i++) {
      *(pos + i) = ' ';
    }
    *(pos + h->position) = '^';
    *(pos + h->position + 1) = 0;

    Serial.write("    ");
    lcd.setCursor(0, 1);
    lcd.print("    ");
    Serial.write(pos);
    lcd.print(pos);
    Serial.write("\n\n");
    if(strcmp(h->current->name, "qF") == 0) {
      lcd.setCursor(0, 1);
      lcd.print("AKCEPT");
    }
    free(pos);
  } else {
    delay(5000);
  }

  cont = machine_step(m, h);
}

void tapeMoveLeft() {
  if (tapePosition <= 0) {
    tapePosition = 0;
  } else {
    tapePosition--;
  }
  createTapeHead();
}

void tapeMoveRight() {
  if (tapePosition >= TAPE_SIZE) {
    tapePosition = TAPE_SIZE;
  } else {
    tapePosition++;
  }
  createTapeHead();
}

void createTapeHead() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(t);

  char *pos = (char *)malloc(sizeof(char) * tapePosition + 2);
  int i;
  for (i = 0; i < tapePosition; i++) {
    *(pos + i) = ' ';
  }
  *(pos + tapePosition) = '_';
  *(pos + tapePosition + 1) = 0;

  lcd.setCursor(0, 1);
  lcd.print(pos);
  free(pos);
}

void tapeUpChange() {
  switch (*(t + tapePosition)) {
    case '#':
      *(t + tapePosition) = '0';
      break;
    case '0':
      *(t + tapePosition) = '1';
      break;
    case '1':
      *(t + tapePosition) = '#';
      break;
  }

  lcd.setCursor(0, 0);
  lcd.print(t);
}

void tapeDownChange() {
  switch (*(t + tapePosition)) {
    case '#':
      *(t + tapePosition) = '1';
      break;
    case '1':
      *(t + tapePosition) = '0';
      break;
    case '0':
      *(t + tapePosition) = '#';
      break;
  }

  lcd.setCursor(0, 0);
  lcd.print(t);
}

void selectProgramLeft() {
  if (selectProgram <= 0) {
    selectProgram = 0;
  } else {
    selectProgram--;
  }
  displayProgramsName();
}

void selectProgramRight() {
  if (selectProgram >= NUMBERS_OF_PROGRAMS) {
    selectProgram = NUMBERS_OF_PROGRAMS;
  } else {
    selectProgram++;
  }
  displayProgramsName();
}

void displayProgramsName() {
  lcd.clear();
  switch (selectProgram) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Count>");
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("<Subtraction>");
      break;
      
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("<0^i 1^j|i<j>");
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print("<Palindrome");
      break;
  }
}

void selectedProgram() {
  switch (selectProgram) {
    case 0:
      countTwoNumber();
      break;
    case 1:
      subtraction();
      break;
    case 2:
      toItoJ();
      break;
    case 3:
      palindrome();
      break;
  }
}

void countTwoNumber() {
   m = machine_alloc();
  
  state_create(&q0, "q0");
  state_create(&q1, "q1");
  state_create(&q2, "q2");
  state_create(&q3, "q3");
  state_create(&qF, "qF");

  transition_create(&t0);
  transition_init(t0, q0, '1', '1', 1, q1);

  transition_create(&t1);
  transition_init(t1, q1, '1', '1', 1, q1);

  transition_create(&t2);
  transition_init(t2, q1, '0', '1', 1, q2);

  transition_create(&t3);
  transition_init(t3, q2, '1', '1', 1, q2);

  transition_create(&t4);
  transition_init(t4, q2, '#', '#', -1, q3);

  transition_create(&t5);
  transition_init(t5, q3, '1', '#', 1, qF);

  m->initial = q0;
  m->final = qF;
  m->transitions = (transition**)calloc(sizeof(transition*), 7);
  m->transitions[0] = t0;
  m->transitions[1] = t1;
  m->transitions[2] = t2;
  m->transitions[3] = t3;
  m->transitions[4] = t4;
  m->transitions[5] = t5;
  
  h = head_alloc();
  h->current = q0;
}

void subtraction() {
  m = machine_alloc();
  
  state_create(&q0, "q0");
  state_create(&q1, "q1");
  state_create(&q2, "q2");
  state_create(&q3, "q3");
  state_create(&q4, "q4");
  state_create(&q5, "q5");
  state_create(&q6, "q6");
  state_create(&qF, "qF");

  transition_create(&t0);
  transition_init(t0, q0, '1', '1', 1, q1);

  transition_create(&t1);
  transition_init(t1, q1, '1', '1', 1, q1);

  transition_create(&t2);
  transition_init(t2, q1, '0', '0', 1, q2);

  transition_create(&t3);
  transition_init(t3, q2, 'X', 'X', 1, q2);

  transition_create(&t4);
  transition_init(t4, q2, '1', 'X', -1, q3);

  transition_create(&t5);
  transition_init(t5, q2, '#', '#', -1, q6);
  
  transition_create(&t6);
  transition_init(t6, q3, 'X', 'X', -1, q3);

  transition_create(&t7);
  transition_init(t7, q3, '0', '0', -1, q4);

  transition_create(&t8);
  transition_init(t8, q4, 'Y', 'Y', -1, q4);

  transition_create(&t9);
  transition_init(t9, q4, '1', 'Y', 1, q5);
  
  transition_create(&t10);
  transition_init(t10, q5, '0', '0', 1, q2);

  transition_create(&t11);
  transition_init(t11, q5, 'Y', 'Y', 1, q5);
  
  transition_create(&t12);
  transition_init(t12, q6, 'X', '#', -1, q6);

  transition_create(&t13);
  transition_init(t13, q6, 'Y', '#', -1, q6);

  transition_create(&t14);
  transition_init(t14, q6, '0', '#', -1, q6);

  transition_create(&t15);
  transition_init(t15, q6, '1', '1', 1, qF);

  m->initial = q0;
  m->final = qF;
  m->transitions = (transition**)calloc(sizeof(transition*), 17);
  m->transitions[0] = t0;
  m->transitions[1] = t1;
  m->transitions[2] = t2;
  m->transitions[3] = t3;
  m->transitions[4] = t4;
  m->transitions[5] = t5;
  m->transitions[6] = t6;
  m->transitions[7] = t7;
  m->transitions[8] = t8;
  m->transitions[9] = t9;
  m->transitions[10] = t10;
  m->transitions[11] = t11;
  m->transitions[12] = t12;
  m->transitions[13] = t13;
  m->transitions[14] = t14;
  m->transitions[15] = t15;
  
  h = head_alloc();
  h->current = q0;
}

void toItoJ() {
  m = machine_alloc();
  
  state_create(&q0, "q0");
  state_create(&q1, "q1");
  state_create(&q2, "q2");
  state_create(&q3, "q3");
  state_create(&qF, "qF");

  transition_create(&t0);
  transition_init(t0, q0, '0', 'X', 1, q2);

  transition_create(&t1);
  transition_init(t1, q0, '1', '1', 1, q1);

  transition_create(&t2);
  transition_init(t2, q0, 'X', 'X', 1, q0);

  transition_create(&t3);
  transition_init(t3, q0, 'Y', 'Y', 1, q0);

  transition_create(&t4);
  transition_init(t4, q1, '1', '1', 1, q1);

  transition_create(&t5);
  transition_init(t5, q1, '#', '#', 1, qF);
  
  transition_create(&t6);
  transition_init(t6, q2, '0', '0', 1, q2);

  transition_create(&t7);
  transition_init(t7, q2, 'Y', 'Y', 1, q2);

  transition_create(&t8);
  transition_init(t8, q2, '1', 'Y', -1, q3);

  transition_create(&t9);
  transition_init(t9, q3, 'Y', 'Y', -1, q3);

  transition_create(&t10);
  transition_init(t10, q3, '0', '0', -1, q3);
  
  transition_create(&t11);
  transition_init(t11, q3, 'X', 'X', 1, q0);

  m->initial = q0;
  m->final = qF;
  m->transitions = (transition**)calloc(sizeof(transition*), 13);
  m->transitions[0] = t0;
  m->transitions[1] = t1;
  m->transitions[2] = t2;
  m->transitions[3] = t3;
  m->transitions[4] = t4;
  m->transitions[5] = t5;
  m->transitions[6] = t6;
  m->transitions[7] = t7;
  m->transitions[8] = t8;
  m->transitions[9] = t9;
  m->transitions[10] = t10;
  m->transitions[11] = t11;
  
  h = head_alloc();
  h->current = q0;
}

void palindrome() {
  m = machine_alloc();
  
  state_create(&q0, "q0");
  state_create(&q1, "q1");
  state_create(&q2, "q2");
  state_create(&q3, "q3");
  state_create(&q4, "q4");
  state_create(&q5, "q5");
  state_create(&qF, "qF");

  transition_create(&t0);
  transition_init(t0, q0, '0', '#', 1, q4);

  transition_create(&t1);
  transition_init(t1, q0, '1', '#', 1, q1);

  transition_create(&t2);
  transition_init(t2, q0, '#', '#', 1, qF);

  transition_create(&t3);
  transition_init(t3, q1, '0', '0', 1, q1);

  transition_create(&t4);
  transition_init(t4, q1, '1', '1', 1, q1);

  transition_create(&t5);
  transition_init(t5, q1, '#', '#', -1, q2);
  
  transition_create(&t6);
  transition_init(t6, q2, '#', '#', 1, qF);

  transition_create(&t7);
  transition_init(t7, q2, '1', '#', -1, q3);

  transition_create(&t8);
  transition_init(t8, q3, '0', '0', -1, q3);

  transition_create(&t9);
  transition_init(t9, q3, '1', '1', -1, q3);
  
  transition_create(&t10);
  transition_init(t10, q3, '#', '#', 1, q0);

  transition_create(&t11);
  transition_init(t11, q4, '0', '0', 1, q4);
  
  transition_create(&t12);
  transition_init(t12, q4, '1', '1', 1, q4);

  transition_create(&t13);
  transition_init(t13, q4, '#', '#', -1, q5);

  transition_create(&t14);
  transition_init(t14, q5, '0', '#', -1, q3);

  transition_create(&t15);
  transition_init(t15, q5, '#', '#', 1, qF);

  m->initial = q0;
  m->final = qF;
  m->transitions = (transition**)calloc(sizeof(transition*), 17);
  m->transitions[0] = t0;
  m->transitions[1] = t1;
  m->transitions[2] = t2;
  m->transitions[3] = t3;
  m->transitions[4] = t4;
  m->transitions[5] = t5;
  m->transitions[6] = t6;
  m->transitions[7] = t7;
  m->transitions[8] = t8;
  m->transitions[9] = t9;
  m->transitions[10] = t10;
  m->transitions[11] = t11;
  m->transitions[12] = t12;
  m->transitions[13] = t13;
  m->transitions[14] = t14;
  m->transitions[15] = t15;
  
  h = head_alloc();
  h->current = q0;
}








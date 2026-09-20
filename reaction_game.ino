#include <LiquidCrystal.h>

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

const int BUTTON_PIN = 2;
const int BUZZER_PIN = 3;

enum GameState {
  WAIT_START,
  COUNTDOWN,
  RANDOM_WAIT,
  WAIT_REACTION,
  SHOW_RESULT,
  TOO_EARLY
};

GameState state = WAIT_START;
unsigned long stateTimer = 0;
unsigned long randomDelay = 0;
unsigned long reactionStart = 0;
unsigned long reactionTime = 0;
unsigned long bestTime = 99999;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  randomSeed(analogRead(A0));  
  
  showCentered("REACTION", 0);
  showCentered("v1.0", 1);
  delay(2000);
  
  lcd.clear();
  showCentered("Press", 0);
  showCentered("button", 1);
  
  state = WAIT_START;
}

void loop() {
  switch (state) {
    case WAIT_START:
      if (isButtonPressed()) {
        beep(1000, 100);
        delay(200);
        lcd.clear();
        state = COUNTDOWN;
        stateTimer = millis();
        showCentered("Get ready", 0);
      }
      break;
    
    case COUNTDOWN: {
      unsigned long elapsed = millis() - stateTimer;
      int count = 3 - (elapsed / 1000);
      
      if (count > 0 && count <= 3) {
        lcd.setCursor(7, 1);
        lcd.print(count);
      }
      
      if (elapsed >= 3000) {
        lcd.clear();
        showCentered("WAIT...", 0);
        randomDelay = random(1000, 5000);
        stateTimer = millis();
        state = RANDOM_WAIT;
      }
      break;
    }
    
    case RANDOM_WAIT:
      if (isButtonPressed()) {
        beep(200, 500);
        lcd.clear();
        showCentered("TOO EARLY!", 0);
        showCentered("Try again", 1);
        stateTimer = millis();
        state = TOO_EARLY;
        break;
      }
      
      if (millis() - stateTimer >= randomDelay) {
        lcd.clear();
        showCentered(">>> GO! <<<", 0);
        tone(BUZZER_PIN, 1500, 50);
        reactionStart = millis();
        state = WAIT_REACTION;
      }
      break;
    
    case WAIT_REACTION:
      if (isButtonPressed()) {
        reactionTime = millis() - reactionStart;
        beep(2000, 50);
        
        lcd.clear();
        showCentered("Result:", 0);
        lcd.setCursor(0, 1);
        lcd.print(reactionTime);
        lcd.print(" ms");
        
        if (reactionTime < bestTime) {
          bestTime = reactionTime;
          delay(1500);
          lcd.clear();
          showCentered("NEW RECORD!", 0);
          lcd.setCursor(4, 1);
          lcd.print(bestTime);
          lcd.print(" ms");
        }
        
        Serial.print("Reaction: ");
        Serial.print(reactionTime);
        Serial.println(" ms");
        
        stateTimer = millis();
        state = SHOW_RESULT;
      }
      
      if (millis() - reactionStart > 10000) {
        lcd.clear();
        showCentered("Sleeping?", 0);
        showCentered("Try again", 1);
        stateTimer = millis();
        state = SHOW_RESULT;
      }
      break;
    
    case SHOW_RESULT:
      if (millis() - stateTimer > 2500) {
        lcd.clear();
        showCentered("Press", 0);
        showCentered("button", 1);
        state = WAIT_START;
      }
      break;
    
    case TOO_EARLY:
      if (millis() - stateTimer > 2000) {
        lcd.clear();
        showCentered("Press", 0);
        showCentered("button", 1);
        state = WAIT_START;
      }
      break;
  }
  
  delay(10);
}

bool isButtonPressed() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(20);
    if (digitalRead(BUTTON_PIN) == LOW) {
      while (digitalRead(BUTTON_PIN) == LOW) delay(10);
      return true;
    }
  }
  return false;
}

void showCentered(String text, int row) {
  int pos = (16 - text.length()) / 2;
  if (pos < 0) pos = 0;
  lcd.setCursor(pos, row);
  lcd.print(text);
}

void beep(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
}

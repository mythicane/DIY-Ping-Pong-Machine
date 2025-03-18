#include <FastLED.h>//FastLED makes the matrix useable by assigning each LED to an index in an array
//Hardcoding of certain notes and thier frequencies
#define NOTE_C5 523
#define NOTE_C#5 554
#define NOTE_D5 587
#define NOTE_D#5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_F#5 740
#define NOTE_G5 784
#define NOTE_G#5 831
#define NOTE_A5 880
#define NOTE_A#5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_C#6 1109
#define NOTE_D6 1175
#define NOTE_D#6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_F#6 1480
#define NOTE_G6 1568
#define NOTE_G#6 1661
#define NOTE_A6 1760
#define NOTE_A#6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093

//Pin used to control the LED matrix
#define LED_PIN 6
//Number of LEDS in the matrix (16x16)
#define NUM_LEDS 256
//Joystick analog inputs
#define p1joypin 0 
#define p2joypin 1 
//tricolor LED pins
#define LED1R 46
#define LED1G 47
#define LED2R 48
#define LED2G 49
//pin to control buzzer
int buzzPin = 2;
//Hardcoded note sequence for round over
int music[]={
  NOTE_C5, 
  NOTE_D5, 
  NOTE_E5, 
  NOTE_F5,
  NOTE_G6, 
  NOTE_A6, 
  NOTE_B6, 
  NOTE_C7};
//Hardcoded note sequence for game over
int gameover[] = {NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5,NOTE_C5};

int duration = 100; // 500 milliseconds}
CRGB leds[NUM_LEDS];//Structure fo LED control
int oldMillis = 0, newMillis = 0;
int digits[] = {0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF};
const byte segmentPins1[8] = {25,29,28,22,23,24,26,27};
const byte segmentPins2[8] = {35,39,38,32,33,34,36,37};
                          // DP, G, F, E, D, C, B, A
const byte numeral[10] = {
B11111100, // 0
B01100000, // 1
B11011010, // 2
B11110010, // 3
B01100110, // 4
B10110110, // 5
/*B00111110, // 6
B11100000, // 7
B11111110, // 8
B11100110, // 9*/
};
int i = 0, j = 0, k = 0;
//different directions the ball can go in
enum balldir{
  STILL,
  LEFTSTRAIGHT,
  RIGHTSTRAIGHT,
  LEFTUPSTEEP,
  LEFTUP45,
  //LEFTUPGRADUAL,
//  LEFTDOWNGRADUAL,
  LEFTDOWN45,
  LEFTDOWNSTEEP,
  RIGHTUPSTEEP,
  RIGHTUP45,
//  RIGHTUPGRADUAL,
//  RIGHTDOWNGRADUAL,
  RIGHTDOWN45,
  RIGHTDOWNSTEEP
};

//COLORS
//1 is background
//2 is paddles
//3 is ball
unsigned char board[16][16];
unsigned char LEDmapmatrix[16][16];
int p1paddlepos=0;
int p2paddlepos=12;
int ballcoords[2]={8,8};
int currentballdirection=RIGHTSTRAIGHT;
long randirection=0;
int speedfactor=100;

//1 is regular play
//2 is done playing
//3 is done playing and score counted
int PLAYING=1;

//0 is no winner
//1 is player 1 wins
//2 is player 2 wins
int WINNER=0;

int p1score=0; //p1 is right side
int p2score=0; //p2 is left side


void setup() {
  //sets LEDs as outputs and initializes them as off
  pinMode(LED1R,1);
  pinMode(LED1G,1);
  pinMode(LED2R,1);
  pinMode(LED2G,1);
  digitalWrite(LED1R,0);
  digitalWrite(LED2G,0);
  digitalWrite(LED1G,0);
  digitalWrite(LED2R,0);

  DDRB |= B01100000; //Sets Pins 11 (Data) and 12 (Clock) as OUTPUTS
  for(int i=0; i < 8; i++){
    pinMode(segmentPins1[i], OUTPUT);
    pinMode(segmentPins2[i], OUTPUT);
  }
  oldMillis = millis();  // Start timing
  Serial.begin(9600);
  randomSeed(analogRead(5));//Randomly seeds randomizer by taking a floating analog value
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);//initializes LEDs for future use
  //Creates the mapping sequence for LED matrix
  for (int i = 0; i <= 255; i++) {
    leds[i] = CRGB (0, 0, 10);
  }
  FastLED.show();
  for (int i = 0; i <= 15; i++){
    for (int j = 0; j <= 15; j++){
      board[i][j]=1;
    }
  } 
  for (int j = 0; j <= 15; j++){
    if(j%2==0){
      for (int i = 0; i <= 15; i++){
      LEDmapmatrix[i][j]=i+j*16;
    }
    } else if (j%2==1) {
      for (int i = 15; i >= 0; i--){
      LEDmapmatrix[i][j]=(15-i)+j*16;
    }
    }
  }
  board321();
}

//Sets the LED color based on the value present at that point in the board matrix
void arrayToLEDs(unsigned char displayboard[16][16]){
  for (int i = 0; i <= 15; i++){
    for (int j = 0; j <= 15; j++){
      if(board[i][j]==1){
        leds[LEDmapmatrix[i][j]]=CRGB(0,0,5);//background color
      } else if (board[i][j]==2){
        leds[LEDmapmatrix[i][j]]=CRGB(100,100,100);//paddle color
      } else if (board[i][j]==3){
        leds[LEDmapmatrix[i][j]]=CRGB(100,100,0);//ball color
      }
    }
  }
  
  FastLED.show();//Shows LEDs
}
//Countdown sequence before the game starts
void board321(){
  //constructs number 3
  for (int i = 0; i <= 15; i++){
    for (int j = 0; j <= 15; j++){
      if(i==0 || i==8 || i==15 || j==15){
        leds[LEDmapmatrix[i][j]]=CRGB(10,0,0);
      } else {
        leds[LEDmapmatrix[i][j]]=CRGB(0,0,5);
      }
    }
  }
  FastLED.show();//shows number 3
  delay(1000);
  //constructs number 2
  for (int i = 0; i <= 15; i++){
    for (int j = 0; j <= 15; j++){
      if(i==0 || i==8 || i==15 || (j==15 && i<=8) || (j==0 && i>=8)){
        leds[LEDmapmatrix[i][j]]=CRGB(20,0,0);
      } else {
        leds[LEDmapmatrix[i][j]]=CRGB(0,0,5);
      }
    }
  }
  FastLED.show();//shows number 2
  delay(1000);
  //constructs number 1
  for (int i = 0; i <= 15; i++){
    for (int j = 0; j <= 15; j++){
      if(j==8){
        leds[LEDmapmatrix[i][j]]=CRGB(30,0,0);
      } else {
        leds[LEDmapmatrix[i][j]]=CRGB(0,0,5);
      }
      
    }
  }
  FastLED.show();//shows number 1
  delay(1000);
}

//creates paddles on the board
void refresh_paddles(int pos1,int pos2){
  int j=0;
  //creates player 1 paddle
  for (int i = 0; i <= 15; i++){
    if((i>=pos1) && (i<=pos1+3)){
      board[i][j]=2;
    } else {
      board[i][j]=1;
    }
    
  }
  j=15;
  //creates player 2 paddle
  for (int i = 0; i <= 15; i++){
    
    if((i>=pos2) && (i<=pos2+3)){
      board[i][j]=2;
    } else {
      board[i][j]=1;
    }
  }
}
//Reads the joystick value and accordingly adjusts the paddle position
void readJOY(){
  if((analogRead(p1joypin)<200)&&(p1paddlepos>0)){
    p1paddlepos-=2;
  } else if ((analogRead(p1joypin)>800)&&(p1paddlepos<12)) {
    p1paddlepos+=2;
  }
  if((analogRead(p2joypin)<200)&&(p2paddlepos>0)){
    p2paddlepos-=2;
  } else if ((analogRead(p2joypin)>800)&&(p2paddlepos<12)) {
    p2paddlepos+=2;
  }
}
//Deals with the movement of the ball
void refresh_ball(){
  //removes the previous position of the ball
  if(!(ballcoords[1]<=0 || ballcoords[1]>=15)){
    board[ballcoords[0]][ballcoords[1]]=1;
  }
  //Determines the next direction of the ball based on the previous direction 
  switch(currentballdirection){
    //ball stays still and does nothing
    case STILL:
      break;
    case LEFTSTRAIGHT://previous direction
      ballcoords[1]-=1;//adjusts location
      if(board[ballcoords[0]][ballcoords[1]]==2){//generates random dirction
        randirection=random(2);
        tone(buzzPin, NOTE_C6, duration);//plays note once the paddle hits the ball
        if(randirection==0){
          currentballdirection=RIGHTUP45;
        } else if(randirection==1){
          currentballdirection=RIGHTDOWN45;
        }
        
      }
      break;
    case RIGHTSTRAIGHT:
      ballcoords[1]+=1;
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(2);
        tone(buzzPin, NOTE_A6, duration);
        if(randirection==0){
          currentballdirection=LEFTUP45;
        } else if(randirection==1){
          currentballdirection=LEFTDOWN45;
        } 
      }
      break;
    case LEFTUP45:
      ballcoords[1]-=1;
      ballcoords[0]-=1;
      if(ballcoords[0]<=0){
        currentballdirection=LEFTDOWN45;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_C6, duration);
        if(randirection==0){
          currentballdirection=RIGHTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=RIGHTUP45;
        } else if(randirection==2){
          currentballdirection=RIGHTUPSTEEP;
        }
      }
      
      break;
    case LEFTDOWN45:
      ballcoords[1]-=1;
      ballcoords[0]+=1;
      if(ballcoords[0]>=15){
        currentballdirection=LEFTUP45;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_C6, duration);
        if(randirection==0){
          currentballdirection=RIGHTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=RIGHTDOWN45;
        } else if(randirection==2){
          currentballdirection=RIGHTDOWNSTEEP;
        }
      }
      break;
    case RIGHTUP45:
      ballcoords[1]+=1;
      ballcoords[0]-=1;
      if(ballcoords[0]<=0){
        currentballdirection=RIGHTDOWN45;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_A6, duration);
        if(randirection==0){
          currentballdirection=LEFTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=LEFTUP45;
        } else if(randirection==2){
          currentballdirection=LEFTUPSTEEP;
        }
      }
      break;
    case RIGHTDOWN45:
      ballcoords[1]+=1;
      ballcoords[0]+=1;
      if(ballcoords[0]>=15){
        currentballdirection=RIGHTUP45;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_A6, duration);
        if(randirection==0){
          currentballdirection=LEFTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=LEFTDOWN45;
        } else if(randirection==2){
          currentballdirection=LEFTDOWNSTEEP;
        }
      }
      break;
    case LEFTUPSTEEP:
      ballcoords[1]-=1;
      ballcoords[0]-=2;
      if(ballcoords[0]<0){
        ballcoords[0]=0;
      } 
      if(ballcoords[0]==0){
        currentballdirection=LEFTDOWNSTEEP;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_C6, duration);
        if(randirection==0){
          currentballdirection=RIGHTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=RIGHTUP45;
        } else if(randirection==2){
          currentballdirection=RIGHTUPSTEEP;
        }
      }
      break;
    case LEFTDOWNSTEEP:
      ballcoords[1]-=1;
      ballcoords[0]+=2;
      if(ballcoords[0]>15){
        ballcoords[0]=15;
      } 
      if(ballcoords[0]==15){
        currentballdirection=LEFTUPSTEEP;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_C6, duration);
        if(randirection==0){
          currentballdirection=RIGHTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=RIGHTDOWN45;
        } else if(randirection==2){
          currentballdirection=RIGHTDOWNSTEEP;
        }
      }
      break;
    case RIGHTUPSTEEP:
      ballcoords[1]+=1;
      ballcoords[0]-=2;
      if(ballcoords[0]<0){
        ballcoords[0]=0;
      } 
      if(ballcoords[0]==0){
        currentballdirection=RIGHTDOWNSTEEP;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_A6, duration);
        if(randirection==0){
          currentballdirection=LEFTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=LEFTUP45;
        } else if(randirection==2){
          currentballdirection=LEFTUPSTEEP;
        }
      }
      break;
    case RIGHTDOWNSTEEP:
      ballcoords[1]+=1;
      ballcoords[0]+=2;
      if(ballcoords[0]>15){
        ballcoords[0]=15;
      } 
      if(ballcoords[0]==15){
        currentballdirection=RIGHTUPSTEEP;
      }
      if(board[ballcoords[0]][ballcoords[1]]==2){
        randirection=random(3);
        tone(buzzPin, NOTE_A6, duration);
        if(randirection==0){
          currentballdirection=LEFTSTRAIGHT;
        } else if(randirection==1){
          currentballdirection=LEFTDOWN45;
        } else if(randirection==2){
          currentballdirection=LEFTDOWNSTEEP;
        }
      }
      break;
  }

  
  if((ballcoords[1]<=0) && board[ballcoords[0]][ballcoords[1]]==1){
    //checks if player 2 wins
    WINNER=2;
    PLAYING=2;
    currentballdirection=STILL;
    board[ballcoords[0]][ballcoords[1]]=3;
  } else if ((ballcoords[1]>=15) && board[ballcoords[0]][ballcoords[1]]==1){
    //checks if player 1 wins
    WINNER=1;
    PLAYING=2;
    currentballdirection=STILL;
    board[ballcoords[0]][ballcoords[1]]=3;
  } else {
    board[ballcoords[0]][ballcoords[1]]=3;
  }

  
}
//controls behavior once round ends
void update_score_reset(){
  if(PLAYING==1){
    if(i>=1){
      write(digits[0], digits[i], digits[j], digits[k]);
      PLAYING=3;
      if(p1score>p2score){
        digitalWrite(LED1R,1);
        digitalWrite(LED2G,1);
      } else if (p1score<p2score){
        digitalWrite(LED1G,1);
        digitalWrite(LED2R,1);
      } else if (p1score==p2score){
        digitalWrite(LED2R,1);
        digitalWrite(LED1R,1);
      }
      currentballdirection=STILL;
    }
  }
  if(PLAYING==2){
    Serial.println(digitalRead(7));
    //changes player score based on last round winner
    PLAYING=3;
    if(WINNER==1){
      p1score+=1;
      digitalWrite(LED1R,1);
      digitalWrite(LED2G,1);
    } else if(WINNER==2){
      p2score+=1;
      digitalWrite(LED1G,1);
      digitalWrite(LED2R,1);
    }
  } else if (PLAYING==3){
    if(p1score>=5 || p2score>=5){
      for (int noteIndx = 0; noteIndx < 10; noteIndx++) {
          tone(buzzPin, gameover[noteIndx], duration); // Output the music
          delay(100);
        }
    } else {
    for (int noteIndx = 0; noteIndx < 8; noteIndx++) {
      tone(buzzPin, music[noteIndx], duration); // Output the music
      delay(100);
    }
    }
    PLAYING=4;
  } else if (PLAYING==4){
    //Round reset, does basically the same things as setup
    if((digitalRead(7)==0) && (digitalRead(8)==0)){
      digitalWrite(LED1G,0);
      digitalWrite(LED2R,0);
      digitalWrite(LED1R,0);
      digitalWrite(LED2G,0);
      speedfactor=100;
      Serial.println("check1");
      PLAYING=1;
      p1paddlepos=0;
      p2paddlepos=12;
      ballcoords[0]=8;
      ballcoords[1]=8;
      currentballdirection=RIGHTSTRAIGHT;
      //Ends the full game if the score goes above 5
      for (int i = 0; i <= 15; i++){
        for (int j = 0; j <= 15; j++){
          board[i][j]=1;
        }
      } 
      if(p1score >= 5){
        board321();
        p1score = 0;
        p2score = 0;
        i = 0;
        j = 0;
        k = 0;
      }
      else if(p2score >= 5){
        board321();
        p1score = 0;
        p2score = 0;
        i = 0;
        j = 0;
        k = 0;
      } else if(i>=1) {
        board321();
        p1score = 0;
        p2score = 0;
        i = 0;
        j = 0;
        k = 0;
      }
      
    }
  }
  }

void startClock() { 
    //PORTB = PORTB | B01100000; //Set both Clock and Data Pin HiGH
    PORTB = PORTB | B00100000; //Set Data Pin HiGH
    delayMicroseconds(2);
    PORTB = PORTB | B01000000; //Set Clock Pin HiGH
    delayMicroseconds(2);
    PORTB &= ~B00100000; //LOW Clock Pin
    delayMicroseconds(2);
    PORTB &= ~B01000000; //LOW Data Pin
    delayMicroseconds(2);
} 

void stopClock() { 
    PORTB &= ~B01000000; //LOW Clock Pin
    delayMicroseconds(2);
    PORTB &= ~B00100000; // LOW Data Pin
    delayMicroseconds(2);
    PORTB = PORTB | B01000000; //HIGH Clock Pin
    delayMicroseconds(2);
    PORTB = PORTB | B00100000; // HIGH Data Pin
    delayMicroseconds(2);
} 

void writeValue(int value) { 
    for (int i = 0; i < 8; i++) { 
        PORTB &= ~B01000000; //LOW Clock Pin
        if (value & (1 << i)) {
          PORTB = PORTB | B00100000; // HIGH Data Pin
        } else {
          PORTB &= ~B00100000; // LOW Data Pin
        }
        PORTB = PORTB | B01000000; //HIGH Clock Pin
    } 
    PORTB &= ~B01000000; //LOW Clock Pin
    PORTB = PORTB | B01000000; //HIGH Clock Pin
}

void write(int first, int second, int third, int fourth) {
    startClock();
    writeValue(0x40);
    stopClock();

    startClock();
    writeValue(0xc0);
    writeValue(first);
    writeValue(second);
    writeValue(third);
    writeValue(fourth);
    stopClock();

    startClock();
    writeValue(0x8F);  // Display ON, max brightness
    stopClock();
}

void gimmeFive() {
    newMillis = millis();
    if (newMillis - oldMillis >= 1000) {
        oldMillis = newMillis;
        write(digits[0], digits[i], digits[j], digits[k]); // Display time
        k++;
        if (k == 10) {
            k = 0;
            speedfactor-=30;
            j++;}
        if (j == 6) { 
            j = 0;
            
            i++;}
        if (i == 1) {
            k = 0;
            j = 0;}
    }
}
void showDigit(int number, byte segmentPins[8]){
  boolean isBitSet;
  for(int segment = 1; segment < 8; segment++){
    if(number < 0 || number > 9){
      isBitSet = 0; // turn off all segments
    }
    else{
    // isBitSet will be true if given bit is 1
      isBitSet = bitRead(numeral[number], segment); 
      // bitRead() returns the value of the bit as either 0 or 1
    }
    digitalWrite(segmentPins[segment], isBitSet);
  }
}

void loop() {
  
  arrayToLEDs(board);
  if(PLAYING==1){
    readJOY();
    refresh_paddles(p2paddlepos,p1paddlepos);
    gimmeFive();
  }
  showDigit(p1score,segmentPins1);
  showDigit(p2score,segmentPins2);
  refresh_ball();
  update_score_reset();
  
  delay(100+speedfactor);
  
}
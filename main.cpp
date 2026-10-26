// Pin 8 (PB0) -> 12k Resistor -> IR LED -> Ground
const int txPin = 8; 

// Baud rate is 45.45. Bit time = (1 / 45.45) * 1,000,000 = 22002 us
 const unsigned long  BIT_DURATION = 22002; 

void setup() {
  pinMode(txPin,  OUTPUT);

  // Stop interrupts while configuring timers
  noInterrupts();

  // Clear Timer 1 control registers
  TCCR1A = 0;
  TCCR1B = 0 ;
  TCNT1 = 0; 

  // CTC Mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);

  // Start with Idle State: Mark frequency (2295 Hz)
  setFrequency(2295);

  // Re-enable interrupts
  interrupts();
}

void loop() {
  // Milestone 2D: Repeatedly send string
  String message = "Hello World! ";
  
   for (unsigned int i = 0; i < message.length(); i++) {
     transmitChar(message[i]);
  }
  
  delay(1000 ); 
}

// Timer 1 ISR: Runs exactly when TCNT1 matches OCR1A
ISR(TIMER1_COMPA_vect) {
  PINB = bit(0); 
}

// Update the hardware timer for the required frequency
void setFrequency(int freq) {
  unsigned int newOCR;
  
  if (freq == 2295) {        // Mark frequency
     newOCR = 3485;            // (16,000, 000 / (2 * 1 * 2295)) - 1
  } else if (freq == 2125) { // Space frequency
    newOCR = 3764;           // (16,000,000 / (2 * 1 * 2125)) - 1
  } else {
    return;
}

  noInterrupts();
  OCR1A = newOCR;

  if (TCNT1 >= newOCR) {
    TCNT1 = 0;
  }
  interrupts();
}

// Package character into UART frame: 1 Start (0), 8 Data (LSB first), 1 Stop (1)
void transmitChar(char c) {
  unsigned long nextBitTime = micros();

  // 1. Send Start bit (0)
  setFrequency(2125);
  nextBitTime += BIT_DURATION;
  while (micros() < nextBitTime); // Busy-wait precisely

  // 2. Send 8 Data bits (LSB first)
  for (int i = 0; i < 8; i++) {
    if (bitRead(c, i)== 1) {
       setFrequency(2295); // Mark
    } else {
     setFrequency(2125); //  Space
  }
    
    // Add bit duration sequentially
    nextBitTime += BIT_DURATION;
     while (micros() < nextBitTime);
   }

  // 3. Send Stop bit (Mark / 1)
  setFrequency(2295);
  nextBitTime += BIT_DURATION;
  while (micros() < nextBitTime);
}



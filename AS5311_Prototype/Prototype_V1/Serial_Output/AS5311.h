// 56 milisecond delay
// Returns the serial output from AS5331
uint32_t bitbang(int CLK, int CS, int DO) {
  // write clock high to select the angular position data
  digitalWrite(CLK, HIGH);
  delay(1);
  // select the chip
  digitalWrite(CS, LOW);
  delay(1);
  digitalWrite(CLK, LOW);
  // read the value in it's entirety
  uint32_t value = 0;
  for (uint8_t i = 0; i < 18; i++) {
    delay(1);
    digitalWrite(CLK, HIGH);
    delay(1);
    digitalWrite(CLK, LOW);
    delay(1);
    auto readval = digitalRead(DO);
    if (readval == HIGH)
      value |= (1U << i);
  }
  digitalWrite(CS, HIGH);
  return value;
}

// Isolates the bottom 12 bits position value to decimal
uint32_t convertBits(uint32_t num) {
      uint32_t readval = num & 0xFFF;
      uint32_t newval = 0;
    // Flips bits order
      for (int i = 11; i >= 0; i--) 
      {
        uint32_t exists = (readval & (1 << i)) ? 1 : 0;
        newval |= (exists << (11 - i));
      }
      return newval;
}

uint32_t getSerialPosition(int CLK, int CS, int DO){
  return convertBits(bitbang(CLK, CS, DO));
}


// Todo: Make more robust than just checking first two bits
float computeElapsed(uint32_t curr, uint32_t &prevTwoSig, float elapsed) {
  uint32_t currTwoSig = curr & 0xC00;
  if((currTwoSig == 0xC00 && prevTwoSig == 0x0)) {
    Serial.println("ROLLOVER UNDERFLOW");
    elapsed -= 2.0;
  } else if (prevTwoSig == 0xC00 && currTwoSig == 0x0) {
    Serial.println("ROLLOVER OVERFLOW");
    elapsed += 2.0;
  }
  prevTwoSig = currTwoSig;
  return elapsed;
}
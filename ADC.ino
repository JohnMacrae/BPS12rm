//#ifdef ADC
bool ADC_Setup() {
  Serial.println("Setup ADC");

  bool ok = true;
  Wire.begin( mySDA, mySCL);
  MCP342x::generalCallReset();

  delay(1); // MC342x needs 300us to settle, wait 1ms

  // Check device present
  Wire.requestFrom(MCPaddress, (uint8_t)1);
  if (!Wire.available()) {
    Serial.print("No device found at address ");
    Serial.println(MCPaddress, HEX);
    ok = false;
  }
  Wire.requestFrom(MCPaddress2, (uint8_t)1);
  if (!Wire.available()) {
    Serial.print("No device found at address ");
    Serial.println(MCPaddress2, HEX);
    ok = false;
  }
  Serial.println("ADC Setup Finished...");
  return (ok);
}

void readVoltage()
{
  Serial.println ("Reading...");

  long adc0 = 0, adc1 = 0, adc2 = 0, adc3 = 0, adc4 = 0, adc5 = 0, adc6 = 0, adc7 = 0;
  uint16_t ADC[] = {0, 0, 0, 0, 0, 0, 0, 0};
  char mymsg[50];
  int iterations = 9;
  unsigned long uptimeStart;

  uptimeStart =  millis();

  MCP342x::Config status;

  uint8_t err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                                   MCP342x::resolution16, MCP342x::gain1,
                                   1000000, adc0, status);

  err = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
                           MCP342x::resolution16, MCP342x::gain1,
                           1000000, adc1, status);

  err = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
                           MCP342x::resolution16, MCP342x::gain1,
                           1000000, adc2, status);

  err = adc.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
                           MCP342x::resolution16, MCP342x::gain1,
                           1000000, adc3, status);

  err = adc02.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                             MCP342x::resolution16, MCP342x::gain1,
                             1000000, adc4, status);

  err = adc02.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
                             MCP342x::resolution16, MCP342x::gain1,
                             1000000, adc5, status);
  err = adc02.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
                             MCP342x::resolution16, MCP342x::gain1,
                             1000000, adc6, status);
  err = adc02.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
                             MCP342x::resolution16, MCP342x::gain1,
                             1000000, adc7, status);

  /*
    Convert to Voltage
    20/20 4/2
    60/20 8/2
    100/20 12/2
    140/20 16/2
    180/20 20/2
    220/20 24/2
    260/20 28/2
    300/20 32/2
  */
  /*
    Serial.print ("Adc): ");
    Serial.print ("\n");

    Serial.println(adc0);
    Serial.println(adc1);
    Serial.println(adc2);
    Serial.println(adc3);

    Serial.println(adc4);
    Serial.println(adc5);
    Serial.println(adc6);
    Serial.println(adc7);

    Serial.println("---- -*****---- -");
  */

  ADC[0] = adc0 * 2 * 2048 / 32768 ;
  ADC[1] = adc1 * 4 * 2048 / 32768 ;
  ADC[2] = adc2 * 6 * 2048 / 32768 ;
  ADC[3] = adc3 * 8 * 2048 / 32768 ;

  ADC[4] = adc4 * 10 * 2048 / 32768 ;
  ADC[5] = adc5 * 12 * 2048 / 32768 ;
  ADC[6] = adc6 * 14 * 2048 / 32768 ;
  ADC[7] = adc7 * 16 * 2048 / 32768 ;

  ADC[0] = ADC[0];//+20;
  ADC[1] = ADC[1];//+61;
  ADC[2] = ADC[2];//+84;
  ADC[3] = ADC[3];//+106;

  ADC[4] = ADC[4];//+180;
  ADC[5] = ADC[5];//+150;
  ADC[6] = ADC[6];//+150;
  ADC[7] = ADC[7];//+150;

  Cell[0] = ADC[0] + 25 ;
  Cell[1] = ADC[1] - ADC[0] + 48;
  Cell[2] = ADC[2] - ADC[1] + 28;
  Cell[3] = ADC[3] - ADC[2] + 26;

  Cell[4] = ADC[4] - ADC[3] + 79;
  Cell[5] = ADC[5] - ADC[4] + 45;
  Cell[6] = ADC[6] - ADC[5] + 44;
  Cell[7] = ADC[7] - ADC[6] + 16;

  bank = ADC[7] + 300;
  bank = bank / 1000;
  highcell = CellMax(Cell, sizeof(Cell) / sizeof(Cell[0]));
  lowcell = CellMin(Cell, sizeof(Cell) / sizeof(Cell[0]));
  cellsum = Cell[0] + Cell[1] + Cell[2] + Cell[3] + Cell[4] + Cell[5] + Cell[6] + Cell[7];
  highcellv = Cell[highcell];
  lowcellv = Cell[lowcell];
  delta = highcellv - lowcellv;

  Serial.println(Cell[0]);
  Serial.println(Cell[1]);
  Serial.println(Cell[2]);
  Serial.println(Cell[3]);
  Serial.println(Cell[4]);
  Serial.println(Cell[5]);
  Serial.println(Cell[6]);
  Serial.println(Cell[7]);
  Serial.println("-----***-----");
  Serial.println(bank);
  Serial.println(delta);
  Serial.println("-----***-----");
  Serial.println("");

  snprintf(cellStatus, 50, "%d,%d,%d,%d,%d,%d,%d,%d", Cell[0], Cell[1], Cell[2], Cell[3], Cell[4], Cell[5], Cell[6], Cell[7]);
}

int cellState()
{
  int action = 0;
  if ((highcellv > 3650) || (bank > maxVoltage))
  {
    Serial.println(highcellv);
    action = 1; //HighCut
  }
  if ((lowcellv < 2500) || (bank < minVoltage))
  {
    Serial.println(lowcellv);

    action = -1; //LowCut
  }
  if (delta > maxDelta)
  {
    if (bank > 26.6)
    {
      action = 1; //charging so Highcut
    } else {
      action = -1;
    }
  }
  return action;
}

void cutoffs(int action)
{
  strcpy(bankStatus , "Normal");

  if (action > 0)
  {
    strcpy(bankStatus, "HighCut");
    digitalWrite(pchargeEnable, false);//Highcut - stop charging
  }

  if (action < 0)
  {
    strcpy(bankStatus , "Lowcut");
    digitalWrite(ploadEnable, false);
  }
  Serial.print("Action: ");
  Serial.print(action);
  Serial.print(": ");
  Serial.println(bankStatus);
}

int CellMax( int arry[], int arrysize) {
  int result = 0;
  int i;
  for (i = 1; i < arrysize ; i++) {
    if (arry[i] > arry[result]) {
      result = i;
    }
  }
  return result;
}

int CellMin( int arry[], int arrysize) {
  int result = 0;
  int i ;
  for ( i = 1; i < arrysize ; i++) {
    if (arry[i] < arry[result]) {
      result = i;
    }
  }
  return result;
}
//#endif

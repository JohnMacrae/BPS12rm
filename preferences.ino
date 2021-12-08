/*
   Load preferences
*/

void initPrefs()
{
  Serial.println(prefs.getBool("prefState"));
  prefs.begin("BMS", false);
  bool prefState = prefs.getBool("prefState");
  prefs.end();

  if (!prefState)
  {
    savePrefs();
    Serial.println("prefs Saved");
  }
}

void loadPrefs()
{
  prefs.begin("BMS", false);
  prefState = prefs.getBool("prefState");
  minVoltage = prefs.getFloat("minVoltage", DEFAULT_MIN_VOLTAGE);
  maxVoltage = prefs.getFloat("highVoltage", DEFAULT_MAX_VOLTAGE);
  chargeEnable = prefs.getBool("chargeEnable", DEFAULT_CHARGE_ENABLE);
  loadEnable = prefs.getBool("loadEnable", DEFAULT_LOAD_ENABLE);
  maxDelta = prefs.getInt("maxDelta", DEFAULT_MAX_DELTA);

  Serial.println(prefState);
  Serial.println(minVoltage);
  Serial.println(maxVoltage);
  Serial.println(chargeEnable);
  Serial.println(loadEnable);
  Serial.println(maxDelta);
  prefs.end();
}

void savePrefs()
{
  prefs.begin("BMS", false);
  prefs.putBool("prefState", true);
  prefs.putFloat("minVoltage", DEFAULT_MIN_VOLTAGE);
  prefs.putFloat("highVoltage", DEFAULT_MAX_VOLTAGE);
  prefs.putBool("chargeEnable", DEFAULT_CHARGE_ENABLE);
  prefs.putBool("loadEnable", DEFAULT_LOAD_ENABLE);
  prefs.putInt("maxDelta", DEFAULT_MAX_DELTA);
  prefs.end();
  Serial.println("Prefs Saved");
}

void updatePrefs(void)
{
  prefs.begin("BMS", false);
  prefs.putBool("prefState", true);
  prefs.putFloat("minVoltage", minVoltage);
  prefs.putFloat("highVoltage", maxVoltage);
  prefs.putBool("chargeEnable", chargeEnable);
  prefs.putBool("loadEnable", loadEnable);
  prefs.putInt("maxDelta", maxDelta);
  prefs.end();
  Serial.println("Prefs Updated");
}

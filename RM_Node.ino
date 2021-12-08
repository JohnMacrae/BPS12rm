void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();
  prefs.begin("BMS", false);

  if (strcmp(param_name, "Low Voltage") == 0) {
    Serial.printf("Received value = %2.2f for %s - %s\n", val.val.f, device_name, param_name);
    minVoltage = val.val.f;
    updatePrefs();
    param->updateAndReport(val);
  }

  else if (strcmp(param_name, "High Voltage") == 0) {
    Serial.printf("Received value = %2.2f for %s - %s\n", val.val.f, device_name, param_name);
    maxVoltage = val.val.f;
    updatePrefs();
    param->updateAndReport(val);
  }

  else if (strcmp(param_name, "Charge Enable") == 0) {
    Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
    chargeEnable = val.val.b;
    updatePrefs();
    param->updateAndReport(val);
    digitalWrite(pchargeEnable, chargeEnable);
  }
  else if (strcmp(param_name, "Load Enable") == 0) {
    Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
    loadEnable = val.val.b;
    updatePrefs();
    param->updateAndReport(val);
    digitalWrite(ploadEnable, loadEnable);
  }
  else if (strcmp(param_name, "Max Delta (mV)") == 0) {
    Serial.printf("Received value = %d for %s - %s\n", val.val.i, device_name, param_name);
    maxDelta = val.val.i;
    updatePrefs();
    param->updateAndReport(val);
  }
  prefs.end();
}

void RM_Setup()
{
  /*
     Create the Node
  */
  Node my_node;
  my_node = RMaker.initNode("BMS Node");

  /*
    Create and add custom voltage parameters
  */
  Param voltageParam("Voltage (V)", "custom.param.voltage", value((float)DEFAULT_VOLTAGE), PROP_FLAG_READ );
  battery.addParam(voltageParam);
  battery.assignPrimaryParam(battery.getParamByName("Voltage (V)"));

  Param deltaParam("Delta (mV)", "custom.param.dvoltage", value((int)maxDelta - 10), PROP_FLAG_READ );
  battery.addParam(deltaParam);
/*
  Param extra("Extra", "custom.param.enable", value(loadEnable), PROP_FLAG_READ | PROP_FLAG_WRITE);
  extra.addUIType(ESP_RMAKER_UI_TOGGLE);
  battery.addParam(extra);
*/
  Param lowVoltageParam("Low Voltage", "custom.param.lvoltage", value((float)minVoltage), PROP_FLAG_WRITE | PROP_FLAG_READ);
  battery.addParam(lowVoltageParam);

  Param low2VoltageParam("Low Voltage2", "custom.param.lvoltage", value((float)minVoltage), PROP_FLAG_WRITE );
  battery.addParam(low2VoltageParam);

  Param highVoltageParam("High Voltage", "custom.param.hvoltage", value((float)maxVoltage), PROP_FLAG_WRITE );
  battery.addParam(highVoltageParam);

  Param maxDeltaParam("Max Delta (mV)", "custom.param.maxdelta", value((int)maxDelta), PROP_FLAG_WRITE );
  battery.addParam(maxDeltaParam);

  Param Bank_Status("Status", "custom.param.status", value( &bankStatus[0]), PROP_FLAG_READ );
  battery.addParam(Bank_Status);
  Bank_Status.addUIType(ESP_RMAKER_UI_TEXT);

  Param Cell_Status("CellStatus", "custom.param.status", value( &cellStatus[0]), PROP_FLAG_READ );
  battery.addParam(Cell_Status);
  Cell_Status.addUIType(ESP_RMAKER_UI_TEXT);

  Param hiCell("Hicell", "custom.param.hcell", value((int) highcell), PROP_FLAG_READ );
  battery.addParam(hiCell);

  Param loCell("Locell", "custom.param.lcell", value((int) lowcell), PROP_FLAG_READ );
  battery.addParam(loCell);

//  Now add the switches
  Param Charge_Enable("Charge Enable", "custom.param.enable", value(chargeEnable), PROP_FLAG_READ | PROP_FLAG_WRITE);
  Charge_Enable.addUIType(ESP_RMAKER_UI_TOGGLE);
  battery.addParam(Charge_Enable);

  Param Load_Enable("Load Enable", "custom.param.enable", value(loadEnable), PROP_FLAG_READ | PROP_FLAG_WRITE);
  Load_Enable.addUIType(ESP_RMAKER_UI_TOGGLE);
  battery.addParam(Load_Enable);



  /*
    Now point at the callback code so we know what to do when something happens
  */
  battery.addCb(write_callback);

  /*
    Add custom battery device to the node
  */
  my_node.addDevice(battery);

  /*
    This is for OTA updates via dashboard
  */
  RMaker.enableOTA(OTA_USING_TOPICS);

  /*
    Start the Rainmaker Service
  */
  RMaker.start();

  /*
    Provisioning via BLE or WiFI
  */
  WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif
}

/*
   Say how you want provisioning done
*/
void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
  }
}

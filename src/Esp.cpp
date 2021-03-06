#include "Esp.hpp"

#include <functional>

static Esp* gEsp = nullptr;

Esp::Esp(String const& hostname, String const& ApSsid,
         String const& ApPass, std::function<std::unique_ptr<IJob>(TimerManager&)> createJob)
: _hostname(hostname), _connected(false) {
  if (gEsp) {
    Serial.println("An Esp object have already been created -> reset");
    ESP.reset();
  }
  gEsp = this;

  _wifiManager.setDebugOutput(false);
  _wifiManager.setConfigPortalTimeout(3600); //sets timeout until configuration portal gets turned off in seconds
  _wifiManager.setAPCallback([] (WiFiManager*) { gEsp->_apCallback(); });
  _wifiManager.setSaveConfigCallback([] () { gEsp->_newconfCallback(); });
  Serial.println("Waiting wifi...");
  if (!_wifiManager.autoConnect(ApSsid.c_str(), ApPass.c_str())) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
    return;
  }
  _connected = true;

  MDNS.begin(_hostname.c_str());
  displayConnectionInfos();

//   setupOta();
  _timerManager.every(500000, []() {
    Serial.print("."); // Blink using serial
  }); // Blinking led
  _job = std::move(createJob(_timerManager));
}

void Esp::run() {
  _timerManager.update();
  _job->run();
//   handleOta();
}

void Esp::displayConnectionInfos() const {
  char szRet[16];
  auto address = WiFi.localIP();
  sprintf(szRet,"%u.%u.%u.%u", address[0], address[1], address[2], address[3]);
  Serial.println(String("Ip: ") + szRet);
  Serial.println("SSID: " + WiFi.SSID());
  Serial.println("MAC: " + WiFi.macAddress());
}


void Esp::_apCallback() {
  Serial.print("Entered config mode with SSID: ");
  Serial.println(_wifiManager.getConfigPortalSSID());
}

void Esp::_newconfCallback() {
  Serial.println("------- New wifi config ----------");
}

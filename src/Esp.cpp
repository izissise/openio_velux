#include "Esp.hpp"

#include <functional>

static Esp* gEsp = nullptr;

Esp::Esp(String const& hostname, String const& ApSsid, String const& ApPass)
: _hostname(hostname), _connected(false), _jobNumber(0) {
  if (gEsp) {
    Serial.println("Dev error -> reset");
    ESP.reset();
  }
  gEsp = this;

  _wifiManager.setDebugOutput(false);
  _wifiManager.setConfigPortalTimeout(3600); //sets timeout until configuration portal gets turned off in seconds
  _wifiManager.setAPCallback([] (WiFiManager*) { gEsp->_apCallback(); });
  _wifiManager.setSaveConfigCallback([] () { gEsp->_newconfCallback(); });
  Serial.println(F("Waiting wifi..."));
  if (!_wifiManager.autoConnect(ApSsid.c_str(), ApPass.c_str())) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
    return;
  }
  _connected = true;
  displayConnectionInfos();
  MDNS.begin(_hostname.c_str());
}

void Esp::run() {
  for (unsigned int i = 0; i < _jobNumber; ++i) {
    _jobs[i]->run();
  }
}

void Esp::addJob(std::shared_ptr<IJob> job) {
  _jobs[_jobNumber] = job;
  _jobNumber += 1;
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

// https://github.com/esp8266/Arduino/issues/1923
void Esp::tcpCleanup()
{
  while(tcp_tw_pcbs != nullptr) {
    tcp_abort(tcp_tw_pcbs);
  }
}


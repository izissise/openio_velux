#include "Ota.hpp"

Ota::Ota(WebServer& svr, String username, String password, String updatePath)
: _username(username), _password(password), _path(updatePath) {
  _httpUpdater.setup(&svr, _username.c_str(), _password.c_str(), _path.c_str());
}

void Ota::run() {
}

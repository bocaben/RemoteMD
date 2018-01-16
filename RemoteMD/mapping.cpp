#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;


int main() {
  string command;
  string conf;

  while(true) {
    cin >> command;

    if (command == "FL_1") conf = "S_FL_1"; 
    if (command == "FL_0") conf = "S_FL_0";
    if (command == "TU_20") conf = "S_TU_20";
    if (command == "TU_45") conf = "S_TU_45";
    if (command == "TU_90") conf = "S_TU_90";
    if (command == "FO_30") conf = "S_FO_30";
    if (command == "ST") conf = "S_ST";
    if (command == "FO_-30") conf = "S_FO_-30";
    if (command == "TU_-20") conf = "S_TU_-20";
    if (command == "TU_-45") conf = "S_TU_-45";
    if (command == "TU_-90") conf = "S_TU_-90";
    cout << conf << flush;
  }
  return 0;
}
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;


int main() {
  string command;
  string conf;
  string c = "C_";
  string s = "S_";
  cout << "WELCOME" << endl;

  while(true) {
    cin >> command;
    c += command;
    s += command;
    cout << c << endl;
    sleep(2);
    cout << s << endl;
    c = "C_";
    s = "S_";
  }
  return 0;
}
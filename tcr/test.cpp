#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;

int main() {
  cout << "Who are you?\n";
  string name;
  cin >> name;
  cout << "Hello " << name << "!\n";
 cout.flush();

  int i = 0;
  while (i < 10) {
    cout << i++ <<flush;
    sleep(1);
    
}

    return 0;
}
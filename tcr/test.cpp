#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;

int main() {
	string input;
	while (true) {
		cin >> input;
		sleep(rand() % 10);		// Randomly sleeps for up to 10 seconds
		if (rand() % 10 < 8)	// Succeeds 10% of the time
			cout << "S_" << input << endl << flush;
		else
			cout << "F_" << input << endl << flush;
	}
    return 0;
}
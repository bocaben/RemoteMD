#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include <windows.h>

using namespace std;



int main() {
	string input;
	int i;
	DWORD total_available_bytes = 0;
	HANDLE hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
	while (true) {
		if (FALSE == PeekNamedPipe(hStdIn,
			0,
			0,
			0,
			&total_available_bytes,
			0))
		{
			cout << "#Something went wrong polling stdin" << endl;
		}
		else if (total_available_bytes > 0)
		{
			cin >> input;	
			i = 0;
			//sleep(rand() % 10);		// Randomly sleeps for up to 10 seconds
			if (rand() % 10 < 8)		// Succeeds 20% of the time
				cout << "S_" << input << endl << flush;
			else
				cout << "F_" << input << endl << flush;
		}
		else {
			if (i % 10 == 0)
				cout << "#It's been " << i << " seconds since last cin" << endl << flush;
		}
		i++;
		sleep(1);
	}
    return 0;
}
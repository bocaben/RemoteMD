/*
#include "stdafx.h"
#include "SerialClass.h" // arduino comunication header
#include <string>
#include <iostream>
#include<vector>

using namespace std;

void send_message (Serial *SP, char message[], int mode)
{
	for (int i = 0; message[i - 1] != '\n'; i++)
	{
		char *ch = &message[i];
		SP->WriteData(ch, 1);
	}

}

void read_sensors(Serial* SP)
{
	double REvalue;
	double LEvalue;
	double Compassvalue;
	double Accelarationvalue;

	if (SP->IsConnected())
		std::cout << "We're connected";
	char out[255] = "SENS\n";
	const int dataLength = 255;

	int readResult = 0;

	int indx = 0;
	char endMarker = '\n';

	int m = 0;
	int num_of_sensors = 4;
	cout << "d1";
	while (m < num_of_sensors)
	{
		string incomingstring;
		char incomingchar[1] = "";			// don't forget to pre-allocate memory
		cout << out;
		m = 0;
		//SP->WriteData(out, dataLength);
		readResult = 0;

		//while (readResult == 0)
		//{
		//SP->WriteData(out, dataLength);
		send_message(SP, out, 1);

		int i = 0;
		char old_char = 0;
		while (incomingchar[0] != '\n')
		{
			i++;
			readResult = SP->ReadData(incomingchar, 1);
			if (i > 1000)
			{
				send_message(SP, out, 1);
			}

			if (old_char != incomingchar[0])
			{
				incomingstring.push_back(incomingchar[0]);
				old_char = incomingchar[0];
			}
		}
		//	cout << readResult;
		//}

		cout << "in: " << incomingstring << endl;
		//string incomingstring = incomingData;
		//cout << endl<<incomingstring;
		istringstream tempstring(incomingstring);

		do
		{
			string title;
			tempstring >> title;
			if (title == " ")
			{
			}
			if (title == "ER" || title == " ER")			// recive right encoder
			{
				tempstring >> REvalue;
				std::cout << "\nRE: " << REvalue << endl;
				//delay(500);
				m = m + 1;
			}
			if (title == "EL")			// recive right encoder
			{
				tempstring >> LEvalue;
				std::cout << "LE: " << LEvalue << endl;
				//delay(500);
				m = m + 1;
			}
			if (title == "C")				// recive compass engle
			{
				tempstring >> Compassvalue;
				std::cout << "C: " << Compassvalue << endl;
				//delay(500);
				m = m + 1;
			}
			if (title == "AC")				// recive accelaration
			{
				tempstring >> Accelarationvalue;
				std::cout << "AC: " << Accelarationvalue << endl;
				//delay(500);
				m = m + 1;
			}
		} while (tempstring);
	}
	if (m == num_of_sensors)
	{

		//std::cin >> REvalue;
		//std::cin >> LEvalue;
		std::cin >> Compassvalue;
		//std::cin >> Accelarationvalue;
		right_encoder_mesure.push_back(REvalue);
		left_encoder_mesure.push_back(LEvalue);
		compass_mesure.push_back(Compassvalue);
		accelaration_mesure.push_back(Accelarationvalue);

		timer.push_back((long)clock() / CLOCKS_PER_SEC);
		cout << "time: " << timer.back() << endl;

	}
	else
	{
		std::cout << "cant read sensors, trying again..";
		read_sensors(SP);
	}

}

void send_control_power(Serial* SP, double u, int mode)
{

	// mode 1: turning.
	//mode 2: forward.

	if (mode == 1)
	{
		char out[255];
		sprintf_s(out, "T %f \n", u);
		const int dataLength = 255;
		char incomingData[dataLength] = "";			// don't forget to pre-allocate memory
		int readResult = 0;
		char incoming_char[1] = { 0 };
		int i = 0;
		SP->flush();
		send_message(SP, out, 1);
		//SP->WriteData(out, dataLength);
		//delay(300);
		/*do		{
		readResult = SP->ReadData(incoming_char, 1);
		} 	while (incoming_char[0] != 'T');

		do		{
		incomingData[i] = incoming_char[0];
		i++;
		readResult = SP->ReadData(incoming_char, 1);
		}	while (incoming_char[0] != '\n');

		std::cout << "Bytes read: (0 means no data available)" << readResult << endl;
		std::cout << incomingData << endl;

	}
	if (mode == 2)
	{
		char out[255];
		sprintf_s(out, "F %f \n", u);
		const int dataLength = 255;
		char incomingData[dataLength] = "";			// don't forget to pre-allocate memory
		int readResult = 0;

		send_message(SP, out, 1);
		//SP->WriteData(out, dataLength);
		readResult = SP->ReadData(incomingData, dataLength);
		std::cout << "Bytes read: (0 means no data available)" << readResult << endl;
		std::cout << incomingData << endl;
	}

}


pair <float, float> XY_2_dist_theta(pair<int, int > XY)
{
	pair <double, double> dist_theta;
	double xdouble = (float)(XY.first);
	double ydouble = (float)(XY.second);

	dist_theta.second = atan2(xdouble, ydouble) * 180 / pi; // angle in degrees

	dist_theta.first = sqrt(pow(xdouble, 2) + pow(ydouble, 2));

	return dist_theta;
}

void control_turn(Serial* SP, double  theta_ref, double max = 1, double min = -1, double Kp = 3, double Kd = 0, double Ki = 1)
{
	double KI = 1;
	double KD = 1;
	double KP = 1;
	double theta_reading = compass_mesure.back();
	double error = theta_ref - theta_reading;
	//theta_ref = theta_ref - theta_reading;
	double u;
	double dt = 0.1;
	clock_t pretimer = clock() / CLOCKS_PER_SEC;
	double integral = 0;
	//PID pid_turn( dt, max, min, Kp, Kd, Ki);
	do
	{
		double pre_error = error;
		read_sensors(SP);
		theta_reading = compass_mesure.back();
		//dt = timer.back() - pretimer;
		pretimer = clock() / CLOCKS_PER_SEC;
		if (dt > 0.1)
			Kd = KD;				//need to adjust.

		error = theta_ref - theta_reading;
		double P = error*Kp;
		integral += error * dt;
		double I = Ki * integral;
		double D = 0;
		if (dt > 0.05)
		{
			double derivative = (error - pre_error) / dt;
			double D = Kd * derivative;
		}



		u = P + I + D;

		//u = pid_turn.calculate(theta_ref, theta_reading);




		send_control_power(SP, u, 1);
	} while (abs(error) > 0.1);
}

void control_forward(Serial* SP, double dist_ref, double max = 1, double min = -1, double Kp = 3, double Kd = 0, double Ki = 1)
{
	double KI = 1;
	double KD = 1;
	double KP = 1;
	double dist0 = (pi / 180)*(right_encoder_mesure.back() + left_encoder_mesure.back()) / 2;
	double dist_reading = 0;
	double u;
	double error = dist_ref - dist_reading;
	double dt;

	double integral = 0;
	//PID pid_turn( dt, max, min, Kp, Kd, Ki);

	do
	{
		double pre_error = error;
		read_sensors(SP);

		dist_reading = (pi / 180)*(right_encoder_mesure.back() + left_encoder_mesure.back()) / 2 - dist0;
		dt = timer.back() - timer[timer.size() - 1];
		if (dt > 0.1)
			Kd = KD;				//need to adjust.

		error = dist_ref - dist_reading;
		double P = error*Kp;
		integral += error * dt;
		double I = Ki * integral;
		double D = 0;
		if (dt > 0.1)
		{
			double derivative = (error - pre_error) / dt;
			double D = Kd * derivative;
		}

		u = P + I + D;

		//u = pid_turn.calculate(theta_ref, theta_reading);
		send_control_power(SP, u, 2);
	} while (abs(error) > 0.1);
}

void second_position_and_orientation_chek(int i)
{
	vector<double> delta_right_encoder_mesure;
	vector<double> delta_left_encoder_mesure;
	vector<double> W;
	vector<double> R;
	vector<double> X;
	vector<double> Y;
	vector<double> theta;
	vector <pair<double, double>> ICC;
	pair <int, int> start_pozition = oldpoints.back();

	X.push_back(start_pozition.first);
	Y.push_back(start_pozition.second);
	theta.push_back(oldheading.back());
	double delt = 0.1;
	for (unsigned int i = 0; i < min(right_encoder_mesure.size() - 1, left_encoder_mesure.size() - 1); i++)
	{
		delta_right_encoder_mesure.push_back(right_encoder_mesure[i + 1] - right_encoder_mesure[i]);
		delta_left_encoder_mesure.push_back(left_encoder_mesure[i + 1] - left_encoder_mesure[i]);
		W.push_back((delta_right_encoder_mesure[i] - delta_left_encoder_mesure[i]) / (wheels_dist));
		R.push_back((wheels_dist / 2)*(delta_right_encoder_mesure[i] + delta_left_encoder_mesure[i]) / (delta_right_encoder_mesure[i] - delta_left_encoder_mesure[i]));
		ICC.push_back({ X[i] - R[i] * sin(theta[i]), Y[i] + R[i] * cos(theta[i]) });
		X.push_back(cos(W[i] * delt)*(X[i] - ICC[i].first) - sin(W[i] * delt)*(Y[i] - ICC[i].second) + ICC[i].first);
		Y.push_back(sin(W[i] * delt)*(X[i] - ICC[i].first) + cos(W[i] * delt)*(Y[i] - ICC[i].second) + ICC[i].second);
		theta.push_back(theta[i] + W[i] * delt);
	}
	// the current position of the robot from the center of the world map.
	location[0] = (int)X[X.size()];
	location[1] = (int)Y[Y.size()];
	heading = theta[theta.size()];
}

void control(Serial* SP)   //takes the points in pathpoints, uses "control_turn()","control_farward()" and "final_position_and_orientation_chek()" to take the robot to next point.
{
	int i = 0;
	while (pathpoints.size() != 0)
	{
		int delx = pathpoints[i + 1].first - pathpoints[i].first;
		int dely = pathpoints[i + 1].second - pathpoints[i].second;

		//pair<int, int> delta = {delx, dely};   // cartezioan vector to the next point on the path. 
		pair <float, float> dist_theta = XY_2_dist_theta({ delx, dely });  // the distance and engle to the next point on the path.
		control_turn(SP, dist_theta.second);
		//control_forward(SP, dist_theta.first);

		i++;
	}
	second_position_and_orientation_chek;
}*/

//Developer: Dr. Mohammad Roni
//Copyright 2021, Battelle Energy Alliance, LLC All Rights Reserved
#include <iostream>
#include "Arc.h"
#include"county.h"
#include "Input_Data.h"
#include "ImportData.h"
#include "Optimization Model.h"
#include<cmath>
#include<fstream>
#include <ctime>
using namespace std;


int main(int argc, const char * argv[])
{
	float start_s = clock();
	int i;
	int j;
	ofstream outputarc("generatedArct.txt");
	ofstream outputnode("generatedNodet.txt");
	vector<county> buyers; // time and location of each charge demand ; 
	vector<county>sellers; // charging station info 
	map<int, int >map_period; //to keep information about each period
	map<int, county>map_ev_location; // to keep information about each EV_location which needs to be recharged
	map<int, county>map_chrg_loc; //to locate charging station

	map<int, map<int, Arc>>ODMap;

	map<int, map<int, Arc>>Hr_week_Map; //to keep track charging station utlilization and waiting time at specif hour and week/week end

	map<int, Specific_hour_weekend>map_period_2; //to keep track hour and week of a period

	vector<Arc>Arcs11;
	
	ODMap= LoadCounty(buyers,sellers, map_period, map_ev_location, map_chrg_loc, map_period_2, Hr_week_Map);
	
	
	vector<Arc> allArcs; //Here I declared a vector of Arcs
	LoadArcs(allArcs, buyers, sellers, map_period, map_ev_location,map_chrg_loc, ODMap, map_period_2, Hr_week_Map, Arcs11);



	vector<county>period, EV_loc, chrg_loc;

	int YvarSize = 0, ZvarSize = 0;
	
	
	
	CPLEXProgram2(allArcs, buyers, sellers, map_period, map_ev_location, map_chrg_loc, period, EV_loc, chrg_loc, YvarSize, ZvarSize, Arcs11); //use for CBS

	
	int stop_s = clock();

   cout << "time: in seconds " << (stop_s-start_s)/double(CLOCKS_PER_SEC) << endl;
 
    return 0;
}


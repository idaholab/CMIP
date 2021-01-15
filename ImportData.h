//Developer: Dr. Mohammad Roni
//Copyright 2021, Battelle Energy Alliance, LLC All Rights Reserved
#include <iostream>
#include <fstream>
#include<list>
#include<algorithm>
#include<vector>
#include<cmath>
#include <iterator>
#include <map>
#include<queue>
#include<string>
#include<numeric>

using namespace std;



 
bool myfunction (int i,int j) { return (i<j); }


//new charging time calculation based on BMW-i3 18.8 

float cal_charging_time(float SOC_start, float SOC_end) //% SOC start and SOC end 
{
	float charging_time = 0, charging_time1 = 0, charging_time2 = 0;
	double c7 = 8.86559510405027E-11;
	double c6 = 2.77766563784911E-08;
	double c5 = 3.47267805273874E-06;
	double c4 = 0.000219059376902553;
	double c3 = 0.00727559575683029;
	double c2 = 0.120652957440723;
	double c1 = 1.16002067938863;
	double c0 = 1.67574644348688;

	SOC_start = SOC_start * 100;
	SOC_end = SOC_end * 100;

	charging_time1 = c7*pow(SOC_start, 7) - c6*pow(SOC_start, 6) + c5*pow(SOC_start, 5) - c4*pow(SOC_start, 4) + c3*pow(SOC_start, 3) - c2*pow(SOC_start, 2) + c1*SOC_start - c0;

	charging_time2 = c7*pow(SOC_end, 7) - c6*pow(SOC_end, 6) + c5*pow(SOC_end, 5) - c4*pow(SOC_end, 4) + c3*pow(SOC_end, 3) - c2*pow(SOC_end, 2) + c1*SOC_end - c0;

	charging_time = charging_time2 - charging_time1;

	return  charging_time;
}






//this function determines the waiting time due to occupancy of other vehicle 
float cal_wait_t_occupancy(float travel_time, county &c, map<int, map<int, Arc>>&Hr_week_Map) //if trip is assigned in the sampe period it started
{
	float delay_time = 0;
	int hour_of_day = int(c.specific_hour + travel_time);
	delay_time = Hr_week_Map[hour_of_day][c.weekend].getLongitude();
	return delay_time;
}


float cal_wait_t_occupancy2(map<int, Specific_hour_weekend>&map_period_2, int p, map<int, map<int, Arc>>&Hr_week_Map) //this is different than first one . Because it  measures the occupancy if trip is not assigned in the sampe period it started  
{
	float delay_time = 0;
	int hour_of_day = map_period_2[p].specific_hour;
	int weekend_ = map_period_2[p].weekend;
	delay_time = Hr_week_Map[hour_of_day][weekend_].getLongitude();
	return delay_time;
}
//find first and last period: pick a random number among those period and random location that generate a random trip location and period 
void trip_increase(int trip, int count_vehicle, vector<county> &buyers, map<int, int >&map_period, map<int, Specific_hour_weekend>&map_period_2, map<int, county>&map_ev_location)
{
	int loc_rand = 1;
	int t_rand = 0;
	int i = 0;

	int last_period = (--map_period.end())->first; //measuring the last period 
	map<int, int >::iterator it;
	it = map_period.begin();
	int first_period = it->first; //

								  //cout << first_period << "  " << last_period << endl;
	int last_loc = (--map_ev_location.end())->first; //measuring the last period 

													 //cout << last_loc << "  " << endl;

	map<int, county>::iterator it2;
	it2 = map_ev_location.begin();
	int first_loc = it2->first; //

								//cout << first_loc << "  " << endl;


	for (i = count_vehicle; i < count_vehicle + trip; i++)
	{
		loc_rand = rand() % last_loc + first_loc;
		t_rand = rand() % last_period + first_period;

		county *a =new county;
		a->loadPeriod(t_rand); //period
		a->loadcountyLatitude(map_ev_location[loc_rand].getcountyLatitude());
		a->loadcountyLongitude(map_ev_location[loc_rand].getcountyLongitude());
		a->loadcountyID(loc_rand); //location id
		a->loadMode(map_period[t_rand]); //period duration , last period duration =99999999
		a->load_hour_weekend(map_period_2[t_rand].specific_hour, map_period_2[t_rand].weekend);
		buyers.push_back(*a); //all trip information
		delete a;
	}

}







//buyers contataing time and location of each charge demand ; 
//sellers contatin charging station info 
//map<int, int >map_period; //to keep information about each period
//map<int, county>map_ev_location; // to keep information about each EV_location which needs to be recharged

map<int, map<int, Arc>> LoadCounty(vector<county> &buyers, vector<county>&sellers, map<int, int >&map_period, map<int, county>&map_ev_location,
	map<int, county>&map_chrg_loc, map<int, Specific_hour_weekend>&map_period_2, map<int, map<int, Arc>>&Hr_week_Map) //loading historical trip information 
{
    ifstream theFile;
	theFile.open("time_location.txt");
	ofstream inputCounty ("input1.txt");

    //declaring variable according to input file sequence`
	
	int p_ = 0;
	int t_ = 0;
	int thecountyID=0;
	float thecountylat=0;
	float thecountylng=0;
	int p_duration = 0;
	int count_vehicle = 0;
	int spec_hour = 0, weekend_ = 0;
	
    if (theFile.is_open() )
    {
		while (theFile >> p_ >> t_ >> thecountylat >> thecountylng>> thecountyID>> p_duration>> spec_hour>>weekend_)
		{
			{
				county *a= new county;
				Specific_hour_weekend *p =new Specific_hour_weekend;
				p->load_hour_weekend(spec_hour, weekend_);
				a->loadPeriod(p_); //period
				a->loadTime(t_); //time of period
				a->loadcountyLatitude(thecountylat);
				a->loadcountyLongitude(thecountylng);
				a->loadcountyID(thecountyID); //location id
				a->loadMode(p_duration); //period duration , last period duration =99999999
				a->load_hour_weekend(spec_hour, weekend_);
				buyers.push_back(*a); //all trip information 
				map_period[p_] = p_duration;
				map_period_2[p_] = *p;
				map_ev_location[thecountyID] = *a; //keep tracking information countywise 
				delete a;
				delete p;
				count_vehicle++; 
			}
		}
	}
	theFile.close();
	//vehicle_to_charged = count_vehicle;

	//int trip_inrease = 628;
	
	//Randomly increase trip: 
	//trip_increase(trip_inrease, count_vehicle, buyers, map_period, map_period_2, map_ev_location);

	
	cout << "number of trip is " << buyers.size() << endl;


	//**************************end of creating randomly trip increase *******************************


	
	thecountyID = 0;//charging station id
	thecountylat = 0;
	thecountylng = 0;
	p_ = 0; //number of port
	t_ = 0; //parking capacity
	float voltage = 0;
	int station_type = 0; //owerneship type
	float chrg_time = 0;
	
	theFile.open("charging_station.txt");
	
	if (theFile.is_open())
	{
		while (theFile >> thecountyID >> thecountylat >> thecountylng>>p_>>t_>>voltage>>station_type>>chrg_time)
		{
			{
				county *a= new county;
				a->loadcountyID(thecountyID);  //charging station id
				a->loadcountyLatitude(thecountylat);
				a->loadcountyLongitude(thecountylng);
				a->loadPeriod(p_);// number of port at each charging station
				a->loadTime(t_); // number of parking station at each chargning station
				a->loadcountysupply(voltage); //voltage at each charging staiton ( e.g 25kw or 50 kw)
				a->loadMode(station_type); // ownership type ( e.g BMW or public 1= BMW, 2= public
				a->loadcountysupply2(chrg_time); // charging time 
				sellers.push_back(*a);
				map_chrg_loc[thecountyID] = *a;
				delete a;
			}
		}
	}
	theFile.close();

	//create distance and time file: 
	map<int, map<int, Arc>>ODMap;


	theFile.open("O_D.txt"); //this file containts data about origin id, destinatin id, actual distance, travel time 
	if (theFile.is_open())
	{
		while (theFile >> thecountyID >> p_ >>thecountylat>> thecountylng)
		{
			{
				Arc *a=new Arc;
				a->loadLatitude(thecountylat); //actural distance 
				a->loadLongitude(thecountylng); //travel time per minuet
				ODMap[thecountyID][p_] = *a;
				delete a;
				//cout << ODMap[thecountyID][p_].getLatitude() << "   " << ODMap[thecountyID][p_].getLongitude() << endl;
			}
		}
	}
	theFile.close();

	//loading expected waiting time at specific hour-week/days/ weekend combination 
	theFile.open("chrg_availability.txt"); //this file containts data about origin id, destinatin id, actual distance, travel time 
	if (theFile.is_open())
	{
		while (theFile >> thecountyID >> p_ >> thecountylat >> thecountylng)
		{
			{
				Arc *a=new Arc;
				a->loadLatitude(thecountylat); //probability
				a->loadLongitude(thecountylng); //waiting time per minuet
				Hr_week_Map[thecountyID][p_] = *a;
				delete a;

			}
		}
	}
	theFile.close();


	return ODMap;
	
}

//defining arc type : 1= wait at vehicle location, 2= travel to charging location and  wait at charging location to be scheduled , 3= travel and assinged to charger 
// attribute of arc : time period, vehicle location, charging location, wait or parked, distance/cost,  example x[t][i][j] where j can be 1,2,or 3 
//charging location o means vehicle has not moved 
//cost of arc if arc type 1, then T[t+1][i][j]-T[t][i][j] , if arc type 2 , then travel time + waiting time , if arc type 3, then travel time+ charging time 

void LoadArcs(vector<Arc> &allArcs, vector<county> &buyers, vector<county>&sellers, map<int, int >&map_period, map<int, county>&map_ev_location,
	map<int, county>&map_chrg_loc, map<int, map<int, Arc>>&ODMap, map<int, Specific_hour_weekend>&map_period_2, map<int,
	map<int, Arc>>&Hr_week_Map, vector<Arc>&Arcs11)
{
    int i, j,k,l_;
    float thedistance = 0; 
	ifstream theFile;
	int st = 0, en = 0;
	ofstream output_p("bugtest___.txt");
	float dis = 0;
	//buyers contataing time and location of each charge demand ; 
	//sellers contatin charging station info 
	thedistance=0;
	float Max_Dist = 20;

	map<int, int >::iterator pit;
	
	int max_period_count = 0;
	float arc_cost = 0;
	float arc_cost2 = 0;
	float arc_cost3 = 0;
	float arc_cost23 = 0;

	float arc_cost12 = 0;
	float arc_cost12_t = 0;

	float arc_cost13_t = 0;

	float arc_cost122_t = 0;

	float tmptime = 0;
	int countP = 0;
	float total_time_count = 0;

	float tmpt_time = 0;

	float tmp_arc_cost2 = 0; //declared to estiamte cost assignment type 2-2

	float tmp_arc_cost2_begin=0 ; //to track the waiting time at l-0 for 22 type arc
	

	//Assignment explanation
	//11=wait at ev location, 12=wait at ev location and then traveal to wait again at Charging location , 1-3=waited at evlocation and then assigned for charging 
	//122= waited at ev location ,then traveled to charing station to wait and again wait upto next period at charging station
	//123= waited at ev location ,then traveled to charing station to wait and then assigned for charging
	//22=Travel to wait at charging staion , 23=waited at charging station and then charged
	//30= assigned to charge without waiting 
	
	
	vector<Arc>Arcs12;
	vector<Arc>Arcs22;
	vector<Arc>Arcs122;


	float t_waiting_EV = 0;
	float t_travel_time = 0;
	float t_waiting_at_parking_station = 0;
	float t_waiting_at_busy_hour = 0;
	float t_charging_time = 0;  // waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;

	float final_soc = 0; //unit is in kwhr

	float SOC_start = 0; //delcared to estimate the % of SOC after finishing the trip for charging 

	float energy_consumption_per_mile = 0; //0.305555556=if we assume linear energy consumption; BMW i-3 assumeing 22 kwh has 72 driving range. This assumption is for paper  


	int last_period = (--map_period.end())->first; //measuring the last period 

	cout << last_period << " is last_period " << endl;

	int lastp; //the period where assinged , used as function argument 

	//master loop to create all arcs
	for (i = 0; i <buyers.size(); i++) //for all trip with ev location and time period
	{
		t_waiting_EV = 0;
		arc_cost2 = 0;
		arc_cost3 = 0;
		arc_cost23 = 0;
		arc_cost12 = 0;
		
		for (l_ = 0; l_ < max_period_waiting; l_++)
		{
			if (buyers[i].getPeriod() + l_ < last_period) //(--map_period.end())->first=last key 
			{
				//cout << buyers[i].getPeriod() <<"  "<<l<<"   " << (--map_period.end())->first<< endl;

				t_waiting_EV = t_waiting_EV + map_period[buyers[i].getPeriod() + l_];

				Arc *u=new Arc; //X_τtij
				u->loadTripId(i); //τ=trip, t=time period, i= ev_loc, j=assingmnet 
				u->loadStart(buyers[i].getPeriod()); //time period
				u->loadMid1(buyers[i].getCountyID()); //vehicle location id
				u->loadEnd(buyers[i].getCountyID()); //charging location
				u->loadMid2(buyers[i].getPeriod()); //// additional id to track the intermediate period it has waited
				u->loadEnd1(buyers[i].getPeriod() + l_); //assigned period
				u->loadArcCost(t_waiting_EV, 0, 0, 0, 0); // period length 
				u->loadArcType(11); //waiting at EV location  //new waiting
				u->loadSecArcType(99999); //port id ;99999 means not applicable
				u->loadLatitude(0);//u.loadLatitude=distance
				if (buyers[i].getPeriod() + l_ < last_period)
				{
					allArcs.push_back(*u);
					Arcs11.push_back(*u);
				}
				delete u;


			}
		}
		for (j = 0; j <sellers.size(); j++)
			for (k = 0; k < sellers[j].getPeriod(); k++) //getperiod =number of charging port
			{
				tmp_arc_cost2 = 0;
				tmp_arc_cost2_begin = 0;
				t_travel_time = ODMap[buyers[i].getCountyID()][sellers[j].getCountyID()].getLongitude(); //travel time

				thedistance = ODMap[buyers[i].getCountyID()][sellers[j].getCountyID()].getLatitude();//distance 

				velocity = 60*thedistance / t_travel_time; //velocity  mile per hour 
				energy_consumption_per_mile = a_*velocity*velocity + b_*velocity + c_; // kwh/mile unit equation for energy cosnumpiotn as a function of velocity for BMWi3

				final_soc = min_SOC_b_trip - thedistance*energy_consumption_per_mile; //unit is in kwh

				
				if (final_soc > zero_SOC) 
				{
					
					SOC_start = final_soc / battery_capacity; //calculating final SOC as percetange ;
					for (l_ = 0; l_ < max_period_waiting; l_++)
						if (buyers[i].getPeriod() + l_ <= last_period) //there is no assignment beyon period size 
						{
							arc_cost12= arc_cost12+ map_period[buyers[i].getPeriod() + l_]; // total arc cost type 1-2
							if (l_ == 0)
							{
								arc_cost2 = __max(float(buyers[i].getMode())- t_travel_time,0); //if period is longer than it wait for the remaining period 
								arc_cost3 = thedistance / velocity + float(sellers[j].getcountysupply2()); 
								
								t_waiting_at_busy_hour = cal_wait_t_occupancy(t_travel_time/60, buyers[i], Hr_week_Map);
								
								t_charging_time = cal_charging_time(SOC_start, desired_SOC_end); // waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;

								arc_cost12 = map_period[buyers[i].getPeriod() + l_]; 

								if (t_travel_time <= map_period[buyers[i].getPeriod()])
								{
									Arc *u=new Arc;
									u->loadTripId(i);
									u->loadStart(buyers[i].getPeriod()); //time period at which the trip begins 
									u->loadMid1(buyers[i].getCountyID()); //vehicle location id
									u->loadEnd(sellers[j].getCountyID()); //charging location
									u->loadMid2(buyers[i].getPeriod()); // additional id to track the intermediate period it has waited 
									u->loadEnd1(buyers[i].getPeriod());//The period at which EV is assigned for charging . In this case same period 
									u->loadArcCost(0, t_travel_time, 0, t_waiting_at_busy_hour, t_charging_time); //sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;
									u->loadArcType(30); //30= charging new arrival
									u->loadSecArcType(k); // tracking port id ;
									u->loadLatitude(final_soc);
									allArcs.push_back(*u);
									delete u;
								}
								else if(t_travel_time > map_period[buyers[i].getPeriod()])//mean if travel time greater then the lenght of the period
								{
									countP = buyers[i].getPeriod();// begining period of a trip
									total_time_count = map_period[countP]; // intilizing the start time of period 
									do {
										countP = countP + 1; //counting number of period passed  for assignment 
										total_time_count = total_time_count + map_period[countP]; //temporary calculation 
									} while (total_time_count < t_travel_time); //end of while loop

									t_waiting_at_busy_hour = cal_wait_t_occupancy(t_travel_time / 60, buyers[i], Hr_week_Map);

									t_charging_time = cal_charging_time(SOC_start, desired_SOC_end); // waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;

									Arc *u =new Arc;
									u->loadTripId(i);
									u->loadStart(buyers[i].getPeriod()); //time period
									u->loadMid1(buyers[i].getCountyID()); //vehicle location id
									u->loadEnd(sellers[j].getCountyID()); //charging location
									u->loadMid2(buyers[i].getPeriod()); // additional id to track the intermediate period it has waited
									u->loadEnd1(countP);//tracking waiting upto period buyers[i].getPeriod() + l
									u->loadArcCost(0, t_travel_time, 0, t_waiting_at_busy_hour, t_charging_time); ////sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e; 
									u->loadArcType(30); //30= charging new arrival
									u->loadSecArcType(k); // tracking port id ;
									u->loadLatitude(final_soc); //final state of charge
									allArcs.push_back(*u);
									delete u;
								}
								
								////end of creating 30
							}
							else //else mean ( if l>0)
							{
								arc_cost2 = arc_cost2 + map_period[buyers[i].getPeriod() + l_];
							}
							
							//creating 22 type arc 

							if (t_travel_time < map_period[buyers[i].getPeriod()])
							{
								Arc *u= new Arc;
								u->loadTripId(i);
								u->loadStart(buyers[i].getPeriod()); //time period
								u->loadMid1(buyers[i].getCountyID()); //vehicle location id
								u->loadEnd(sellers[j].getCountyID()); //charging location 
								u->loadMid2(buyers[i].getPeriod()); // additional id to track the intermediate period it has waited
								u->loadEnd1(buyers[i].getPeriod() + l_);//tracking waiting upto period buyers[i].getPeriod() + l
								u->loadArcCost(0, t_travel_time, __max(arc_cost2 - t_travel_time, 0), t_waiting_at_busy_hour, 0); //cost = max( travel time,period length ) // ////sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e; 
								u->loadArcType(22); // 22= waiting at CS new waiting 
								u->loadSecArcType(k); //tracking parking id
								u->loadLatitude(final_soc);//u.loadLatitude=final state of charge
								if (buyers[i].getPeriod() + l_ < map_period.size())
								{
									allArcs.push_back(*u);
									Arcs22.push_back(*u);
								}
								delete u;
								
							}
							else
							{
								countP = buyers[i].getPeriod();
								total_time_count = map_period[countP]; // intilizing the start time of period 
								do {
									countP = countP + 1; //counting number of period passed  for assignment 
									total_time_count = total_time_count + map_period[countP]; //temporary calculation 
									//output_p <<l<<"  "<< countP << "   " << map_period[countP] << "  " << total_time_count << "   " << thedistance / velocity << endl;
								} while (total_time_count < t_travel_time); //end of while loop
								
								tmp_arc_cost2 = tmp_arc_cost2 + map_period[countP + l_];
								lastp = countP;
								t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map);

								Arc *u= new Arc;
								u->loadTripId(i);
								u->loadStart(buyers[i].getPeriod()); //time period
								u->loadMid1(buyers[i].getCountyID()); //vehicle location id
								u->loadEnd(sellers[j].getCountyID()); //charging location 
								u->loadMid2(buyers[i].getPeriod()); // additional id to track the intermediate period it has waited
								u->loadEnd1(countP + l_);//tracking waiting upto period buyers[i].getPeriod() + l
								u->loadArcCost(0, t_travel_time, __max(tmp_arc_cost2 - t_travel_time, 0), t_waiting_at_busy_hour, 0); //sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e; 
								u->loadArcType(22); // 22= waiting at CS new waiting 
								u->loadSecArcType(k); //tracking parking id
								u->loadLatitude(final_soc);//u.loadLatitude=final state of charge
								if (countP + l_ < last_period) //can only be assigned upto lastperiod-1
								{
									allArcs.push_back(*u);
									Arcs22.push_back(*u);
								}
								delete u;
							}
}
} //end of if statment for maximum distance
				} //end of charging port loop
			} //end of first loop
	
			//cout << " end of creating arc 3-3 , 2-2 and 1-1 " << endl;
	//creating 2-3 from 2-2 and expanding upto l period ;
	 
	for (i = 0; i < Arcs22.size(); i++)
	{
		t_charging_time = cal_charging_time(Arcs22[i].getLatitude()/battery_capacity, desired_SOC_end); // waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;
		lastp = Arcs22[i].getEnd1() + 1;
		t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map);
		Arc *u=new Arc;
		u->loadTripId(Arcs22[i].getTripId());
		u->loadStart(Arcs22[i].getStart()); //time period
		u->loadMid1(Arcs22[i].getMid1()); //vehicle location id
		u->loadEnd(Arcs22[i].getEnd()); //charging location 
		u->loadMid2(Arcs22[i].getMid2()); // additional id to track the intermediate period it has waited
		u->loadEnd1(Arcs22[i].getEnd1() + 1);//assigned to next period for charging 
		u->loadArcCost(Arcs22[i].waiting_EV, Arcs22[i].travel_time, Arcs22[i].waiting_at_parking_station, t_waiting_at_busy_hour, t_charging_time); ////sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e; 
		u->loadArcType(23); // 23= waited at CS and then assigned to charging
		u->loadSecArcType(Arcs22[i].getSecArcType()); //tracking parking id
		u->loadLatitude(Arcs22[i].getLatitude());//u.loadLatitude=distance
		allArcs.push_back(*u);
		delete u;
	}


	cout << " end of creating arc 2-3  from 22 " << endl;
	////end of ***** 2-3 


	////begining 1-2 and 1-3 from 1-1***************************
	//loop explanation: 
	// 1-3 :  for each arctype11 create arc to all charging station and parking and find the period at which the trip be assigned for charging. 
	//if trip can be assigned  same period then assigned period is the arc11  end period , if not identify the period where trip can be assigned 
	// 1-2  : Estimate the time period where trip from 1-1 type assignment will be done. Same if travel time and period legnth of 1-1th assimnemnt period+1 is same ,
	//other wise  use do-while loop to count number of period elaspsed during travel

	for (i = 0; i <Arcs11.size(); i++)
	{
		arc_cost2 = 0;
		for (j = 0; j < sellers.size(); j++)
		{
			for (k = 0; k < sellers[j].getTime(); k++)
			{
				
				arc_cost2 = 0;
				tmp_arc_cost2 = 0;
				t_travel_time = ODMap[Arcs11[i].getMid1()][sellers[j].getCountyID()].getLongitude(); //travel time Arcs11[i].getMid1()= ev location ID

				thedistance = ODMap[Arcs11[i].getMid1()][sellers[j].getCountyID()].getLatitude();//distance 

				velocity = 60 * thedistance / t_travel_time; //velocity  mile per hour 
				energy_consumption_per_mile = a_*velocity*velocity + b_*velocity + c_; //equation for energy cosnumpiotn as a function of velocity for BMWi3
				final_soc = min_SOC_b_trip - thedistance*energy_consumption_per_mile;
				if (final_soc > zero_SOC)
				{
					for (l_ = 0; l_ < max_period_waiting; l_++)
					{
						if (Arcs11[i].getEnd1() + l_ <= last_period)
						{
							//creating 1-3
							if (l_ == 0)
							{
								
								lastp = Arcs11[i].getEnd1() + 1;
								t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map);

								t_charging_time = cal_charging_time(final_soc/battery_capacity, desired_SOC_end); // waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;


								if (t_travel_time <= map_period[Arcs11[i].getEnd1() + 1])
								{
									Arc *u =new Arc;
									u->loadTripId(Arcs11[i].getTripId());
									u->loadStart(Arcs11[i].getStart()); //time period
									u->loadMid1(Arcs11[i].getMid1()); //vehicle location id
									u->loadEnd(sellers[j].getCountyID()); //charging location
									u->loadMid2(Arcs11[i].getEnd1()); // additional id to track the intermediate period it has waited
									u->loadEnd1(Arcs11[i].getEnd1() + 1);//tracking waiting upto period buyers[i].getPeriod() + l
																		//u.loadArcCost(arc_cost3); //total time= travel time + charging time ; .getcountysupply2()= charging time 
									u->loadArcCost(Arcs11[i].getArcCost(), t_travel_time, 0, t_waiting_at_busy_hour, t_charging_time); //sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;
									u->loadArcType(13); //13=waited at evlocation and then assigned for charging
									u->loadSecArcType(k); // tracking port id ;
									u->loadLatitude(final_soc);
									allArcs.push_back(*u);
									delete u;
								}
								else
								{
									
									if (t_travel_time > map_period[Arcs11[i].getEnd1() + 1])
									{
										countP = Arcs11[i].getEnd1() + 1;// begining period of a trip from 1-1 type arc
										total_time_count = map_period[countP]; // intilizing the start time of period 
										do {
											countP = countP + 1; //counting number of period passed  for assignment 
											total_time_count = total_time_count + map_period[countP]; //temporary calculation 
										} while (total_time_count < t_travel_time); //end of while loop

										lastp = countP; 
										t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map); //calculating the expected waiting time for assigned period;
										Arc *u =new Arc;
										u->loadTripId(Arcs11[i].getTripId());
										u->loadStart(Arcs11[i].getStart()); //time period
										u->loadMid1(Arcs11[i].getMid1()); //vehicle location id
										u->loadEnd(sellers[j].getCountyID()); //charging location
										u->loadMid2(Arcs11[i].getEnd1()); // additional id to track the intermediate period it has waited
										u->loadEnd1(countP);//tracking waiting upto period buyers[i].getPeriod() + l
														   //u.loadArcCost(arc_cost3); //total time= travel time + charging time ; .getcountysupply2()= charging time 
										u->loadArcCost(Arcs11[i].getArcCost(), t_travel_time, 0, t_waiting_at_busy_hour, t_charging_time); //sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e
										u->loadArcType(13); //13=waited at evlocation and then assigned for charging
										u->loadSecArcType(k); // tracking port id ;
										u->loadLatitude(final_soc);
										allArcs.push_back(*u);
										delete u;

									} //end of if  thedistance / velocity < map_period[Arcs11[i].getEnd1() + 1]) clause
								}
								
							} // end of if (l=0 bracket
							
							//creating 1-2 type arc 
							if (t_travel_time <= map_period[Arcs11[i].getEnd1() + 1])
							{
								arc_cost2 =  arc_cost2 + map_period[Arcs11[i].getEnd1() + 1 + l_];

								lastp = Arcs11[i].getEnd1() + 1;
								t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map); //calculating the expected waiting time for assigned period;
								Arc *u =new Arc;
								u->loadTripId(Arcs11[i].getTripId());
								u->loadStart(Arcs11[i].getStart()); //
								u->loadMid1(Arcs11[i].getMid1()); //vehicle location id
								u->loadEnd(sellers[j].getCountyID()); //charging location
								u->loadMid2(Arcs11[i].getEnd1()); // additional id to track the intermediate period it has waited
								u->loadEnd1(Arcs11[i].getEnd1() + 1 + l_);//tracking waiting upto period buyers[i].getPeriod() + l
								u->loadArcCost(Arcs11[i].getArcCost(), t_travel_time, __max(arc_cost2 - t_travel_time, 0), t_waiting_at_busy_hour, 0); //sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e
																																					  //u.loadArcCost(Arcs11[i].getArcCost()+arc_cost2); //total time= 1-1 cost upto t +  waiting time ( t+1 +l ) for differetn l 
								u->loadArcType(12); //13=waited at evlocation and then assigned for charging
								u->loadSecArcType(k); // tracking port id ;
								u->loadLatitude(final_soc);
								//cout << " found arc 12 " << endl;
								if (Arcs11[i].getEnd1() + 1 + l_ < last_period)
								{
									allArcs.push_back(*u);
									Arcs12.push_back(*u);
								}
								delete u;
							}
							else
							{
								countP = Arcs11[i].getEnd1() + 1;// begining period of a trip from 1-1 type arc

								
								total_time_count = map_period[countP]; // intilizing the start time of period 
								do {
									countP = countP + 1; //counting number of period passed  for assignment 
									total_time_count = total_time_count + map_period[countP]; //temporary calculation 
								} while (total_time_count < t_travel_time); //end of while loop

								tmp_arc_cost2 = tmp_arc_cost2 + map_period[countP + l_];

								lastp = countP;
								t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map); //calculating the expected waiting time for assigned period;
								Arc *u =new Arc;
								u->loadTripId(Arcs11[i].getTripId());
								u->loadStart(Arcs11[i].getStart()); //time period
								u->loadMid1(Arcs11[i].getMid1()); //vehicle location id
								u->loadEnd(sellers[j].getCountyID()); //charging location
								u->loadMid2(Arcs11[i].getEnd1()); // additional id to track the intermediate period it has waited
								u->loadEnd1(countP + l_);//tracking waiting upto period buyers[i].getPeriod() + l
								u->loadArcCost(Arcs11[i].getArcCost(), t_travel_time, __max(tmp_arc_cost2 - t_travel_time, 0), t_waiting_at_busy_hour, 0); //sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e
								u->loadArcType(12); // 12= wait at ev location and then traveal to wait again at Charging location 
								u->loadSecArcType(k); //tracking parking id
								u->loadLatitude(final_soc);//u.loadLatitude=distance
								if (countP + l_ < map_period.size()) //can only be assigned upto lastperiod-1
								{
									allArcs.push_back(*u);
									Arcs12.push_back(*u);
								}
								delete u;
							}

						}
					}
				}
			}
		}

	}

	cout << " end of creating arc 122, 12 and 1-3  from 11 " << endl;
	
	// 1-2-3 from 1-2
	//122= waited at ev location ,then traveled to charging station to wait and again wait upto next period at charging station
	//123= waited at ev location ,then traveled to charging station to wait and then assigned for charging
	// this loop perform this task 1) assign each 1-2-2 to next period  and 2) assign 1-2-3  to the charging station 

	float arc_cost123 = 0;

	float arc_cost122 = 0;

	for (i = 0; i < Arcs12.size(); i++)
	{
	    t_charging_time = cal_charging_time(Arcs12[i].getLatitude() / battery_capacity, desired_SOC_end); // waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e;
		lastp = Arcs12[i].getEnd1() + 1;
		t_waiting_at_busy_hour = cal_wait_t_occupancy2(map_period_2, lastp, Hr_week_Map);
		Arc *u =new Arc;
		u->loadTripId(Arcs12[i].getTripId());
		u->loadStart(Arcs12[i].getStart()); //time period 
		u->loadMid1(Arcs12[i].getMid1()); //vehicle location id
		u->loadEnd(Arcs12[i].getEnd()); //charging location 
		u->loadMid2(Arcs12[i].getMid2()); // additional id to track the intermediate period it has waited
		u->loadEnd1(__min(Arcs12[i].getEnd1() + 1, last_period));//assigned to next period for charging 
																//u.loadArcCost(arc_cost123); //cost = max( travel time,period length )
		u->loadArcCost(Arcs12[i].waiting_EV, Arcs12[i].travel_time, Arcs12[i].waiting_at_parking_station, t_waiting_at_busy_hour, t_charging_time); ////sum of   waiting_EV = a,travel_time = b,waiting_at_parking_station = c,waiting_at_busy_hour = d, charging_time = e; 
		u->loadArcType(123); // 123= waited at ev location ,then traveled to charging station to wait and then assigned for charging
		u->loadSecArcType(Arcs12[i].getSecArcType()); //tracking parking id
		u->loadLatitude(Arcs12[i].getLatitude());//u.loadLatitude=distance
		allArcs.push_back(*u);
		delete u;
	}

	cout << " End of loading arc type 1-1, 2-2 and 2-3 " << endl;
	//////**************************end of 2-3 arc ********************************************
	cout << " End of loading allArs and number of arc is  " << allArcs.size() << endl;
	
	cout << endl;


} 


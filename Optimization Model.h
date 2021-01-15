
//Developer: Dr. Mohammad Roni
//Copyright 2021, Battelle Energy Alliance, LLC All Rights Reserved
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include<map>
#include<cmath>
#include<list>
#include <ilcplex/ilocplex.h>
using namespace std;

typedef IloArray<IloIntVarArray> Arcs;


void createMainModel(IloModel model, IloNumVarArray x, IloNumVarArray z, vector<Arc> &allArcs, vector<county> &buyers, vector<county>&sellers, map<int, int >&map_period,
	map<int, county>&map_ev_location, map<int, county>&map_chrg_loc , vector<county>&period, vector<county>&EV_loc, vector<county>& chrg_loc,
	map<int,int>&Zmap, int ZvarSize, vector<Arc>&Arcs11);



void CPLEXProgram2(vector<Arc> &allArcs, vector<county> &buyers, vector<county>&sellers, map<int, int >&map_period, map<int, county>&map_ev_location, map<int, county>&map_chrg_loc, vector<county>&period,
	vector<county>&EV_loc, vector<county>& chrg_loc, int YvarSize, int ZvarSize, vector<Arc>&Arcs11)
{
	
	IloEnv env;
	try {
		
		int i=0,j=0,k=0,t=0; 
		map<int, county>::iterator itw;
		map<int, int>::iterator itw1;
		ofstream output_2("print_result.txt");
	
		//create a vector for time period
		for (itw1 = map_period.begin(); itw1 != map_period.end(); ++itw1)
		{
			county *a =new county;
			a->loadPeriod(itw1->first);
			a->loadTime(itw1->second);
			period.push_back(*a);
			delete a;
		}
		for (itw = map_ev_location.begin(); itw != map_ev_location.end(); ++itw)
		{
			
			county *a =new county;
			a->loadcountyID(itw->first);
			a->loadcountyLatitude(itw->second.getcountyLatitude());
			a->loadcountyLongitude(itw->second.getcountyLongitude());
			EV_loc.push_back(*a);
			delete a;
		}
		
		for (itw = map_chrg_loc.begin(); itw != map_chrg_loc.end(); ++itw)
		{

			county *a =new county;
			a->loadcountyID(itw->first);
			a->loadcountyLatitude(itw->second.getcountyLatitude());
			a->loadcountyLongitude(itw->second.getcountyLongitude());
			a->loadPeriod(itw->second.getPeriod()); //number of port at each charging station
			a->loadTime(itw->second.getTime()); // number of parking station at each chargning station
			a->loadcountysupply(itw->second.getcountysupply()); // voltage at each charging staiton ( e.g 25kw or 50 kw)
			a->loadMode(itw->second.getMode());
			a->loadcountysupply2(itw->second.getcountysupply2());
			chrg_loc.push_back(*a);
			delete a;
		}
	
		cout << "the model started with " << buyers.size() << " trip  and  " << sellers.size() << " charging station     " << allArcs.size()<<" arcs "<< endl;
		//print 
		
		//creating map container for y and z variables:
		

		map<int, map<int, map<int, int>>>Ymap;
		map<int, int>Zmap;

	

		for (j = 0; j < chrg_loc.size(); j++)
		{
			Zmap[chrg_loc[j].getCountyID()] = j;
		}
			
		
		//declaring variables
		IloModel model(env);
		IloNumVarArray x(env, allArcs.size(), 0, 1, ILOINT); //assignmnet variables  
		IloNumVarArray z(env, chrg_loc.size(), 0, 1, ILOINT); //assignmnet variables 

		
		//writing objective function: 



		double target_downtime = 52500.00;
		
		double cost_per_minuet = 0.41;



		IloExpr obj(env); //time objective

		IloExpr lhs(env);
		for (i = 0; i < allArcs.size(); i++)
			if (allArcs[i].getArcType() == 13 || allArcs[i].getArcType() == 23 || allArcs[i].getArcType() == 30 || allArcs[i].getArcType() == 13 || allArcs[i].getArcType() == 123)
			{
				obj += allArcs[i].getArcCost()*x[i];
			}
		
		for (i = 0; i < chrg_loc.size(); i++)
		{
			lhs += z[i];
		}
		
		



		createMainModel(model, x, z, allArcs, buyers, sellers,map_period,map_ev_location, map_chrg_loc, period, EV_loc, chrg_loc, Zmap,ZvarSize,Arcs11);

		//clear memory

		buyers.clear();
		sellers.clear();
		map_period.clear();
		map_ev_location.clear();
		map_chrg_loc.clear();
		period.clear();
		Arcs11.clear();
		Zmap.clear();
		//lhs.end();

		cout<< "End of creating main model "<<endl;

       
		
		IloObjective objective (env);
		objective=IloAdd(model,IloMinimize(env, obj));

		
		IloConstraint con_new_chrg;
		
		IloCplex cplex(model);
		cplex.exportModel("EV optimization model.lp");

		vector<Arc>results_summary;

		vector<Arc>result_assignment;
		vector<county>result_chrg_loc;
		int total_charging_station = 0;
		float total_down_time = 0;
		float wait_EV = 0;
		float travel_t = 0;
		float wait_chrg = 0;
		float wait_rush_h = 0;
		float charging_t = 0;

		///start for loop
		int ncharger = 6;
		int n = 21; //number of new charging stations
		for (j = 0; j <n; j++)
		{
			total_down_time = 0;
			wait_EV = 0;
			travel_t = 0;
			wait_chrg = 0;
			wait_rush_h = 0;
			charging_t = 0;
			total_charging_station = 0;

			//con_new_chrg = (lhs<= 16); //to identify 10 new charging stations if trip increased by 50%  
			con_new_chrg = (lhs <= j + ncharger);

			model.add(con_new_chrg);
			IloCplex cplex(model);
			cplex.setParam(IloCplex::EpGap, .01);
			cplex.solve();
			if (cplex.getStatus() == IloAlgorithm::Infeasible)
			{
				model.remove(con_new_chrg);
				cplex.end();
				continue;
			}

			Arc *r1 =new Arc;
			for (i = 0; i < allArcs.size(); i++) 
				if (cplex.getValue(x[i])>.01 )
				{
					if (allArcs[i].getArcType() == 30 || allArcs[i].getArcType() == 23 || allArcs[i].getArcType() == 13 || allArcs[i].getArcType() == 123)
					{
						total_down_time = total_down_time + allArcs[i].getArcCost();
						wait_EV = wait_EV + allArcs[i].waiting_EV;
						travel_t = travel_t + allArcs[i].travel_time;
						wait_chrg = wait_chrg + allArcs[i].waiting_at_parking_station;
						wait_rush_h = wait_rush_h + allArcs[i].waiting_at_busy_hour;
						charging_t = charging_t + allArcs[i].charging_time;
					}
					
					r1->loadArcID(j + ncharger);
					r1->loadTripId(allArcs[i].getTripId());
					r1->loadArcType(allArcs[i].getArcType());
					r1->loadEnd(allArcs[i].getEnd());
					r1->loadSecArcType(allArcs[i].getSecArcType());
					r1->loadMid1(allArcs[i].getMid1());
					r1->loadEnd1(allArcs[i].getEnd1());
					r1->loadArcCost(allArcs[i].waiting_EV, allArcs[i].travel_time, allArcs[i].waiting_at_parking_station, allArcs[i].waiting_at_busy_hour, allArcs[i].charging_time);
					result_assignment.push_back(*r1);
				}

			for (i = 0; i < chrg_loc.size(); i++) 
				if (cplex.getValue(z[i])>.01)
				{
					total_charging_station = total_charging_station + cplex.getValue(z[i]);
					county *r2 =new county;
					r2->loadTime(j + ncharger);
					r2->loadcountyID(chrg_loc[i].getCountyID());
					r2->loadcountyLatitude(chrg_loc[i].getcountyLatitude());
					r2->loadcountyLongitude(chrg_loc[i].getcountyLongitude());
					result_chrg_loc.push_back(*r2);
					delete r2;
				}
			model.remove(con_new_chrg);
			cplex.end();
			Arc *a =new Arc;
			a->loadArcID(total_charging_station);
			a->loadArcCost(wait_EV, travel_t, wait_chrg, wait_rush_h, charging_t);
			results_summary.push_back(*a);

			delete a;
			delete r1;


			cout << "end of loop " << j << endl;

		}

		//end of for loop 


		
		output_2 << "printing results summary" << endl;
		for (i = 0; i < results_summary.size(); i++)
		{
			output_2 <<results_summary[i].getArcID() << " " << results_summary[i].waiting_EV << "   " << results_summary[i].travel_time << "  " << results_summary[i].waiting_at_parking_station << "   ";
			output_2<< results_summary[i].waiting_at_busy_hour<<"   "<< results_summary[i].charging_time<<"   "<< results_summary[i].getArcCost()<<endl;
		}
		output_2 << "printing results charging location " << endl;
		for (i = 0; i <result_chrg_loc.size(); i++)
		{
			output_2 << result_chrg_loc[i].getTime() << "  " << result_chrg_loc[i].getCountyID() << "  " << result_chrg_loc[i].getcountyLatitude() << "   " << result_chrg_loc[i].getcountyLongitude() << endl;
		}
		output_2 << "printing results assignment  " << endl;

		for (i = 0; i < result_assignment.size(); i++)
		{
			output_2 << result_assignment[i].getArcID() << "  " << result_assignment[i].getTripId()<<"  "<< result_assignment[i].getArcType()<<"   " << result_assignment[i].getEnd()<<"  ";
			output_2 << result_assignment[i].getSecArcType() << "  " << result_assignment[i].getMid1() << "   " << result_assignment[i].getEnd1() << "  ";
			output_2<< result_assignment[i].waiting_EV<<"   "<< result_assignment[i].travel_time<<"   "<< result_assignment[i].waiting_at_parking_station<<"  "<< result_assignment[i].waiting_at_busy_hour<<"   "<< result_assignment[i].charging_time<<"  "<< result_assignment[i].getArcCost() << endl;

		}
	}

	catch (IloException& e) {
		cerr << " ERROR: " << e << endl;
	}
	catch (...) {
		cerr << " ERROR" << endl;
	}

	env.end();
}








void createMainModel(IloModel model, IloNumVarArray x, IloNumVarArray z, vector<Arc> &allArcs, vector<county> &buyers, vector<county>&sellers, map<int, int >&map_period,
	map<int, county>&map_ev_location, map<int, county>&map_chrg_loc, vector<county>&period, vector<county>&EV_loc, vector<county>& chrg_loc,
	map<int, int>&Zmap, int ZvarSize, vector<Arc>&Arcs11)
{
	int i = 0, j = 0, k = 0, l = 0, m = 0;
	double totalSupply = 0;
	IloEnv env = model.getEnv();
	double M = 100000000000;
	IloNumVarArray lambda_4(env, allArcs.size(), 0, IloInfinity, ILOFLOAT); //price dual variabe;
	IloNumVarArray t(env, allArcs.size(), 0, 1, ILOINT); //binary for price dual variable
	ofstream output_m("model_test.txt");
	char *VarName;
	VarName = new char[200];

	for (i = 0; i < allArcs.size(); i++)
	{
		sprintf(VarName, "x[%d]", i);
		x[i].setName(VarName);
	}

	for (i = 0; i < chrg_loc.size(); i++)
	{
		sprintf(VarName, "z[%d]", i);
		z[i].setName(VarName);
	}

	delete[] VarName;

	cout << " End of declaring printing variable w[i] " << endl;

	//constraint 1 (single assignment ) //all vehicles at a time t , at location i must be assigned to either charging , waiting at ev location, or travel to a charging station and waiting 

	map<int, county>::iterator it_c;
	int con1_count = 0;
	int con1_count2 = 0;

	map<int, map<int, map<int, map<int, map<int, map<int, int>>>>>>map_arcs;

	map<int, map<int, map<int, map<int, map<int, map<int, int>>>>>>map_arcs2; //created to complete enforce/block assignment

	map<int, int>::iterator it1; // Please use it1 this way : it1 = map_arcs[t][l][it3->first][it2->first].begin()
	map<int, map<int, int>>::iterator it2;
	map<int, map<int, map<int, int>>>::iterator it3;

	map<int, map<int, map<int, map<int, int>>>>::iterator it4;

	map<int, map<int, map<int, map<int, map<int, int>>>>>::iterator it5; //please aware that it5=map_arcs.begin()

	map<int, map<int, map<int, map<int, map<int, map<int, int>>>>>>::iterator it6;


	//interator for map_arcs_2


	map<int, int>::iterator its1; // Please use it1 this way : it1 = map_arcs[t][l][it3->first][it2->first].begin()
	map<int, map<int, int>>::iterator its2;
	map<int, map<int, map<int, int>>>::iterator its3;

	map<int, map<int, map<int, map<int, int>>>>::iterator its4;

	map<int, map<int, map<int, map<int, map<int, int>>>>>::iterator its5; //please aware that it5=map_arcs.begin()

	map<int, map<int, map<int, map<int, map<int, map<int, int>>>>>>::iterator its6;




	//map<int, map<int, map<int, map<int, map<int, int>>>>>::iterator it5; //please aware that it5=map_arcs.begin()

	map<int, map<int, map<int, map<int, map<int, map<int, IloExpr>>>>>>map1_con;

	map<int, map<int, map<int, map<int, map<int, IloExpr>>>>>map1v_con;


	//iterator for map1_con

	map<int, IloExpr>::iterator it1_; // Please use it1 this way : it1 = map_arcs[t][l][it3->first][it2->first].begin()
	map<int, map<int, IloExpr>>::iterator it2_;
	map<int, map<int, map<int, IloExpr>>>::iterator it3_;

	map<int, map<int, map<int, map<int, IloExpr>>>>::iterator it4_;

	map<int, map<int, map<int, map<int, map<int, IloExpr>>>>>::iterator it5_; //please aware that it5=map_arcs.begin()

	map<int, map<int, map<int, map<int, map<int, map<int, IloExpr>>>>>>::iterator it6_;


	for (i = 0; i < allArcs.size(); i++)
	{
		map_arcs[allArcs[i].getTripId()][allArcs[i].getArcType()][allArcs[i].getEnd()][allArcs[i].getSecArcType()][allArcs[i].getMid2()][allArcs[i].getEnd1()] = i; //maparcs[trip][assignment type][Chrg_loc][num.of charger/parking or 999999][inermdedita][assigned period]
		map_arcs2[allArcs[i].getEnd()][allArcs[i].getSecArcType()][allArcs[i].getEnd1()][allArcs[i].getArcType()][allArcs[i].getMid2()][allArcs[i].getTripId()] = i; //maparcs[charg][port][assigned period][assignment type][intermediate][trip]
	}

	
	int con2_count = 0;
	int count11 = 0;

	map<int, Arc>map11count;

	map<int, vector<IloNumVar>> map_var_11;

	map<int, vector<IloNumVar>>::iterator it_map_var_11;

	map<int, IloExpr>::iterator it_11;

	//container for busy/availabiluty constraint;

	map<int, vector<IloNumVar>> map_var_3;

	map<int, map<int, map<int, vector<IloNumVar>>>> map_var_2;


	//iterator for mapvar
	map<int, map<int, map<int,map<int, vector<IloNumVar>>>>>::iterator it_3_4;
	map<int, map<int, map<int, vector<IloNumVar>>>>::iterator it_3_3;

	map<int, map<int, vector<IloNumVar>>>::iterator it_3_2;
	map<int, vector<IloNumVar>>::iterator it_3_1;
	
	//container for charging location capacity

	map<int, map<int, map<int, vector<IloNumVar>>>> map_loct_epxr1_;

	map<int, map<int, map<int, vector<IloNumVar>>>> map_loct_epxr2_;

	float charging_time; //charging time
	float total_time;
	int period_duration_count = 0;

	int last_period = (--map_period.end())->first;

	for (it6 = map_arcs.begin(); it6 != map_arcs.end(); it6++) //trip
	{
		IloExpr con1(env);
		IloExpr con2(env);
		for (it5 = map_arcs[it6->first].begin(); it5 != map_arcs[it6->first].end(); it5++) //assignment
			//if (map_arcs[it6->first].find(it5->first) != map_arcs[it6->first].end())
			{
				for (it4 = map_arcs[it6->first][it5->first].begin(); it4 != map_arcs[it6->first][it5->first].end(); it4++)//chaging location
				{
					//if (map_arcs[it6->first][it5->first].find(it4->first) != map_arcs[it6->first][it5->first].end())
					{
						for (it3 = map_arcs[it6->first][it5->first][it4->first].begin(); it3 != map_arcs[it6->first][it5->first][it4->first].end(); it3++) //parking/waiting spot
						{ 
							//if (map_arcs[it6->first][it5->first][it4->first].find(it3->first) != map_arcs[it6->first][it5->first][it4->first].end())
							{
								for (it2 = map_arcs[it6->first][it5->first][it4->first][it3->first].begin(); it2 != map_arcs[it6->first][it5->first][it4->first][it3->first].end(); it2++) //intermediate peiod
								{
									//if (map_arcs[it6->first][it5->first][it4->first][it3->first].find(it2->first) != map_arcs[it6->first][it5->first][it4->first][it3->first].end())
									for (it1 = map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first].begin(); it1 != map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first].end(); it1++)//assigned period
									{
										//declared for con3 expression

										//loading con1
										if (it5->first == 13 || it5->first == 23 || it5->first == 30 || it5->first == 13 || it5->first == 123)
										{
											con1 += x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]];

											//creating loop to identify number of occupied period at a charging station

											charging_time = allArcs[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]].charging_time;
											total_time = map_period[it1->first];
											period_duration_count = 0;

											if (total_time <= charging_time && it1->first + period_duration_count <= last_period)
											{
												period_duration_count = 1;
												while (total_time <= charging_time && it1->first + period_duration_count <= last_period)
												{
													if (map_period.find(it1->first + period_duration_count) == map_period.end()) //
													{
														total_time = total_time;
														period_duration_count = period_duration_count;
														break;
													}
													total_time = total_time + map_period[it1->first + period_duration_count];
													period_duration_count = period_duration_count + 1;
												}

												//cout << charging_time<<"   "<< period_duration_count << "   " << total_time << "   " << endl;
											}
											IloExpr exprbusy(env);


											for (its3 = map_arcs2[it4->first][it3->first][it1->first].begin(); its3 != map_arcs2[it4->first][it3->first][it1->first].end(); its3++) //chargloc-port-assigned time
												for (m = 0; m <= period_duration_count; m++)
													for (its2 = map_arcs2[it4->first][it3->first][it1->first + m][its3->first].begin(); its2 != map_arcs2[it4->first][it3->first][it1->first + m][its3->first].end(); its2++) //+assignemt type
													if (its3->first == 13 || its3->first == 23 || its3->first == 30 || its3->first == 13 || its3->first == 123)
														for (its1 = map_arcs2[it4->first][it3->first][it1->first+m][its3->first][its2->first].begin(); its1 != map_arcs2[it4->first][it3->first][it1->first+m][its3->first][its2->first].end(); its1++)
														{
															if (map_arcs2[it4->first][it3->first][it1->first + m][its3->first][its2->first][its1->first] != map_arcs2[it4->first][it3->first][it1->first + m][its3->first][its2->first].empty())
															{
																exprbusy += x[map_arcs[its1->first][its3->first][it4->first][it3->first][its2->first][it1->first + m]];
															}
														}

											model.add(exprbusy <= 1);

											exprbusy.end();
													
											//container for capacity 1
											if (map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first] != map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first].empty())
											{
												map_loct_epxr1_[it4->first][it1->first][it3->first].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}
											

										}

										//loading con2
										if (it5->first == 11 || it5->first == 22 || it5->first == 30)
										{
											con2 += x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]];
										}

										//loading constraint 3:
										for (i = 0; i < Arcs11.size(); i++)
										{
											if (Arcs11[i].getTripId() == it6->first && it5->first == 11 && Arcs11[i].getEnd1() == it1->first)
											{
												map_var_11[i].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}
											if (Arcs11[i].getTripId() == it6->first && it5->first == 11 && Arcs11[i].getEnd1() + 1 == it1->first)
											{
												map_var_11[i].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}
											if (Arcs11[i].getTripId() == it6->first && it5->first == 12 && Arcs11[i].getEnd1() + 1 >= it1->first)
											{
												map_var_11[i].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}
											if (Arcs11[i].getTripId() == it6->first && it5->first == 13 && Arcs11[i].getEnd1() + 1 >= it1->first)
											{
												map_var_11[i].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}
										}

										//loading constraint 4,5,6; //t=t+1 for each 12,22 and 122 type
										if (it5->first == 12)
										{
											if (map_arcs[it6->first][123][it4->first][it3->first][it2->first][it1->first + 1] != map_arcs[it6->first][13][it4->first][it3->first][it2->first].empty())
											{
												model.add(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]] - x[map_arcs[it6->first][123][it4->first][it3->first][it2->first][it1->first + 1]] == 0);
												map_var_2[it4->first][it3->first][it1->first].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);

												map_loct_epxr2_[it4->first][it1->first][it3->first].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}

										}
										if (it5->first == 22)
										{
											if (map_arcs[it6->first][23][it4->first][it3->first][it2->first][it1->first + 1] != map_arcs[it6->first][23][it4->first][it3->first][it2->first].empty())
											{
												model.add(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]] - x[map_arcs[it6->first][23][it4->first][it3->first][it2->first][it1->first + 1]] == 0);
												map_var_2[it4->first][it3->first][it1->first].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
												map_loct_epxr2_[it4->first][it1->first][it3->first].push_back(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]]);
											}

										}
										//constraints : assigend if charging station exists 
										if (it5->first == 12 || it5->first == 22 || it5->first == 30 || it5->first == 23 || it5->first == 13 || it5->first == 123)
										{
											model.add(x[map_arcs[it6->first][it5->first][it4->first][it3->first][it2->first][it1->first]] - z[Zmap[it4->first]] <= 0);
										}

									}
								}
							}
							
						}
					}
					
				}
			}

		model.add(con1 == 1); //all trips must be scheduled at the begining of period
		model.add(con2 == 1); //all trips must be scheduled
		con1.end();
		con2.end();

	}


	cout << " End of big loop to add constrints " << endl;
	//forcing existing charging station
	for (j = ncharger_begin; j <= ncharger_end; j++)
	{
		model.add(z[j] == 1);
	}
	
	cout << " End of constratins that force existing charging stations " << endl;
	//scheduling 11 type arc
	for (it_map_var_11 = map_var_11.begin(); it_map_var_11 != map_var_11.end(); it_map_var_11++)
	{
		IloExpr test3(env);
		for (k = 0; k < map_var_11[it_map_var_11->first].size(); k++)
		{
			if (k == 0)
			{
				test3 -= map_var_11[it_map_var_11->first][k];
			}
			else
			{
				test3 += map_var_11[it_map_var_11->first][k];
			}
		}
		model.add(test3 == 0);
		test3.end();
	}
	
	cout << " End of constratins that force 11 type arc " << endl;
	
	
	//adding constraint from map_var_2;
	//blocking parking station until it's used for again
	for (it_3_3 = map_var_2.begin(); it_3_3 != map_var_2.end(); it_3_3++)
		for (it_3_2 = map_var_2[it_3_3->first].begin(); it_3_2 != map_var_2[it_3_3->first].end(); it_3_2++)
			for (it_3_1 = map_var_2[it_3_3->first][it_3_2->first].begin(); it_3_1 != map_var_2[it_3_3->first][it_3_2->first].end(); it_3_1++)
			{
				IloExpr expr_busy2(env);
				//cout << map_var_2[it_3_3->first][it_3_2->first][it_3_1->first].size() << " size of map_var_ " << endl;
				for (i = 0; i < map_var_2[it_3_3->first][it_3_2->first][it_3_1->first].size(); i++)
				{
					expr_busy2 += map_var_2[it_3_3->first][it_3_2->first][it_3_1->first][i];
				}
				model.add(expr_busy2 <= 1);

				expr_busy2.end();
			}

	cout << " End of constratins that force that block parking station at each chager  " << endl;

	//adding capacity constraints at charging station

	for (it_3_3 = map_loct_epxr1_.begin(); it_3_3 != map_loct_epxr1_.end(); it_3_3++)
		for (it_3_2 = map_loct_epxr1_[it_3_3->first].begin(); it_3_2 != map_loct_epxr1_[it_3_3->first].end(); it_3_2++)
		{
			IloExpr expr_capacity1(env);
			for (it_3_1 = map_loct_epxr1_[it_3_3->first][it_3_2->first].begin(); it_3_1 != map_loct_epxr1_[it_3_3->first][it_3_2->first].end(); it_3_1++)
			{
				for (i = 0; i < map_loct_epxr1_[it_3_3->first][it_3_2->first][it_3_1->first].size(); i++)
				{
					expr_capacity1 += map_loct_epxr1_[it_3_3->first][it_3_2->first][it_3_1->first][i];
				}
			}
			model.add(expr_capacity1 <= map_chrg_loc[it_3_3->first].getPeriod());
			expr_capacity1.end();
		}


	cout << " End of capacity constratins 1  " << endl;
	//adding capacity constraints at parking stations
	for (it_3_3 = map_loct_epxr2_.begin(); it_3_3 != map_loct_epxr2_.end(); it_3_3++)
		for (it_3_2 = map_loct_epxr2_[it_3_3->first].begin(); it_3_2 != map_loct_epxr2_[it_3_3->first].end(); it_3_2++)
		{
			IloExpr expr_capacity2(env);
			for (it_3_1 = map_loct_epxr2_[it_3_3->first][it_3_2->first].begin(); it_3_1 != map_loct_epxr2_[it_3_3->first][it_3_2->first].end(); it_3_1++)
			{
				for (i = 0; i < map_loct_epxr2_[it_3_3->first][it_3_2->first][it_3_1->first].size(); i++)
				{
					expr_capacity2 += map_loct_epxr2_[it_3_3->first][it_3_2->first][it_3_1->first][i];
				}
			}
			model.add(expr_capacity2 <= map_chrg_loc[it_3_3->first].getTime());
			expr_capacity2.end();
		}
			
	cout << " End of capacity constratins 2  " << endl;

	//clean memory
	map_arcs.clear();
	map_arcs2.clear();
	map_var_11.clear();
	map_var_2.clear();
	map_loct_epxr2_.clear();
	map_loct_epxr1_.clear();


}

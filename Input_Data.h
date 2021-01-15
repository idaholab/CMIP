

//Developer: Dr. Mohammad Roni
//Copyright 2021, Battelle Energy Alliance, LLC All Rights Reserved
	using namespace std;

	///////EV specific information 

	float velocity = .417; //mile/minuets 25 miles per hour

	//SOc calculation 
	float min_SOC_b_trip = 3.384; //base SOC 3.384, 18%
	float min_SOC_a_trip = 0.188;
	float energyConsum_per_mile = .28947;
	float winding_factor = 1.2;

	float zero_SOC = .00001;

	float a_ = .0001153, b_ = -.008357, c_ = .3988; //this is the coefficient to calculate energy consumpiton per mile

	double bigM = 9999999;
	int vehicle_to_charged = 0;

	float desired_SOC_end =.9; //unit percentage 
	float battery_capacity = 18.8; //kwh/to C
	float capacity_csstation = 50; //kwh


	int ncharger_end = 105; // 106th  
	int ncharger_begin = 100; //101th charging station

	int max_period_waiting = 3;

		 /***************** End of model Input parater ************************************************/

	      
	
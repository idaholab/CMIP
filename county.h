
//Developer: Dr. Mohammad Roni
//Copyright 2021, Battelle Energy Alliance, LLC All Rights Reserved
#include<string>
#include<map>
using namespace std;

class Specific_hour_weekend {
public:
	int specific_hour;
	int weekend;

	void load_hour_weekend(int a, int b)
	{
		specific_hour = a;
		weekend = b;
	}
};

class county:public Specific_hour_weekend
{
private:
	int countyID;
	int time_period;
	int period_duration;
	int time;
	float countylat;
	float countylng;
	string State;
	float supply;
	float supply2;
	double demand;
	string countyName; 
	int modeId;
	map<int,map<int,float>>price_supply;
	float price;
	float intercept;
	double slope;
	float Penalty = 0;

public:
	void loadPeriod(int a);
	void loadcountyID(int a);
	void loadTime(int a);

	void loadcountyLatitude(float a);
	void loadcountyLongitude(float a);
	void loadState(string a);
	void loadcountysupply(float a);
	void loadcountysupply2(float a);
	void loadcountydemand(double a);
	void loadcountyName(string a);
	void loadMode(int m);



	int getPeriod();

	int getTime();
	

	int getCountyID();
	float getcountyLatitude();
	float getcountyLongitude();
	string getState();
	float getcountysupply();
	float getcountysupply2();
	double getcountydemand();
	string getcountyName( );
	int getMode();

};
void county::loadPeriod(int st)
{
	time_period = st;
}

void county::loadTime(int st)
{
	time = st;
}

void county::loadcountyID(int st)
{
	countyID = st;
}

void county::loadcountyLatitude(float en)
{
	countylat = en;
}

void county::loadcountyLongitude(float cap)
{
	countylng = cap;
}

void county::loadState( string sta)
{
	State = sta;
}

void county::loadcountysupply(float spl)
{
	supply = spl;
}
void county::loadcountysupply2(float spl2)
{
	supply2 = spl2;
}
void county ::loadcountydemand( double dmnd)
{
	demand =dmnd;
}

void county::loadcountyName( string CName)
{
	countyName= CName;
}
void county::loadMode(int tmode)
{
	modeId = tmode;
}


int county::getPeriod()
{
	return time_period;
}

int county::getTime()
{
	return time;
}
int county::getCountyID()
{
	return countyID;
}

float county::getcountyLatitude()
{
	return countylat;
}

float county::getcountyLongitude()
{
	return countylng;
}
string county::getState()
{
	return State;
}
float county::getcountysupply()
{
	return supply;
}
float county::getcountysupply2()
{
	return supply2;
}
double county::getcountydemand()
{
	return demand;
}
string county::getcountyName()
{
	return countyName;
}
int county::getMode()
{
	return modeId;

}





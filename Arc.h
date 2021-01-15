
//Developer: Dr. Mohammad Roni
//Copyright 2021, Battelle Energy Alliance, LLC All Rights Reserved
#include<string>
#include<list>
using namespace std;

class Cost {
public:
	float waiting_EV = 0;
	float travel_time = 0;
	float waiting_at_parking_station = 0;
	float waiting_at_busy_hour = 0;
	float charging_time = 0;

};

class Arc:public Cost












































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































{
private:

	int ArcID;
	int trip_id;
	int start;
	int mid1;
	int mid2;
	int end;
	int end1;
	float ArcCost;
	int ArcType;
	int sec_type; 
	string arcOwner;
	list<int>path;
	float lat;
	float lng;
	
	string textId;


public:
	void loadStart(int a);

	void loadTripId(int a);

	void loadMid1(int a);

	void loadMid2(int a);


	void loadEnd(int a);
	void loadEnd1(int a);
	//void loadArcCost(float a);
	void loadArcCost(float a, float b, float c, float d, float e);
	void loadArcID(int ID);
	void loadArcType(int Type);
	void loadSecArcType(int Type);
	void loadarcOwner(string arcO);
	void loadLatitude(float a);
	void loadLongitude(float a);
	void loadTextId(string txt);
		
	int getStart();
	int getTripId();
	int getMid1();

	int getMid2();

	int getEnd();
	int getEnd1();
	float getArcCost();
	int getArcID();
	int getArcType();
	int getSecArcType();

	string getarcOwner();
	float getLatitude();
	float getLongitude();
	string getTextId();
};


void Arc::loadStart(int st)
{
	start = st;
}

void Arc::loadTripId(int st_)
{
	trip_id = st_;
}

void Arc::loadMid1(int md)
{
	mid1 = md;
}

void Arc::loadMid2(int md)
{
	mid2 = md;
}

void Arc::loadEnd(int en)
{
	end = en;
}

void Arc::loadEnd1(int en)
{
	end1 = en;
}

void Arc::loadArcCost(float a, float b, float c, float d, float e)
{
	waiting_EV = a;
	travel_time = b;
	waiting_at_parking_station = c;
	waiting_at_busy_hour = d;
	charging_time = e;
}

void Arc:: loadArcID( int aID)
{ 
	ArcID = aID;
}
void Arc::loadSecArcType(int Type)
{ 
	sec_type = Type;
}

void Arc::loadArcType(int Type)
{
	ArcType = Type;
}

void Arc:: loadarcOwner(string arcOw)
{ 
	arcOwner = arcOw;
}
void Arc::loadLatitude(float en)
{
	lat = en;
}

void Arc::loadLongitude(float cap)
{
	lng = cap;
}

void Arc::loadTextId(string txt1)
{
	textId = txt1;
}




int Arc::getStart()
{
	return start;
}
int Arc::getTripId()
{
	return trip_id;
}
int Arc::getMid1()
{
	return mid1;
}

int Arc::getMid2()
{
	return mid2;
}


int Arc::getEnd()
{
	return end;
}

int Arc::getEnd1()
{
	return end1;
}


float Arc::getArcCost()
{
	return waiting_EV + travel_time + waiting_at_parking_station + waiting_at_busy_hour + charging_time;
}

int Arc::getArcID()
{
	return ArcID;
}
int Arc::getArcType()
{
	return ArcType;
}

int Arc::getSecArcType()
{
	return sec_type;
}

string Arc::getarcOwner()
{
	return arcOwner;
}
float Arc::getLatitude()
{
	return lat;
}

float Arc::getLongitude()
{
	return lng;
}

string Arc::getTextId()
{
	return textId;
}




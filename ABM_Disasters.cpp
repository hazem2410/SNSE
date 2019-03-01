//============================================================================
// Name        : SNSE
// Author      : Hazem Krichene
// Description : Simulation of Negative Shocks on Economy
//============================================================================

/*
 * Declaration of all libraries used
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <time.h>
#include <algorithm>
#include <iterator>
#include <set>
#include <cmath>
#include <numeric>
#include <ctime>
#include <chrono>
#include <limits>
#include <random>
#include <typeinfo>

using namespace std;

/*
 * Declaration of variables and data structure used in the simulator
*/

/*
 * The number of day of utilization of products by firms i (n_i).
 * The number of days to adjust the inventory size (tau)
*/

/*
 * Model parameters
*/
int n;
int tau;
int t;
int disaster;
int HelpFirms;
double GammaMin;
double GammaMax;
double NumberDamagedFirms;
double DamageMagnitude;
double LimitSolvencyRatio;
int StartRecover; //Number of days before starting recovery
int LoanMaturity; //The maturity of all loans in number of days
int LTLoanMaturity;
int WithPayment; //Model with payment from customers to suppliers or not
int ShortLoans; //Model with short term loans given by banks to firms or not
int LTLoansModel; //Model with short term loans given by banks to firms or not
int ShortInterestRate; //Model with 0 short term interest rate or not
int LimitToDefault; //Number of day without payment before declaring loan default
int BankRiskManager; //If 1 the bank is a risk manager; 0: the bank has no risk policy in case of disaster; Give priority of the economic recovery
int location_of_disaster; //35: Tokyo, 52: Kyoto, 53: Osaka, 54: Hyogo
int sector_of_disaster ;
int community_of_disaster;
int DisasterScenario; //0: Random Selection from the whole network; 1: Random selection from one prefecture ; 2: Random selection from one sector; 3: Random selection from one community
int SimTime = 365; //Size of the simulation; e.g. 365 days: 1 year.
int GlobalSim =100; //Number of trial with different random seeds. If supercomputer is used, it is to make it parallel: simulations are independent.

unordered_map<int, int > DaysH;

/*
 * two unordered maps for the weighted production network
 * 1- The out links of each firm: the list of customers j of each firm i, where each customer j is defined by his attributes: sector, location, number of employee, weight (A_ji)
 * 2- The in links of each firm: the list of suppliers j of each firm i, where each supplier j is defined by his attributes: sector, location, number of employee, weight (A_ij)
*/

unordered_map<int, unordered_map<int, vector<double> > > OutPutFirmHoH; //Aji

unordered_map<int, unordered_map<int, vector<double> > > InPutFirmHoH; //Aij

/*
 * Each firm i produces at each step Y_{i,t}.
 * The effective realized production is saved in an unordered map.
*/

unordered_map<int, double > ProductionIniH; //Pini in the paper notation

unordered_map<int, double > CurrentProductionH; //Pact in the paper notation

/*
 * The final consumption (goods sold to households) is kept constant as supposed in Inoue and Todo (2018): C_i
*/

unordered_map<int, double > cVectorH; //Ci

/*
 * At each step t, each firm i realizes a list of orders from its suppliers j denoted by O_{ij,t}^{*} as in Inoue and Todo (2018)
 * All orders are saved in an unordered map
 * All desired orders are also saved for each firm i (total desires addressed to all of each suppliers)
*/

unordered_map<int, unordered_map<int, double > > rOrdersHoH; //O_{ij,t}^{*}

unordered_map<int, double > dOrdersH;

unordered_map<int, double > GROrdersH; //Save the sum of O_{ij,t}^{*} for each firm i

/*
 * Each supplier i receives desired orders from its client.
 * The sum of the desired orders is the received demand.
 * Another data structure for the received demand firm by firm: for each supplier, we note all orders client by client.
*/

unordered_map<int, double > rcDemandVectorH; //D_{i,t}

unordered_map<int, unordered_map <int, double > > rcDemandFirmLevelH; //D_{ij,t}

/*
 * Each supplier i realizes based on its production the desired orders received from its client.
 * The sum of the realized orders is the realized demand.
*/

unordered_map<int, double> rzDemandVectorH; //D_{i,t}^{*}

/*
 * Each firm i holds inventory from different of its suppliers j.
 * The firm level inventory evolution is saved in an unordered map.
 * Inventory is measured by sector for the production.
 * Inventory evolves by sector too, where values are saved in another unordered map.
 * For seek of utility, we use AijSectorHoH as the total of initial input for each firm by sector
*/

unordered_map<int, unordered_map<int, vector<double> > > fInventoryHoH; //Sij

unordered_map<int, unordered_map<int, double> > AccfInventoryHoH; //employed during trading only to calculate the added new inventory

unordered_map<int, unordered_map<int, double > > sInventoryHoH; //Sum_{sector}(Sij)

unordered_map<int, unordered_map<int, double > > AccsInventoryHoH; //employed during trading only to calculate the added new inventory

unordered_map<int, unordered_map<int, double > > AijSectorHoH; //Sum_{sector}(Aij)

unordered_map<int, double > fUsedInventoryH; //used inventory for production

/*
 * Each firm has its own value added as an economic performance indicator
*/

unordered_map<int, vector<double> > ValueAddedVectorH;

/*
 * List of damaged firms randomly selected
 * List of all firms
*/

set <int> DamagedFirmsH; //List of damaged firms
set <int> ListSupplier; //List of Suppliers
set <int> ListCustomer; //List of customers
set <int> Firms; //List of all firms
set <int> InactiveFirms; //List of firms which have no Pini no Ci; They don't produce, they only submit constant orders equal to initial input
unordered_map<int,int> ListofFirmsH; //Firm ID, sector (190 sector of IO table)
/*
 * The factor Delta of loosing production capacity
*/

unordered_map<int, double > DeltaH;
unordered_map<int, double > RecoveryH;
unordered_map<int, double > LTLoansH;
unordered_map<int, double > ProfitToSalesH;

vector< double > GDP;
double ValueGDP;
unordered_map<int, double > Total_Output;
unordered_map<int, double > Total_Input;

/*
 * Firm BS
 * Firm - Bank network
*/

unordered_map<int, vector < double > > FirmBSH; // ID Firm; Deposit; OA; Loan; Equity; OL
unordered_map<int, unordered_map<int, vector < double > > > FirmBankHoH; //ID Firm; ID Bank; Loan; Deposit

/*
 * CurrentLoansHoH: This structure contains the whole current loans given to Firm i by Bank j
 * LoanKeyHoH: This structure contains the keys for the unordered map CurrentLoansHoH
 * LoanFlagH: unordered map containing a bool flag indicating if the firm got the loan or not
 * PayByLoanHoH: unordered map containing the amount of loan taken by the firm from each of its banks. These amounts are used first to pay suppliers
 * NPLHoH: unordered map containing the non-performing loans for each firm with its banks (bank by bank)
*/

unordered_map<int, unordered_map<int, unordered_map<int, vector < double > > > > CurrentLoansHoH; //The borrowed capital; The interest rate; The periodic amortization; the current period
unordered_map<int, unordered_map<int, int > > LoanKeyHoH;
unordered_map<int, int > LoanFlagH; //Values: 2 don't need loans; 1: Need and got loans; 0: Need but didn't get loans
unordered_map<int, unordered_map<int, double > > PayByLoanHoH;
unordered_map<int, unordered_map<int, double > > NPLHoH;
unordered_map<int, int > FirmsNoLoansH; //List of firms which could not take loans due to bank risk policy; How many time they were rejected: Value 0 is the default.

/*
 * Save financial statistics
*/
vector< double > NPLH;
double GNPL;

vector< double > DepositH;
double GDeposit;

vector< double > LoanH;
double GLoan;

vector< double > EquityH;
double GEquity;

vector< double > GvtSupportH;
double GvtSupport;

vector< double > RateNPLH;
double RateNPL;
double CountNPL;

unordered_map<int, vector< double > > NetworkStatisticsHoH;

unordered_map<int, vector< double>> BankNPLH;

unordered_map<int, vector<int> > BankDegreeH;

unordered_map<int, vector<int> > BankLoanDepositH;

unordered_map<int, int > GeographicLocationH;

unordered_map<int, int > SectorH;

unordered_map<int, int > CommunityH;

/*
 * Define the profit of the firm:
 * 1- ExpensesH: This unordered map is update each step in the payment function
 * 2- For the revenue, we can use the rzDemandVectorH
*/

unordered_map<int, double > ExpensesH;

unordered_map<int, vector < double >> SimGDPH;
unordered_map<int, vector<double> > SimLoansH;
unordered_map<int, vector<double> > SimNPLH;
unordered_map<int, vector<double> > SimNPLRateH;
unordered_map<int, vector<double> > SimDepositH;

/*
 * All functions and procedure of the artificial economy
*/

/*This function is used to calculate the minimum of unordered map*/
bool compare(std::pair<int ,double> i, pair<int, double> j) {
  return i.second < j.second;
}

const double epsilon = 1e-10;
inline bool almost_equal(double x, double y)
{
  return std::fabs(x - y) < epsilon;
}

const double epsilon2 = 1e-5;
inline bool almost_equal2(double x, double y)
{
  return std::fabs(x - y) < epsilon2;
}

const double epsilon3 = 1e-20;
inline bool almost_equal3(double x, double y)
{
  return std::fabs(x - y) < epsilon3;
}

typedef long unsigned int luint;
luint poisson(luint lambda) {
    double L = exp(-double(lambda));
    luint k = 0;
    double p = 1;
    do {
        k++;
	double rNum=((double)(rand()/(double)RAND_MAX));
        p *= rNum;
    } while( p > L);
    return (k-1);
}

double Scale(double min, double max, double x)
{
    double number;
    number = (GammaMax - GammaMin)*(x - min)/(max - min) + GammaMin;
    return number;
}

void Clearing()
{
	unordered_map<int, int >().swap(DaysH);
	unordered_map<int, unordered_map<int, vector<double> > >().swap(OutPutFirmHoH);
	unordered_map<int, unordered_map<int, vector<double> > >().swap(InPutFirmHoH);
	unordered_map<int, double >().swap(ProductionIniH);
	unordered_map<int, double >().swap(CurrentProductionH);
	unordered_map<int, double >().swap(cVectorH);
	unordered_map<int, unordered_map<int, double > >().swap(rOrdersHoH);
	unordered_map<int, double >().swap(dOrdersH);
	unordered_map<int, double >().swap(GROrdersH);
	unordered_map<int, double >().swap(rcDemandVectorH);
	unordered_map<int, unordered_map <int, double > >().swap(rcDemandFirmLevelH);
	unordered_map<int, double>().swap(rzDemandVectorH);
	unordered_map<int, unordered_map<int, vector<double> > >().swap(fInventoryHoH);
	unordered_map<int, unordered_map<int, double> >().swap(AccfInventoryHoH);
	unordered_map<int, unordered_map<int, double > >().swap(sInventoryHoH);
	unordered_map<int, unordered_map<int, double > >().swap(AccsInventoryHoH);
	unordered_map<int, unordered_map<int, double > >().swap(AijSectorHoH);
	unordered_map<int, double >().swap(fUsedInventoryH);
	unordered_map<int, vector<double> >().swap(ValueAddedVectorH);
	set <int>().swap(DamagedFirmsH);
	set <int>().swap(ListSupplier);
	set <int>().swap(ListCustomer);
	set <int>().swap(Firms);
	set <int>().swap(InactiveFirms);
	unordered_map<int,int>().swap(ListofFirmsH);
	unordered_map<int, double >().swap(DeltaH);
	unordered_map<int, double >().swap(RecoveryH);
	unordered_map<int, double >().swap(LTLoansH);
	unordered_map<int, double >().swap(ProfitToSalesH);
	vector< double >().swap(GDP);
	unordered_map<int, double >().swap(Total_Output);
	unordered_map<int, double >().swap(Total_Input);
	unordered_map<int, vector < double > >().swap(FirmBSH);
	unordered_map<int, unordered_map<int, vector < double > > >().swap(FirmBankHoH);
	unordered_map<int, unordered_map<int, unordered_map<int, vector < double > > > >().swap(CurrentLoansHoH);
	unordered_map<int, unordered_map<int, int > >().swap(LoanKeyHoH);
	unordered_map<int, int >().swap(LoanFlagH);
	unordered_map<int, unordered_map<int, double > >().swap(PayByLoanHoH);
	unordered_map<int, unordered_map<int, double > >().swap(NPLHoH);
	vector< double >().swap(NPLH);
	vector< double >().swap(DepositH);
	vector< double >().swap(LoanH);
	vector< double >().swap(EquityH);
	vector< double >().swap(RateNPLH);
	unordered_map<int, vector< double > >().swap(NetworkStatisticsHoH);
	unordered_map<int, vector< double>>().swap(BankNPLH);
	unordered_map<int, vector<int> >().swap(BankDegreeH);
	unordered_map<int, vector<int> >().swap(BankLoanDepositH);
	unordered_map<int, int >().swap(GeographicLocationH);
	unordered_map<int, int >().swap(SectorH);
	unordered_map<int, int >().swap(CommunityH);
	unordered_map<int, double >().swap(ExpensesH);
	unordered_map<int, int >().swap(FirmsNoLoansH);
}

void Initial_Data()
{
	n = 15;
	tau = 6;
	t = 0;
	disaster = 0;
	HelpFirms = 1;
	GammaMin = 0.001;
	GammaMax = 0.004;
	NumberDamagedFirms = 0.03;
	DamageMagnitude = 0.512040958832949;
	LimitSolvencyRatio = 0.034;
	StartRecover = 5; //Number of days before starting recovery
	LoanMaturity = 53; //The maturity of all loans in number of days
	LTLoanMaturity = 399;
	WithPayment = 1; //Model with payment from customers to suppliers or not
	ShortLoans = 1; //Model with short term loans given by banks to firms or not
	LTLoansModel = 1; //Model with short term loans given by banks to firms or not
	ShortInterestRate = 1; //Model with 0 short term interest rate or not
	LimitToDefault = 10; //Number of day without payment before declaring loan default
	BankRiskManager = 1; //If 1 the bank is a risk manager; 0: the bank has no risk policy in case of disaster; Give priority of the economic recovery
	location_of_disaster = 35; //35: Tokyo, 52: Kyoto, 53: Osaka, 54: Hyogo; 48: Aichi
	sector_of_disaster = 3111;
	community_of_disaster = 2;
	DisasterScenario = 0; //0: Random Selection from the whole network; 1: Random selection from one prefecture ; 2: Random selection from one sector; 3: Random selection from one community

	GNPL = 0;
	GDeposit = 0;
	GLoan = 0;
	GEquity = 0;
    RateNPL = 0;
	CountNPL = 0;
	GvtSupport = 0;
	/*
	 * Upload network data
	 * Initialize all economic variables: price, demand, inventory...
	*/
	ifstream OutPut1,OutPut2,CVector, ListFirms, Production, FirmBS, FirmBank,NetStat,Location, Sector,Profit,ProfitTax;
    /*
     * File structure: supplier - customer - weight (Aij) - sector of supplier - location of supplier- sector of customer - location of customer - final goods (consumption Ci of supplier) - final goods (consumption Ci of customer)
     * This part upload data from the file InOutput.dat (OutPutFirmHoH and InPutFirmHoH)
     * Initialization of the price: All firms have the same initial price: P0 = 1
     * Upload consumption goods data: cVectorH
     * Initialize the Inventory at firm level: suppose that the inventory at t = 0 is the amount of initial trade Aij.
    */


	OutPut1.open("Data/ToyTable1.txt");
	OutPut2.open("Data/ToyTable1.txt");
	CVector.open("Data/CToy1.txt");
	ListFirms.open("Data/ToyKJ.txt");
	Production.open("Data/ToyPini.txt");
	FirmBS.open("Data/BSToy.txt");
	FirmBank.open("Data/FBToy.txt");
	NetStat.open("Data/FirmClusters.txt");
    Location.open("Data/toyGeography.txt");
	Sector.open("Data/toySector.txt");
	Profit.open("Data/toyProfitToSales.txt");

	std::string opline, cline, kjline, prodline, bsline, fbline, statline, locline, secline, profitline,taxline;

	while(getline(NetStat,statline))
	{
		std::string ID;
		std::string clusters;
		std::string knn;

		istringstream stat(statline);
		stat >> ID;
		stat >> clusters;
		stat >> knn;

		NetworkStatisticsHoH[std::stoi(ID)].push_back(std::stod(clusters));
		NetworkStatisticsHoH[std::stoi(ID)].push_back(std::stod(knn));
	}
	NetStat.close();

	while(getline(Profit,profitline))
	{
		std::string ID;
		std::string ratio;

		istringstream stat(profitline);
		stat >> ID;
		stat >> ratio;

		ProfitToSalesH[std::stoi(ID)] = std::stod(ratio);
	}
	Profit.close();

	while(getline(Location,locline))
	{
		std::string ID;
		std::string Pref;

		istringstream stat(locline);
		stat >> ID;
		stat >> Pref;
		GeographicLocationH[std::stoi(ID)]=std::stoi(Pref);
	}
	Location.close();

	while(getline(Sector,secline))
	{
		std::string ID;
		std::string Index;
		std::string Sector;
		std::string New_Index;
		std::string New_Sector;


		istringstream stat(secline);
		stat >> ID;
		stat >> Index;
		stat >> Sector;
		stat >> New_Index;
		stat >> New_Sector;
		SectorH[std::stoi(ID)]=std::stoi(New_Index);
	}
	Sector.close();

	while(getline(FirmBank,fbline))
	{
		std::string FID;
		std::string BID;
		std::string loan;
		std::string deposit;

		istringstream fb(fbline);
		fb >> FID;
		fb >> BID;
		fb >> loan;
		fb >> deposit;

		FirmBankHoH[std::stoi(FID)][std::stoi(BID)].push_back(std::stod(loan));
		FirmBankHoH[std::stoi(FID)][std::stoi(BID)].push_back(std::stod(deposit));
		LoanKeyHoH[std::stoi(FID)][std::stoi(BID)] = 0;
		BankNPLH[std::stoi(BID)] = {0,0};
		BankLoanDepositH[std::stoi(BID)] = {0,0};
		BankDegreeH[std::stoi(BID)].push_back(std::stoi(FID));
	}
	FirmBank.close();

	while(getline(FirmBS,bsline))
	{
		std::string ID;
		std::string deposit;
		std::string OA;
		std::string loan;
		std::string equity;
		std::string OL;

		istringstream bs(bsline);
		bs >> ID;
		bs >> deposit;
		bs >> OA;
		bs >> loan;
		bs >> equity;
		bs >> OL;

		FirmBSH[std::stoi(ID)].push_back(std::stod(deposit));
		FirmBSH[std::stoi(ID)].push_back(std::stod(OA));
		FirmBSH[std::stoi(ID)].push_back(std::stod(loan));
		FirmBSH[std::stoi(ID)].push_back(std::stod(equity));
		FirmBSH[std::stoi(ID)].push_back(std::stod(OL));

		FirmsNoLoansH[std::stoi(ID)] = 0; //Loan rejection data

	}

	FirmBS.close();
	while(getline(OutPut1,opline))
	{
		std::string supplier;
		std::string customer;
		std::string weight;

		istringstream op(opline);
		op >> supplier;
		op >> customer;
		op >> weight;

		ListSupplier.insert(std::stoi(supplier));
		ListCustomer.insert(std::stoi(customer));
	}

	OutPut1.close();

	while (getline(CVector,cline))
	{
		std::string ID;
		std::string Ci;

		istringstream cl(cline);
		cl >> ID;
		cl >> Ci;
		if (((ListSupplier.find(std::stoi(ID)) != ListSupplier.end())==1) or ((ListCustomer.find(std::stoi(ID)) != ListCustomer.end())==1))
		{cVectorH[std::stoi(ID)] = std::stod(Ci);}
	}

	CVector.close();

	while (getline(Production,prodline))
	{
		std::string ID;
		std::string Pini;

		istringstream prod(prodline);
		prod >> ID;
		prod >> Pini;
		if (((cVectorH.find(std::stoi(ID)) != cVectorH.end())==1))
		{
			ProductionIniH[std::stoi(ID)]= std::stod(Pini);
			rzDemandVectorH[std::stoi(ID)]= std::stod(Pini);
		}
	}

	Production.close();

	while (getline(ListFirms,kjline))
	{
		std::string ID, sector;

		istringstream kj(kjline);
		kj >> ID;
		kj >> sector;
		if (((cVectorH.find(std::stoi(ID)) != cVectorH.end())==1))
		{ListofFirmsH[std::stoi(ID)] = std::stoi(sector);}
	}

	ListFirms.close();

	while(getline(OutPut2,opline))
	{
		std::string supplier;
		std::string customer;
		std::string weight;

		istringstream op(opline);
		op >> supplier;
		op >> customer;
		op >> weight;

		OutPutFirmHoH[std::stoi(supplier)][std::stoi(customer)].push_back(std::stod(weight));
	    OutPutFirmHoH[std::stoi(supplier)][std::stoi(customer)].push_back(ListofFirmsH[std::stoi(customer)]);
	    OutPutFirmHoH[std::stoi(supplier)][std::stoi(customer)].push_back(1/std::stod(weight));

	    InPutFirmHoH[std::stoi(customer)][std::stoi(supplier)].push_back(std::stod(weight));
	    InPutFirmHoH[std::stoi(customer)][std::stoi(supplier)].push_back(ListofFirmsH[std::stoi(supplier)]);
	    InPutFirmHoH[std::stoi(customer)][std::stoi(supplier)].push_back(1/std::stod(weight));

	    int n_cust = 0, n_supp = 0;

	    while(n_cust == 0){n_cust=poisson(n);}
	    while(n_supp == 0){n_supp=poisson(n);}

	    DaysH[std::stoi(customer)] = n_cust;
	    DaysH[std::stoi(supplier)] = n_supp;

	    Firms.insert(std::stoi(supplier));
	    Firms.insert(std::stoi(customer));

	    DeltaH[std::stoi(supplier)] = 0;
	    DeltaH[std::stoi(customer)] = 0;

	    if (((ProductionIniH.find(std::stoi(supplier)) != ProductionIniH.end())==0))
	    {
		    InactiveFirms.insert(std::stoi(supplier));
	    }
	    if (((ProductionIniH.find(std::stoi(customer)) != ProductionIniH.end())==0))
	    {
		    InactiveFirms.insert(std::stoi(customer));
	    }
	}
	OutPut2.close();

	for(unordered_map<int, unordered_map<int, vector<double> > >::iterator itS=OutPutFirmHoH.begin(); itS!=OutPutFirmHoH.end();itS++)
	{
		double output = 0;
		for(unordered_map<int, vector<double> >::iterator itC=OutPutFirmHoH[(*itS).first].begin(); itC!=OutPutFirmHoH[(*itS).first].end();itC++)
		{
			output += OutPutFirmHoH[(*itS).first][(*itC).first][0];
		}
		Total_Output[(*itS).first] = output+cVectorH[(*itS).first];
	}

	for(unordered_map<int, unordered_map<int, vector<double> > >::iterator itC=InPutFirmHoH.begin(); itC!=InPutFirmHoH.end();itC++)
	{
		double input = 0;
		for(unordered_map<int, vector<double> >::iterator itS=InPutFirmHoH[(*itC).first].begin(); itS!=InPutFirmHoH[(*itC).first].end();itS++)
		{
			input += InPutFirmHoH[(*itC).first][(*itS).first][0];
			fInventoryHoH[(*itC).first][(*itS).first].push_back((double)DaysH[(*itC).first]*InPutFirmHoH[(*itC).first][(*itS).first][0]);
			fInventoryHoH[(*itC).first][(*itS).first].push_back(ListofFirmsH[(*itS).first]);

			sInventoryHoH[(*itC).first][ListofFirmsH[(*itS).first]] += (double)DaysH[(*itC).first]*InPutFirmHoH[(*itC).first][(*itS).first][0];
			AijSectorHoH[(*itC).first][ListofFirmsH[(*itS).first]] += InPutFirmHoH[(*itC).first][(*itS).first][0];
		}

		Total_Input[(*itC).first] = input;
		FirmBSH[(*itC).first][0]=input;
		FirmBSH[(*itC).first][3]=FirmBSH[(*itC).first][0]+FirmBSH[(*itC).first][1]-FirmBSH[(*itC).first][2]-FirmBSH[(*itC).first][4];
	}

}

void Desired_Goods()
{
	/*
	 * 1- Each firm defines its desired orders to each of its suppliers.
	 * 2- Each supplier collects all orders and defines its received demand in rcDemandHoH
	*/
	/*First loop is to initialize at each step the received demand vector of all firms: Initial values is equal to the final consumption*/
	for(unordered_map<int, double >::iterator itr=cVectorH.begin(); itr!=cVectorH.end();itr++)
	{
		rcDemandVectorH[(*itr).first] = cVectorH[(*itr).first];
		dOrdersH[(*itr).first] = 0;
		GROrdersH[(*itr).first] = 0;
		ExpensesH[(*itr).first] = 0;
	}

    double order_ij, quantity;
    for (unordered_map<int, unordered_map<int, vector<double> > >::iterator itC=InPutFirmHoH.begin(); itC!=InPutFirmHoH.end();itC++)
    {
        for (unordered_map<int, vector<double> >::iterator itS=InPutFirmHoH[(*itC).first].begin(); itS!=InPutFirmHoH[(*itC).first].end();itS++)
        {
            if ((InactiveFirms.find((*itC).first) != InactiveFirms.end())==0)
            {
                quantity = InPutFirmHoH[(*itC).first][(*itS).first][0]*rzDemandVectorH[(*itC).first]/ProductionIniH[(*itC).first];
                if(almost_equal((double)DaysH[(*itC).first]*quantity , fInventoryHoH[(*itC).first][(*itS).first][0]))
                {
                    order_ij = quantity;
                }
                else if ((double)DaysH[(*itC).first]*quantity < fInventoryHoH[(*itC).first][(*itS).first][0])
                {
                     order_ij = quantity;
                }
                else if ((double)DaysH[(*itC).first]*quantity > fInventoryHoH[(*itC).first][(*itS).first][0])
                {
                    order_ij = quantity +  ((double)DaysH[(*itC).first]*quantity - fInventoryHoH[(*itC).first][(*itS).first][0])/(double)tau;
                }
                if (order_ij < 0.0){order_ij = 0;}
            }
            else
            {
            	order_ij = InPutFirmHoH[(*itC).first][(*itS).first][0];
            }
           	rcDemandVectorH[(*itS).first]+=order_ij;
           	rcDemandFirmLevelH[(*itS).first][(*itC).first] = order_ij;
           	dOrdersH[(*itC).first]+=order_ij;
        }
    }
}

void DamagedFirms()
{
	/*
	 * Random selection of X% of firms as damaged
	 * Damaged firms are saved in the set DamagedFirmsH
	*/
	int total_size = Firms.size();
	double min = 0.0;
	double max = 0.0;

	while(DamagedFirmsH.size() < NumberDamagedFirms*total_size)
	{
		int random_damage = rand() % Firms.size();
		auto itr_damage = std::next(std::begin(Firms), random_damage);
		if(DisasterScenario == 0)
		{
			DamagedFirmsH.insert(*itr_damage);
			DeltaH[*itr_damage] = DamageMagnitude;
			Firms.erase(*itr_damage);
			double recover = FirmBSH[*itr_damage][0]/(ProductionIniH[*itr_damage]*DamageMagnitude);
			LTLoansH[*itr_damage] = ProductionIniH[*itr_damage]*DamageMagnitude;
			if (min < recover){min = recover;}
			if (max > recover){max = recover;}
		}
		else if(DisasterScenario == 1)
		{
			if(GeographicLocationH[*itr_damage] == location_of_disaster)
			{
				DamagedFirmsH.insert(*itr_damage);
				DeltaH[*itr_damage] = DamageMagnitude;
				Firms.erase(*itr_damage);
				double recover = FirmBSH[*itr_damage][0]/(ProductionIniH[*itr_damage]*DamageMagnitude);
				LTLoansH[*itr_damage] = ProductionIniH[*itr_damage]*DamageMagnitude;
				if (min < recover){min = recover;}
				if (max > recover){max = recover;}
			}
		}

		else if (DisasterScenario == 2)
		{
			if(SectorH[*itr_damage] == sector_of_disaster)
			{
				DamagedFirmsH.insert(*itr_damage);
				DeltaH[*itr_damage] = DamageMagnitude;
				Firms.erase(*itr_damage);
				double recover = FirmBSH[*itr_damage][0]/(ProductionIniH[*itr_damage]*DamageMagnitude);
				LTLoansH[*itr_damage] = ProductionIniH[*itr_damage]*DamageMagnitude;
				if (min < recover){min = recover;}
				if (max > recover){max = recover;}
			}
		}
	}
	for(set<int>::iterator it = DamagedFirmsH.begin(); it != DamagedFirmsH.end(); it++)
	{
		double recover = FirmBSH[*it][0]/(ProductionIniH[*it]*DamageMagnitude);
		RecoveryH[*it] = Scale(min,max,recover);
	}
}


void ProductionInoue18(int ID)
{
	/*
	 * The production function for one firm ID
	 * This production function is a reproduction of Inoue and Todo (2018)
	*/
    double Pcap;
    vector<double> Pproi;
    double minPproi;
    double Pmax;

    /* Calculate the capacity of production after disaster Equation (4) */
	if ((DamagedFirmsH.find(ID) != DamagedFirmsH.end())==1)
	{
		if (t < StartRecover){Pcap = (1-DeltaH[ID])*ProductionIniH[ID];}
		else
		{
			if (DeltaH[ID]!=0.0){DeltaH[ID] = (1-RecoveryH[ID])*DeltaH[ID];}
			Pcap = (1-DeltaH[ID])*ProductionIniH[ID];
		}
	}
	else
	{
		Pcap = ProductionIniH[ID];
	}

    /*Calculate the production under inventory constraint Equation (7)*/
	if ((sInventoryHoH.find(ID) != sInventoryHoH.end())==0){Pproi.push_back(ProductionIniH[ID]);}
	else
	{
		for (unordered_map<int, double>::iterator itr=sInventoryHoH[ID].begin(); itr!=sInventoryHoH[ID].end();itr++)
		{
			Pproi.push_back(ProductionIniH[ID]*sInventoryHoH[ID][(*itr).first]/AijSectorHoH[ID][(*itr).first]);
		}
	}

	/*Calculate the real actual production of the firm equations (8 and 9)*/
	minPproi = std::min_element(Pproi.begin(), Pproi.end())[0];
	Pmax = std::min(Pcap,minPproi);
	CurrentProductionH[ID]= std::min(Pmax,rcDemandVectorH[ID]);
}

void Rationing(int ID)
{
	/*
	 * 1- Each supplier ID decided about his production level.
	 * 2- If the production is less than the received demand: Rationing policy as in Inoue and Todo (2018).
	 * 3- Fill in these unordered maps: rOrdersHoH, rzDemandHoH, fInventoryVectorH, ValueAddedVectorH.
	*/
	unordered_map<int, double > RatioOrdersH; // customer; pre-to-post disaster ratio
	unordered_map<int, double > TentativeOrdersH; // customer; new tentative orders based on the rationing policy
	unordered_map<int, double > TentativeOrdersFirstH;
	rzDemandVectorH[ID]=0.0; // Initialize the realized demand of the supplier
    /*Calculate the pre-to-disaster ratio of orders of all clients of firm ID*/
	for (unordered_map<int, double>::iterator itr=rcDemandFirmLevelH[ID].begin(); itr!=rcDemandFirmLevelH[ID].end();itr++)
	{
		RatioOrdersH[(*itr).first] = rcDemandFirmLevelH[ID][(*itr).first]*OutPutFirmHoH[ID][(*itr).first][2];
	    rOrdersHoH[(*itr).first][ID] = 0; /*Initialize the realized orders variable*/
	    TentativeOrdersFirstH[(*itr).first] = RatioOrdersH[(*itr).first]*OutPutFirmHoH[ID][(*itr).first][0];
	}

	TentativeOrdersFirstH[0] =cVectorH[ID]; //neeeeew

	RatioOrdersH[0] = 1; //Ratio of the household consumption which is always constant.

	std::pair<int, double> min_ratio = *min_element(RatioOrdersH.begin(), RatioOrdersH.end(), compare);

	TentativeOrdersH[0] = min_ratio.second*cVectorH[ID]; //Tentative order for the household

	for (unordered_map<int, double>::iterator itr=rcDemandFirmLevelH[ID].begin(); itr!=rcDemandFirmLevelH[ID].end();itr++)
	{
		TentativeOrdersH[(*itr).first] = min_ratio.second*OutPutFirmHoH[ID][(*itr).first][0];
	}

	double firm_production = CurrentProductionH[ID];
	
	while((almost_equal(firm_production,0.0) == 0) and (firm_production > 0.0) and (TentativeOrdersH.size()>1)) /*While firm ID has a production, it continues satisfying orders*/
	{
		if(disaster == 0)
		{
			for (unordered_map<int, double>::iterator itr=TentativeOrdersH.begin(); itr!=TentativeOrdersH.end();itr++)
			{
				if ((*itr).first != 0)
				{
					rOrdersHoH[(*itr).first][ID]+=OutPutFirmHoH[ID][(*itr).first][0];
					AccfInventoryHoH[(*itr).first][ID] +=OutPutFirmHoH[ID][(*itr).first][0];
					AccsInventoryHoH[(*itr).first][InPutFirmHoH[(*itr).first][ID][1]]+=OutPutFirmHoH[ID][(*itr).first][0];
					rzDemandVectorH[ID]+=OutPutFirmHoH[ID][(*itr).first][0];
					GROrdersH[(*itr).first]+=OutPutFirmHoH[ID][(*itr).first][0];
				}
				else{rzDemandVectorH[ID]+=cVectorH[ID];}
				firm_production=0.0;
			}
		}

		double sum_orders_first = 0;
		for (unordered_map<int, double>::iterator itr=TentativeOrdersFirstH.begin(); itr!=TentativeOrdersFirstH.end();itr++)
		{
			sum_orders_first+=TentativeOrdersFirstH[(*itr).first];
		}

		if((firm_production >sum_orders_first) || (almost_equal(firm_production,sum_orders_first)==1))
		{
			for (unordered_map<int, double>::iterator itr=TentativeOrdersFirstH.begin(); itr!=TentativeOrdersFirstH.end();itr++)
			{
				if ((*itr).first != 0)
				{
					rOrdersHoH[(*itr).first][ID]+= TentativeOrdersFirstH[(*itr).first];
					AccfInventoryHoH[(*itr).first][ID] +=TentativeOrdersFirstH[(*itr).first];
					AccsInventoryHoH[(*itr).first][InPutFirmHoH[(*itr).first][ID][1]]+=TentativeOrdersFirstH[(*itr).first];
					GROrdersH[(*itr).first]+= TentativeOrdersFirstH[(*itr).first];
				}
				rzDemandVectorH[ID]+=TentativeOrdersFirstH[(*itr).first];
			}

			firm_production = 0.0;
		}

		double sum_orders = 0;
		for (unordered_map<int, double>::iterator itr=TentativeOrdersH.begin(); itr!=TentativeOrdersH.end();itr++)
		{
			sum_orders+=TentativeOrdersH[(*itr).first];
		}

		if (sum_orders > firm_production)
		{

			double Initial_OutPut = 0;
			for (unordered_map<int, double>::iterator itr=TentativeOrdersH.begin(); itr!=TentativeOrdersH.end();itr++)
			{
				if ((*itr).first != 0){Initial_OutPut+=OutPutFirmHoH[ID][(*itr).first][0];}
				else{Initial_OutPut+=cVectorH[ID];}
			}
			double effective_ratio = firm_production/Initial_OutPut;
			for (unordered_map<int, double>::iterator itr=TentativeOrdersH.begin(); itr!=TentativeOrdersH.end();itr++)
			{
				if ((*itr).first != 0)
				{
					rOrdersHoH[(*itr).first][ID]+=effective_ratio*OutPutFirmHoH[ID][(*itr).first][0];
					AccfInventoryHoH[(*itr).first][ID] +=effective_ratio*OutPutFirmHoH[ID][(*itr).first][0];
					AccsInventoryHoH[(*itr).first][InPutFirmHoH[(*itr).first][ID][1]]+=effective_ratio*OutPutFirmHoH[ID][(*itr).first][0];
					rzDemandVectorH[ID]+=effective_ratio*OutPutFirmHoH[ID][(*itr).first][0];
					GROrdersH[(*itr).first]+=effective_ratio*OutPutFirmHoH[ID][(*itr).first][0];
				}
				else{rzDemandVectorH[ID]+=effective_ratio*cVectorH[ID];}
			}
			firm_production=0.0;
		}
		else
		{
			for (unordered_map<int, double>::iterator itr=TentativeOrdersH.begin(); itr!=TentativeOrdersH.end();itr++)
			{
				if ((*itr).first != 0)
				{
					rOrdersHoH[(*itr).first][ID]+= TentativeOrdersH[(*itr).first];
					AccfInventoryHoH[(*itr).first][ID] +=TentativeOrdersH[(*itr).first];
					AccsInventoryHoH[(*itr).first][InPutFirmHoH[(*itr).first][ID][1]]+=TentativeOrdersH[(*itr).first];
					GROrdersH[(*itr).first]+= TentativeOrdersH[(*itr).first];
				}
				firm_production-=TentativeOrdersH[(*itr).first];
				rzDemandVectorH[ID]+=TentativeOrdersH[(*itr).first];
				TentativeOrdersFirstH[(*itr).first]-=TentativeOrdersH[(*itr).first];
			}
			TentativeOrdersH.erase(min_ratio.first);
			RatioOrdersH.erase(min_ratio.first);
			TentativeOrdersFirstH.erase(min_ratio.first);
			for (unordered_map<int, double>::iterator itr=RatioOrdersH.begin(); itr!=RatioOrdersH.end();itr++)
			{
				RatioOrdersH[(*itr).first]-=min_ratio.second;
				if ((almost_equal2(RatioOrdersH[(*itr).first] , (double)0) == 1) || (RatioOrdersH[(*itr).first] < 0))
				{
					TentativeOrdersH.erase((*itr).first);
					RatioOrdersH.erase((*itr).first);
					TentativeOrdersFirstH.erase((*itr).first);
				}
			}
			if(RatioOrdersH.size()>1)
			{
				min_ratio = *min_element(RatioOrdersH.begin(), RatioOrdersH.end(), compare);

				for (unordered_map<int, double>::iterator itr=TentativeOrdersH.begin(); itr!=TentativeOrdersH.end();itr++)
				{
					if((*itr).first!=0)
					{
						TentativeOrdersH[(*itr).first] = min_ratio.second*OutPutFirmHoH[ID][(*itr).first][0];
					}
					else
					{
						TentativeOrdersH[(*itr).first] = min_ratio.second*cVectorH[ID];
					}
				}
			}
		}
	}
}

void RationingCustomers(int ID)
{
	rzDemandVectorH[ID]=0.0;

	if(CurrentProductionH[ID]>cVectorH[ID])
	{
		rzDemandVectorH[ID]+=cVectorH[ID];
	}

	else
	{
		rzDemandVectorH[ID]+=CurrentProductionH[ID];
	}
}

void Trading(int ID)
{
	if (almost_equal2(CurrentProductionH[ID] , rcDemandVectorH[ID])==0)
	{
		if ((ListSupplier.find(ID) != ListSupplier.end())==0)
		{
			RationingCustomers(ID);
		}
		else
		{
			Rationing(ID);
		}
	}

	else
	{
		rzDemandVectorH[ID]=cVectorH[ID]; //Ci: consumption for households
        double rc;
		for (unordered_map<int, double>::iterator itr=rcDemandFirmLevelH[ID].begin(); itr!=rcDemandFirmLevelH[ID].end();itr++)
		{
			rc = rcDemandFirmLevelH[ID][(*itr).first];
			rOrdersHoH[(*itr).first][ID]=rc;
			AccfInventoryHoH[(*itr).first][ID] +=rc;
			AccsInventoryHoH[(*itr).first][InPutFirmHoH[(*itr).first][ID][1]]+=rc;
			rzDemandVectorH[ID]+=rc;
			GROrdersH[(*itr).first]+=rc;
		}
	}

	FirmBSH[ID][0]+=ProfitToSalesH[ID]*rzDemandVectorH[ID];
}


void CannotPay(int ID)
{
    /*
     * Firms who cannot pay are those who could not get a loan.
     * These firms will try to buy only based on there available deposit.
     * They try to minimize their expenses.
    */
	double Quantity = (GROrdersH[ID]- FirmBSH[ID][0])/GROrdersH[ID];
	for(unordered_map<int, double >::iterator itr=rOrdersHoH[ID].begin();itr!=rOrdersHoH[ID].end();itr++)
	{
		double order_to_return = rOrdersHoH[ID][(*itr).first]*Quantity;
		rOrdersHoH[ID][(*itr).first] -= order_to_return;
		AccfInventoryHoH[ID][(*itr).first] -= order_to_return;
		AccsInventoryHoH[ID][InPutFirmHoH[ID][(*itr).first][1]]-= order_to_return;
		rzDemandVectorH[(*itr).first]-= order_to_return;
		FirmBSH[(*itr).first][0]-= ProfitToSalesH[(*itr).first]*order_to_return;
		GROrdersH[ID]-= order_to_return;
	}

	ExpensesH[ID]+=GROrdersH[ID];
}

void LoanDemandSupply(int ID)
{
    double TotalLoanDde = dOrdersH[ID] - FirmBSH[ID][0];

    for(unordered_map<int, vector < double > >::iterator  itr=FirmBankHoH[ID].begin(); itr!=FirmBankHoH[ID].end();itr++)
    {
    	/*
    	 * If equity is positive: the firm get loans from all its banks
    	*/
    	double ln = TotalLoanDde/FirmBankHoH[ID].size();
    	double rate = (1 - CurrentProductionH[ID]/ProductionIniH[ID])*0.04;
    	double periodic = ln/LoanMaturity;
    	if (ShortInterestRate == 1) {periodic = ln*rate/(1-pow(1+rate,-LoanMaturity));}
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(ln); //The amount of loans
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(rate); //The applied interest rate
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(periodic); //The amount paid monthly
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(0); //Number of paid monthly
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(0); //Count periods of non-payment before declaring loan default
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(0); //Index for loan situation; 0: healthy loan with payment; 1: paid loan 2: defaulted loan
    	CurrentLoansHoH[ID][(*itr).first][LoanKeyHoH[ID][(*itr).first]].push_back(0); //0: short-term loan; 1: long-term loan

    	LoanKeyHoH[ID][(*itr).first]++;
    	PayByLoanHoH[ID][(*itr).first]=ln;

    	/*
    	 * After getting loan: Update of the balance sheet
    	 * 1- Rise the level of loans
    	 * 2- Rise the level of deposits
    	*/

    	FirmBSH[ID][2]+=ln;
    	FirmBSH[ID][0]+=ln;

    	/*
    	 * Update the accounts of the firm in their banks
    	*/
    	FirmBankHoH[ID][(*itr).first][0]+=ln;
    	FirmBankHoH[ID][(*itr).first][1]+=ln;
    }
}

void NeedLoan(int ID)
{
	if(dOrdersH[ID] <=  FirmBSH[ID][0]){LoanFlagH[ID] = 2;}
	else
	{
		if(BankRiskManager == 1)
		{
			double TotalLoanDde = dOrdersH[ID] - FirmBSH[ID][0];
			double risk = (TotalLoanDde + FirmBSH[ID][2])/(FirmBSH[ID][3] + TotalLoanDde + FirmBSH[ID][2]);
			if(risk < LimitSolvencyRatio)
			{
				LoanDemandSupply(ID);
				FirmsNoLoansH[ID] = 0;
			}
			else
			{
				FirmsNoLoansH[ID]++;
				if((FirmsNoLoansH[ID] > 0) and (HelpFirms == 1))
				{
					FirmBSH[ID][0]+=TotalLoanDde;
					GvtSupport+=TotalLoanDde;
					FirmsNoLoansH[ID] = 0;
				}
			}
		}

		else if(BankRiskManager == 2)
		{
			if(FirmBSH[ID][3] > 0.0)
			{
				LoanDemandSupply(ID);
			}
		}

		else if(BankRiskManager == 0) {LoanDemandSupply(ID);}

	}
}

void Payment(int ID)
{
	if ((FirmBSH[ID][0] > GROrdersH[ID]) or (almost_equal2(FirmBSH[ID][0],GROrdersH[ID]) == 1))
	{
		double used_deposit = GROrdersH[ID];
		//FirmBSH[ID][0]-=used_deposit;
		/*
		 * After payment, the new deposit is updated as a weight of current loans from each bank
		*/
		ExpensesH[ID]+=GROrdersH[ID];
	}

	else
	{
		if(FirmBSH[ID][0] > 0.0 ){CannotPay(ID);}
		else
		{
			for(unordered_map<int, double >::iterator itr=rOrdersHoH[ID].begin();itr!=rOrdersHoH[ID].end();itr++)
			{
				rzDemandVectorH[(*itr).first]-= rOrdersHoH[ID][(*itr).first];
				FirmBSH[(*itr).first][0]-= ProfitToSalesH[(*itr).first]*rOrdersHoH[ID][(*itr).first];
				rOrdersHoH[ID][(*itr).first] = 0;
				AccfInventoryHoH[ID][(*itr).first] = 0;
				AccsInventoryHoH[ID][InPutFirmHoH[ID][(*itr).first][1]]= 0;
				GROrdersH[ID]= 0;
			}
		}
	}
}


void FirmBS_Update(int ID)
{
	double paid_capital = 0;
	double total_amortization = 0;
	double profit = rzDemandVectorH[ID] - ExpensesH[ID];
	bool TotallyPaid;
	for(unordered_map<int, unordered_map<int, vector < double > > >::iterator  itr1=CurrentLoansHoH[ID].begin(); itr1!=CurrentLoansHoH[ID].end();itr1++)
    {
		/*
		 * Update of the deposit in each bank account based on the profit only (not final update)
		*/
		FirmBankHoH[ID][(*itr1).first][1]=FirmBSH[ID][0]/FirmBankHoH[ID].size();

    	for(unordered_map<int, vector < double > >::iterator  itr2=CurrentLoansHoH[ID][(*itr1).first].begin(); itr2!=CurrentLoansHoH[ID][(*itr1).first].end();itr2++)
    	{
    		TotallyPaid = 0;
    		if((CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] == 0) or (CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] == 2))
    		{
    			if(FirmBankHoH[ID][(*itr1).first][1] >= CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][2])
    			{
        			/*
        			 * The current loan level update
        			*/
    				CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] = 0;
        			double paid_capital_bank = CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][0]/LoanMaturity;
        			FirmBankHoH[ID][(*itr1).first][0]-=paid_capital_bank;
        			paid_capital+=paid_capital_bank;
        			/*
        			 * Deposits after paying loans
        			*/
        			FirmBankHoH[ID][(*itr1).first][1]-= CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][2];
        			total_amortization+=CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][2];
        			CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][3]++;
        			if(CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][3] >= LoanMaturity)
        			{
        				CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] = 1;
        				TotallyPaid = 1;
        			}
    			}

    			else
    			{
    				CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][4]++;
    				if((CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][4] >= LimitToDefault) and (CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][6] == 0))
    				{
        				double NPL = CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][0];
        				NPLHoH[ID][(*itr1).first]+=NPL;
        				BankNPLH[(*itr1).first][0] += NPL/CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][0];
        				BankNPLH[(*itr1).first][1]++;
        				CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] = 2;
    				}
    			}
    		}
    		if (TotallyPaid == 0)
    		{
        		if((CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] == 0) and (CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][6] == 0))
        		{
        			GLoan+=CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][0];
        		}

        		else if ((CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][5] == 2) and (CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][6] == 0))
        		{
        			GNPL+=CurrentLoansHoH[ID][(*itr1).first][(*itr2).first][0];
        		}
    		}
    	}
    }
	/*
	 * Final update of the balance sheet
	 * Deposit / Loans / Equity
	*/

	FirmBSH[ID][0]-= total_amortization;
	FirmBSH[ID][2]-= paid_capital;
	FirmBSH[ID][3]= FirmBSH[ID][0]+FirmBSH[ID][1] - FirmBSH[ID][2] - FirmBSH[ID][4];
	GDeposit+=rzDemandVectorH[ID]*ProfitToSalesH[ID] - total_amortization;
	GEquity+=FirmBSH[ID][3];
}

void OneStepSimulation()
{
	/*
	 * This function calls all the previous functions.
	 * It represents a simulation across all firms in one step: from t to t+1
	*/
	ValueGDP = 0;
	Desired_Goods();
	for (unordered_map<int,int>::iterator itr=ListofFirmsH.begin(); itr!=ListofFirmsH.end();itr++)
	{
		if (((InactiveFirms.find((*itr).first) != InactiveFirms.end())==0))
		{
			ProductionInoue18((*itr).first);
			Trading((*itr).first);
		}
	}

	for (unordered_map<int,int>::iterator itr=ListofFirmsH.begin(); itr!=ListofFirmsH.end();itr++)
	{
		if (((InactiveFirms.find((*itr).first) != InactiveFirms.end())==0))
		{
			if(ShortLoans==1){NeedLoan((*itr).first);}
			if(WithPayment==1){Payment((*itr).first);}
			FirmBS_Update((*itr).first);

			/*
			 * A loop to erase paid loans
			*/
			for(unordered_map<int, unordered_map<int, vector < double > > >::iterator  itr1=CurrentLoansHoH[(*itr).first].begin(); itr1!=CurrentLoansHoH[(*itr).first].end();itr1++)
		    {
				set<int> key;
				unordered_map<int, vector < double > >::iterator  itr2=CurrentLoansHoH[(*itr).first][(*itr1).first].begin();
		    	while(itr2!=CurrentLoansHoH[(*itr).first][(*itr1).first].end())
				{
		    		if(CurrentLoansHoH[(*itr).first][(*itr1).first][(*itr2).first][5] == 1){key.insert((*itr2).first);}
		    		itr2++;
		    	}
		    	for(set<int>::iterator itKey = key.begin(); itKey != key.end(); itKey++)
		    	{
		    		CurrentLoansHoH[(*itr).first][(*itr1).first].erase(*itKey);
		    	}
		    }
		}
	}

	double min = 0.0;
	double max = 0.0;
	for(set<int>::iterator it = DamagedFirmsH.begin(); it != DamagedFirmsH.end(); it++)
	{
		double recover = FirmBSH[*it][0]/(ProductionIniH[*it]*DamageMagnitude*(1-1/LTLoanMaturity));
		if (min < recover){min = recover;}
		if (max > recover){max = recover;}
	}

	for(set<int>::iterator it = DamagedFirmsH.begin(); it != DamagedFirmsH.end(); it++)
	{
		double recover = FirmBSH[*it][0]/(ProductionIniH[*it]*DamageMagnitude*(1-1/LTLoanMaturity));
		RecoveryH[*it] = Scale(min,max,recover);
	}

	for (unordered_map<int, unordered_map<int, vector<double>>>::iterator itC=fInventoryHoH.begin(); itC!=fInventoryHoH.end();itC++)
	{
		fUsedInventoryH[(*itC).first] = 0;
        for(unordered_map<int, vector<double>>::iterator itS=fInventoryHoH[(*itC).first].begin(); itS!=fInventoryHoH[(*itC).first].end();itS++)
        {
        	/*
        	 * Update the inventory at the firm level.
        	 * Update the inventory at the sector level.
        	*/

        	double used = InPutFirmHoH[(*itC).first][(*itS).first][0]*CurrentProductionH[(*itC).first]/ProductionIniH[(*itC).first];
        	fInventoryHoH[(*itC).first][(*itS).first][0]=fInventoryHoH[(*itC).first][(*itS).first][0]-used + AccfInventoryHoH[(*itC).first][(*itS).first];
        	sInventoryHoH[(*itC).first][InPutFirmHoH[(*itC).first][(*itS).first][1]]=sInventoryHoH[(*itC).first][InPutFirmHoH[(*itC).first][(*itS).first][1]]-used + AccsInventoryHoH[(*itC).first][InPutFirmHoH[(*itC).first][(*itS).first][1]];
        	fUsedInventoryH[(*itC).first]+=used;
        	AccfInventoryHoH[(*itC).first][(*itS).first] = 0;
        	AccsInventoryHoH[(*itC).first][InPutFirmHoH[(*itC).first][(*itS).first][1]] = 0;
        }
	}

	for (unordered_map<int, double>::iterator itr=rzDemandVectorH.begin(); itr!=rzDemandVectorH.end();itr++)
	{
		double val = rzDemandVectorH[(*itr).first] - fUsedInventoryH[(*itr).first];
		ValueAddedVectorH[(*itr).first].push_back(val);
		ValueGDP+=val;
	}
	RateNPLH.push_back(GNPL/(GLoan+GNPL));
	GDP.push_back(ValueGDP);
	NPLH.push_back(GNPL);
	DepositH.push_back(GDeposit);
	GDeposit = 0;
	LoanH.push_back(GLoan);
	GLoan = 0;
	GNPL = 0;
	EquityH.push_back(GEquity);
	GEquity = 0;

}

void Simulate()
{
	/*
	 * This function is the whole system simulation.
	 * It calls only the procedure OneStepSimulation() over the simulation time.
	*/

	while(t < SimTime)
	{
		auto start = std::chrono::high_resolution_clock::now();
		if(t==1)
		{
			DamagedFirms();
			disaster = 1;
			if(LTLoansModel==1)
			{
				for(set<int>::iterator it = DamagedFirmsH.begin(); it != DamagedFirmsH.end(); it++)
				{

					double TotalLoanDde = ProductionIniH[*it]*DamageMagnitude;

					for(unordered_map<int, vector < double > >::iterator  itr=FirmBankHoH[*it].begin(); itr!=FirmBankHoH[*it].end();itr++)
					{
					    double ln = TotalLoanDde/FirmBankHoH[*it].size();
					    double rate = 0.04;
					    double periodic = ln*rate/(1-pow(1+rate,-LTLoanMaturity));

					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(ln); //The amount of loans
					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(rate); //The applied interest rate
					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(periodic); //The amount paid monthly
					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(0); //Number of paid monthly
					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(0); //Count periods of non-payment before declaring loan default
					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(0); //Index for loan situation; 0: healthy loan with payment; 1: paid loan 2: defaulted loan
					    CurrentLoansHoH[*it][(*itr).first][LoanKeyHoH[*it][(*itr).first]].push_back(1); //0: short-term loan; 1: long-term loan

					    LoanKeyHoH[*it][(*itr).first]++;
					    FirmBankHoH[*it][(*itr).first][0]+=ln;

					}
				}
			}
		}
		OneStepSimulation();
		cout << "step ; " << t << " ; " << GDP[GDP.size()-1] << endl;
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		cout << "Elapsed time: " << elapsed.count() << " s\n";
		++t;
	}
	GvtSupportH.push_back(GvtSupport);
}

/*
 * The main program
 * Execution of all functions and procedures allowing the simulation of our artificial economy
*/


int main()
{
	srand (time(NULL));
	for (int sim = 0 ; sim < GlobalSim ; sim++)
	{
		Initial_Data();
		Simulate();

		for(unordered_map<int, unordered_map<int, vector < double > > >::iterator itr1 = FirmBankHoH.begin(); itr1 != FirmBankHoH.end() ; itr1++)
		{
			for(unordered_map<int, vector < double > >::iterator itr2 = FirmBankHoH[(*itr1).first].begin();itr2 != FirmBankHoH[(*itr1).first].end(); itr2++)
			{
				BankLoanDepositH[(*itr2).first][0]+=FirmBankHoH[(*itr1).first][(*itr2).first][0];
				BankLoanDepositH[(*itr2).first][1]+=FirmBankHoH[(*itr1).first][(*itr2).first][1];
			}
		}

		ofstream gdp_data;
		std::string path = "Results/GDP.txt";
		gdp_data.open(path);
	    for (int i=0; i<GDP.size(); ++i)
		{
			gdp_data << GDP[i] << '\n'  ;
			SimGDPH[sim].push_back(GDP[i]);
		}
		gdp_data.close();

		ofstream npl_data;
		std::string npl_path = "Results/NPL.txt";
		npl_data.open(npl_path);
	    for (int i=0; i<NPLH.size(); ++i)
		{
	    	npl_data << NPLH[i] << '\n'  ;
	    	SimNPLH[sim].push_back(NPLH[i]);
		}
	    npl_data.close();

		ofstream deposit_data;
		std::string deposit_path = "Results/Deposit.txt";
		deposit_data.open(deposit_path);
	    for (int i=0; i<DepositH.size(); ++i)
		{
	    	deposit_data << DepositH[i] << '\n'  ;
	    	SimDepositH[sim].push_back(DepositH[i]);
		}
	    deposit_data.close();

		ofstream equity_data;
		std::string equity_path = "Results/Equity.txt";
		equity_data.open(equity_path);
	    for (int i=0; i<EquityH.size(); ++i)
		{
	    	equity_data << EquityH[i] << '\n'  ;
		}
	    equity_data.close();

		ofstream loan_data;
		std::string loan_path = "Results/Loan.txt";
		loan_data.open(loan_path);
	    for (int i=0; i<LoanH.size(); ++i)
		{
	    	loan_data << LoanH[i] << '\n'  ;
	    	SimLoansH[sim].push_back(LoanH[i]);
		}
	    loan_data.close();

		ofstream rate_data;
		std::string rate_path = "Results/RateNPL.txt";
		rate_data.open(rate_path);
	    for (int i=0; i<RateNPLH.size(); ++i)
		{
	    	rate_data << RateNPLH[i] << '\n'  ;
	    	SimNPLRateH[sim].push_back(RateNPLH[i]);
		}
	    rate_data.close();

		ofstream damage_data;
		std::string damage_path = "Results/DamagedFirms.txt";
		damage_data.open(damage_path);
	    for (set <int>::iterator itr = DamagedFirmsH.begin(); itr != DamagedFirmsH.end(); ++itr)
		{
	    	damage_data << *itr << '\n' ;
		}
	    damage_data.close();

		ofstream degree_data;
		std::string degree_path = "Results/FirmLevelData.txt";
		degree_data.open(degree_path);

		Clearing();
	}

	ofstream full_gdp;
	std::string path = "Results/FullGDP.txt";
	full_gdp.open(path);
	for(int i = 0; i < SimTime; i++)
	{
		for (unordered_map<int, vector<double> >::iterator it = SimGDPH.begin(); it!=SimGDPH.end();++it)
		{
	    	full_gdp << SimGDPH[(*it).first][i] << '\t'  ;
		}
    	full_gdp << '\n';
	}

    full_gdp.close();

	ofstream full_loan;
	path = "Results/FullLoan.txt";
	full_loan.open(path);
	for(int i = 0; i < SimTime; i++)
	{
		for (unordered_map<int, vector<double> >::iterator it = SimLoansH.begin(); it!=SimLoansH.end();++it)
		{
			full_loan << SimLoansH[(*it).first][i] << '\t'  ;
		}
		full_loan << '\n';
	}

	full_loan.close();

	ofstream full_npl;
	path = "Results/FullNPL.txt";
	full_npl.open(path);
	for(int i = 0; i < SimTime; i++)
	{
		for (unordered_map<int, vector<double> >::iterator it = SimNPLH.begin(); it!=SimNPLH.end();++it)
		{
			full_npl << SimNPLH[(*it).first][i] << '\t'  ;
		}
		full_npl << '\n';
	}

	full_npl.close();

	ofstream full_nplrate;
	path = "Results/FullNPLRate.txt";
	full_nplrate.open(path);
	for(int i = 0; i < SimTime; i++)
	{
		for (unordered_map<int, vector<double> >::iterator it = SimNPLRateH.begin(); it!=SimNPLRateH.end();++it)
		{
			full_nplrate << SimNPLRateH[(*it).first][i] << '\t'  ;
		}
		full_nplrate << '\n';
	}

	full_nplrate.close();

	ofstream full_deposit;
	path = "Results/FullDeposit.txt";
	full_deposit.open(path);
	for(int i = 0; i < SimTime; i++)
	{
		for (unordered_map<int, vector<double> >::iterator it = SimDepositH.begin(); it!=SimDepositH.end();++it)
		{
			full_deposit << SimDepositH[(*it).first][i] << '\t'  ;
		}
		full_deposit << '\n';
	}

	full_deposit.close();

	ofstream npl;
	path = "Results/FinalNPL.txt";
	npl.open(path);
	for (unordered_map<int, vector<double> >::iterator it = SimLoansH.begin(); it!=SimLoansH.end();++it)
	{
		double sum_loans = 0;
		double sum_npl = 0;
		for(int i = 0; i < SimTime; i++)
		{
			sum_loans+=SimLoansH[(*it).first][i];
			sum_npl+=SimNPLH[(*it).first][i];
		}

		npl << sum_npl/(sum_npl+sum_loans) << '\n'  ;
	}
	npl.close();

	ofstream liquidity;
	path = "Results/FinalLiquidity.txt";
	liquidity.open(path);
	for (unordered_map<int, vector<double> >::iterator it = SimLoansH.begin(); it!=SimLoansH.end();++it)
	{
		double sum_loans = 0;
		double sum_dep = 0;
		for(int i = 0; i < SimTime; i++)
		{
			sum_loans+=SimLoansH[(*it).first][i];
			sum_dep+=SimDepositH[(*it).first][i];
		}

		liquidity << sum_loans/sum_dep << '\n'  ;
	}
	liquidity.close();

	ofstream support;
	path = "Results/FinalGvtSupport.txt";
	support.open(path);
	for (int it = 0; it < GvtSupportH.size(); ++it)
	{
		support << GvtSupportH[it] << '\n'  ;
	}
	support.close();
	return 0;
}

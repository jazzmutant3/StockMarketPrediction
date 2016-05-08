#ifndef STOCK_MARKET_PREDICTION_HEADER
#define STOCK_MARKET_PREDICTION_HEADER

#include "HTTPAlgorithms.h"
#include "StringAlgorithms.h"
#include <armadillo>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlucode.h>
#include <odbcinst.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace arma;

struct SQLStock
{
	SQLStock(string Date, double Open, double High, double Low, double Close, double Volume, double AdjustedClose, double percentChange)
	{
		this->Date = Date;
		this->Open = Open;
		this->High = High;
		this->Low = Low;
		this->Close = Close;
		this->Volume = Volume;
		this->AdjustedClose = AdjustedClose;
		this->PercentChange = percentChange;
	}
	SQLStock(string Date, string Open, string High, string Low, string Close, string Volume, string AdjustedClose, string PercentChange)
	{
		this->Date = Date;

		stringstream ss;
		ss << Open;
		ss >> this->Open;

		ss.clear();
		ss.str("");
		ss << High;
		ss >> this->High;

		ss.clear();
		ss.str("");
		ss << Low;
		ss >> this->Low;

		ss.clear();
		ss.str("");
		ss << Close;
		ss >> this->Close;

		ss.clear();
		ss.str("");
		ss << Volume;
		ss >> this->Volume;

		ss.clear();
		ss.str("");
		ss << AdjustedClose;
		ss >> this->AdjustedClose;

		ss.clear();
		ss.str("");
		ss << PercentChange;
		ss >> this->PercentChange;
	}

	string Date = "NULL";
	double Open = 0;
	double High = 0;
	double Low = 0;
	double Close = 0;
	double Volume = 0;
	double AdjustedClose = 0;
	double PercentChange = 0;
};

struct SQLStockStatistic
{
	SQLStockStatistic(double Open, double High, double Low, double Close, double Volume, double AdjustedClose, double PercentChange)
	{
		this->Open = Open;
		this->High = High;
		this->Low = Low;
		this->Close = Close;
		this->Volume = Volume;
		this->AdjustedClose = AdjustedClose;
		this->PercentChange = PercentChange;
	}

	double Open;
	double High;
	double Low;
	double Close;
	double Volume;
	double AdjustedClose;
	double PercentChange;
};

void ConnectToDatabase(string driver, string server, string database, SQLHENV * environmentHandle, SQLHDBC * databaseConnectionHandle)
{
	SQLCHAR outputString[1024]; SQLSMALLINT outputStringLength; SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, environmentHandle);
	ret = SQLSetEnvAttr(*environmentHandle, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
	ret = SQLAllocHandle(SQL_HANDLE_DBC, *environmentHandle, databaseConnectionHandle);
	ret = SQLDriverConnectA(*databaseConnectionHandle, NULL, (SQLCHAR *)InsertToString("Driver={%s}; Server=%s; Database=%s; Uid=roberdurfe17; Pwd=greenLantern@3", driver.c_str(), server.c_str(), database.c_str()).c_str(), SQL_NTS, outputString, sizeof(outputString), &outputStringLength, SQL_DRIVER_COMPLETE);
}

void DisconnetFromDatabase(SQLHENV * environmentHandle, SQLHDBC * databaseConnectionHandle)
{
	SQLDisconnect(*databaseConnectionHandle);
	SQLFreeHandle(SQL_HANDLE_DBC, *databaseConnectionHandle);
	SQLFreeHandle(SQL_HANDLE_ENV, *environmentHandle);
}

vector<SQLStock> ExtractHistoricalStockDataYahoo(string rawData, string symbol)
{
	vector<SQLStock> output;

	int index = 0;

	index = (int)rawData.find(",", index);
	index = (int)rawData.find("\n", index) + (int)strlen("\n");

	while (true)
	{
		string data[7];
		for (int i = 0; i < 6; i++)
		{
			int next;
			if ((next = (int)rawData.find(",", index)) == -1)
				return output;
			data[i] = rawData.substr(index, next - index);
			index = next + 1;
		}

		int next;
		if ((next = (int)rawData.find("\n", index)) == -1)
			return output;
		data[6] = rawData.substr(index, next - index);
		index = next + 1;

		output.push_back(SQLStock(data[0], data[1], data[2], data[3], data[4], data[5], data[6], "0"));
	}

	return output;
}

void SendDatabaseQuery(SQLHDBC databaseConnectionHandle, string query)
{
	SQLHSTMT statementHandle;

	SQLAllocHandle(SQL_HANDLE_STMT, databaseConnectionHandle, &statementHandle);

	SQLExecDirectA(statementHandle, (SQLCHAR *)query.c_str(), SQL_NTS);
}

vector<SQLStock> ReceiveDatabaseQuery(SQLHDBC databaseConnectionHandle, string query)
{
	vector<SQLStock> output; SQLHSTMT statementHandle;

	SQLRETURN result = SQLAllocHandle(SQL_HANDLE_STMT, databaseConnectionHandle, &statementHandle);

	SQLExecDirectA(statementHandle, (SQLCHAR *)query.c_str(), SQL_NTS);

	int ID; char Date[255]; double Open, High, Low, Close, Volume, AdjustedClose, PercentChange;

	while (SQLFetch(statementHandle) == SQL_SUCCESS)
	{
		SQLGetData(statementHandle, 1, SQL_C_ULONG, &ID, 0, NULL);
		SQLGetData(statementHandle, 2, SQL_C_CHAR, Date, 255, NULL);
		SQLGetData(statementHandle, 3, SQL_C_DOUBLE, &Open, 0, NULL);
		SQLGetData(statementHandle, 4, SQL_C_DOUBLE, &High, 0, NULL);
		SQLGetData(statementHandle, 5, SQL_C_DOUBLE, &Low, 0, NULL);
		SQLGetData(statementHandle, 6, SQL_C_DOUBLE, &Close, 0, NULL);
		SQLGetData(statementHandle, 7, SQL_C_DOUBLE, &Volume, 0, NULL);
		SQLGetData(statementHandle, 8, SQL_C_DOUBLE, &AdjustedClose, 0, NULL);
		SQLGetData(statementHandle, 9, SQL_C_DOUBLE, &PercentChange, 0, NULL);

		output.push_back(SQLStock(RemoveTrailingSpaces(string(Date)), Open, High, Low, Close, Volume, AdjustedClose, PercentChange));
	}

	return output;
}

void GetTrainingAndTestData(SQLHDBC databaseConnectionHandle, string stockName, int miniBatchSize, vector<vector<Col<double>>> * trainingData, vector<vector<Col<double>>> * testData)
{
	stringstream ss;

	ss << "SELECT * FROM [EconomicData].[dbo].[" << stockName << "]";

	vector<SQLStock> allData = ReceiveDatabaseQuery(databaseConnectionHandle, ss.str());

	vector<SQLStock> mean = ReceiveDatabaseQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[Mean] WHERE [StockName] = '%s'", stockName.c_str()));
	vector<SQLStock> stdDev = ReceiveDatabaseQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[StdDev] WHERE [StockName] = '%s'", stockName.c_str()));

	int counter = 0, sizeOfAllData = (int)allData.size() - 1;

	int sizeOfTestData = 0, sizeOfTrainingData = 0;

	sizeOfTrainingData = (sizeOfAllData / 4) * 3;

	while (sizeOfTrainingData % miniBatchSize != 0)
		sizeOfTrainingData++;

	sizeOfTestData = sizeOfAllData - sizeOfTrainingData;

	for (int i = sizeOfAllData; i > sizeOfAllData - sizeOfTrainingData; i--)
	{
		SQLStock today = allData[i];

		trainingData->push_back(vector<Col<double>>());

		vector<double> temp1;

		temp1.push_back((today.Open - mean[0].Open) / stdDev[0].Open);
		temp1.push_back((today.High - mean[0].High) / stdDev[0].High);
		temp1.push_back((today.Low - mean[0].Low) / stdDev[0].Low);
		temp1.push_back((today.AdjustedClose - mean[0].AdjustedClose) / stdDev[0].AdjustedClose);
		temp1.push_back((today.Volume - mean[0].Volume) / stdDev[0].Volume);

		(*trainingData)[counter].push_back(Col<double>(temp1));

		vector<double> temp2;

		temp2.push_back((today.PercentChange + 1.0) / 2.0);

		(*trainingData)[counter].push_back(Col<double>(temp2));

		counter++;
	}

	counter = 0;

	for (int i = sizeOfTestData; i > 1; i--)
	{
		SQLStock today = allData[i];

		testData->push_back(vector<Col<double>>());

		vector<double> temp1;

		temp1.push_back((today.Open - mean[0].Open) / stdDev[0].Open);
		temp1.push_back((today.High - mean[0].High) / stdDev[0].High);
		temp1.push_back((today.Low - mean[0].Low) / stdDev[0].Low);
		temp1.push_back((today.AdjustedClose - mean[0].AdjustedClose) / stdDev[0].AdjustedClose);
		temp1.push_back((today.Volume - mean[0].Volume) / stdDev[0].Volume);

		(*testData)[counter].push_back(Col<double>(temp1));

		vector<double> temp2;

		temp2.push_back((today.PercentChange + 1.0) / 2.0);

		(*testData)[counter].push_back(Col<double>(temp2));

		counter++;
	}
}

//WARNING: This will take several hours (4+) to complete depending on your internet connection.
void PopulateStockDatabase(SQLHDBC databaseConnectionHandle)
{
	//Read S&P 500 Stock Symbols File into Character Buffer
	ifstream SP500StockSymbolsFile("S&P500Symbols.txt", ios::binary);
	SP500StockSymbolsFile.seekg(0, SP500StockSymbolsFile.end);
	int length = (int)SP500StockSymbolsFile.tellg();
	SP500StockSymbolsFile.seekg(0, SP500StockSymbolsFile.beg);
	char * buffer = new char[length];
	SP500StockSymbolsFile.read(buffer, length);
	SP500StockSymbolsFile.close();

	//Convert Character Buffer into a Vector of Symbols
	string * SymbolsString = new string(buffer);
	vector<string> Symbols;
	int FirstIndex = -1;
	int SecondIndex = 0;
	while ((SecondIndex = (int)SymbolsString->find(",", FirstIndex += 1)) != -1)
	{
		Symbols.push_back(SymbolsString->substr(FirstIndex, SecondIndex - FirstIndex));
		FirstIndex = SecondIndex;
	}

	//Create Mean Data Table
	SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Mean]						\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[StockName] nchar(255) NOT NULL,		\
													[Open] float NOT NULL,					\
													[High] float NOT NULL,					\
													[Low] float NOT NULL,					\
													[Close] float NOT NULL,					\
													[Volume] float NOT NULL,				\
													[AdjustedClose] float NOT NULL,			\
													[PercentChange] float NOT NULL			\
												)");

	//Create StdDev Data Table
	SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [StdDev]						\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[StockName] nchar(255) NOT NULL,		\
													[Open] float NOT NULL,					\
													[High] float NOT NULL,					\
													[Low] float NOT NULL,					\
													[Close] float NOT NULL,					\
													[Volume] float NOT NULL,				\
													[AdjustedClose] float NOT NULL,			\
													[PercentChange] float NOT NULL			\
												)");

	for (int i = 0; i < Symbols.size(); i++)
	{
		//Output table information
		cout << "Table " << i + 1 << ":" << endl;
		cout << "\tSymbol: " << Symbols[i] << endl;
		cout << "\tCompany Name: ";

		//Download website with the company name 
		string rawdata = RetrieveWebpage("finance.yahoo.com", InsertToString("/q/hp?s=%s", Symbols[i].c_str()));

		//Make sure data was recieved
		if (rawdata.size() == 0)
		{
			cout << "CONNECTION ERROR." << endl;
			i--;
			continue;
		}

		//Make sure HTTP request was successful
		if (AnalyzeHeader(rawdata) != 200)
		{
			cout << "HTTP ERROR." << endl;
			continue;
		}

		//Remove the HTTP header
		rawdata = RemoveHTTPHeader(rawdata);

		//Find stock name
		int index1 = (int)rawdata.find(InsertToString("(%s)", Symbols[i].c_str()));
		while (rawdata[--index1] != '>');
		int index2 = (int)rawdata.find('<', index1);
		string stockName = rawdata.substr(index1 + 1, index2 - index1 - 1);

		//Replace Amperstands
		int index3;
		if ((index3 = (int)stockName.find("&amp;")) != -1)
			stockName.replace(index3, 5, "&");

		//Output table information
		cout << stockName << endl;
		cout << "\tDatapoints Extracted: ";

		//Download stock data
		rawdata = RetrieveWebpage("real-chart.finance.yahoo.com", InsertToString("/table.csv?s=%s&a=00&b=01&c=1800&d=00&e=01&f=2017&g=d&ignore=.csv", Symbols[i].c_str()));

		//Make sure data was recieved
		if (rawdata.size() == 0)
		{
			cout << "CONNECTION ERROR." << endl;
			i--;
			continue;
		}

		//Make sure HTTP request was successful
		if (AnalyzeHeader(rawdata) != 200)
		{
			cout << "HTTP ERROR." << endl;
			continue;
		}

		//Remove the HTTP header
		rawdata = RemoveHTTPHeader(rawdata);

		//Take the data out of the file
		vector<SQLStock> HistoricalStockData = ExtractHistoricalStockDataYahoo(rawdata, Symbols[i]);

		//Output table information
		cout << HistoricalStockData.size() << endl;
		cout << "\tMeans: ";

		//Create the table
		SendDatabaseQuery(databaseConnectionHandle, InsertToString("CREATE TABLE [%s]							\
																	(											\
																		[ID] int IDENTITY(1,1) PRIMARY KEY,		\
																		[Date] date NOT NULL,					\
																		[Open] float NOT NULL,					\
																		[High] float NOT NULL,					\
																		[Low] float NOT NULL,					\
																		[Close] float NOT NULL,					\
																		[Volume] float NOT NULL,				\
																		[AdjustedClose] float NOT NULL,			\
																		[PercentChange] float NOT NULL			\
																	)", stockName.c_str()));

		//Calculate the mean of the stock data
		SQLStockStatistic mean(0, 0, 0, 0, 0, 0, 0);
		int numberOfElements = (int)HistoricalStockData.size();
		for (int j = 0; j < numberOfElements; j++)
		{
			mean.AdjustedClose += HistoricalStockData[j].AdjustedClose;
			mean.Close += HistoricalStockData[j].Close;
			mean.High += HistoricalStockData[j].High;
			mean.Low += HistoricalStockData[j].Low;
			mean.Open += HistoricalStockData[j].Open;
			mean.Volume += HistoricalStockData[j].Volume;
			if (j < numberOfElements - 1)
				mean.PercentChange += ((HistoricalStockData[j].AdjustedClose - HistoricalStockData[j + 1].AdjustedClose) / HistoricalStockData[j + 1].AdjustedClose);
		}
		cout << "\n\t\tOpen: ";
		mean.Open /= (double)numberOfElements;
		cout << mean.Open << "\n\t\tHigh: ";
		mean.High /= (double)numberOfElements;
		cout << mean.High << "\n\t\tLow: ";
		mean.Low /= (double)numberOfElements;
		cout << mean.Low << "\n\t\tClose: ";
		mean.Close /= (double)numberOfElements;
		cout << mean.Close << "\n\t\tVolume: ";
		mean.Volume /= (double)numberOfElements;
		cout << mean.Volume << "\n\t\tAdjusted Close: ";
		mean.AdjustedClose /= (double)numberOfElements;
		cout << mean.AdjustedClose << "\n\t\tPercent Change: ";
		mean.PercentChange /= (double)numberOfElements;
		cout << mean.PercentChange << endl << "\tStandard Deviations: ";

		stringstream query;
		query << "INSERT INTO [Mean] VALUES ('" << stockName << "', " << mean.Open << ", " << mean.High << ", " << mean.Low << ", " << mean.Close << ", " << mean.Volume << ", " << mean.AdjustedClose << ", " << mean.PercentChange << ")";
		SendDatabaseQuery(databaseConnectionHandle, query.str());

		//Calculate the standard deviation of the stock data
		SQLStockStatistic stdDev(0, 0, 0, 0, 0, 0, 0);
		for (int j = 0; j < numberOfElements; j++)
		{
			stdDev.AdjustedClose += pow((double)HistoricalStockData[j].AdjustedClose - mean.AdjustedClose, 2);
			stdDev.Close += pow((double)HistoricalStockData[j].Close - mean.Close, 2);
			stdDev.High += pow((double)HistoricalStockData[j].High - mean.High, 2);
			stdDev.Low += pow((double)HistoricalStockData[j].Low - mean.Low, 2);
			stdDev.Open += pow((double)HistoricalStockData[j].Open - mean.Open, 2);
			stdDev.Volume += pow((double)HistoricalStockData[j].Volume - mean.Volume, 2);
			if (j < numberOfElements - 1)
				stdDev.PercentChange += pow(((HistoricalStockData[j].AdjustedClose - HistoricalStockData[j + 1].AdjustedClose) / HistoricalStockData[j + 1].AdjustedClose) - mean.PercentChange, 2);
		}
		cout << "\n\t\tOpen: ";
		stdDev.Open = sqrt(stdDev.Open / (double)numberOfElements);
		cout << stdDev.Open << "\n\t\tHigh: ";
		stdDev.High = sqrt(stdDev.High / (double)numberOfElements);
		cout << stdDev.High << "\n\t\tLow: ";
		stdDev.Low = sqrt(stdDev.Low / (double)numberOfElements);
		cout << stdDev.Low << "\n\t\tClose: ";
		stdDev.Close = sqrt(stdDev.Close / (double)numberOfElements);
		cout << stdDev.Close << "\n\t\tVolume: ";
		stdDev.Volume = sqrt(stdDev.Volume / (double)numberOfElements);
		cout << stdDev.Volume << "\n\t\tAdjusted Close: ";
		stdDev.AdjustedClose = sqrt(stdDev.AdjustedClose / (double)numberOfElements);
		cout << stdDev.AdjustedClose << "\n\t\tPercent Change: ";
		stdDev.PercentChange = sqrt(stdDev.PercentChange / (double)numberOfElements);
		cout << stdDev.PercentChange << endl << "\tStatus: ";

		query.clear();
		query.str("");
		query << "INSERT INTO [StdDev] VALUES ('" << stockName << "', " << stdDev.Open << ", " << stdDev.High << ", " << stdDev.Low << ", " << stdDev.Close << ", " << stdDev.Volume << ", " << stdDev.AdjustedClose << ", " << stdDev.PercentChange << ")";
		SendDatabaseQuery(databaseConnectionHandle, query.str());

		//Add stock data into database
		for (int j = 0; j < numberOfElements; j++)
		{
			stringstream query;
			query << "INSERT INTO [" << stockName.c_str() << "] VALUES ('" << HistoricalStockData[j].Date << "', " << HistoricalStockData[j].Open << ", " << HistoricalStockData[j].High << ", " << HistoricalStockData[j].Low << ", " << HistoricalStockData[j].Close << ", " << HistoricalStockData[j].Volume << ", " << HistoricalStockData[j].AdjustedClose << ", ";
			if (j < numberOfElements - 1)
				query << ((HistoricalStockData[j].AdjustedClose - HistoricalStockData[j + 1].AdjustedClose) / HistoricalStockData[j + 1].AdjustedClose) << ")";
			else
				query << 0 << ")";
			SendDatabaseQuery(databaseConnectionHandle, query.str());
		}
		cout << "Completed" << endl;
	}

	return;
}

#endif
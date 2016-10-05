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

struct SQLTreasury
{
	SQLTreasury(string Date, double Percent)
	{
		this->Date = Date;
		this->Percent = Percent;
	}

	SQLTreasury(string Date, string Percent)
	{
		this->Date = Date;

		stringstream ss;
		ss << Percent;
		ss >> this->Percent;
	}

	string Date = "NULL";
	double Percent = 0;
};

struct SQLCommodity
{
	SQLCommodity(string Date, double Price)
	{
		this->Date = Date;
		this->Price = Price;
	}

	SQLCommodity(string Date, string Price)
	{
		this->Date = Date;

		stringstream ss;
		ss << Price;
		ss >> this->Price;
	}

	string Date = "NULL";
	double Price = 0;
};

struct SQLCurrency
{
	SQLCurrency(string Date, double Rate)
	{
		this->Date = Date;
		this->Rate = Rate;
	}

	SQLCurrency(string Date, string Rate)
	{
		this->Date = Date;

		stringstream ss;
		ss << Rate;
		ss >> this->Rate;
	}

	string Date = "NULL";
	double Rate = 0;
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

struct SQLTreasuryStatistic
{
	SQLTreasuryStatistic(double Percent)
	{
		this->Percent = Percent;
	}

	double Percent = 0;
};

struct SQLCommodityStatistic
{
	SQLCommodityStatistic(double Price)
	{
		this->Price = Price;
	}

	double Price = 0;
};

struct SQLCurrencyStatistic
{
	SQLCurrencyStatistic(double Rate)
	{
		this->Rate = Rate;
	}

	double Rate = 0;
};

struct QuandlSchema
{
	QuandlSchema(string Database, string Datasheet, string Name, string Datatype)
	{
		this->Database = Database;
		this->Datasheet = Datasheet;
		this->Name = Name;
		this->Datatype = Datatype;
	}

	string Database;
	string Datasheet;
	string Name;
	string Datatype;
};

struct YahooSchema
{
	YahooSchema(string Symbol, string Name)
	{
		this->Symbol = Symbol;
		this->Name = Name;
	}

	string Symbol;
	string Name;
};

void ConnectToDatabase(string driver, string server, string database, SQLHENV * environmentHandle, SQLHDBC * databaseConnectionHandle)
{
	SQLCHAR outputString[1024]; SQLSMALLINT outputStringLength; SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, environmentHandle);
	ret = SQLSetEnvAttr(*environmentHandle, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
	ret = SQLAllocHandle(SQL_HANDLE_DBC, *environmentHandle, databaseConnectionHandle);
	ret = SQLDriverConnectA(*databaseConnectionHandle, NULL, (SQLCHAR *)InsertToString("Driver={%s}; Server=%s; Database=%s"/*; Uid=roberdurfe17; Pwd=greenLantern@3"*/, driver.c_str(), server.c_str(), database.c_str()).c_str(), SQL_NTS, outputString, sizeof(outputString), &outputStringLength, SQL_DRIVER_COMPLETE);
}

void DisconnetFromDatabase(SQLHENV * environmentHandle, SQLHDBC * databaseConnectionHandle)
{
	SQLDisconnect(*databaseConnectionHandle);
	SQLFreeHandle(SQL_HANDLE_DBC, *databaseConnectionHandle);
	SQLFreeHandle(SQL_HANDLE_ENV, *environmentHandle);
}

vector<SQLStock> ExtractHistoricalStockDataYahoo(string rawData)
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

vector<SQLTreasury> ExtractHistoricalTreasuryDataQuandl(string rawData)
{
	vector<SQLTreasury> output;

	int index = 0;

	index = (int)rawData.find(",", index);
	index = (int)rawData.find("\n", index) + (int)strlen("\n");

	while (true)
	{
		string data[2];
		for (int i = 0; i < 1; i++)
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
		data[1] = rawData.substr(index, next - index);
		index = next + 1;

		output.push_back(SQLTreasury(data[0], data[1]));
	}

	return output;
}

vector<SQLCommodity> ExtractHistoricalCommodityDataQuandl(string rawData)
{
	vector<SQLCommodity> output;

	int index = 0;

	index = (int)rawData.find(",", index);
	index = (int)rawData.find("\n", index) + (int)strlen("\n");

	while (true)
	{
		string data[2];
		for (int i = 0; i < 1; i++)
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
		data[1] = rawData.substr(index, next - index);
		index = next + 1;

		output.push_back(SQLCommodity(data[0], data[1]));
	}

	return output;
}

vector<SQLCurrency> ExtractHistoricalCurrencyDataQuandl(string rawData)
{
	vector<SQLCurrency> output;

	int index = 0;

	index = (int)rawData.find(",", index);
	index = (int)rawData.find("\n", index) + (int)strlen("\n");

	while (true)
	{
		string data[2];
		for (int i = 0; i < 1; i++)
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
		data[1] = rawData.substr(index, next - index);
		index = next + 1;

		output.push_back(SQLCurrency(data[0], data[1]));
	}

	return output;
}

void SendDatabaseQuery(SQLHDBC databaseConnectionHandle, string query)
{
	SQLHSTMT statementHandle;

	SQLAllocHandle(SQL_HANDLE_STMT, databaseConnectionHandle, &statementHandle);

	SQLExecDirectA(statementHandle, (SQLCHAR *)query.c_str(), SQL_NTS);
}

vector<SQLStock> ReceiveStockQuery(SQLHDBC databaseConnectionHandle, string query)
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

vector<SQLTreasury> ReceiveTreasuryQuery(SQLHDBC databaseConnectionHandle, string query)
{
	vector<SQLTreasury> output; SQLHSTMT statementHandle;

	SQLRETURN result = SQLAllocHandle(SQL_HANDLE_STMT, databaseConnectionHandle, &statementHandle);

	SQLExecDirectA(statementHandle, (SQLCHAR *)query.c_str(), SQL_NTS);

	int ID; char Date[255]; double Percent;

	while (SQLFetch(statementHandle) == SQL_SUCCESS)
	{
		SQLGetData(statementHandle, 1, SQL_C_ULONG, &ID, 0, NULL);
		SQLGetData(statementHandle, 2, SQL_C_CHAR, Date, 255, NULL);
		SQLGetData(statementHandle, 3, SQL_C_DOUBLE, &Percent, 0, NULL);

		output.push_back(SQLTreasury(RemoveTrailingSpaces(string(Date)), Percent));
	}

	return output;
}

vector<SQLCommodity> ReceiveCommodityQuery(SQLHDBC databaseConnectionHandle, string query)
{
	vector<SQLCommodity> output; SQLHSTMT statementHandle;

	SQLRETURN result = SQLAllocHandle(SQL_HANDLE_STMT, databaseConnectionHandle, &statementHandle);

	SQLExecDirectA(statementHandle, (SQLCHAR *)query.c_str(), SQL_NTS);

	int ID; char Date[255]; double Price;

	while (SQLFetch(statementHandle) == SQL_SUCCESS)
	{
		SQLGetData(statementHandle, 1, SQL_C_ULONG, &ID, 0, NULL);
		SQLGetData(statementHandle, 2, SQL_C_CHAR, Date, 255, NULL);
		SQLGetData(statementHandle, 3, SQL_C_DOUBLE, &Price, 0, NULL);

		output.push_back(SQLCommodity(RemoveTrailingSpaces(string(Date)), Price));
	}

	return output;
}

vector<SQLCurrency> ReceiveCurrencyQuery(SQLHDBC databaseConnectionHandle, string query)
{
	vector<SQLCurrency> output; SQLHSTMT statementHandle;

	SQLRETURN result = SQLAllocHandle(SQL_HANDLE_STMT, databaseConnectionHandle, &statementHandle);

	SQLExecDirectA(statementHandle, (SQLCHAR *)query.c_str(), SQL_NTS);

	int ID; char Date[255]; double Rate;

	while (SQLFetch(statementHandle) == SQL_SUCCESS)
	{
		SQLGetData(statementHandle, 1, SQL_C_ULONG, &ID, 0, NULL);
		SQLGetData(statementHandle, 2, SQL_C_CHAR, Date, 255, NULL);
		SQLGetData(statementHandle, 3, SQL_C_DOUBLE, &Rate, 0, NULL);

		output.push_back(SQLCurrency(RemoveTrailingSpaces(string(Date)), Rate));
	}

	return output;
}

void GetTrainingAndTestData(SQLHDBC databaseConnectionHandle, string stockName, /*int miniBatchSize,*/ vector<vector<Col<double>>> * trainingData, string trainingStartDate, string trainingEndDate, vector<vector<Col<double>>> * testData, string testStartDate, string testEndDate)
{
	vector<SQLStock> rawStockTrainingData = ReceiveStockQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[%s] WHERE [Date] >= '%s' AND [Date] <= '%s'", stockName.c_str(), trainingStartDate.c_str(), trainingEndDate.c_str()));
	vector<SQLStock> rawStockTestData = ReceiveStockQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[%s] WHERE [Date] >= '%s' AND [Date] <= '%s'", stockName.c_str(), testStartDate.c_str(), testEndDate.c_str()));
	vector<SQLStock> stockMean = ReceiveStockQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[Stock Mean] WHERE [StockName] = '%s'", stockName.c_str()));
	vector<SQLStock> stockStdDev = ReceiveStockQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[Stock StdDev] WHERE [StockName] = '%s'", stockName.c_str()));
	
	int counter = 0;

	for (int i = (int)rawStockTrainingData.size() - 1; i >= 0; i--)
	{
		trainingData->push_back(vector<Col<double>>());

		vector<double> temp1;

		temp1.push_back((rawStockTrainingData[i].Open - stockMean[0].Open) / stockStdDev[0].Open);
		temp1.push_back((rawStockTrainingData[i].High - stockMean[0].High) / stockStdDev[0].High);
		temp1.push_back((rawStockTrainingData[i].Low - stockMean[0].Low) / stockStdDev[0].Low);
		temp1.push_back((rawStockTrainingData[i].Close - stockMean[0].Close) / stockStdDev[0].Close);
		temp1.push_back((rawStockTrainingData[i].Volume - stockMean[0].Volume) / stockStdDev[0].Volume);

		(*trainingData)[counter].push_back(Col<double>(temp1));

		vector<double> temp2;

		temp2.push_back((rawStockTrainingData[i].PercentChange + 1.0) / 2.0);

		(*trainingData)[counter].push_back(Col<double>(temp2));

		counter++;
	}

	counter = 0;

	for (int i = (int)rawStockTestData.size() - 1; i >= 0; i--)
	{
		testData->push_back(vector<Col<double>>());

		vector<double> temp1;

		temp1.push_back((rawStockTestData[i].Open - stockMean[0].Open) / stockStdDev[0].Open);
		temp1.push_back((rawStockTestData[i].High - stockMean[0].High) / stockStdDev[0].High);
		temp1.push_back((rawStockTestData[i].Low - stockMean[0].Low) / stockStdDev[0].Low);
		temp1.push_back((rawStockTestData[i].Close - stockMean[0].Close) / stockStdDev[0].Close);
		temp1.push_back((rawStockTestData[i].Volume - stockMean[0].Volume) / stockStdDev[0].Volume);

		(*testData)[counter].push_back(Col<double>(temp1));

		vector<double> temp2;

		temp2.push_back((rawStockTestData[i].PercentChange + 1.0) / 2.0);

		(*testData)[counter].push_back(Col<double>(temp2));

		counter++;
	}
}

//WARNING: This will take several hours (4+) to complete depending on your internet connection.
void PopulateStockDatabase(SQLHDBC databaseConnectionHandle)
{
	//Data retrieved from Yahoo! Finance
	{
		//Read S&P 500 Stock Symbols File into Character Buffer
		ifstream SP500StockSymbolsFile("S&P500.csv", ios::binary);
		SP500StockSymbolsFile.seekg(0, SP500StockSymbolsFile.end);
		int length = (int)SP500StockSymbolsFile.tellg();
		SP500StockSymbolsFile.seekg(0, SP500StockSymbolsFile.beg);
		char * buffer = new char[length];
		SP500StockSymbolsFile.read(buffer, length);
		SP500StockSymbolsFile.close();

		//Convert Character Buffer into a Vector of Symbols
		string * SymbolsString = new string(buffer);
		delete buffer;
		int index = 0;
		vector<YahooSchema> Symbols;
		while (true)
		{
			string data[2];
			for (int i = 0; i < 1; i++)
			{
				int next;
				if ((next = (int)SymbolsString->find(",", index)) == -1)
					goto BREAKOUT1;
				data[i] = SymbolsString->substr(index, next - index);
				index = next + 1;
			}
			int next;
			if ((next = (int)SymbolsString->find(",\r\n", index)) == -1)
				goto BREAKOUT1;
			data[1] = SymbolsString->substr(index, next - index);
			index = next + 3;
			Symbols.push_back(YahooSchema(data[0], data[1]));
		}

	BREAKOUT1:

		//Create Stock Mean Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Stock Mean]				\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[StockName] nvarchar(255) NOT NULL,		\
													[Open] float NOT NULL,					\
													[High] float NOT NULL,					\
													[Low] float NOT NULL,					\
													[Close] float NOT NULL,					\
													[Volume] float NOT NULL,				\
													[AdjustedClose] float NOT NULL,			\
													[PercentChange] float NOT NULL			\
												)");

		//Create Stock StdDev Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Stock StdDev]			\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[StockName] nvarchar(255) NOT NULL,		\
													[Open] float NOT NULL,					\
													[High] float NOT NULL,					\
													[Low] float NOT NULL,					\
													[Close] float NOT NULL,					\
													[Volume] float NOT NULL,				\
													[AdjustedClose] float NOT NULL,			\
													[PercentChange] float NOT NULL			\
												)");

		cout << "---YAHOO DATA---" << endl << endl;

		for (int i = 0; i < Symbols.size(); i++)
		{
			//Output table information
			cout << "Table " << i + 1 << ":" << endl;
			cout << "\tSymbol: " << Symbols[i].Symbol << endl;
			cout << "\tName: " << Symbols[i].Name << endl;

			cout << "\tDatapoints Extracted: ";

			//Download stock data
			string rawdata = RetrieveWebpage(InsertToString("http://real-chart.finance.yahoo.com/table.csv?s=%s&a=00&b=01&c=1800&d=00&e=01&f=2017&g=d&ignore=.csv", Symbols[i].Symbol.c_str()));
			
			//Parse data
			vector<SQLStock> HistoricalStockData = ExtractHistoricalStockDataYahoo(rawdata);

			//Output table information
			cout << HistoricalStockData.size() << endl;
			cout << "\tMeans: ";

			//Create the table
			SendDatabaseQuery(databaseConnectionHandle, InsertToString("CREATE TABLE [%s (%s)]					\
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
																	)", Symbols[i].Name.c_str(), Symbols[i].Symbol.c_str()));

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
					mean.PercentChange += ((HistoricalStockData[j].Close - HistoricalStockData[j + 1].Close) / HistoricalStockData[j + 1].Close);
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
			query << "INSERT INTO [Stock Mean] VALUES ('" << Symbols[i].Name << " (" << Symbols[i].Symbol << ")', " << mean.Open << ", " << mean.High << ", " << mean.Low << ", " << mean.Close << ", " << mean.Volume << ", " << mean.AdjustedClose << ", " << mean.PercentChange << ")";
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
					stdDev.PercentChange += pow(((HistoricalStockData[j].Close - HistoricalStockData[j + 1].Close) / HistoricalStockData[j + 1].Close) - mean.PercentChange, 2);
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
			query << "INSERT INTO [Stock StdDev] VALUES ('" << Symbols[i].Name << " (" << Symbols[i].Symbol << ")', " << stdDev.Open << ", " << stdDev.High << ", " << stdDev.Low << ", " << stdDev.Close << ", " << stdDev.Volume << ", " << stdDev.AdjustedClose << ", " << stdDev.PercentChange << ")";
			SendDatabaseQuery(databaseConnectionHandle, query.str());

			//Add stock data into database
			for (int j = 0; j < numberOfElements; j++)
			{
				stringstream query;
				query << "INSERT INTO [" << Symbols[i].Name << " (" << Symbols[i].Symbol << ")] VALUES ('" << HistoricalStockData[j].Date << "', " << HistoricalStockData[j].Open << ", " << HistoricalStockData[j].High << ", " << HistoricalStockData[j].Low << ", " << HistoricalStockData[j].Close << ", " << HistoricalStockData[j].Volume << ", " << HistoricalStockData[j].AdjustedClose << ", ";
				if (j < numberOfElements - 1)
					query << ((HistoricalStockData[j].Close - HistoricalStockData[j + 1].Close) / HistoricalStockData[j + 1].Close) << ")";
				else
					query << 0 << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());
			}
			cout << "Completed" << endl;
		}
	}

	//Data retrieved from Quandl
	{
		//Read Quandl Symbols File into Character Buffer
		ifstream QuandlSymbolsFile("QuandlSymbols.csv", ios::binary);
		QuandlSymbolsFile.seekg(0, QuandlSymbolsFile.end);
		int length = (int)QuandlSymbolsFile.tellg();
		QuandlSymbolsFile.seekg(0, QuandlSymbolsFile.beg);
		char * buffer = new char[length];
		QuandlSymbolsFile.read(buffer, length);
		QuandlSymbolsFile.close();

		//Convert Character Buffer into a Vector of Symbols
		string * SymbolsString = new string(buffer);
		delete buffer;
		int index = 0;
		index = (int)SymbolsString->find("\n", index) + (int)strlen("\n");
		vector<QuandlSchema> Symbols;
		while (true)
		{
			string data[4];
			for (int i = 0; i < 3; i++)
			{
				int next;
				if ((next = (int)SymbolsString->find(",", index)) == -1)
					goto BREAKOUT2;
				data[i] = SymbolsString->substr(index, next - index);
				index = next + 1;
			}
			int next;
			if ((next = (int)SymbolsString->find("\n", index)) == -1)
				goto BREAKOUT2;
			data[3] = SymbolsString->substr(index, next - index);
			index = next + 1;
			Symbols.push_back(QuandlSchema(data[0], data[1], data[2], data[3]));
		}

	BREAKOUT2:

		//Create Treasury Mean Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Treasury Mean]			\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[TreasuryName] nchar(255) NOT NULL,		\
													[Percent] float NOT NULL,				\
												)");

		//Create Treasury StdDev Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Treasury StdDev]			\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[TreasuryName] nchar(255) NOT NULL,		\
													[Percent] float NOT NULL,				\
												)");

		//Create Commodity Mean Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Commodity Mean]			\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[CommodityName] nchar(255) NOT NULL,	\
													[Price] float NOT NULL,					\
												)");

		//Create Commodity StdDev Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Commodity StdDev]		\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[CommodityName] nchar(255) NOT NULL,	\
													[Price] float NOT NULL,					\
												)");

		//Create Currency Mean Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Currency Mean]			\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[CurrencyName] nchar(255) NOT NULL,		\
													[Rate] float NOT NULL,					\
												)");

		//Create Currency StdDev Data Table
		SendDatabaseQuery(databaseConnectionHandle, "CREATE TABLE [Currency StdDev]			\
												(											\
													[ID] int IDENTITY(1,1) PRIMARY KEY,		\
													[CurrencyName] nchar(255) NOT NULL,		\
													[Rate] float NOT NULL,					\
												)");

		cout << endl << "---QUANDL DATA---" << endl << endl;

		string rawdata;

		for (int i = 0; i < (int)Symbols.size(); i++)
		{
			//Output table information
			cout << "Table " << i + 1 << ":" << endl;
			cout << "\tSymbol: " << Symbols[i].Database << "\\" << Symbols[i].Datasheet << endl;
			cout << "\tName: " << Symbols[i].Name << endl;
			
			//Download quandl data
			cout << "\tDatapoints Extracted: ";
			rawdata = RetrieveWebpage(InsertToString("https://www.quandl.com/api/v3/datasets/%s/%s.csv?start_date=2000-01-01", Symbols[i].Database.c_str(), Symbols[i].Datasheet.c_str()));

			if (Symbols[i].Datatype == "Treasury")
			{
				//Parse data
				vector<SQLTreasury>	HistoricalTreasuryData = ExtractHistoricalTreasuryDataQuandl(rawdata);

				//Create the table
				SendDatabaseQuery(databaseConnectionHandle, InsertToString("CREATE TABLE [%s]					\
																	(											\
																		[ID] int IDENTITY(1,1) PRIMARY KEY,		\
																		[Date] date NOT NULL,					\
																		[Percent] float NOT NULL,				\
																	)", Symbols[i].Name.c_str()));

				//Calculate the mean of the treasury data
				SQLTreasuryStatistic mean(0);
				int numberOfElements = (int)HistoricalTreasuryData.size();
				for (int j = 0; j < numberOfElements; j++)
					mean.Percent += HistoricalTreasuryData[j].Percent;
				cout << "\n\t\tPercent: ";
				mean.Percent /= (double)numberOfElements;
				cout << mean.Percent << endl << "\tStandard Deviations: ";
				stringstream query;
				query << "INSERT INTO [Treasury Mean] VALUES ('" << Symbols[i].Name << "', " << mean.Percent << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());

				//Calculate the stddev of the treasury data
				SQLTreasuryStatistic stdDev(0);
				for (int j = 0; j < numberOfElements; j++)
					stdDev.Percent += pow((double)HistoricalTreasuryData[j].Percent - stdDev.Percent, 2);
				cout << "\n\t\tPercent: ";
				stdDev.Percent = sqrt(stdDev.Percent / (double)numberOfElements);
				cout << stdDev.Percent << endl << "\tStatus: ";
				query.clear();
				query.str("");
				query << "INSERT INTO [Treasury StdDev] VALUES ('" << Symbols[i].Name << "', " << stdDev.Percent << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());

				//Add treasury data into database
				for (int j = 0; j < numberOfElements; j++)
				{
					stringstream query;
					query << "INSERT INTO [" << Symbols[i].Name << "] VALUES ('" << HistoricalTreasuryData[j].Date << "', " << HistoricalTreasuryData[j].Percent << ")";
					SendDatabaseQuery(databaseConnectionHandle, query.str());
				}
			}
			else if (Symbols[i].Datatype == "Commodity")
			{
				//Parse data
				vector<SQLCommodity>	HistoricalCommodityData = ExtractHistoricalCommodityDataQuandl(rawdata);

				//Create the table
				SendDatabaseQuery(databaseConnectionHandle, InsertToString("CREATE TABLE [%s]					\
																	(											\
																		[ID] int IDENTITY(1,1) PRIMARY KEY,		\
																		[Date] date NOT NULL,					\
																		[Price] float NOT NULL,					\
																	)", Symbols[i].Name.c_str()));

				//Calculate the mean of the Commodity data
				SQLCommodityStatistic mean(0);
				int numberOfElements = (int)HistoricalCommodityData.size();
				for (int j = 0; j < numberOfElements; j++)
					mean.Price += HistoricalCommodityData[j].Price;
				cout << "\n\t\tPrice: ";
				mean.Price /= (double)numberOfElements;
				cout << mean.Price << endl << "\tStandard Deviations: ";
				stringstream query;
				query << "INSERT INTO [Commodity Mean] VALUES ('" << Symbols[i].Name << "', " << mean.Price << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());

				//Calculate the stddev of the Commodity data
				SQLCommodityStatistic stdDev(0);
				for (int j = 0; j < numberOfElements; j++)
					stdDev.Price += pow((double)HistoricalCommodityData[j].Price - stdDev.Price, 2);
				cout << "\n\t\tPrice: ";
				stdDev.Price = sqrt(stdDev.Price / (double)numberOfElements);
				cout << stdDev.Price << endl << "\tStatus: ";
				query.clear();
				query.str("");
				query << "INSERT INTO [Commodity StdDev] VALUES ('" << Symbols[i].Name << "', " << stdDev.Price << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());

				//Add Commodity data into database
				for (int j = 0; j < numberOfElements; j++)
				{
					stringstream query;
					query << "INSERT INTO [" << Symbols[i].Name << "] VALUES ('" << HistoricalCommodityData[j].Date << "', " << HistoricalCommodityData[j].Price << ")";
					SendDatabaseQuery(databaseConnectionHandle, query.str());
				}
			}
			else if (Symbols[i].Datatype == "Currency")
			{
				//Parse data
				vector<SQLCurrency>	HistoricalCurrencyData = ExtractHistoricalCurrencyDataQuandl(rawdata);

				//Create the table
				SendDatabaseQuery(databaseConnectionHandle, InsertToString("CREATE TABLE [%s]					\
																	(											\
																		[ID] int IDENTITY(1,1) PRIMARY KEY,		\
																		[Date] date NOT NULL,					\
																		[Rate] float NOT NULL,					\
																	)", Symbols[i].Name.c_str()));

				//Calculate the mean of the Currency data
				SQLCurrencyStatistic mean(0);
				int numberOfElements = (int)HistoricalCurrencyData.size();
				for (int j = 0; j < numberOfElements; j++)
					mean.Rate += HistoricalCurrencyData[j].Rate;
				cout << "\n\t\tRate: ";
				mean.Rate /= (double)numberOfElements;
				cout << mean.Rate << endl << "\tStandard Deviations: ";
				stringstream query;
				query << "INSERT INTO [Currency Mean] VALUES ('" << Symbols[i].Name << "', " << mean.Rate << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());

				//Calculate the stddev of the Currency data
				SQLCurrencyStatistic stdDev(0);
				for (int j = 0; j < numberOfElements; j++)
					stdDev.Rate += pow((double)HistoricalCurrencyData[j].Rate - stdDev.Rate, 2);
				cout << "\n\t\tRate: ";
				stdDev.Rate = sqrt(stdDev.Rate / (double)numberOfElements);
				cout << stdDev.Rate << endl << "\tStatus: ";
				query.clear();
				query.str("");
				query << "INSERT INTO [Currency StdDev] VALUES ('" << Symbols[i].Name << "', " << stdDev.Rate << ")";
				SendDatabaseQuery(databaseConnectionHandle, query.str());

				//Add Currency data into database
				for (int j = 0; j < numberOfElements; j++)
				{
					stringstream query;
					query << "INSERT INTO [" << Symbols[i].Name << "] VALUES ('" << HistoricalCurrencyData[j].Date << "', " << HistoricalCurrencyData[j].Rate << ")";
					SendDatabaseQuery(databaseConnectionHandle, query.str());
				}
			}

			cout << "Completed" << endl;

		}
	}

	return;
}

#endif

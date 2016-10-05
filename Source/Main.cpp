#include "StockMarketPrediction.h"
#include "ArtificialNeuralNetwork.h"
#include <direct.h>

using namespace std;

SQLHENV environmentHandle; SQLHDBC databaseConnectionHandle;

int globalIndex = 0;

double Evaluate(NeuralNetwork * nn, vector<vector<Col<double>>> testData)
{
	vector<SQLStock> mean = ReceiveStockQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[Stock Mean] WHERE [StockName] = '%s'", "Apple Incorporated (AAPL)"));
	vector<SQLStock> stdDev = ReceiveStockQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[Stock StdDev] WHERE [StockName] = '%s'", "Apple Incorporated (AAPL)"));

	stringstream ss;

	ss << "Results\\New7\\AAPL" << globalIndex << "_HiddenLayers-4_LearningRate-0.1_MiniBatchSize-1\\Output-" << nn->Epoch << ".csv";

	ofstream file(ss.str());

	file << "Predicted Price (Real),Actual Price (Real),Error (Real),Predicted Price (Normalized),Actual Price (Normalized),Predicted Percent Change (Real),Actual Percent Change (Real),Predicted Percent Change (Normalized),Actual Percent Change (Normalized)" << endl;

	long double meanError = 0;
	for (int testExample = 0; testExample < testData.size() - 1; testExample++)
	{
		Col<double> result = nn->FeedForward(testData[testExample][0]);

		if (((testData[testExample][1](0, 0) * 2.0) - 1.0) != 0.0)
		{
			meanError += (((testData[testExample][1](0, 0) * 2.0) - 1.0) - ((result(0, 0) * 2.0) - 1.0)) / ((testData[testExample][1](0, 0) * 2.0) - 1.0);
			file << ((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close) * ((result(0, 0) * 2.0) - 1.0) + ((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close) << "," << ((testData[testExample + 1][0](3, 0) * stdDev[0].Close) + mean[0].Close)/*((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close) * ((testData[testExample][1](0, 0) * 2.0) - 1.0) + ((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close)*/ << "," << (((testData[testExample][1](0, 0) * 2.0) - 1.0) - ((result(0, 0) * 2.0) - 1.0)) / ((testData[testExample][1](0, 0) * 2.0) - 1.0) << "," << testData[testExample][0](3, 0) * result(0, 0) + testData[testExample][0](3, 0) << "," << testData[testExample + 1][0](3,0)/*testData[testExample][0](3, 0) * testData[testExample][1](0, 0) + testData[testExample][0](3, 0)*/ << "," << (result(0, 0) * 2.0) - 1.0 << "," << (testData[testExample][1](0, 0) * 2.0) - 1.0 << "," << result(0, 0) << "," << testData[testExample][1](0, 0) << endl;
		}
		else
			file << ((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close) * ((result(0, 0) * 2.0) - 1.0) + ((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close) << "," << ((testData[testExample + 1][0](3, 0) * stdDev[0].Close) + mean[0].Close)/*((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close) * ((testData[testExample][1](0, 0) * 2.0) - 1.0) + ((testData[testExample][0](3, 0) * stdDev[0].Close) + mean[0].Close)*/ << "," << "NULL" << "," << testData[testExample][0](3, 0) * result(0, 0) + testData[testExample][0](3, 0) << "," << testData[testExample + 1][0](3, 0)/*testData[testExample][0](3, 0) * testData[testExample][1](0, 0) + testData[testExample][0](3, 0)*/ << "," << (result(0, 0) * 2.0) - 1.0 << "," << (testData[testExample][1](0, 0) * 2.0) - 1.0 << "," << result(0, 0) << "," << testData[testExample][1](0, 0) << endl;
	}

	meanError /= testData.size();

	return meanError;
}

int main()
{
	//Connect To Database
	ConnectToDatabase("SQL Server", "DESKTOP-38HSR7V", "EconomicData3", &environmentHandle, &databaseConnectionHandle);

	//Fill database with stock data
	//PopulateStockDatabase(databaseConnectionHandle);

	string fromDate[11] = { "10-19-15","10-26-15","11-2-15","11-9-15","11-16-15","11-23-15","11-30-15","12-7-15","12-14-15","12-21-15","12-28-15" };
	string toDate[10] = { "10-23-15","10-30-15","11-6-15","11-13-15","11-20-15","11-27-15","12-4-15","12-11-15","12-18-15","12-25-15" };

	//Initialize Neural Network
	vector<int> sizes = { 5, 4, 1 };
	NeuralNetwork nn(sizes, &Evaluate);

	//Create Output Folder
	_mkdir("Results\\New7\\");

	for (globalIndex; globalIndex < 9; globalIndex++)
	{
		stringstream ss; ss << "AAPL" << globalIndex << "_HiddenLayers-4_LearningRate-0.1_MiniBatchSize-1";
		cout << ss.str() << endl;

		//Create Output Folder
		_mkdir(InsertToString("Results\\New7\\%s", ss.str().c_str()).c_str());

		nn.Filename = InsertToString("Results\\New7\\%s\\%s.nnd", ss.str().c_str(), ss.str().c_str());

		//Collect Training and Test Data
		vector<vector<Col<double>>> trainingData, testData;
		GetTrainingAndTestData(databaseConnectionHandle, "Apple Incorporated (AAPL)", &trainingData, fromDate[globalIndex], toDate[globalIndex], &testData, fromDate[globalIndex + 1], fromDate[globalIndex + 2]);

		//Train Neural Network
		nn.StochasticGradientDescent(trainingData, 50, 1, 0.1, testData);
	}

	//Disconnect From Database
	DisconnetFromDatabase(&environmentHandle, &databaseConnectionHandle);

	return 0;
}

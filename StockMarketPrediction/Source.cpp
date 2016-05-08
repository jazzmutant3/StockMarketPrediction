#include "StockMarketPrediction.h"
#include "ArtificialNeuralNetwork.h"
#include <direct.h>

using namespace std;

SQLHENV environmentHandle; SQLHDBC databaseConnectionHandle;

double Evaluate(NeuralNetwork * nn, vector<vector<Col<double>>> testData)
{
	vector<SQLStock> mean = ReceiveDatabaseQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[Mean] WHERE [StockName] = '%s'", "Apple Inc. (AAPL)"));
	vector<SQLStock> stdDev = ReceiveDatabaseQuery(databaseConnectionHandle, InsertToString("SELECT * FROM [EconomicData].[dbo].[StdDev] WHERE [StockName] = '%s'", "Apple Inc. (AAPL)"));

	stringstream ss;

	ss << "Results\\Apple1_HiddenLayers-15,15,15_LearningRate-0.01_MiniBatchSize-10\\Output-" << nn->Epoch << ".csv";

	ofstream file(ss.str());
	 
	file << "Predicted Price (Real),Actual Price (Real),Error (Real),Predicted Price (Normalized),Actual Price (Normalized),Predicted Percent Change (Real),Actual Percent Change (Real),Predicted Percent Change (Normalized),Actual Percent Change (Normalized)" << endl;

	long double meanError = 0;
	for (int trainingExample = 0; trainingExample < testData.size(); trainingExample++)
	{
		Col<double> result = nn->FeedForward(testData[trainingExample][0]);

		if (((testData[trainingExample][1](0, 0) * 2.0) - 1.0) != 0.0)
		{
			meanError += (((testData[trainingExample][1](0, 0) * 2.0) - 1.0) - ((result(0, 0) * 2.0) - 1.0)) / ((testData[trainingExample][1](0, 0) * 2.0) - 1.0);
			file << ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) * ((result(0, 0) * 2.0) - 1.0) + ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) << "," << ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) * ((testData[trainingExample][1](0, 0) * 2.0) - 1.0) + ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) << "," << (((testData[trainingExample][1](0, 0) * 2.0) - 1.0) - ((result(0, 0) * 2.0) - 1.0)) / ((testData[trainingExample][1](0, 0) * 2.0) - 1.0) << "," << testData[trainingExample][0](3, 0) * result(0, 0) + testData[trainingExample][0](3, 0) << "," << testData[trainingExample][0](3, 0) * testData[trainingExample][1](0, 0) + testData[trainingExample][0](3, 0) << "," << (result(0, 0) * 2.0) - 1.0 << "," << (testData[trainingExample][1](0, 0) * 2.0) - 1.0 << "," << result(0, 0) << "," << testData[trainingExample][1](0, 0) << endl;
		}
		else
			file << ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) * ((result(0, 0) * 2.0) - 1.0) + ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) << "," << ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) * ((testData[trainingExample][1](0, 0) * 2.0) - 1.0) + ((testData[trainingExample][0](3, 0) * stdDev[0].AdjustedClose) + mean[0].AdjustedClose) << "," << "NULL" << "," << testData[trainingExample][0](3, 0) * result(0, 0) + testData[trainingExample][0](3, 0) << "," << testData[trainingExample][0](3, 0) * testData[trainingExample][1](0, 0) + testData[trainingExample][0](3, 0) << "," << (result(0, 0) * 2.0) - 1.0 << "," << (testData[trainingExample][1](0, 0) * 2.0) - 1.0 << "," << result(0, 0) << "," << testData[trainingExample][1](0, 0) << endl;
	}

	meanError /= testData.size();

	return meanError;
}

int main()
{
	//Connect To Database
	ConnectToDatabase("SQL Server", "DESKTOP-38HSR7V", "EconomicData", &environmentHandle, &databaseConnectionHandle);

	//Fill database with stock data
	//PopulateStockDatabase(databaseConnectionHandle);

	//Create Output Folder
	_mkdir("Results\\Apple1_HiddenLayers-15,15,15_LearningRate-0.01_MiniBatchSize-10");

	//Initialize Neural Network
	vector<int> sizes = { 5, 15, 15, 15, 1 };
	NeuralNetwork nn(sizes, &Evaluate);
	nn.Filename = "Results\\Apple1_HiddenLayers-15,15,15_LearningRate-0.01_MiniBatchSize-10\\Apple1_HiddenLayers-15,15,15_LearningRate-0.01_MiniBatchSize-10.nnd";

	//Collect Training and Test Data
	vector<vector<Col<double>>> trainingData, testData;
	GetTrainingAndTestData(databaseConnectionHandle, "Apple Inc. (AAPL)", 10, &trainingData, &testData);

	//Train Neural Network
	nn.StochasticGradientDescent(trainingData, 200, 10, 0.01, testData);

	//Disconnect From Database
	DisconnetFromDatabase(&environmentHandle, &databaseConnectionHandle);

	return 0;
}

#ifndef ARTIFICIAL_NEURAL_NETWORK_HEADER
#define ARTIFICIAL_NEURAL_NETWORK_HEADER

#include <armadillo>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>

using namespace std;
using namespace arma;

Col<double> ActiviationFunction(Col<double> z)
{
	return 1.0 / (1.0 + exp(-z));
}
Col<double> ActivationFunctionPrime(Col<double> z)
{
	return ActiviationFunction(z) % (1 - ActiviationFunction(z));
}

class NeuralNetwork
{
public:
	NeuralNetwork(vector<int> sizes, double(*Evaluate)(NeuralNetwork *, vector<vector<Col<double>>>))
	{
		arma_rng::set_seed((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());

		this->NumberOfLayers = (int)sizes.size();
		this->sizes = sizes;
		for (int l = 1; l < this->NumberOfLayers; l++)
		{
			this->b.push_back(Col<double>(this->sizes[l], fill::randn));
			this->empty_b.push_back(Col<double>(this->sizes[l], fill::zeros));
			this->w.push_back(Mat<double>(this->sizes[l], this->sizes[l - 1], fill::randn));
			this->empty_w.push_back(Mat<double>(this->sizes[l], this->sizes[l - 1], fill::zeros));
		}

		this->Evaluate = Evaluate;
		this->Epoch = 0;
	}
	NeuralNetwork(string filename, double(*Evaluate)(NeuralNetwork *, vector<vector<Col<double>>>))
	{
		this->Filename = filename;

		ifstream ifile(this->Filename, ios::binary);

		ifile.read((char *)&this->NumberOfLayers, sizeof(int));

		for (int l = 0; l < this->NumberOfLayers; l++)
		{
			int temp;
			ifile.read((char *)&temp, sizeof(int));
			this->sizes.push_back(temp);
		}

		for (int l = 0; l < this->NumberOfLayers - 1; l++)
		{
			this->b.push_back(Col<double>(this->sizes[l + 1]));
			for (int j = 0; j < this->sizes[l + 1]; j++)
				ifile.read((char *)&this->b[l](j), sizeof(double));
			this->empty_b.push_back(Col<double>(this->sizes[l + 1], fill::zeros));
		}

		for (int l = 0; l < this->NumberOfLayers - 1; l++)
		{
			this->w.push_back(Mat<double>(this->sizes[l + 1], this->sizes[l]));
			for (int j = 0; j < this->sizes[l + 1]; j++)
				for (int k = 0; k < this->sizes[l]; k++)
					ifile.read((char *)&this->w[l](j, k), sizeof(double));
			this->empty_w.push_back(Mat<double>(this->sizes[l + 1], this->sizes[l], fill::zeros));
		}

		ifile.close();

		this->Evaluate = Evaluate;
		this->Epoch = 0;
	}
	Col<double> FeedForward(Col<double> a)
	{
		for (int l = 0; l < this->b.size(); l++)
			a = ActiviationFunction((this->w[l] * a) + this->b[l]);

		return a;
	}
	void StochasticGradientDescent(vector<vector<Col<double>>> trainingData, int epochs, int miniBatchSize, double eta, vector<vector<Col<double>>> testData)
	{
		this->Epoch = 0;

		double bestResult;

		if (testData.size() > 0)
		{
			bestResult = this->Evaluate(this, testData);
			this->Save();
			cout << "Epoch " << this->Epoch << ": " << (double)bestResult << " {Saved}" << endl;
		}

		int n = (int)trainingData.size();
		for (this->Epoch = 1; this->Epoch <= epochs; this->Epoch++)
		{
			vector<vector<vector<Col<double>>>> miniBatches = this->SplitIntoMiniBatches(trainingData, miniBatchSize);

			for (int miniBatch = 0; miniBatch < miniBatches.size(); miniBatch++)
			{
				vector<Col<double>> nabla_b = this->empty_b;
				vector<Mat<double>> nabla_w = this->empty_w;

				for (int trainingExample = 0; trainingExample < miniBatches[miniBatch].size(); trainingExample++)
				{
					vector<Col<double>> delta_nabla_b = this->empty_b;
					vector<Mat<double>> delta_nabla_w = this->empty_w;

					this->Backpropogation(miniBatches[miniBatch][trainingExample][0], miniBatches[miniBatch][trainingExample][1], &delta_nabla_b, &delta_nabla_w);

					for (int l = 0; l < nabla_b.size(); l++)
					{
						nabla_b[l] = nabla_b[l] + delta_nabla_b[l];
						nabla_w[l] = nabla_w[l] + delta_nabla_w[l];
					}
				}

				for (int l = 0; l < this->b.size(); l++)
				{
					this->b[l] = b[l] - (eta / miniBatches[miniBatch].size()) * nabla_b[l];
					this->w[l] = w[l] - (eta / miniBatches[miniBatch].size()) * nabla_w[l];
				}
			}

			if (testData.size() > 0)
			{
				double result = this->Evaluate(this, testData);
				cout << "Epoch " << this->Epoch << ": " << (double)result;
				if (abs(result) < abs(bestResult))
				{
					bestResult = result;
					this->Save();
					cout << " {Saved}" << endl;
				}
				else
					cout << " {NotSaved}" << endl;
			}
			else
				cout << "Epoch " << this->Epoch << " complete.";
		}
	}
	void Save()
	{
		ofstream ofile(this->Filename, ios::binary);

		ofile.write((char *)&this->NumberOfLayers, sizeof(int));

		for (int l = 0; l < this->NumberOfLayers; l++)
			ofile.write((char *)&this->sizes[l], sizeof(int));

		for (int l = 0; l < this->NumberOfLayers - 1; l++)
			for (int j = 0; j < this->sizes[l + 1]; j++)
				ofile.write((char *)&this->b[l](j), sizeof(double));

		for (int l = 0; l < this->NumberOfLayers - 1; l++)
			for (int j = 0; j < this->sizes[l + 1]; j++)
				for (int k = 0; k < this->sizes[l]; k++)
					ofile.write((char *)&this->w[l](j, k), sizeof(double));

		ofile.close();
	}
	double(*Evaluate)(NeuralNetwork *, vector<vector<Col<double>>>);
	string Filename;
	int Epoch;

private:
	int NumberOfLayers;
	vector<int> sizes;
	vector<Col<double>> b;
	vector<Mat<double>> w;
	vector<Col<double>> empty_b;
	vector<Mat<double>> empty_w;
	void Backpropogation(Col<double> x, Col<double> y, vector<Col<double>> * delta_nabla_b, vector<Mat<double>> * delta_nabla_w)
	{
		Col<double> a = x;
		vector<Col<double>> as; as.push_back(a);
		Col<double> z;
		vector<Col<double>> zs;

		for (int l = 0; l < this->b.size(); l++)
		{
			z = (this->w[l] * a) + this->b[l];
			zs.push_back(z);
			a = ActiviationFunction(z);
			as.push_back(a);
		}

		Col<double> delta = (as[as.size() - 1] - y) % ActivationFunctionPrime(zs[zs.size() - 1]);
		(*delta_nabla_b)[delta_nabla_b->size() - 1] = delta;
		(*delta_nabla_w)[delta_nabla_w->size() - 1] = delta * as[as.size() - 2].t();

		for (int l = (int)delta_nabla_b->size() - 2; l >= 0; l--)
		{
			delta = (this->w[l + 1].t() * delta) % ActivationFunctionPrime(zs[l]);
			(*delta_nabla_b)[l] = delta;
			(*delta_nabla_w)[l] = delta * as[l].t();
		}
	}
	vector<vector<vector<Col<double>>>> SplitIntoMiniBatches(vector<vector<Col<double>>> trainingData, int miniBatchSize)
	{
		std::default_random_engine random;
		random.seed((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<int> discrete(0, (int)trainingData.size() - 1);

		vector<vector<vector<Col<double>>>> Output;

		for (int index = 0; index < trainingData.size() - 1; index++)
			swap(trainingData[discrete(random)], trainingData[index]);

		int NumberOfMiniBatches = (int)trainingData.size() / miniBatchSize;
		for (int MiniBatchIndex = 0; MiniBatchIndex < NumberOfMiniBatches; MiniBatchIndex++)
		{
			Output.push_back(vector<vector<Col<double>>>());
			for (int i = MiniBatchIndex; i < MiniBatchIndex + miniBatchSize; i++)
				Output[MiniBatchIndex].push_back(trainingData[i]);
		}

		return Output;
	}
};

#endif
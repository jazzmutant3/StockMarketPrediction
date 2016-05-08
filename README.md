# StockMarketPrediction

This is a project that I spent my junior year of high school working on. The purpose of this project was to take historical stock data for any given stock in the S&P 500 and use that data to train an artificial neural network to use today's open, high, low, close, and volume to predict tomorrow's closing price.

After a significant amount of research, I wrote a (mostly) reusable ArtificialNeuralNetwork class that I would use in this project to create a resizable artificial neural network so I could run different tests with the stock input and output data. This ArtificialNeuralNetwork class works very similarly to the one described by Michael Nielsen in his [Neural Networks and Deep Learning](http://neuralnetworksanddeeplearning.com/) book (translated from Python to C++).

In order to feed the data into the artificial neural network, I first needed to normalize the data. I normalized the input data using [z-scores](https://en.wikipedia.org/wiki/Standard_score) and the output data using min-max normalization. Unfortunately, this normalization process had negative effects on the results of the project.

After experimentation, I reached the best results with the above normalization procedure and having a single output: percent change in closing price. Any other output had significantly more error.

However, since the artificial neural network is trying to match the normalized input with the normalized output, it is completely unaware that this normalization process introduces more error to the prediction. As you can see below, the first image is the normalized closing price. The actual normalized closing price is shown in red and the predicted normalized closing price is shown in blue. This graph displays the daily predicted close for the first 365 business days following the training data after it was trained through 75 epochs with three hidden layers each containing 15 hidden neurons:

![alt tag](https://raw.githubusercontent.com/rbdurfee/StockMarketPrediction/master/Normalized.jpg)

And the second image is the real closing price (after the normalization process is reversed):

![alt tag](https://raw.githubusercontent.com/rbdurfee/StockMarketPrediction/master/Real.jpg)

There is significantly more error introduced after the normalization process is reversed. As a result, I would greatly appreciate anyone's input on minimizing this error. This coming summer, I plan to alter the calculus involved in the ArtificialNeuralNetwork class to see if I can get the artificial neural network to become aware of this additional error.

If you wish to work with my code yourself, feel free! I used Visual Studio 2015 to develop this project and I uploaded it using the Team Explorer. As long as you run the code on an x64 computer (the matrix multiplication libraries only work on x64 architectures), everything should work right out of the box with no additional setup required. However, my database will not always be up and running, unfortunately, as I don't have the resources to remain online 24/7. My apologies. You can always use the PopulateStockDatabase command to fill your own database, but be aware that this may take a while depending on your computer and Internet connection speed.

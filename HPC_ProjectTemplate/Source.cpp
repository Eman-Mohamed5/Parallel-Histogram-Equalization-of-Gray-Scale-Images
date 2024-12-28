#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;
	int OriginalImageWidth, OriginalImageHeight;

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int* Red = new int[BM.Height * BM.Width];
	int* Green = new int[BM.Height * BM.Width];
	int* Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i * width + j] < 0)
			{
				image[i * width + j] = 0;
			}
			if (image[i * width + j] > 255)
			{
				image[i * width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}



void seq() {
	cout << "hello";
	int ImageWidth = 4, ImageHeight = 4;

	int start_s, stop_s, TotalTime = 0;
	System::String^ imagePath;
	std::string img;
	img = "..//Data//Input//5N.png";
	imagePath = marshal_as<System::String^>(img);
	start_s = clock();
	int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
	float arr[256];
	for (int i = 0; i < 256; i++) {
		arr[i] = 0;
	}
	for (int i = 0; i < ImageWidth*ImageHeight ; i++) {
		arr[imageData[i]]++;
	}
	float cuml[256];
	float total_n_of_pixels = ImageWidth*ImageHeight;
	for (int i = 0; i < 256; i++) {
		arr[i] = arr[i] / total_n_of_pixels;
		if (i == 0) {
			cuml[i] = arr[i];
		}
		else {
			cuml[i] = cuml[i - 1] + arr[i];
		}
	}
	int x;
	for (int i = 0; i < 256; i++) {
		x = cuml[i] * 256;
		cuml[i] = x;
	}

	int* fi;
	fi = new int[ImageWidth*ImageHeight];

	for (int i = 0; i < ImageWidth*ImageHeight; i++) {
		for (int ind = 0; ind < 256; ind++) {
			if (imageData[i] == ind) {
				fi[i] = cuml[ind];
			}
		}
	}
	
	createImage(fi, ImageWidth, ImageHeight, 0);
	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC);
	cout << "time: " << TotalTime << endl;
	free(imageData);
	system("pause");


}
int main()
{
	seq();
}



void par() {
	MPI_Init(NULL, NULL);
	int size;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int* input = NULL;
	int ImageWidth = 4, ImageHeight = 4;
	float freqArrAt0[256];
	int start_s, stop_s, TotalTime = 0;

	if (rank == 0)
	{


		System::String^ imagePath;
		std::string img;
		img = "..//Data//Input//download.jpg";

		imagePath = marshal_as<System::String^>(img);
		input = inputImage(&ImageWidth, &ImageHeight, imagePath);
		start_s = clock();

	}
	MPI_Bcast(&ImageHeight, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&ImageWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);

	float freqArr[256];
	int* Allfinres = new int[ImageHeight * ImageWidth];
	int Crange = 256;
	for (int i = 0; i < 256; i++)
	{
		freqArr[i] = 0;
	}
	//cout << BM.Width << "width " << BM.Height << "height\n";

	float* local_freq_arr = new float[256];

	float culsum;

	int* finres = new int[int(ImageHeight * ImageWidth) / size];
	int* local_arr = new int[(ImageHeight * ImageWidth) / size];

	MPI_Scatter(input, int(ImageHeight * ImageWidth) / size, MPI_INT, local_arr, int(ImageHeight * ImageWidth) / size, MPI_INT, 0, MPI_COMM_WORLD);
	for (long j = 0; j < (ImageHeight * ImageWidth) / size; j++)
	{
		//cout << input[i * BM.Width + j];
		//cout << "h";
		freqArr[local_arr[j]]++;
	}
	MPI_Reduce(freqArr, freqArrAt0, 256, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		float sum = 0;
		for (int i = 0; i < Crange; i++)
		{
			freqArrAt0[i] /= (ImageHeight * ImageWidth);
		}
		for (int i = 0; i < Crange; i++)
		{
			sum += freqArrAt0[i];
			freqArrAt0[i] = sum;
		}
		for (int i = 0; i < Crange; i++)
		{
			freqArrAt0[i] *= 256;
			freqArrAt0[i] = round(freqArrAt0[i]);

		}
	}
	MPI_Bcast(freqArrAt0, 256, MPI_FLOAT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < int(ImageHeight * ImageWidth) / size; i++)
	{
		finres[i] = freqArrAt0[local_arr[i] - 1];
	}
	MPI_Gather(finres, int(ImageHeight * ImageWidth) / size, MPI_INT, Allfinres, int(ImageHeight * ImageWidth) / size, MPI_INT, 0, MPI_COMM_WORLD);
	//MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
	{
		//	Allfinres = Allfinres;
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time in parralel: " << TotalTime << endl;
		cout << "size is " << ImageHeight << " * " << ImageWidth;

		createImage(Allfinres, ImageWidth, ImageHeight, 0);
		free(input);
	}
	MPI_Finalize()
}
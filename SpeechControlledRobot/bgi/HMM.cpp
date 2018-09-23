#include<iostream>
#include<stdlib.h>
#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>


//using namespace std;

#include "graphics.h"
#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define STND 4
#define min(a,b) (a<b?a:b)


using namespace std;
double PI=3.14159265358979323846;
int P=12;
double new_sample[];


double DTW();
void LPC(string refFile,string testFile);
double TokhuraDistance(double ref[],double test[]);
void applySineWindow(double W[]);
void computeCi(double Ai[],double Ci[]);
void computeAi(double Ri[],double Ai[]);
void computeRi(double *arr, double Ri[],int fStart,int end);
int readTest(string filename);
const int N=5;
const int M=8;
const int T_c=160;
//const int P=12;
int T; 
int readReference(string filename);
void frame_block(double sample[],int &size);
void DCShiftCorrection(double *arr, int size);
void Normalization(double *arr,int size);
long double pi[N]={1,0,0,0,0};
long double A[N][N];//={{0.8,0.2,0,0,0},{0,0.8,0.2,0,0},{0,0,0.8,0.2,0},{0,0,0,0.8,0.2},{0,0,0,0,1}};
long double B[N][M];//={{0.125,0.125,0.125,0.125,0.125,0.125,0.125,0.125},{0.125,0.125,0.125,0.125,0.125,0.125,0.125,0.125},{0.125,0.125,0.125,0.125,0.125,0.125,0.125,0.125},{0.125,0.125,0.125,0.125,0.125,0.125,0.125,0.125},{0.125,0.125,0.125,0.125,0.125,0.125,0.125,0.125}};
long double Alpha[N][T_c]={0.0};
long double Beta[N][T_c]={0.0};
long double Gamma[N][T_c]={0.0};
long double Delta[N][T_c]={0.0};
int Psi[N][T_c]={0};
long double Zhi[N][N][T_c]={0.0};
int O[T_c];
long double p_star;
int q_star[T_c];

void HMM_train();
void random();
long double forward_procedure();
void backward_procedure();
void Gamma_comp();
long double viterbi_algo();
void Baum_welch();
void adjustB();



void left_fn(int prev);
void right_fn(int prev);
void forward(int prev);
void backward(int prev);
void move(int prev);
void stop();
int a=450,b=450,c=450+25,d=450+25;
int state=4;


long double tempp[65]={0};
ofstream myfile1("a_vector.txt", std::ios_base::out);  //to save p*, q*, A, B for each iteration
ifstream myfile2("address.txt",std::ios_base::in);   // to save file addresses of sounds
ifstream myfile3("matrices.txt",std::ios_base::in);  // to save model after every instance of training
ofstream myfile4("converged.txt", std::ios_base::out); // to save model for all iteration 
ifstream myfile5("converged.txt", std::ios_base::in); // to read model of all iteration
ifstream myfile6("address_test.txt",std::ios_base::in);   // to save file addresses of sounds


int main( )
{

    initwindow(900, 900, "First Sample");


	string left[]={"left_1.txt","left_2.txt","left_3.txt","left_4.txt","left_5.txt","left_6.txt","left_7.txt","left_8.txt","left_9.txt","left_10.txt","left_11.txt","left_12.txt","left_13.txt","left_14.txt","left_15.txt","left_16.txt","left_17.txt","left_18.txt","left_19.txt","left_20.txt"};
	string right[]={"right_1.txt","right_2.txt","right_3.txt","right_4.txt","right_5.txt","right_6.txt","right_7.txt","right_8.txt","right_9.txt","right_10.txt","right_11.txt","right_12.txt","right_13.txt","right_14.txt","right_15.txt","right_16.txt","right_17.txt","right_18.txt","right_19.txt","right_20.txt"};
	string move[]={"move_1.txt","move_2.txt","move_3.txt","move_4.txt","move_5.txt","move_6.txt","move_7.txt","move_8.txt","move_9.txt","move_10.txt","move_11.txt","move_12.txt","move_13.txt","move_14.txt","move_15.txt","move_16.txt","move_17.txt","move_18.txt","move_19.txt","move_20.txt"};
	string back[]={"back_1.txt","back_2.txt","back_3.txt","back_4.txt","back_5.txt","back_6.txt","back_7.txt","back_8.txt","back_9.txt","back_10.txt","back_11.txt","back_12.txt","back_13.txt","back_14.txt","back_15.txt","back_16.txt","back_17.txt","back_18.txt","back_19.txt","back_20.txt"};
	while(true){
		system("Recording_Module.exe 2 test.wav test.txt");
		string testfname="test.txt";
		double left_avg=0,right_avg=0,move_avg=0,back_avg=0;
		for(int i=0;i<1;i++)
		{
			LPC(left[i],testfname);
			left_avg=left_avg+DTW();
		}
		for(int i=0;i<1;i++)
		{
			LPC(right[i],testfname);
			right_avg=right_avg+DTW();
		}
		for(int i=0;i<1;i++)
		{
			LPC(back[i],testfname);
			back_avg=back_avg+DTW();
		}
		for(int i=0;i<1;i++)
		{
			LPC(move[i],testfname);
			move_avg=move_avg+DTW();
		}
		cout << left_avg << " " << right_avg << " " << move_avg << " " << back_avg << endl;
		double allavg[4]={left_avg,right_avg,move_avg,back_avg};
		int min=allavg[0],ind=0;
		for(int i=1;i<4;i++)
			if(min > allavg[i])
			{
				min=allavg[i];
				ind=i;
			}
	
	
		
			switch(ind){
			case 0:
				left_fn(state);
				break;
			case 1:
				right_fn(state);
				break;
			case 2:
				forward(state);
				break;
			case 3:
				backward(state);
				break;
			case 4:
				stop();
				break;
			default:
				break;

			}
			//cin>>x;
	}
	

    printf("Hello");    
    return 0;
}
void right_fn(int prev){
	int cnt=0;
     switch(prev){
	     case STND:
		 case UP:
			  cnt=100;
			  while(c<=900 && cnt--){
				   cleardevice();
			       rectangle(a++,b,c++,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   
				   delay(10);
				   
			  }
			  state=1;
		 break;
		 case RIGHT:
			  cnt=100;
			  while(d<=900 && cnt--){
				  cleardevice();
			       rectangle(a,b++,c,d++);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=3;
		 break;
		 case LEFT:
			 cnt=100;
			 while(b>=0 && cnt--){
			       cleardevice();
				   rectangle(a,b--,c,d--);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=2;
		 break;
		 case DOWN:
			 cnt=100;
			 while(a>=0 && cnt--){
			       cleardevice();
				   rectangle(a--,b,c--,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=LEFT;
		 break;
	 }
}

void left_fn(int prev){
	int cnt=0;
     switch(prev){
	     case STND:
			 cnt=100;
			 while(a>=0 && cnt--){
			       cleardevice();
				   rectangle(a--,b,c--,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=LEFT;
		 break;
		 case RIGHT:
			 cnt=100;
			 while(b>=0 && cnt--){
				   cleardevice();
			       rectangle(a,b--,c,d--);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=UP;
		 break;
		 case LEFT:
			  cnt=100;
			  while(d<=900 && cnt--){
			       cleardevice();
				   rectangle(a,b++,c,d++);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=DOWN;
		 break;
		 case UP:
			 cnt=100;
			 while(a>=0 && cnt--){
			       cleardevice();
				   rectangle(a--,b,c--,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=LEFT;
		 break;
		 case DOWN:
			 cnt=100;
			  while(c<=900 && cnt--){
			       cleardevice();
				   rectangle(a++,b,c++,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=RIGHT;
		 break;
	 }
}

void forward(int prev){
	int cnt=0;
     switch(prev){
	     case STND:
			 cnt=100;
			 while(b>=0 && cnt--){
			       cleardevice();
				   rectangle(a,b--,c,d--);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   
				   delay(10);
				   //cleardevice();
			 }
			 state=UP;
		 break;
		 case UP:
			  cnt=100;
			 while(b>=0 && cnt--){
			       cleardevice();
				   rectangle(a,b--,c,d--);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=UP;
		 break;
		 case RIGHT:
			  cnt=100;
			  while(c<=900 && cnt--){
			       cleardevice();
				   rectangle(a++,b,c++,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=RIGHT;
		 break;
		 case LEFT:
			 cnt=100;
			 while(a>=0 && cnt--){
			       cleardevice();
				   rectangle(a--,b,c--,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=LEFT;
		 break;
		 case DOWN:
			 cnt=100;
			  while(d<=900 && cnt--){
			       cleardevice();
				   rectangle(a,b++,c,d++);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=DOWN;
		 break;
	 }
}

void backward(int prev){
	int cnt=0;
     switch(prev){
	     case STND:
			 cnt=100;
			  while(d<=900 && cnt--){
			       cleardevice();
				   rectangle(a,b++,c,d++);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=DOWN;
		 break;
		 case UP:
			  cnt=100;

			  while(d<=900 && cnt--){
			       cleardevice();
				   rectangle(a,b++,c,d++);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=DOWN;
		 break;
		 case RIGHT:
			  cnt=100;
			 while(a>=0 && cnt--){
			       cleardevice();
				   rectangle(a--,b,c--,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=LEFT;
		 break;
		 case LEFT:
			 cnt=100;
			  while(c<=900 && cnt--){
			       cleardevice();
				   rectangle(a++,b,c++,d);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			  }
			  state=RIGHT;
		 break;
		 case DOWN:
			 cnt=100;
			 while(b>=0 && cnt--){
			       cleardevice();
				   rectangle(a,b--,c,d--);
				   rectangle(a+8,b-10,c-8,d-20);rectangle(a+2,b+25,a+9,b+40);rectangle(a+2+13,b+25,a+9+13,b+40);
				   delay(10);
				   //cleardevice();
			 }
			 state=UP;
		 break;
	 }
}

void move(int prev){
	forward(prev);
}

void stop(){
	return;
}









//#include "stdafx.h"

void Normalization(double *arr,int size)				// Normalization of samples
{
	double max=0;
	for (int i = 0; i < size; i++)
	{
		if (fabs(arr[i])>max)
			max = fabs(arr[i]);
	}
	
	max = 5000 / max;
	for (int i = 0; i < size; i++)
		arr[i] = arr[i] * max;	
}
void DCShiftCorrection(double *arr, int size)		//DC Shift correction
{
	long long int sum=0;
	double average;
	for (int i = 0; i < size; i++)
		sum = sum + (int)arr[i];
	average = (double)sum / size;
	for (int i = 0; i < size; i++)
		arr[i] =double( (double)arr[i] - average);
}
void frame_block(double sample[],int &size)
{   vector<double> temp;
	ofstream outputFile;
	outputFile.open("test5.txt", ios::out);
     int max=0;
	 int st=0,ed=size;
	 for(int i=0;i<size;i++){
	      if(sample[i]<=-700||sample[i]>=700){
		       st=i;
			   break;
		  }
	 }
	 //st=st-500;
	 for(int i=size-1;i>=0;i--){
	      if(sample[i]<=-700||sample[i]>=700){
		       ed=i;
			   break;
		  }
	 }
	 ed=ed+1000;
	 for(int i=0;i<size;i++){
	     if(i>=st&&i<=ed)
		 {   temp.push_back(sample[i]);
			 outputFile << sample[i] <<endl;
		 }
	 }
	 for(int i=0;i<temp.size();i++)
		  sample[i]=temp[i];
	 size=temp.size();
	outputFile.close();
}
double *ref;
int readReference(string filename)
{
	string word;
	ifstream inputFile;
	inputFile.open(filename, ios::in);
	if (inputFile.is_open())					//checking whether file is opened or not
	{
		int nSamples = 0,index=0;
		while (getline(inputFile,word))
			nSamples++;
		inputFile.close();
		inputFile.open(filename, ios::in);
		ref=new double[nSamples];
		while (getline(inputFile,word))							//reading samples from text to array(inputSamples)
			ref[index++] =(double) atoi(word.c_str());
		DCShiftCorrection(ref,nSamples);
		Normalization(ref,nSamples);
		inputFile.close();
		
		return nSamples;
	}
	else
	{
		cout << "Unalbe to open the reference file" <<endl;
		return 0;
	}
}
double *test;
int readTest(string filename)
{
	string word;
	ifstream inputFile;
	inputFile.open(filename, ios::in);
	if (inputFile.is_open())					//checking whether file is opened or not
	{
		int nSamples = 0,index=0;
		while (getline(inputFile,word))
			nSamples++;
		inputFile.close();
		inputFile.open(filename, ios::in);
		test=new double[nSamples];
		while (getline(inputFile,word))							//reading samples from text to array(inputSamples)
			test[index++] =(double) atoi(word.c_str());
		DCShiftCorrection(test,nSamples);
		Normalization(test,nSamples);
		frame_block(test,nSamples);
		cout << "size: " <<nSamples << endl;
		inputFile.close();
		return nSamples;
	}
	else
	{
		cout << "Unalbe to open the test file" <<endl;
		return 0;
	}
}
void computeRi(double *arr, double Ri[],int fStart,int end)
{
	for(int Ri_ind=0;Ri_ind<P+1;Ri_ind++)
	{
		for(int i=fStart;i<(fStart+320-Ri_ind) && (fStart+320-Ri_ind)<=end;i++)
			Ri[Ri_ind]=Ri[Ri_ind]+arr[i]*arr[i+Ri_ind];
	}		
}
void computeAi(double Ri[],double Ai[])		//computing Ai's
{
	double E=Ri[0];
	double ai[13][13]={};
	for(int i=1;i<P+1;i++)
	{
		ai[i][i]=Ri[i];
		for(int j=1;j<i;j++)
			ai[i][i]=ai[i][i]-ai[j][i-1]*Ri[i-j];
		ai[i][i]=ai[i][i]/E;
		E=(1-ai[i][i]*ai[i][i])*E;
			
		for(int j=1;j<i;j++)
			ai[j][i]=ai[j][i-1]-ai[i][i]*ai[i-j][i-1];
	}
	for(int i=1;i<P+1;i++)
		Ai[i]=ai[i][P];
}
void computeCi(double Ai[],double Ci[])	//computing Ci's
{
	
	for(int i=1;i<P+1;i++)
	{
		Ci[i]=Ai[i];
		for(int k=1;k<i;k++)
			Ci[i]=Ci[i]+(double(k)/i)*Ci[k]*Ai[i-k];
	}
}
void applySineWindow(double W[])	//applying sinsodial window
{
	for(int i=1;i<P+1;i++)
		W[i]=W[i]*(1+6*sin((double)(PI*i)/P));
}
double TokhuraDistance(double ref[],double test[])	//calculating tokhura distance
{
	int w[]={0,1,3,7,13,19,22,25,33,42,50,56,61};
	double dist=0;
	for(int i=1;i<P+1;i++)
		dist=dist+w[i]*((test[i]-ref[i])*(test[i]-ref[i]));
	return dist;
}

double **refCapstral,**testCapstral;
int refFrames,testFrames,refEnd,testEnd;
void LPC(string refFile,string testFile)
{
	refEnd=readReference(refFile);
	testEnd=readTest(testFile);
	refFrames=1+(refEnd-320)/80;
	testFrames=1+(testEnd-320)/80;
	refCapstral=new double*[refFrames];
	testCapstral=new double*[testFrames];
	for(int i=0;i<refFrames;i++)
		refCapstral[i]=new double[P];
	for(int i=0;i<testFrames;i++)
		testCapstral[i]=new double[P];
	//cout << "ref " << refEnd << " " << refFrames << endl;
	//cout << "test " << testEnd << " " << testFrames << endl;
	for(int i=0;i+320<=refEnd;i=i+80)
	{
		double Ri[13]={0},Ai[13]={0},Ci[13]={0};
		computeRi(ref,Ri,i,refEnd);
		computeAi(Ri,Ai);
		computeCi(Ai,Ci);
		applySineWindow(Ci);
		for(int j=0;j<P;j++)
			refCapstral[i/80][j]=Ci[j+1];
	}
	for(int i=0;i+320<=testEnd;i=i+80)
	{
		double Ri[13]={},Ai[13]={},Ci[13]={};
		computeRi(test,Ri,i,testEnd);
		computeAi(Ri,Ai);
		computeCi(Ai,Ci);
		applySineWindow(Ci);
		for(int j=0;j<P;j++)
			testCapstral[i/80][j]=Ci[j+1];
	}
}

double DTW()
{
	double **Distance=new double*[testFrames];
	for(int i=0;i<testFrames;i++)
		Distance[i]=new double[refFrames];
	for(int i=1;i<testFrames;i++)
		Distance[i][0]=(double)INT_MAX;
	for(int i=1;i<refFrames;i++)
		Distance[0][i]=(double)INT_MAX;
	Distance[0][0]=0;
	double cost;
	for(int i=1;i<testFrames;i++)
	{
		for(int j=1;j<refFrames;j++)
		{
			cost=TokhuraDistance(refCapstral[j],testCapstral[i]);
			Distance[i][j]=cost+min(min(Distance[i-1][j-1],Distance[i-1][0]),Distance[0][j-1]);
		}
	}
	/*for(int j=refFrames-1;j>=0;j--)
	{
		for(int i=0;i<testFrames;i++)
			cout << Distance[i][j] << " ";
		cout << endl;
	}*/
	return Distance[testFrames-1][refFrames-1];
}


void HMM_train(){
	 long double p_star=0.0;
	 ifstream myfile3("matrices.txt",std::ios_base::in);
	 for(int i=0; i<N; i++){
		for(int j=0; j<N; j++)
			myfile3>>A[i][j];
	}
	for(int i=0; i<N; i++){
		for(int j=0; j<M; j++)
			myfile3>>B[i][j];
	}
	myfile3.close();
	 
	 do {
		long double v1=forward_procedure();
		backward_procedure();
		Gamma_comp();
		p_star=viterbi_algo();
		Baum_welch();      
		cout<<"A matrix:"<<endl;
		for(int i=0; i<N; i++){
			for(int j=0; j<N; j++)
				cout<<A[i][j]<<" ";
			cout<<endl;
		}
		cout<<endl;
		cout<<"B matrix:"<<endl;
		for(int i=0; i<N; i++){
			for(int j=0; j<M; j++)
				cout<<B[i][j]<<" ";
			cout<<endl;
		}
		cout<<endl;

		myfile1<<"A matrix:"<<endl;
		for(int i=0; i<N; i++){
			for(int j=0; j<N; j++)
				myfile1<<A[i][j]<<" ";
			myfile1<<endl;
		}
		myfile1<<endl;

		myfile1<<"B matrix:"<<endl;
		for(int i=0; i<N; i++){
			for(int j=0; j<M; j++)
				myfile1<<B[i][j]<<" ";
			myfile1<<endl;
		}
		myfile1<<endl;
	 }while(p_star<1E-100);
	 
	 for(int i=0; i<N; i++){
		for(int j=0; j<N; j++)
			myfile4<<A[i][j]<<" ";
		myfile4<<endl;
	}
	//cout<<endl;
	//cout<<"B matrix:"<<endl;
	for(int i=0; i<N; i++){
		for(int j=0; j<M; j++)
			myfile4<<B[i][j]<<" ";
		myfile4<<endl;
	}

	 
}
// calculation of Alpha and Probalility of obsation given lambda
long double forward_procedure(){
	long double pol=0.0;
	for(int i=0; i<N; i++){
        Alpha[i][0]=pi[i]*B[i][O[0]];
    }
    for(int t=0; t<T-1; t++){
        for(int j=0; j<N; j++){
            long double temp=0.0;
            for(int i=0; i<N; i++)
               temp+=Alpha[i][t]*A[i][j];
            Alpha[j][t+1]=temp*B[j][O[t+1]];
        }
    }
    for(int i=0; i<N; i++){
        pol+=Alpha[i][T-1];
		
	}
	return pol;
}
//calculation of Beta matrix
void backward_procedure(){
	for(int i=0; i<N; i++)
        Beta[i][T-1]=1;
    for(int t=T-2; t>=0; t--){
        for(int i=0; i<N; i++){
            //long double temp=0;
            for(int j=0; j<N; j++){
                Beta[i][t]+=A[i][j]*B[j][O[t+1]]*Beta[j][t+1];
            }
            //Beta[i][t]+=temp;
        }
    }
}
// calucation of gamma matrix
void Gamma_comp(){
	for(int t=0; t<T; t++){
        long double temp=0;
        for(int j=0; j<N; j++){
            temp+=Alpha[j][t]*Beta[j][t];
        }
        for(int i=0; i<N; i++){
            Gamma[i][t]=(Alpha[i][t]*Beta[i][t])/temp;
        }
    }
}
// calculation of Delta, p* and q*
long double viterbi_algo(){
	long double p_star=0.0;
	for(int i=0; i<N; i++){
        Delta[i][0]=pi[i]*B[i][O[0]];
        Psi[i][0]=0;
    }
    for(int t=1; t<T; t++){
        for(int j=0; j<N; j++){
            long double temp=0.0;
            int index=0;
            for(int i=0; i<N; i++){
                if(temp<Delta[i][t-1]*A[i][j]){
                        temp=Delta[i][t-1]*A[i][j];
                        index=i;
                }
            }
            Delta[j][t]=temp*B[j][O[t]];
            Psi[j][t]=index;
        }
    }
    long double temp=0;
    int index=0;
    for(int i=0; i<N; i++){
        if(temp<Delta[i][T-1]){
            temp=Delta[i][T-1];
            index=i;
        }
    }
    p_star = temp;
	myfile1<<p_star<<endl;
    q_star[T-1] = index;
	//myfile1<<q_star[T-1]<<" ";
    for(int t=T-2; t>=0; t--){
        q_star[t]=Psi[q_star[t+1]][t+1];
		//myfile1<<q_star[t]<<" ";
    }
	for(int t=0; t<T; t++){
        //q_star[t]=Psi[q_star[t+1]][t+1];
		myfile1<<q_star[t]<<" ";
    }
	myfile1<<endl;
	return p_star;
}
// calculation of zhi and updation of A and B matrices 
void Baum_welch(){
	for(int t=0; t<T-1; t++){
		  long double sum=0.0;
	      for(int i=0;i<N;i++){
		       for(int j=0;j<N;j++){
			        sum+=Alpha[i][t]*A[i][j]*B[j][O[t+1]]*Beta[j][t+1];
			   }
		  }
		  for(int i=0;i<N;i++){
		       for(int j=0;j<N;j++){
			        Zhi[i][j][t]=(Alpha[i][t]*A[i][j]*B[j][O[t+1]]*Beta[j][t+1])/sum;
			   }
		  }
	 }
	 for(int i=0;i<N;i++){
	     pi[i]=Gamma[i][0];
		 myfile1<<pi[i]<<" ";
	 }
	 myfile1<<endl;
	 for(int i=0;i<N;i++){
	     for(int j=0;j<N;j++){
			 long double d1=0.0,d2=0.0;
		     for(int t=0;t<=T-2;t++){
			     d1+=Zhi[i][j][t];
				 d2+=Gamma[i][t];
			 }
			 A[i][j]=d1/d2;
			 //myfile1<<A[i][j]<<" ";
		 }
		 //myfile1<<endl;
	 }
	 myfile1<<endl;
	 for(int j=0;j<N;j++){
	     for(int k=0;k<M;k++){
		     long double d1=0.0,d2=0.0;
		     for(int t=0;t<=T-2;t++){
			       if(O[t]==k){
				       d1+=Gamma[j][t];
				   }
				   d2+=Gamma[j][t];
			 }
			 B[j][k]=d1/d2;
			 //myfile1<<B[j][k]<<" ";
		 }
		 //myfile1<<endl;
	 }
	 adjustB();
	 //myfile1<<endl;
	/* for(int i=0;i<N;i++){
	      long double sum=0.0;
		  for(int j=0;j<M;j++){
		      sum+=B[i][j];
		  }
		  printf("%Lf\n",sum);
	 }*/

}
// post processing after updating B 
void adjustB()
{
	long double adjval=1.0e-080;
	int maxInd,count;
	for(int i=0;i<N;i++)
	{
		maxInd=0;count=0;
		for(int j=0;j<M;j++)
		{
			if(B[i][j]==0)
			{
				B[i][j]=adjval;
				count++;
			}
			if(B[i][maxInd] < B[i][j])
				maxInd=j;
		}
		B[i][maxInd]=B[i][maxInd]-count*adjval;
	}
}


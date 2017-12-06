# include<stdlib.h>
# include <iostream>
# include <fstream>
# include <cmath>
# include <memory.h>
# include <cstring>
# include "Timer.h"
# include "dirent.h"
# include "hashtable.h"
# include "data_type.h"
# include <time.h> 
//#include <Windows.h>
//#include <process.h>
//#include <ppl.h>
//using namespace Concurrency;
using namespace std;
//using System;
//using System.Threading;

typedef float real_t;
typedef HashTable<_ULonglong,real_t> HashGraph;

int seed = 1;
clock_t total_time = 0;
ofstream fout;


//typedef struct arg
//{
//	int id;
//	int n0;
//	int numThread;
//	double *tQ;
//	real_t *tknm_i;
//	real_t *tkm;
//	real_t k_i;	
//	real_t Wii;
//	int node_i;
//	int Mi;
//	double s0;
//	int *max_index;
//	double *max_Q;
//
//} ARG;

_ULonglong GetKey(_ULonglong x, _ULonglong y)
{
	return ((x<<32) | y);
}

int Compare(const void *elem1,const void *elem2)
{
	return*((int*)(elem1))-*((int*)(elem2));
}

void randperm(int *a, int n)
{	
	srand(seed++);
	for (int i=n-1; i>0; --i)
	{  
		int idx = rand()%i;  
		if (idx == i) continue;  
		int tmp=a[idx];  
		a[idx] = a[i];  
		a[i] = tmp;
	}
}

double maxq(double *x, int n, int *idx)
{
	double max_x = x[0];
	*idx = 0;
	for (int i = 0; i<n; i++)
		if (max_x < x[i])
		{
			max_x = x[i];
			*idx = i;
		}
	return max_x;
}

int unique(int *M, int N)
{	
	int *M1 = new int [N];
	memcpy(M1,M,sizeof(int)*N);
	qsort(M1, N, sizeof(int),Compare);
	int idx = 0;
	for (int i=0; i<N; i++)
	{
		if (i>0 && M1[i]==M1[i-1])
				continue;
		
		for (int j = 0; j < N; j++)
			if (M[j]==M1[i])
				M[j] = idx;
		
		idx++;
	}
	return idx;
}

double Louvain_modularity_sub_parr( real_t* &W, long long *n1, int *Ci, long long N, double s)
{	
	//real_t *W = *W0;
	int i=0,j=0;
	long long n0=*n1;
	double q_gain = 0;
	real_t *k = new real_t [n0];
	memset(k,0,sizeof(real_t)*n0);
	//clock_t time = clock();
	/*parallel_for(long long(0), n0,  [&](long long ii) 
	{	
		double tmp_k = 0;
		for(int jj=0; jj<n0; jj++)
			tmp_k += *(W+ii*n0+jj);
		k[ii] = tmp_k;
	});*/
	for(long long ii=0;ii<n0;++ii)
	{
		double tmp_k = 0;
		for(int jj=0;jj<n0;++jj)
		{
			tmp_k += *(W+ii*n0+jj);
		}
		k[ii] = tmp_k;
	}
	//time = clock() - time;
	//cout<<"time for calc_k = "<<time<<endl;
		
	real_t *km = new real_t [n0];
	memcpy(km, k, sizeof(real_t)*n0);
	/*memset(km,0,sizeof(real_t)*n0);
	//for(i=0; i<n0; i++) cout<<km[i]<<endl;
	for(i=0; i<n0; i++)
	{
		for(j=0; j<n0; j++)
			km[i] += *(W+i*n0+j);	
		//if (km[i] != k[i])
		printf("km[%d] = %f, k[%d] = %f\n",i, km[i], i, k[i]);
	}*/
	real_t *knm = new real_t [n0*n0];
	memcpy(knm, W, sizeof(real_t)*n0*n0);
	
	int *M = new int [n0];
	for (i = 0; i < n0; i++)
		M[i]=i;
		
	int *Nm = new int[n0];
	for (i = 0; i < n0; i++)
		Nm[i]=i;
	
	double *dQ = new double [n0];
    int cycle = 0;
	bool flag = true;
	while (flag)
	{
		flag=false;
		cout<<"cycle : "<<++cycle<<endl;
		clock_t time = clock();
		int *a = new int[n0];
		for (i=0; i<n0; i++)
			a[i] = i;
		randperm(a, n0);
		//for(int idx=0; idx<n0; idx++)
		//	cout<<a[idx]<<endl;

		
		for(int idx=0; idx<n0; idx++)
		{   i=a[idx];
		   
			
			//cout<<"i = "<<i<<endl;
			double tmp1 = W[i*n0+i] - knm[i*n0+M[i]] ;
			double tmp2 = k[i] - km[M[i]];
			
			//clock_t time = clock();
			/*parallel_for (long long(0), n0, [&](long long jj)
			{						
				dQ[jj]=(*(knm+i*n0+jj)+tmp1)-k[i]*(km[jj]+tmp2)*s;
			});*/
			for (j=0; j<n0; j++)
			{						
				dQ[j]=(*(knm+i*n0+j)+tmp1)-k[i]*(km[j]+tmp2)*s;
			}
			dQ[M[i]]=0;
			//time = clock() - time;
			//cout<<"time for calc_Q = "<<time<<endl;

			//for (j=0; j<n0; j++) cout<<"dQ "<<j<<" : "<<dQ[j]<<endl;

			double max_dQ = maxq(dQ,n0,&j);
			  
			//cout<<"max_dQ and id= "<<max_dQ<<' '<<j<<endl;
			if (max_dQ > ep)
			{				
				R_type z=0;
				for (z=0; z<n0; z++)
					*(knm+z*n0+j) += *(W+z*n0+i);
				for (z=0; z<n0; z++)
					*(knm+z*n0+M[i]) -= *(W+z*n0+i);

				km[j] += k[i];
				km[M[i]] -= k[i];

				Nm[j]++;
				Nm[M[i]]--;

				M[i]=j;
				q_gain += 2.0 * max_dQ;
				flag = true;
			}
		}
		time = clock() - time;
		cout<<"cycle elapsed time : "<<time<<endl;
	}
	delete []dQ;
		
	long long n = (long long) unique(M, n0); 
	//if (n==n0)       // No change
	//{cout<<"No change"<<endl;return -1;}
	//cout<<"n = "<<n <<endl;

	for (i=0; i<N; i++)
	{
		Ci[i] = M[Ci[i]];
		//cout<<Ci[i];
	}

	real_t *W1 = new real_t[n*n];
	memset(W1,0,sizeof(real_t)*n*n);

	for (i=0; i<n0; i++)
	{
		*(W1+M[i]*n+M[i]) += *(W+n0*i+i);
		for (j=i+1; j<n0; j++)
		{
			*(W1+M[i]*n+M[j]) += *(W+n0*i+j);
			*(W1+M[j]*n+M[i]) += *(W+n0*i+j);
		}
	}
	/*for (i=0; i<n; i++)
	{
		cout<<endl;
		for (j=0; j<n; j++)
			cout<<*(W1+i*n+j)<<' ';
	}*/

	delete [] W;
	W = new real_t[n*n];
	memcpy(W, W1, sizeof(real_t)*n*n);
	delete []W1;

	//W=*W0;
		
	//double s2=s*s;
	
	
	q_gain *= s;
	//cout<<q_gain + q_old<<endl;
	//double s2 = s*s;
	//double q_check = 0;
	//for (i = 0;  i<n; i++)
	//{	q_check +=*(W+n*i+i)*s;
	//	for (j=0; j<n; j++)
	//	{				
	//		for( int z=0; z<n; z++)
	//			q_check-=(*(W+i*n+z))*(*(W+z*n+j))*s2;
	//		//cout<<i<<' '<<j<<' '<<q_new<<endl;
	//	}
	//}
	//cout << q_check<<endl;
	//total_time += time;

	*n1 = n;
	delete []M;
	delete []Nm;
	return q_gain;
}

double Louvain_modularity_sub( real_t **W0, long long *n1, int *Ci, long long N, double s)
{	
	real_t *W = *W0;	
	
	int i=0,j=0;
	long long n0=*n1;
	//real_t *tmp = new real_t [n0];
	//memcpy(tmp, W+(n0-2)*n0,sizeof(real_t)*n0);

	double q_new = 0;
	real_t *k = new real_t [n0];
	memset(k,0,sizeof(real_t)*n0);
	
	//clock_t time = clock(); 
	for(i=0; i<n0; i++)
		for(j=0; j<n0; j++)
			k[i] += *(W+i*n0+j);
	//time = clock() - time;
	//cout<<"time for calc_k = "<<time<<endl;

	real_t *km = new real_t [n0];
	memcpy(km, k, sizeof(real_t)*n0);
	//for(i=0; i<n0; i++) cout<<km[i]<<endl;
		
	real_t *knm = new real_t [n0*n0];
	memcpy(knm, W, sizeof(real_t)*n0*n0);
	
	int *M = new int [n0];
	for (i=0; i<n0; i++)
		M[i]=i;
	
	int *Nm = new int[n0];
	for (i=0; i<n0; i++)
		Nm[i]=1;
	
	double *dQ = new double [n0];
        
	bool flag = true;
	int cycle = 0;
	while (flag)
	{
		flag=false;

		cout<<"cycle : "<<++cycle<<endl;
		clock_t time = clock();

		int *a = new int[n0];
		for (i=0; i<n0; i++)
			a[i] = i;
		randperm(a, (int) n0);
		//for(int idx=0; idx<n0; idx++)
		//	cout<<a[idx]<<endl;

		
		for(int idx=0; idx<n0; idx++)
		{   i=a[idx];
		   
			
			//cout<<"i = "<<i<<endl;
			//clock_t time = clock();
			for (j=0; j<n0; j++)
			{						
				dQ[j]=(*(knm+i*n0+j)-*(knm+i*n0+M[i])+*(W+i*n0+i))-k[i]*(km[j]-km[M[i]]+k[i])*s;
			}
			dQ[M[i]]=0;
			//time = clock() - time;
			//cout<<"time for calc_Q = "<<time<<endl;
			//for (j=0; j<n0; j++) cout<<"dQ "<<j<<" : "<<dQ[j]<<endl;

			double max_dQ = maxq(dQ,(int)n0,&j);
			  
			//cout<<"max_dQ and id= "<<max_dQ<<' '<<j<<endl;
			if (max_dQ > ep)
			{				
				R_type z=0;
				for (z=0; z<n0; z++)
					*(knm+z*n0+j) += *(W+z*n0+i);
				for (z=0; z<n0; z++)
					*(knm+z*n0+M[i]) -= *(W+z*n0+i);

				km[j] += k[i];
				km[M[i]] -= k[i];

				Nm[j]++;
				Nm[M[i]]--;

				M[i]=j;
				flag = true;
			}
		}
		time = clock() - time;
		cout<<"cycle elapsed time : "<<time<<endl;
	}
	free(dQ);
		
	int n = unique(M, (int)n0); 
	//if (n==n0)       // No change
	//{cout<<"No change"<<endl;return -1;}
	//cout<<"n = "<<n <<endl;

	for (i=0; i<N; i++)
	{
		Ci[i] = M[Ci[i]];
		//cout<<Ci[i];
	}

	real_t *W1 = new real_t[n*n];
	memset(W1,0,sizeof(real_t)*n*n);

	for (i=0; i<n0; i++)
	{
		*(W1+M[i]*n+M[i]) += *(W+n0*i+i);
		for (j=i+1; j<n0; j++)
		{
			*(W1+M[i]*n+M[j]) += *(W+n0*i+j);
			*(W1+M[j]*n+M[i]) += *(W+n0*i+j);
		}
	}
	/*for (i=0; i<n; i++)
	{
		cout<<endl;
		for (j=0; j<n; j++)
			cout<<*(W1+i*n+j)<<' ';
	}*/

	delete [](*W0);
	*W0 = new real_t[n*n];
	memcpy(*W0, W1, sizeof(real_t)*n*n);
	delete []W1;

	W=*W0;
		
	double s2=s*s;
		
	for (i = 0;  i<n; i++)
	{	q_new +=*(W+n*i+i)*s;
		for (j=0; j<n; j++)
		{				
			for( R_type z=0; z<n; z++)
				q_new-=(*(W+i*n+z))*(*(W+z*n+j))*s2;
			//cout<<i<<' '<<j<<' '<<q_new<<endl;
		}
	}
	*n1 = n;
	delete []M;
	delete []Nm;
	return q_new;
}

double Louvain_modularity_sub1(  real_t* &W, R_type *R, C_type *C, long long *n1, int *Ci, long long N, double s)
{
	//real_t *knm = *W0;
	long long n0=*n1;
	long long m = R[n0];	
	//HashGraph knm(m);
	/*for(u_int i = 0; i < n0; i++)
	{
		for(u_int j = R[i]; j < R[i+1]; j++)	
		{
				_ULonglong key = GetKey(i,C[j]);
				knm.Insert(key, 1.0);
		}
	}*/
	//real_t *ki2Mi = new real_t [n0];
	//memset(ki2Mi, 0, sizeof(real_t)*n0);
	//real_t *W = *W0;
	
	int i=0,j=0;
	double q_gain = 0;
	real_t *k = new real_t [n0];
	memset(k,0,sizeof(real_t)*n0);
	//clock_t time = clock();
	for(i = 0; i < n0; i++) 
		k[i] = (real_t) R[i+1] - R[i];
	
	//time = clock() - time;
	//cout<<"time for calc_k = "<<time<<endl;
		
	real_t *km = new real_t [n0];
	memcpy(km, k, sizeof(real_t)*n0);
	
	int *M = new int [n0];
	for (i = 0; i < n0; i++)
		M[i] = i;
	
	
	int *Nm = new int[n0];
	for (i = 0; i < n0; i++)
		Nm[i] = (int) i;
		
	double *dQ = new double [n0];
        
	bool flag = true;
	int cycle = 0;
	while (flag)
	{
		flag=false;
		
		cout<<"cycle : "<<++cycle<<endl;
		Setup(1);
		Start(1);
		int *a = new int[n0];
		for (i=0; i<n0; i++)
			a[i] = i;
		randperm(a, n0);
		//for(int idx=0; idx<n0; idx++)
		//	cout<<a[idx]<<endl;

		
		for(int idx=0; idx<n0; idx++)
		{   
			i=a[idx];
				
			//cout<<"i = "<<i<<endl;
			//double tmp1 = 0 - knm.GetValue(GetKey(i,M[i])) ;
					
			//clock_t time = clock();
			/*parallel_for (long long(0), n0, [&](long long jj)
			{						
				dQ[jj]=(*(knm+i*n0+jj)+tmp1)-k[i]*(km[jj]+tmp2)*s;
			});*/
			memset(dQ,0,sizeof(double)*n0);		
			
			/*for (j = R[i]; j < R[i+1]; j++)
				if(dQ[M[C[j]]] == 0)
					dQ[M[C[j]]]=(knm.GetValue(GetKey(i,M[C[j]]))+tmp1)-k[i]*(km[M[C[j]]]+tmp2)*s;
			*/
			//double tmp2 = ki2Mi[i];
			double tmp1 =  km[M[i]] - k[i];
			double ki_Mi = 0;
			for (R_type z = R[i]; z < R[i+1]; z++)
				ki_Mi += (M[C[z]]==M[i]);
			
			for (R_type z = R[i]; z < R[i+1]; z++)
			{
				if(dQ[M[C[z]]] == 0)
					dQ[M[C[z]]] += s*k[i]*(tmp1 - km[M[C[z]]]) - ki_Mi;
				dQ[M[C[z]]] += 1.0;     //should be V[j] for weighted network. degree of i to M[C[j]];
			}								
			dQ[M[i]]=0;
			//time = clock() - time;
			//cout<<"time for calc_Q = "<<time<<endl;

			//for (j=0; j<n0; j++) cout<<"dQ "<<j<<" : "<<dQ[j]<<endl;
			//int max_j;
			double max_dQ = maxq(dQ,n0,&j);
			
			//if (idx%1000 == 0) cout<<"max_dQ and id= "<<max_dQ<<' '<<j<<endl;
			
			if (max_dQ > ep)
			{				
				//ki2Mi[i] = 0;

				//for (int z=R[i]; z<R[i+1]; z++)
				//{
				//	//knm[GetKey(C[z],j)] += 1;
				//	//knm.SubtractVal(GetKey(C[z],M[i]),1);
				//	if (M[C[z]] == M[i])
				//		ki2Mi[C[z]] -= 1.0;
				//	if (M[C[z]] == j){
				//		ki2Mi[C[z]] += 1.0;
				//		ki2Mi[i] += 1.0;
				//	}
				//}
					//for (z=0; z<n0; z++)
				//	*(knm+z*n0+M[i]) -= *(W+z*n0+i);
				
				km[j] += k[i];
				km[M[i]] -= k[i];

				Nm[j]++;
				Nm[M[i]]--;

				M[i] = j;
				q_gain += 2.0*max_dQ;
				/*cout<<q_new*s<<endl;
	
		double q_check = 0;
		for (int ii = 0; ii < N; ii++)
			for (int jj = R[ii]; jj < R[ii+1]; jj++)
				q_check += 1.0 * (M[ii] == M[C[jj]]);
		for (int ii = 0; ii < N; ii++)
			for (int jj = 0; jj < N; jj++)
				q_check -= 1.0 * (M[ii] == M[jj]) * k[ii] * k[jj] *s;
		q_check *= s;		
		cout<<q_check<<endl;*/
				flag = true;
			}				

		}
		Stop(1);
		/*cout<<q_gain*s<<endl;
	
		double q_check = 0;
		for (int ii = 0; ii < N; ii++)
			for (int jj = R[ii]; jj < R[ii+1]; jj++)
				q_check += 1.0 * (M[ii] == M[C[jj]]);
		for (int ii = 0; ii < N; ii++)
			for (int jj = 0; jj < N; jj++)
				q_check -= 1.0 * (M[ii] == M[jj]) * k[ii] * k[jj] *s;
		q_check *= s;		
		cout<<q_check<<endl;*/
		cout<<"q_gain this cycle: "<<q_gain*s<<endl;
		cout<<"Cycle elapsed time : "<<GetElapsedTime(1)<<'s'<<endl;
	}
	delete []dQ;
		
	long long n = (long long) unique(M, n0); 
	
	for (i=0; i<N; i++)
	{
		Ci[i] = M[Ci[i]];
		//cout<<Ci[i];
	}

	if (W != NULL)
		delete []W;
	W = new real_t[n*n];
	memset(W,0,sizeof(real_t)*n*n);

	for (i = 0; i < n0; i++)
		for (j=R[i]; j<R[i+1]; j++)
			*(W+M[i]*n+M[C[j]]) += 1;
	
	/*for (i=0; i<n; i++)
	{
		cout<<endl;
		for (j=0; j<n; j++)
			cout<<*(W1+i*n+j)<<' ';
	}*/
		
	
	q_gain *= s;
	/*cout<<q_gain<<endl;
	double q_check = 0;
	for (i = 0; i < N; i++)
		for (j = R[i]; j < R[i+1]; j++)
			q_check += 1.0 * (Ci[i] == Ci[C[j]]);
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			q_check -= 1.0 * (Ci[i] == Ci[j]) * (R[i+1]-R[i]) * (R[j+1]-R[j]) *s;
	q_check *= s;
	cout <<q_check<<endl;*/
	//total_time += time;
	*n1 = n;
	//delete []ki2Mi;
	delete []M;
	delete []Nm;
	return q_gain;
}

double Louvain_modularity( int *Ci, R_type *R, C_type *C, long long N)
{   
	double s = 0;
	double q_gain = 0;
	long long n0 = N;
	long long i=0,j=0;
	

	s = (R[N] - R[0]);
	//cout<<"edge number : "<<s<<endl;
	s = 1/s;
	
	for (i=0; i<N; i++)
		Ci[i]=i;
	
	i=1;
	real_t *W = NULL;
	q_gain = Louvain_modularity_sub1( W, R, C, &n0, Ci, N, s);  
	cout <<"Round "<<i<<endl;
	fout <<"Round "<<i++<<endl;
	cout <<"Number of modules : "<<n0<<endl;
	fout <<"Number of modules : "<<n0<<endl;
	cout <<"Modularity Q gain: "<<q_gain<<endl<<endl;
	fout <<"Modularity Q gain: "<<q_gain<<endl<<endl;

	while (true)
	{
		//q=q_new;
		q_gain=Louvain_modularity_sub_parr(W, &n0, Ci, N, s);
		//q_new=Louvain_modularity_sub(&W, &n0, Ci, N, s);
		//q_new=Louvain_modularity_GPU_sub(&W, &n0, Ci, N, s);
		
		/*for (i=0; i<n0; i++)
		{
		cout<<endl;
		for (j=0; j<n0; j++)
			cout<<*(W+i*n0+j)<<' ';
		}*/

		//if (q_new < 0)   //No change from the last merging
		//{  q_new = q; break;  }
		cout <<"Round "<<i<<endl;
		fout <<"Round "<<i++<<endl;
		cout <<"Number of modules : "<<n0<<endl;
		fout <<"Number of modules : "<<n0<<endl;
		cout <<"Modularity Q gain: "<<q_gain<<endl<<endl;
		fout <<"Modularity Q gain: "<<q_gain<<endl<<endl;
		if (q_gain < ep)
			break;
	}
	double s2 = s*s;
	
	double q = 0;
	for (i = 0;  i<n0; i++)
	{	q +=*(W+n0*i+i)*s;
		for (j=0; j<n0; j++)
		{				
			for( R_type z=0; z<n0; z++)
				q -= (*(W+i*n0+z))*(*(W+z*n0+j))*s2;
			//cout<<i<<' '<<j<<' '<<q_new<<endl;
		}
	}
	if (W!=NULL)
		delete []W;
	//cout << q<<endl;
	return q;	
}


int main(int argc, char * argv[])
{
	ofstream flog("BNA_time_log", ios::app);
	clock_t total_time = clock();
	if (argc != 3) 
	{
		cerr<<"Input format: .\\Modularity.exe dir_for_csr num_of_random_networks \nFor example: .\\Modularity_CPU.exe d:\\data 10"<<endl;
		exit(1);	
	}

	DIR *dp;
	struct dirent *dirp;
	if (NULL == (dp = opendir(argv[1])))
	{
		printf("can't open %s", argv[1]);
		exit (1);
	}
	int FileNumber = 0;
	string filenametmp;
	while((dirp = readdir(dp)) != NULL)
	{
		filenametmp = string(dirp->d_name);

		if (filenametmp.find_last_of('.') == -1)
			continue;
		if(filenametmp.length()>4 && filenametmp.substr(filenametmp.find_last_of('.'),4).compare(".csr") == 0 && filenametmp.size() - filenametmp.find_last_of('.') - 1 == 3)
		{
			FileNumber++;
		}
	}
	cout<<FileNumber<<" files to be processed."<<endl;

	closedir(dp);
	string *filename = new string[FileNumber];
	dp = opendir(argv[1]);
	int i = 0;
	while((dirp = readdir(dp)) != NULL)
	{
		filenametmp = string(dirp->d_name);
		if (filenametmp.find_last_of('.') == -1)
			continue;
		if(filenametmp.length()>4 && filenametmp.substr(filenametmp.find_last_of('.'),4).compare(".csr") == 0 && filenametmp.size() - filenametmp.find_last_of('.') - 1 == 3)
		{
			filename[i++] = filenametmp;
		}
	}

	for (i = 0; i < FileNumber; i++)
	{
		string a = string(argv[1]).append("/").append(filename[i]);
		cout<<"\nModular analysis for "<<a.c_str()<<" ..."<<endl;
		ifstream fin(a.c_str(), ios_base::binary);
		if (!fin.good())
		{	cout<<"Can't open\t"<<a.c_str()<<endl;	return 0;}
		// Read x.csr
		int Rlength = 0, Clength = 0;
		fin.read((char*)&Rlength, sizeof(int));
		R_type * R = new R_type [Rlength];
		fin.read((char*)R, sizeof(R_type) * Rlength);
		fin.read((char*)&Clength, sizeof(R_type));
		C_type * C = new C_type [Clength];
		fin.read((char*)C, sizeof(C_type) * Clength);
		fin.close();
		long long N;
		N = Rlength - 1;
		cout<<"Number of voxel = "<<N<<endl;
		int *index = new int [N];
		memset(index, 0 , sizeof(int)*N);
		long long i,j;
		int idx = 0;
		long long N_C;
		//int leaves = 0;	
		for (i = 0; i < N; i++)
			if(R[i+1]-R[i] != 0)
				index[i] = idx++;
			
			/*if(R[i+1]-R[i] <= 1)	
				index[i] = idx++;
			else if (R[i+1]-R[i] == 1){
				index[i] = -1;
				++leaves;
			}*/
			
		N_C = idx;	
		cout<<"Number of connected voxel = "<<N_C<<endl;
		//N=8000;
		//long long N2 = N * N; 
		long long N2 = N_C * N_C;
		//real_t *W = new real_t[N2];
		//memset(W, 0, sizeof(real_t)*N2);
		
		R_type *R_new = new R_type [N_C+1];
		C_type *C_new = new C_type [Clength];
		//leaves = 0;	
		idx = 0;
		for (i = 0; i < N; i++)
		{
			if(R[i] == R[i+1]) continue;
			/*if(R[i+1] - R[i] == 1) {++leaves; continue;} 
			R_new[index[i]] = R[i]-leaves;*/
			R_new[index[i]] = R[i];
			for (j = R[i]; j < R[i+1] && C[j] < N; j++)
			{
				//if (index[C[j]] != -1) C_new[idx++] = index[C[j]];
				C_new[idx++] = index[C[j]];				
				//W[index[i]*N_C+C_new[j]] = 1;
			}
		}
		//R_new[N_C] = R[N]-leaves;
		R_new[N_C] = R[N];
			
		int *Ci = new int [N_C];
		
		string X_modu = a.substr(0, a.find_last_of('.') + 1).append("modu");
		string X_cp_mas = a.substr(0, a.find_last_of('.')).append("_modu.txt");
			
		fout.open(X_cp_mas.c_str(), ios::out);	// Open the log file


		Setup(0);
		Start(0);
		double q = Louvain_modularity(Ci, R_new, C_new, N_C);
		Stop(0);
		


		flog<<"Modularity\t"<<a.c_str()<<"CPU\tkernel time\t"<<GetElapsedTime(0)<<"s"<<endl;
				
		cout<<"elapsed time : "<<GetElapsedTime(0)<<'s'<<endl;
		fout<<"elapsed time : "<<GetElapsedTime(0)<<'s'<<endl;
		//cout<<"total_time for calc_q_new : "<<total_time<<endl;
		cout<<"final q = "<<q<<endl;
		fout<<"Modularity Q = "<<q<<endl;
		int *result = new int [N];
		idx = 0;
		for (i = 0; i < N; i++)
		{
			if (R[i+1] == R[i])
				result[i] = -1;
			/*else if(R[i+1]-R[i] == 1)
				result[i] = Ci[index[C[R[i]]]];*/
			else
				result[i] = Ci[idx++];
		}
		
		/*
		int M = (R[N] - R[0])/ 2;	
		double Q = 0;
		for (i = 0; i < N_C; i++)
			for (j = R_new[i]; j < R_new[i+1]; j++)
				Q += 1.0 * (Ci[i] == Ci[C_new[j]]);
		for (i = 0; i < N_C; i++)
			for (j = 0; j < N_C; j++)
				Q -= 1.0 * (Ci[i] == Ci[j]) * (R_new[i+1]-R_new[i]) * (R_new[j+1]-R_new[j]) / 2 / M;
		Q /= 2 * M;
		cout<<"Q = "<<Q<<endl;	
		*/

		ofstream fresult;
		fresult.open(X_modu.c_str(), ios::binary|ios::out);
		fresult.write((char*)&N, sizeof(int));
		fresult.write((char*)result, sizeof(int) * N);
		fresult.close();
		fout.close();
		delete [] R;
		delete [] R_new;
		delete [] C;
		delete [] C_new;
		//system("pause");
	}
}

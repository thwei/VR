#include "curve.h"

#include <vector>
#include <algorithm>
#include "Eigen\Dense"
#include "utility.h"

using namespace std;
using namespace Eigen;
//vector<float> ctrP;
//vector<float> verlist;
//vector<float> verlist2;

curve::curve(vector<float> ctr_P):
ctrP(ctr_P)
{
	SubDiv = 4;
	par_u = 0.5;
	type = 3;
}

curve::~curve()
{
}

vector<float> curve::curve_interp(int num_sol)
{

	vector<float> solution;
#if 1
	int *temp = new int[num_sol];//1,4,1,0
	temp[num_sol-3]=1;temp[num_sol-2]=4;temp[num_sol-1]=1;
	for(int i=0;i<num_sol-3;i++)
		temp[i]=0;

	MatrixXd M(num_sol,num_sol);
	MatrixXd A(num_sol,num_sol);
	MatrixXd V(num_sol,2);
	MatrixXd solv(num_sol,2);

	V(0,0) = 6*ctrP[2]-ctrP[0];
	V(0,1) = 6*ctrP[3]-ctrP[1];
	V(num_sol-1,0) = 6*ctrP[ctrP.size()-4]-ctrP[ctrP.size()-2];
	V(num_sol-1,1) = 6*ctrP[ctrP.size()-3]-ctrP[ctrP.size()-1];
	for(int i=0;i<num_sol-2;i++)
		for(int j=0;j<2;j++)
			V(i+1,j) = 6*ctrP[(i+2)*2+j];

	M(0,0)=4;M(0,1)=1;
	M(num_sol-1,num_sol-2)=1;
	M(num_sol-1,num_sol-1)=4;	
	for(int i=0;i<num_sol-2;i++)
	{
		M(0,i+2) = 0;
		M(num_sol-1,i)=0;
	}
	for(int i=0;i<num_sol-2;i++)
	{
		for(int j=0;j<num_sol;j++)
			M(num_sol-2-i,j) = temp[(j+i)%num_sol];
	}
	A=M.inverse();

	solv = A*V;

	solution.push_back(ctrP[0]);
	solution.push_back(ctrP[1]);
	solution.push_back(ctrP[0]);
	solution.push_back(ctrP[1]);
	solution.push_back(ctrP[0]);
	solution.push_back(ctrP[1]);
	for(int i=0;i<num_sol;i++)
		for(int j=0;j<2;j++)
			solution.push_back(solv(i,j));
	solution.push_back(ctrP[(num_sol+1)*2]);
	solution.push_back(ctrP[(num_sol+1)*2+1]);
	solution.push_back(ctrP[(num_sol+1)*2]);
	solution.push_back(ctrP[(num_sol+1)*2+1]);
	solution.push_back(ctrP[(num_sol+1)*2]);
	solution.push_back(ctrP[(num_sol+1)*2+1]);

	delete [] temp;
#endif
	return solution;

}

vector<float> curve::UniQuaBSP(bool closed)
{
	n = ctrP.size()/2;
	if(n>=3)
	{
		verlist.clear();
		float step=0.01f;
		float x,y;
		float par[3];

		if(!closed)
			terminal_cond = n-1;
		else
			terminal_cond = n+1;

		for(int i=1;i<terminal_cond;i++)
		{
			for(float u=0.0;u<1.0;u=u+step)
			{
				par[0] = u*u-2*u+1;
				par[1] = -2*u*u+2*u+1;
				par[2] = u*u;

				x = 0.5*( par[0]*ctrP[((i-1)%n)*2] + par[1]*ctrP[(i%n)*2] + par[2]*ctrP[((i+1)%n)*2]);
				y = 0.5*( par[0]*ctrP[((i-1)%n)*2+1] + par[1]*ctrP[(i%n)*2+1] + par[2]*ctrP[((i+1)%n)*2+1]);

				verlist.push_back(x);
				verlist.push_back(y);
			}
		}
	}

	return verlist;
}


vector<float> curve::UniCubBSP(bool closed)
{

	n = ctrP.size()/2;
	if(n>=4)
	{
		verlist.clear();
		float step=0.01f;
		float x,y;
		float par[4];

		if(!closed)
			terminal_cond = n-2;
		else
			terminal_cond = n+1;

		for(int i=1;i<terminal_cond;i++)
		{
			for(float u=0.0;u<1.0;u=u+step)
			{
				par[0] = -1*u*u*u+3*u*u-3*u+1;
				par[1] = 3*u*u*u-6*u*u+4;
				par[2] = -3*u*u*u+3*u*u+3*u+1;
				par[3] = u*u*u;

				x = (1.0/6.0) *( par[0]*ctrP[((i-1)%n)*2] + par[1]*ctrP[(i%n)*2] + par[2]*ctrP[((i+1)%n)*2] +
						     par[3]*ctrP[((i+2)%n)*2] );

				y = (1.0/6.0) *( par[0]*ctrP[((i-1)%n)*2+1] + par[1]*ctrP[(i%n)*2+1] + par[2]*ctrP[((i+1)%n)*2+1] +
						     par[3]*ctrP[((i+2)%n)*2+1] );

				verlist.push_back(x);
				verlist.push_back(y);
			}
		}

	}
	else if(n>=3)  //only have 3 control points
		UniQuaBSP(closed);

	return verlist;

}

vector<float> curve::UniQuaBSPSubDiv(bool closed)
{
	n = ctrP.size()/2;
	if(n>=3)
	{
		verlist.clear();
		verlist = ctrP;
		float step=0.01f;
		float x,y;

		for(int k=0;k<SubDiv;k++)
		{
			n=verlist.size()/2;
			verlist2.clear();	

			if(!closed)
				terminal_cond = n;
			else
				terminal_cond = n+1;

			for(int i=1;i<terminal_cond;i++)
			{
				x = 0.75*verlist[((i-1)%n)*2] + 0.25*verlist[(i%n)*2];
				y = 0.75*verlist[((i-1)%n)*2+1] + 0.25*verlist[(i%n)*2+1];

				verlist2.push_back(x);
				verlist2.push_back(y);

				x = 0.25*verlist[((i-1)%n)*2] + 0.75*verlist[(i%n)*2];
				y = 0.25*verlist[((i-1)%n)*2+1] + 0.75*verlist[(i%n)*2+1];

				verlist2.push_back(x);
				verlist2.push_back(y);
			}
			verlist.clear();
			verlist = verlist2;
		}

	}
	return verlist;
}

vector<float> curve::UniCubBSPSubDiv(bool closed)
{
	n = ctrP.size()/2;
	if(n>=4)
	{
		verlist.clear();
		verlist = ctrP;
		float step=0.01f;
		float x,y;
		int i=1;

		for(int k=0;k<SubDiv;k++)
		{
			n=verlist.size()/2;
			i=1;
			verlist2.clear();

			if(!closed)
				terminal_cond = n-2;
			else
				terminal_cond = n;

			x = 0.5*verlist[((i-1)%n)*2] + 0.5*verlist[(i%n)*2];
			y = 0.5*verlist[((i-1)%n)*2+1] + 0.5*verlist[(i%n)*2+1];

			verlist2.push_back(x);
			verlist2.push_back(y);

			x = 1.0/8.0*verlist[((i-1)%n)*2] + 6.0/8.0*verlist[(i%n)*2] + 1.0/8.0*verlist[((i+1)%n)*2];
			y = 1.0/8.0*verlist[((i-1)%n)*2+1] + 6.0/8.0*verlist[(i%n)*2+1] + 1.0/8.0*verlist[((i+1)%n)*2+1];

			verlist2.push_back(x);
			verlist2.push_back(y);	

			x = 0.5*verlist[(i%n)*2] + 0.5*verlist[((i+1)%n)*2];
			y = 0.5*verlist[(i%n)*2+1] + 0.5*verlist[((i+1)%n)*2+1];

			verlist2.push_back(x);
			verlist2.push_back(y);

			for(i=1;i<terminal_cond;i++)
			{
				x = 1.0/8.0*verlist[(i%n)*2] + 6.0/8.0*verlist[((i+1)%n)*2] + 1.0/8.0*verlist[((i+2)%n)*2];
				y = 1.0/8.0*verlist[(i%n)*2+1] + 6.0/8.0*verlist[((i+1)%n)*2+1] + 1.0/8.0*verlist[((i+2)%n)*2+1];

				verlist2.push_back(x);
				verlist2.push_back(y);

				x = 0.5*verlist[((i+1)%n)*2] + 0.5*verlist[((i+2)%n)*2];
				y = 0.5*verlist[((i+1)%n)*2+1] + 0.5*verlist[((i+2)%n)*2+1];

				verlist2.push_back(x);
				verlist2.push_back(y);
			}

			if(closed)
			{
				verlist2.pop_back();
				verlist2.pop_back();
			}
			verlist.clear();
			verlist = verlist2;
		}

	}
	else if(n>=3)
		UniQuaBSPSubDiv(closed);

	return verlist;

}

inline vector<float> OneSubdiv(vector<float> ctr_P, vector<float> poly_1, vector<float> poly_2, float u)
{
	vector<float> output;
	if(ctr_P.size()==2)
	{
		output = poly_1;
		output.push_back(ctr_P[0]);
		output.push_back(ctr_P[1]);
		output.insert(output.end(),poly_2.begin(),poly_2.end());
	}
	else
	{

		poly_1.push_back(ctr_P[0]);
		poly_1.push_back(ctr_P[1]);

		poly_2.insert(poly_2.begin(),ctr_P[ctr_P.size()-1]);
		poly_2.insert(poly_2.begin(),ctr_P[ctr_P.size()-2]);

		vector<float> temp_p;
		float x,y;

		for(int i=0;i<(ctr_P.size()/2)-1;i++)
		{
			x = ctr_P[i*2] + u*(ctr_P[(i+1)*2]-ctr_P[i*2]);
			y = ctr_P[i*2+1] + u*(ctr_P[(i+1)*2+1]-ctr_P[i*2+1]);

			temp_p.push_back(x);
			temp_p.push_back(y);
		}

		output = OneSubdiv(temp_p,poly_1,poly_2,u);
	}

	return output;

}

inline vector<float> Subdivide(vector<float> ctr_P, int m, float u)
{
	vector<float> poly1;
	vector<float> poly2;
	vector<float> temp_ver;
	vector<float> temp_ver2;
	vector<float> output;
	vector<float> output1;

	if(m==1)
	{
		output = OneSubdiv(ctr_P,poly1,poly2,u);
	}
	else
	{
		vector<float> temp;
		temp = OneSubdiv(ctr_P,poly1,poly2,u);	
		int n = (temp.size()/2-1)/2;

		temp_ver2.assign(temp.begin()+(n*2),temp.end());
		temp_ver.assign(temp.begin(),temp.begin()+((n+1)*2));

		output = Subdivide(temp_ver,m-1,u);
		output1 = Subdivide(temp_ver2,m-1,u);

		output.insert(output.end(),output1.begin(),output1.end());
	}
	return output;
}

vector<float> curve::BezierSubdiv(bool closed)
{
	verlist.clear();


	if(ctrP.size()>=6)
	{
		if(closed)
		{
			float new_start[2],new_end[2];

			new_start[0] = ctrP[0] + 0.5*(ctrP[ctrP.size()-2]-ctrP[0]); 
			new_start[1] = ctrP[1] + 0.5*(ctrP[ctrP.size()-1]-ctrP[1]);
			new_end[0] = ctrP[0] + 0.5*(ctrP[ctrP.size()-2]-ctrP[0]);
			new_end[1] = ctrP[1] + 0.5*(ctrP[ctrP.size()-1]-ctrP[1]);

			ctrP.push_back(new_end[0]);
			ctrP.push_back(new_end[1]);

			ctrP.insert(ctrP.begin(),new_start[1]);
			ctrP.insert(ctrP.begin(),new_start[0]);

		}

		verlist = Subdivide(ctrP,SubDiv,par_u);
	}

	return verlist;
}
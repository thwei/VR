#ifndef CURVE_H
#define CURVE_H

#include <vector>

using namespace std;
class curve
{
public:
    curve(vector<float> ctr_P);
	~curve();
	vector<float> UniQuaBSP(bool closed);
	vector<float> UniCubBSP(bool closed);
	vector<float> UniQuaBSPSubDiv(bool closed);
	vector<float> UniCubBSPSubDiv(bool closed);
	vector<float> BezierSubdiv(bool closed);
	vector<float> curve_interp(int num_sol);
	float par_u;
	int SubDiv;
	int type;
	int n;
	int terminal_cond;
	vector<float> ctrP;
	vector<float> verlist;
	vector<float> verlist2;

};

#endif // CURVE_H

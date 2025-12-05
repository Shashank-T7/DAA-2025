// case09_pollution.cpp
#include <bits/stdc++.h>
using namespace std;
double medianVec(vector<double> v){ sort(v.begin(), v.end()); int n=v.size(); if(n%2) return v[n/2]; return 0.5*(v[n/2-1]+v[n/2]); }
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}

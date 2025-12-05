// case10_wte.cpp
#include <bits/stdc++.h>
using namespace std;
struct Item{int id; double e,m;};
double greedyKnapsack(vector<Item>& items, double cap){
    sort(items.begin(), items.end(), [](const Item&a,const Item&b){ return (a.e/a.m) > (b.e/b.m); });
    double used=0, value=0;
    for(auto &it: items){
        if(used + it.m <= cap){ used+=it.m; value+=it.e; }
        else { double can = cap-used; value += it.e * (can/it.m); break; }
    }
    return value;
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    return 0;
}

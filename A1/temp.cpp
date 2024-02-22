#include<bits/stdc++.h>
using namespace std;
int main(){
    vector<int>a={1,2,3,4};
    int i;
    for(i=0;i<a.size();++i){
        if(a[i]==1)break;
    }
    cout<<i;
}
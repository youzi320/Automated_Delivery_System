#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(){
    ifstream r;
    ofstream w;

    vector<string> s_f;
    string s_s,s_i = "\\\"";

    r.open("format.txt");

    if(r.fail()){
        cout << "Error!!!\n";
        exit(0);
	}

    while(getline(r,s_s)){
        s_f.push_back(s_s);
    }

    r.close();

    w.open("format.txt");

    if(w.fail()){
        cout << "Error!!!\n";
        exit(0);
	}

    for(int i=0;i<s_f.size();i++){
        for(int j=0;j<s_f[i].size();j++){
            if(s_f[i][j] == '"'){
                s_f[i].erase(s_f[i].begin()+j);
                s_f[i].insert(j,s_i);
                j++;
            }
        }
        w  << "client.println(\"" << s_f[i] << "\");" << endl;
    }

    w.close();

    cout << "Finish!!!\n";

    return 0;
}

#include "gurobi_c++.h"
#include <fstream> 
#include <iostream> 
#include <string> 
#include <map>
#include <vector>

using namespace std;

string readKey(ifstream& myfile)
{
    char mychar = myfile.get();
    string key = "";
    if (myfile.is_open())
    {
        while ((mychar != '"') && myfile)
        {
            mychar = myfile.get();
        }
        mychar = myfile.get();
        while ((mychar != '"') && myfile)
        {
            key += mychar;
            mychar = myfile.get();
        }
    }
    return key;
}

int readValue(ifstream& myfile)
{
    string value = "";
    if (myfile.is_open())
    {
        char mychar = myfile.get();
        while (!isdigit(mychar) && myfile)
        {
            mychar = myfile.get();
        }
        while (mychar != ',' && mychar != ']' && mychar != '}' && myfile)
        {
            value += mychar;
            mychar = myfile.get();
        }
    }
    return stoi(value);
}

bool readValues(ifstream& myfile, vector<int>& values)
{
    if (myfile.is_open())
    {
        char mychar = myfile.get();
        while (mychar != '[' && myfile)
        {
            mychar = myfile.get();
        }
        while (mychar != ',' && mychar != '}' && myfile)
        {
            values.push_back(readValue(myfile));
            mychar = myfile.get();
        }
        return mychar != '}';
    }
    return true;
}

void parseJson(string namefile, vector<int>& set, vector<int>& subsets, map<string, vector<int>>& covering, map<string, int>& costs)
{
    ifstream myfile;
    myfile.open(namefile);
    if (myfile.is_open())
    {
        while (myfile){
            string key = readKey(myfile);
            if (key == "set")
            {
                readValues(myfile, set);
            }
            else if (key == "subsets")
            {
                readValues(myfile, subsets);
            }
            else if (key == "covering")
            {
                char mychar = myfile.get();
                bool continuer = true;
                while (continuer)
                {
                    string key = readKey(myfile);
                    continuer = readValues(myfile, covering[key]);
                }
            }
            else if (key == "costs")
            {
                char mychar = myfile.get();
                while (mychar != '}' && myfile)
                {
                    string key = readKey(myfile);
                    if (key != "")
                    {
                        costs[key] = readValue(myfile);
                        char mychar = myfile.get();
                    }
                }
            }
        }
    }
}

map<int, vector<bool>> transformCovering(map<string, vector<int>>& covering, vector<int>& set, vector<int>& subsets)
{
    map<int, vector<bool>> couverture;
    for (int i : subsets)
    {
        for (int j : set)
        {
            couverture[i].push_back(false);
        }
    }
    for (const auto& [key, value] : covering)
    {
        int keyInt = stoi(key);
        for (int element : value)
        {
            couverture[keyInt][element] = true;
        }
    }
    return couverture;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        throw std::invalid_argument( "setcover requires excatly one parameter, which is the json file name." );
    }
    vector<int> set;
    vector<int> subsets;
    map<string, vector<int>> covering;
    map<string, int> costs;
    parseJson(argv[1], set, subsets, covering, costs);
    map<int, vector<bool>> couverture = transformCovering(covering, set, subsets);

   GRBEnv env  = GRBEnv();
   GRBModel model = GRBModel(env);
    
    //Add variables
    vector<GRBVar> x;
    for (int i : subsets)
    {
        x.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "is subset number " + to_string(i) + " used"));
    }

    //Add constraints
    for (int j : set)
    {
        GRBLinExpr expr = 0;
        for (int i : subsets)
        {
            expr += couverture[i][j] * x[i];
        }
        model.addConstr(expr >= 1, "Set number " + to_string(j) + " is covered");
    }

    //Set objective
    GRBLinExpr obj = 0;
    for (int i : subsets)
    {
        obj += costs[to_string(i)] * x[i];
        cout << costs[to_string(i)];
    }
    model.setObjective(obj, GRB_MINIMIZE);

    model.optimize();
    
}

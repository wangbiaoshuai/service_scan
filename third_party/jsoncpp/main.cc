#include "json/json.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int ParseJsonFromFile(const char* filename)
{
    Json::Reader reader;
    Json::Value root;

    std::ifstream is;
    is.open(filename, std::ios::binary);
    std::string out;
    std::string out2;

    if(reader.parse(is, root))
    {
        out = root.toStyledString();
        Json::FastWriter writer;
        out2 = writer.write(root);
    }
    cout<<"out:"<<out<<endl;
    cout<<"out2:"<<out2<<endl;
    is.close();
    return 0;
}

int main()
{
    ParseJsonFromFile("test.json");
    return 0;
}

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
using namespace std;

struct Message{
    int id;
    long long timeStamp;
    string payLoad;
    
    string to_string() const {
        return std::to_string(id) + "," + std::to_string(timeStamp) + "," + payLoad;
    }

    static Message from_string(const string& str){
        Message msg;
        size_t first = str.find(',');
        size_t second = str.find(',', first + 1);

        msg.id = stoi(str.substr(0, first));
        msg.timeStamp = stoll(str.substr(first + 1, second - first - 1));
        msg.payLoad = str.substr(second + 1);
        return msg; 
    }
};

#endif
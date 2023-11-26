#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include "AppClient.h"
#include "helpers/SocketClient.h"
#include "helpers/UtilString.h"
#include "helpers/UtilFile.h"
#include <map>
#include <Windows.h>



bool Client::send(const std::string& url, const std::string& msg)
{
    SocketClient s;
    int sleepTime = 5000;
    int timesCount = 60000 / sleepTime;

    while ( timesCount > 0 &&(!s.init() || !s.connect(url))) {
        Sleep(sleepTime);
        timesCount--;
        s.close();
    }

    if (timesCount <= 0)
        return false;

    printf("sending text message \"%s\"\n", msg.c_str());

    int len;


    std::ifstream file(msg);
    if ( file) {
        

            

            std::ifstream fileBin(msg, std::ios::binary);

            std::string data = std::string(std::istreambuf_iterator<char>(fileBin),
                std::istreambuf_iterator<char>());

            int index = msg.find_last_of(".");
            std::string fileType = msg.substr(index + 1, msg.length() - index - 1);
            
            len = s.sendStr(fileType + " " + data );
        
    }
    else {
        
        len = s.sendStr("msg " + msg);
    }
        
    file.close();

    

    printf("sent %d bytes\n", len);
    return len > 0;
}

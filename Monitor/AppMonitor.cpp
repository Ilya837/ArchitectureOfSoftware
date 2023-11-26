#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include<iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "windows.h"
#include "AppMonitor.h"
#include "helpers/Process.h"
#include "helpers/UtilString.h"
#include "helpers/UtilFile.h"

static Process sServer;
static std::string sPort = "No";
std::string pid = "";
std::filesystem::file_time_type lastTime;

bool Monitor::init()
{
    m_console.handleCtrlC(Monitor::reset); // if Monitor's execution is aborted via Ctrl+C, reset() cleans up its internal state
    char cmd[256] = "";

    

    
    

    /*FILE* ServerInfo;

    ServerInfo = fopen(".\\resources\\CREATED","rb");

    fscanf(ServerInfo, "%s", cmd);

    fclose(ServerInfo);*/
    if(sPort == "No")
        sprintf(cmd, ".\\Server.exe ");
    else
        sprintf(cmd, ".\\Server.exe %s", sPort.c_str());
    bool ok = sServer.create(cmd); // launching Server
    printf(ok ? "monitoring \"%s\"\n" : "error: cannot monitor \"%s\"\n", cmd);

    Sleep(1000);

    if (ok) {

        char buffer[256];
        std::string info;

        FILE* ServerInfo;

        ServerInfo = fopen(".\\resources\\CREATED","rb");

        while (fgets(buffer, 256, ServerInfo) != NULL) {
            info += buffer;
        }

        fclose(ServerInfo);

        

        if (sPort == "No") {
            std::string tmp;
            tmp = info;
            sPort = info.erase(info.find(","), info.size() - info.find(","));
            
            info = tmp;
        }

        info.erase(0,info.find(",")+ 1);

        pid = info;

        lastTime = std::filesystem::last_write_time(".\\resources\\ALIVE" + pid);
        
    }

    return ok;
}

bool Monitor::check()
{
    bool noCrash = sServer.wait(3000);

    Sleep(2000);


    auto time = std::filesystem::last_write_time(".\\resources\\ALIVE"+pid);

    if (noCrash && (time > lastTime)) {
        
        lastTime = time;

        return true;
    }
    
    return false;
}

void Monitor::reset()
{
    sServer.terminate();

    std::string path = ".\\resources";

    

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.path().filename() == "CREATED" || entry.path().filename() == ("ALIVE" + pid))
            std::filesystem::remove(entry.path());
    }

}

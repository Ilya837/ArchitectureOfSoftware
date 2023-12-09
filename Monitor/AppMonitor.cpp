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
#include <fstream>

static Process sServer;
static Process sSecondServer;
static std::string sPort = "No";
std::string pid = "";
std::filesystem::file_time_type lastTime;
bool ServerNow; // false - first server; true - second server

bool Monitor::init()
{
    m_console.handleCtrlC(Monitor::terminate); // if Monitor's execution is aborted via Ctrl+C, terminate() cleans up its internal state
    char cmd[256] = "";
    
    ServerNow = 0;
    auto path = (std::filesystem::path(".\\Server1\\Server.exe"));
    
    if(sPort == "No")
        sprintf(cmd, ".\\Server1\\Server.exe ");
    else
        sprintf(cmd, ".\\Server1\\Server.exe %s", sPort.c_str());
    bool ok = sServer.create(cmd, path.parent_path().string()); // launching Server


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


        char cmd2[256] = "";

        path = (std::filesystem::path(".\\Server2\\Server.exe"));

        sprintf(cmd2, ".\\Server2\\Server.exe %s", sPort.c_str());
         ok &= sSecondServer.create(cmd2, path.parent_path().string()); // launching Second Server
         printf(ok ? "monitoring \"%s\"\n" : "error: cannot monitor \"%s\"\n", cmd2);
         sSecondServer.suspend();
    }

    return ok;
}

bool Monitor::check()
{
    bool noCrash = sServer.wait(3000);

    Sleep(1000);

    auto time = std::filesystem::last_write_time(".\\resources\\ALIVE" + pid);

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

    

    sSecondServer.resume();


    Sleep(1000);


    char cmd[256] = "";
    auto pathToServer = (std::filesystem::path(".\\Server1\\Server.exe"));

    if (ServerNow) {
        sprintf(cmd, ".\\Server2\\Server.exe %s", sPort.c_str());
        pathToServer = (std::filesystem::path(".\\Server2\\Server.exe"));
    }
    else {
        sprintf(cmd, ".\\Server1\\Server.exe %s", sPort.c_str());
        pathToServer = (std::filesystem::path(".\\Server1\\Server.exe"));
    }
    
    bool ok = sServer.create(cmd, path);
    printf(ok ? "monitoring \"%s\"\n" : "error: cannot monitor \"%s\"\n", cmd);

    sServer.suspend();

   

    pid = sSecondServer.pid();

    ServerNow = !ServerNow;

    lastTime = std::filesystem::last_write_time(".\\resources\\ALIVE" + pid);
    


    std::swap(sServer, sSecondServer);


}

void Monitor::terminate() {
    sServer.terminate();
    sSecondServer.terminate();
    std::string path = ".\\resources";



    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.path().filename() == "CREATED" || entry.path().filename() == ("ALIVE" + pid))
            std::filesystem::remove(entry.path());
    }

}

void Monitor::Synchronise() {


    std::ifstream file;
    std::ifstream InSecondfile;
    std::ofstream OutSecondfile;

    if (ServerNow) {
         file = std::ifstream(".\\Server2\\resources\\STATE");
         InSecondfile = std::ifstream(".\\Server1\\resources\\STATE");
         OutSecondfile = std::ofstream(".\\Server1\\resources\\STATE", std::ios::app);
    }
    else {
         file = std::ifstream(".\\Server1\\resources\\STATE");
         InSecondfile = std::ifstream(".\\Server2\\resources\\STATE");
         OutSecondfile = std::ofstream(".\\Server2\\resources\\STATE",std::ios::app);
       
    }

    

    if (file) {

        
        std::string str1;
        std::string str2;
        int StringCounter = 0;

        

        while (std::getline(InSecondfile, str2)) {
            std::getline(file, str1);
            StringCounter++;
        }

       

        while (std::getline(file, str1)) {
            std::cout << "Synchronised " << str1 << std::endl << std::endl;
            OutSecondfile.write( (str1 + "\n").c_str(), str1.size() + 1);
        }

        InSecondfile.close();
    }

    OutSecondfile.close();
    InSecondfile.close();
    file.close();
}
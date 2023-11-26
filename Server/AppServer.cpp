#define _CRT_SECURE_NO_WARNINGS
#include "AppServer.h"
#include "helpers/UtilString.h"
#include "helpers/UtilFile.h"
#include <fstream>
#include <string>
#include <process.h>

bool Server::init(int port)
{

    

    
   /* std::string filePort = "No";

    std::ifstream file("\\resources\\CREATED", std::ios::binary);
    if (file) {

        std::string Info;
        std::getline(file, Info);
        filePort = Info.substr(0, Info.find(","));



        if (!m_socket.init(1000) || !m_socket.listen(std::stoi(filePort)))
            return false;
    }
    else {

        if (!m_socket.init(1000) || !m_socket.listen(port))
            return false;

        if (!fileWriteExclusive("resources\\CREATED", toStr(m_socket.port()) + "," + toStr(_getpid())))
            return false;
    }

    file.close();*/

    if (!m_socket.init(1000) || !m_socket.listen(port))
        return false;

    if (!fileWriteExclusive("resources\\CREATED", toStr(m_socket.port()) + "," + toStr(_getpid())))
        return false;

    printf("server started: port %d, pid %d\n", m_socket.port(), _getpid());

    char* state = fileReadStr("resources\\STATE"); // load state from previous run
    if(state)
    {
        for(std::string& line : split(state, "\n"))
            if(!line.empty())
                m_data.push_back(line);
        delete [] state;
    }

    return true;
}

void Server::run()
{

    


    while(1)
    {
        fileWriteStr(std::string("resources\\ALIVE") + toStr(_getpid()), ""); // pet the watchdog
        std::shared_ptr<Socket> client = m_socket.accept(); // accept incoming connection
        if(!client->isValid())
            continue;

        int n = client->recv(); // receive data from the connection, if any
        char* data = client->data();
        printf("-----RECV-----\n%s\n--------------\n", n > 0 ? data : "Error");
        const std::vector<std::string>& tokens = split(data, " ");

        if(tokens.size() >= 2 && tokens[0] == "GET") // this is browser's request
        {
            // convert URL to file system path, e.g. request to img/1.png resource becomes request to .\img\1.png file in Server's directory tree
            const std::string& filename = join(split(tokens[1], "/"), "\\");
            if(filename == "\\")
            { // main entry point (e.g. http://localhost:12345/)
                std::string payload = "";
                for (auto s : m_data) {

                    std::ifstream file(s, std::ios::binary);
                    if (file) {
                        if ((s.find(".png") != std::string::npos) || (s.find(".jpg") != std::string::npos)) {

                            payload += ("<img src=\"" + s + "\" style=\"max-width: 250px; max-height:250px;\"><br>");

                        }
                        else {

                            std::string data = std::string(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());

                            payload += (data + "<br>");
                        }
                    }
                    else {
                        payload += (s + "<br>"); // collect all the feed and send it back to browser
                    }

                    file.close();

                }
                client->sendStr("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + toStr(payload.length()) + "\r\n\r\n" + payload);
            }
            
            else {

                std::ifstream file(filename.substr(1,filename.length()-1), std::ios::binary);

                if (file) {
                    std::string data = std::string(std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>());
                    
                    client->sendStr("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + toStr(data.length()) + "\r\n\r\n" + data);

                }
                else {
                    client->sendStr("HTTP/1.1 404 Not Found\r\n\r\n");
                }

                
            }
        }

        else if(n > 0) // this is Client's request who wants to upload some data
        {

            if (tokens[0] == "msg") {
                m_data.push_back(std::string(data).substr(4)); // store it in the feed
                fileAppend("resources\\STATE", m_data.back() + "\n"); // store it in the file for subsequent runs
            }
            else{

                time_t t = time(0);
                struct tm* now = localtime(&t);

                char buffer[80];
                strftime(buffer,80, "resources\\%Y-%m-%d-%I-%M-%S.", now);

                int typeLength = 1 + (tokens[0]).length();

                std::ofstream file((buffer + tokens[0]), std::ios::binary);

                char* res =new char[n- typeLength];

                memcpy(res, &data[typeLength],n - typeLength);

                file.write(res, n - typeLength);

                

                file.close();

                m_data.push_back(std::string(buffer + tokens[0])); // store it in the feed
                fileAppend("resources\\STATE", m_data.back() + "\n"); // store it in the file for subsequent runs


            }
            
            




           
        }
    }
}

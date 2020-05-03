/*
 *   Component of the D-ITG 2.8.1 (r1023) platform (http://traffic.comics.unina.it/software/ITG)
 *
 *   Copyright     : (C) 2004-2013 by Alessio Botta, Walter de Donato, Alberto Dainotti,
 *                                      Stefano Avallone, Antonio Pescape' (PI)
 *                                      COMICS (COMputer for Interaction and CommunicationS) Group
 *                                      Department of Electrical Engineering and Information Technologies
 *                                      University of Napoli "Federico II".
 *   email         : a.botta@unina.it, walter.dedonato@unina.it, alberto@unina.it,
 *                   stavallo@unina.it, pescape@unina.it
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 		     
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *				     
 *   For commercial use please refer to D-ITG Professional.
 */



#include "../common/thread.h"
#include "../libITG/ITGapi.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


#include "ITGManager.h"
#include "../common/json.hpp"
#ifdef UNIX


#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <iostream>
#endif

#define STATS_LEN 8


using string=std::string;

double STATS[STATS_LEN];
std::string specifier[5];


std::vector<std::string> ips;
std::string self_ip="";


void Terminate(int sig) {
#ifdef UNIX
    exit(1);
#endif
#ifdef WIN32
    ExitProcess(0);
#endif
}
using json=nlohmann::json;


static std::default_random_engine generator;
void append_params(std::string& command,std::string key,std::string value){
    command.append(" ");
    command.append(key);
    command.append(" ");
    command.append(value);
}


void read_ip_files(std::string& fn){
    std::ifstream infile(fn);
    std::string line;
    while(std::getline(infile,line)){
        if(self_ip==""){
            self_ip=line;
            continue;
        }
        ips.emplace_back(line);
    }
}
bool send_all(int socket, char *ptr, size_t length)
{
    while (length > 0)
    {
        int i = send(socket, ptr, length,0);
        if (i < 1) return false;
        ptr += i;
        length -= i;
    }
    return true;
}


bool report_to_controller(char* dst_ip,int dst_port,json& obj){
    int sockfd;
    struct sockaddr_in servaddr;
    memset(&servaddr,0, sizeof(sockaddr_in));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(dst_port);
    inet_pton(AF_INET,dst_ip,&(servaddr.sin_addr));

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("Cannot create socket \n");
        return false;
    }
    if(connect(sockfd,(struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        perror("Cannot connect to remote ip:");
        printf("%s\n",dst_ip);
        return false;
    }
    string content=obj.dump();
    return send(sockfd,content.c_str(),content.size(),MSG_CONFIRM);
}


//ips_fn
//lambda
//duration
//controller ip
//controller port

int main(int argc, char *argv[]) {
    json report;
    argc--;

    std::string ip_fn=std::string(argv[1]);
    std::cout<<"ip_fn "<<ip_fn<<std::endl;
    read_ip_files(ip_fn);

    int lambda=5;
    int duration=190;

    argc--;
    if(argc==2){
        char* p;
        char* q;
        //lambda
        lambda=strtol(argv[2],&p,10);
        duration=strtol(argv[3],&q,10);
        if(errno!=0||*p!='\0'||lambda>100||duration<=0){
            perror("ITGManager: Invalid Argument");
            exit(1);
        }
    }

    std::exponential_distribution<double> distribution(lambda);
    std::uniform_int_distribution<int> port_uniform_distribution(1500, 65534);
    std::uniform_int_distribution<int> remote_ip_uniform_distribution(0,ips.size()-1);
    std::uniform_real_distribution<double> float_uniform_distribution(10,20);

    std::string params="";
    double sleep_time_in_milli;
    int rp;
    int lp;
    std::string remote_ip="";
    while (1) {
        //generate random stats
        for(int i=0;i<STATS_LEN;i++){
            STATS[i]=float_uniform_distribution(generator);
        }


        params="";


        rp = port_uniform_distribution(generator);
        lp = port_uniform_distribution(generator);

        remote_ip=ips[remote_ip_uniform_distribution(generator)];
        specifier[0]=std::to_string(lp);
        specifier[1]=std::to_string(rp);
        specifier[2]=self_ip;
        specifier[3]=remote_ip;
        specifier[4]="TCP";
        report["specifier"]=specifier;
        report["stats"]=STATS;
        std::cout<<report<<std::endl;

        append_params(params, "-a", remote_ip);
        append_params(params,"-rp",std::to_string(rp));
        append_params(params,"-sp",std::to_string(lp));
        append_params(params,"-t",std::to_string(duration));
        append_params(params,"-T","TCP");
//        std::cout<<params<<std::endl;

//        std::cout<<report.dump()<<std::endl;

        char *controller_ip=argv[4];

        char* p=nullptr;
        int controller_port=strtol(argv[5], &p, 10);

        int report_res=report_to_controller(controller_ip, controller_port, report);
        if(report_res==-1){
            printf("error report\n");
        }

        int res = DITGsend("localhost", const_cast<char *>(params.c_str()));


        sleep_time_in_milli = distribution(generator);
        std::this_thread::sleep_for(std::chrono::milliseconds(int(sleep_time_in_milli * 1000)));
        if (res == -1) {
            exit(1);
        }
    }
}


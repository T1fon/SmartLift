#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>
#include <fstream>
#include <string>

using namespace std;

inline
int
load_server_certificate(boost::asio::ssl::context& ctx, string path_to_cert, string path_to_key)
{

    ifstream fin;
    fin.open(path_to_cert, ifstream::in);

    if(!fin.is_open()){
        return -1;
    }

    int count_temp_byte = 50;
    char temp[count_temp_byte];
    
    std::string cert = "";
    for(bool exit = false;!exit;){
        fin.read(temp,count_temp_byte);
        cert += temp;
        fill_n(temp,count_temp_byte, 0);
        if(fin.gcount() != count_temp_byte){
            exit = true;
        }
    }
    fin.close();

    fin.open(path_to_key);
    if(!fin.is_open()){
        return -1;
    }
    std::string key = "";
    for(bool exit = false;!exit;){
        fin.read(temp,count_temp_byte);
        key += temp;
        fill_n(temp,count_temp_byte, 0);
        if(fin.gcount() != count_temp_byte){
            exit = true;
        }
    }
    fin.close();

    ctx.set_password_callback(
        [](std::size_t,
            boost::asio::ssl::context_base::password_purpose)
        {
            return "test";
        });

    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::single_dh_use);

    ctx.use_certificate_chain(
        boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    return 0;
    //ctx.use_tmp_dh(
    //    boost::asio::buffer(dh.data(), dh.size()));
}
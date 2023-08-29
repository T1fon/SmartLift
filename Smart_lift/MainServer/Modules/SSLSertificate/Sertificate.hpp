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
load_server_certificate(boost::asio::ssl::context& ctx, string path_to_sertificate, string path_to_key)
{
    ctx.set_password_callback(
        [](std::size_t,
            boost::asio::ssl::context_base::password_purpose)
        {
            return "";
        });

    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2);

    ctx.use_certificate_chain_file(path_to_sertificate);
    ctx.use_private_key_file(path_to_key, boost::asio::ssl::context::file_format::pem);

    return 0;
}
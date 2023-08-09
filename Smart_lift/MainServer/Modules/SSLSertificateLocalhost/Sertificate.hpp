#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>

/*  Load a signed certificate into the ssl context, and configure
    the context for use with a server.

    For this to work with the browser or operating system, it is
    necessary to import the "Beast Test CA" certificate into
    the local certificate store, browser, or operating system
    depending on your environment Please see the documentation
    accompanying the Beast certificate for more details.
*/
inline
void
load_server_certificate(boost::asio::ssl::context& ctx)
{
    /*
        The certificate was generated from bash on Ubuntu (OpenSSL 1.1.1f) using:

        openssl dhparam -out dh.pem 2048
        openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "/C=US/ST=CA/L=Los Angeles/O=Beast/CN=www.example.com"
    */

    std::string const cert =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIFQDCCA6igAwIBAgIRAMfQEyB6MZYDAAcfR7xUxY0wDQYJKoZIhvcNAQELBQAw\n"
        "gbcxHjAcBgNVBAoTFW1rY2VydCBkZXZlbG9wbWVudCBDQTFGMEQGA1UECww9REVT\n"
        "S1RPUC1BMTEyOFFMXERhbmlsIElnbmF0ZXZAREVTS1RPUC1BMTEyOFFMIChEYW5p\n"
        "bF9JZ25hdGV2KTFNMEsGA1UEAwxEbWtjZXJ0IERFU0tUT1AtQTExMjhRTFxEYW5p\n"
        "bCBJZ25hdGV2QERFU0tUT1AtQTExMjhRTCAoRGFuaWxfSWduYXRldikwHhcNMjMw\n"
        "ODA5MTIwNDQ5WhcNMzMwODA5MTIwNDQ5WjCBtzEeMBwGA1UEChMVbWtjZXJ0IGRl\n"
        "dmVsb3BtZW50IENBMUYwRAYDVQQLDD1ERVNLVE9QLUExMTI4UUxcRGFuaWwgSWdu\n"
        "YXRldkBERVNLVE9QLUExMTI4UUwgKERhbmlsX0lnbmF0ZXYpMU0wSwYDVQQDDERt\n"
        "a2NlcnQgREVTS1RPUC1BMTEyOFFMXERhbmlsIElnbmF0ZXZAREVTS1RPUC1BMTEy\n"
        "OFFMIChEYW5pbF9JZ25hdGV2KTCCAaIwDQYJKoZIhvcNAQEBBQADggGPADCCAYoC\n"
        "ggGBALEs9203rNsLbzRKHCtvuIvxr8qIvmKIdb2XlUJS5MG5jKUXPP3fYlqRWQAX\n"
        "EBQNBG2URvGeMF0k0/xYsyA399CDRrY3w1P2VctBtlU4VIqJvOUO+W1moeo+8+IE\n"
        "nd4XuXtaPtUoVuVwWzNsXz7KBE55dA/dtN9d0DTZs+O0Fctw6/6hmjptiBUqn09l\n"
        "ylKiwOuf1U2QKpzYMiANCmTwLWRgVGSCaL7Lr457yLVNPakYakO1rCPFy/Kw+RWi\n"
        "U3eNm76tKsy162qDKoRkZ+t7okKy55CJ+W9MNVFNbOvj6yig1+L7pwgYD8ZHfOYn\n"
        "OahuMXyrDYQCFMy2IYbTlTt+AaL3TkmSY0Egnu1KyNqkLE6ZSa2faLPPqHm6GbqF\n"
        "c8NMJJ9DFIAQ3aAXTA1hyLCffves4sakdo9dujIjjVjKRgLZmcocLyMPPCzdLkt6\n"
        "L+RnQecQoDR9OPCPcG1puGDJAXmdbNvZgwK9xlceknoRZXfrhwxfV3zyZXHWyrpZ\n"
        "zHygMQIDAQABo0UwQzAOBgNVHQ8BAf8EBAMCAgQwEgYDVR0TAQH/BAgwBgEB/wIB\n"
        "ADAdBgNVHQ4EFgQUj8lDM5NTxH3Ljcc05qbuef/siSowDQYJKoZIhvcNAQELBQAD\n"
        "ggGBAJQpO8Pnk4eD9fzwvq2/xdO68YG0QneyJkDh1twP71QsJP2HMkw0XPUn8DkD\n"
        "KF7HSmZ1kZM2857mjB5K8P2eyV/9BabKhI01iYnk+qvrf6zr7A+jtuqh1srAqs4L\n"
        "2P66iVO6Hzrmw4lAC0MtOFvo766jpRi3crtMPPcWXjQP6jEhrnS+5bZMWotTPTWS\n"
        "wmlL0NlKw82vIu9D4x/vdlyAoeWDuLlPWxDbVzNLUFa7BHDDdJOpDJB+4hRjv4S4\n"
        "2N+Hen10YrL7e91LwQjO+t5EsawZJpni9k/g9xDF4Cq7RJQgadU11uLN3SqeESqk\n"
        "YCHCA0Q4t5HAhQOpGj7gs4o5fcGWz72qlcuN3aqbiqpF2GFm+TBwr9CLnjip6/yV\n"
        "xqNvhb2FICLT7hrnLvVAZieTa2fJK2vvYFg8vWpQvs0xGTOGvZphBJHqK+pOwmuK\n"
        "M/g0joeBEGPrKBzlc6N5W/cmAViU4NaJXXhTv89AsWO615+gNXnKePSLpi6jGMZZ\n"
        "s6N7Qg==\n"
        "-----END CERTIFICATE-----\n";



    std::string const key =
        "-----BEGIN PRIVATE KEY-----\n"
        "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\n"
        "MfBmAL02Aa6BvRRQE0Uqpw==\n"
        "-----END PRIVATE KEY-----\n";



    /*std::string const dh =
        "-----BEGIN DH PARAMETERS-----\n"
        "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
        "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
        "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
        "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
        "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
        "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
        "-----END DH PARAMETERS-----\n";
        */
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

    //ctx.use_tmp_dh(
    //    boost::asio::buffer(dh.data(), dh.size()));
}
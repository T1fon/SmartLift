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
        "MIIEXDCCAsSgAwIBAgIRAPgWQ9Ga8i2ZDh4g5yuXiJIwDQYJKoZIhvcNAQELBQAw\n"
        "gZMxHjAcBgNVBAoTFW1rY2VydCBkZXZlbG9wbWVudCBDQTE0MDIGA1UECwwrREVT\n"
        "S1RPUC1SRlJOSEg1XFZhdG9AREVTS1RPUC1SRlJOSEg1IChWYXRvKTE7MDkGA1UE\n"
        "AwwybWtjZXJ0IERFU0tUT1AtUkZSTkhINVxWYXRvQERFU0tUT1AtUkZSTkhINSAo\n"
        "VmF0bykwHhcNMjMwNzAzMTI0OTM3WhcNMjUxMDAzMTI0OTM3WjBfMScwJQYDVQQK\n"
        "Ex5ta2NlcnQgZGV2ZWxvcG1lbnQgY2VydGlmaWNhdGUxNDAyBgNVBAsMK0RFU0tU\n"
        "T1AtUkZSTkhINVxWYXRvQERFU0tUT1AtUkZSTkhINSAoVmF0bykwggEiMA0GCSqG\n"
        "SIb3DQEBAQUAA4IBDwAwggEKAoIBAQDJOOdf82peSpI3GZwsgAY6vYwd5CA/dllG\n"
        "2ai236ZtgEVIgmYdGqCmaX72KkSp8iaiW0JDpWZPtPVNzgTEI4luVRAeyAmEgapr\n"
        "or5+KmVt4f8/GX9zt2KxJ+FaGpg12Ku7o88xKpaQmo3BFSWjQ+ALsvpmGaZD3KWW\n"
        "KaP0TWjAR+L5z7OsljoPA9H6fu7vtgQ63zLmSwgE0CWQBZROuCb2EIXkNi8j++N3\n"
        "uvrzN0eOk9N2D7A4B8hHBYebw6dlKmV6NJweuokuy+A+Va1ZA8dZljGW2kbsKRBe\n"
        "1v6PDVE6R5MG5+yAZQFUX+dremgl6x5rk0JOqOv3IsLboj1LTCthAgMBAAGjXjBc\n"
        "MA4GA1UdDwEB/wQEAwIFoDATBgNVHSUEDDAKBggrBgEFBQcDATAfBgNVHSMEGDAW\n"
        "gBT7vfBXjI5J8yHG/MfsA5kbTFr/QjAUBgNVHREEDTALgglsb2NhbGhvc3QwDQYJ\n"
        "KoZIhvcNAQELBQADggGBAIIJt27VN2YNeWDQfX0/ddKIm2P9M17xYcPWz85bRURK\n"
        "iANT+kepdZr8/CWVuhSpmjxdYOdEp66HFstqmCUWmx+LRmUIUFWAi8SmX+vH/1iX\n"
        "YBi4bH3XyC2mkdg/sOcwlvTTjniTHUbHAPHDAJgkWVi5R3ydpdtrmCA9Y6ObIlh8\n"
        "5+nzYKxTsAuA2mGcrBZiw5QdZ5njgI7zW1scnIUNZWXB2IwacvqOPj9OvCo+eiqU\n"
        "05d1QPd6rZYcEGe2Jm+n2ajuN28smvgdSBchStgGQpL9fbGUIPVmiPQtERhgaMVO\n"
        "CrnOAHJLc8I71NNUYvguAL+COykle2NN24tqFhUCTtU6tFbCkXcCd0xST89hfYGh\n"
        "py38skV9bx397uE/6Cw/rskp0AmkFQRLS4LbwbgY3kAgqBY471jyxbB6vXDy+xou\n"
        "RqVg3bNGXXOATiSMy90dabmxVnlfApQrBBF0rgfEwBeBIWSbyu6/dVKWgjOnU7Ek\n"
        "g7K1FkWE16P2TpujCpF5OA==\n"
        "-----END CERTIFICATE-----\n";

    std::string const key =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJOOdf82peSpI3\n"
        "GZwsgAY6vYwd5CA/dllG2ai236ZtgEVIgmYdGqCmaX72KkSp8iaiW0JDpWZPtPVN\n"
        "zgTEI4luVRAeyAmEgapror5+KmVt4f8/GX9zt2KxJ+FaGpg12Ku7o88xKpaQmo3B\n"
        "FSWjQ+ALsvpmGaZD3KWWKaP0TWjAR+L5z7OsljoPA9H6fu7vtgQ63zLmSwgE0CWQ\n"
        "BZROuCb2EIXkNi8j++N3uvrzN0eOk9N2D7A4B8hHBYebw6dlKmV6NJweuokuy+A+\n"
        "Va1ZA8dZljGW2kbsKRBe1v6PDVE6R5MG5+yAZQFUX+dremgl6x5rk0JOqOv3IsLb\n"
        "oj1LTCthAgMBAAECggEBALS3IEUyLXNlcY9+tp3YlL+of/BQcw3L0j4pOLaUgbmS\n"
        "G6cVRXQZ9/G8iUO3jlc6nKvvXcG3B/3JvQ0VW35zk/e8+W9LpBCXC2EDliVlGhbn\n"
        "gv8+fRKFvmLwOBleDRk+8Gziny++2d4gj/K6dUT9rneTVALiQkpsHWKRHyhHZS5Z\n"
        "SM52NNIGZl9Vn4tgQNlMY+Mf57+MWbzKvmmL/48v1a9Y+YIV4smxi1xzIHc1gIzI\n"
        "4sgf3p82USVuYCywWZgbTPnejp7FobfRDFxZKn1Siex5bVlZoVeT058lG17dv9uZ\n"
        "ZIZ+IAIem6uk6tQaQcRt7D5tQ2r2gdRTv9qHa10Q+uECgYEA8rK7w8Q8TOtYg66N\n"
        "Z8oaWYGGL6LVJUhKNuOcvT+Ox3xH2pllysgdqcGr/pjO2RK0Q4jlMXNdvn2Fm0cK\n"
        "Gkl6I48HgQuCLjupkZUT5xNgu7XZR8Y7yc6Lqfr/3+Kyjit7YmAxdGHPX67AGYJ2\n"
        "hVuVtp4eYWhaYCzCnRKUECk1nU0CgYEA1EA6OWCm25GWqNbUvgWDLtqPnuOmfVQO\n"
        "U2xDFP+7fkpxCpWIEwIwjBk3F6hVNULhar4waD33YOzT+UL3005qpyJNnIa/yU5x\n"
        "bErQpEHW9rtS0Ee/De4En+clprll5LU+5Kqz5p7dUeuhZOj3iu/qCmsXbUgS+PeB\n"
        "31lhzKAbjGUCgYBXb9M7zWgYIjXqgymIugRSONraMKTv4KJ0Mf6XI93RutG8TMWi\n"
        "kAQ304GeoPjstdNh/WDtNu4QIfLPpYFbRRuO0gnXEQlelLobrATWnGGeXv6f4ChM\n"
        "JXCl1br3sxm3JK04HQXUMyTeCPxGdH00wHEosMCrQUhGG/olUrsrM12QhQKBgQDA\n"
        "01TSGbedFGgOHZYKyBFEuLTvwZvYkkutiFZtNmOMuW53EiAcPMoEDUhaysgWahtG\n"
        "EWEC4wDh4mY1NpzVMJF+I5Dk2hpUdVpf6pwgDdyQLOBwbi3pJsq2xpUTIa2rGOn5\n"
        "bgegRcWFjg8DEQ8K/yjdrCsNUgRCgQFQ+u/5qZ89bQKBgH4gsL0IaO8+f/4rcZ3w\n"
        "XrvwRlccZ0zdcSlsi4zIWmd5VcMrUXzv1H6cLXjVHFJJ74K2Ge5H95T1Y8EQct9W\n"
        "CMc+b+iMOXG+bb0cdGrTQFvouFzh9IvbJhnp7bqwxIs23GP0RknboAdmaElKXM6t\n"
        "/G0fU7rMlH6XsrOnK7xNRpXu\n"
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
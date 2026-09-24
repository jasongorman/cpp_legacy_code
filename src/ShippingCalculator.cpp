#include "ShippingCalculator.hpp"
#include "Order.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream_base.hpp>

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>

#include <openssl/ssl.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace json = boost::json;

using tcp = asio::ip::tcp;

double ShippingCalculator::calculateShipping(int orderId) {
    try {
        const std::string host = "codemanship.co.uk";
        const std::string target =
            "/api/orders.php?orderId=" + std::to_string(orderId);

        asio::io_context io;
        ssl::context sslContext(ssl::context::tls_client);
        sslContext.set_default_verify_paths();

        tcp::resolver resolver(io);
        beast::ssl_stream<beast::tcp_stream> stream(io, sslContext);

        SSL_set_tlsext_host_name(
            stream.native_handle(),
            host.c_str()
        );

        auto endpoints = resolver.resolve(host, "443");
        beast::get_lowest_layer(stream).connect(endpoints);

        stream.handshake(ssl::stream_base::client);

        http::request<http::empty_body> request{
            http::verb::get,
            target,
            11
        };

        request.set(http::field::host, host);

        http::write(stream, request);

        beast::flat_buffer buffer;
        http::response<http::string_body> response;

        http::read(stream, buffer, response);

        const json::value parsed =
            json::parse(response.body());

        const json::object& obj = parsed.as_object();

        Order order;
        order.shippingType =
            obj.at("shippingType").as_string().c_str();
        order.weightKg =
            obj.at("weightKg").to_number<double>();
        order.distanceKm =
            obj.at("distanceKm").to_number<double>();

        if (order.shippingType == "STANDARD") {
            return order.weightKg * 0.5;
        }

        if (order.shippingType == "EXPRESS") {
            return order.weightKg * 0.8
                 + order.distanceKm * 0.1;
        }

        if (order.shippingType == "OVERNIGHT") {
            return order.weightKg * 1.2 + 25;
        }

        throw std::runtime_error(
            "Unknown shipping type: " + order.shippingType
        );

    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
        return -1;
    }
}
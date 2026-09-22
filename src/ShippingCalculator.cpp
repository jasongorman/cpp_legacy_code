#include "ShippingCalculator.hpp"
#include "Order.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace json = boost::json;
using tcp = asio::ip::tcp;

namespace {

Order parseOrder(const std::string& body) {
    const json::value jv = json::parse(body);
    const json::object& obj = jv.as_object();

    Order order;
    order.shippingType = obj.at("shippingType").as_string().c_str();
    order.weightKg = obj.at("weightKg").to_number<double>();
    order.distanceKm = obj.at("distanceKm").to_number<double>();
    return order;
}

std::string fetchOrderJson(int orderId) {
    const std::string host = "codemanship.co.uk";
    const std::string port = "443";
    const std::string target = "/api/orders.php?orderId=" + std::to_string(orderId);

    asio::io_context ioc;
    ssl::context ctx(ssl::context::tlsv12_client);
    ctx.set_default_verify_paths();

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // Required so the server's TLS handshake matches the requested hostname (SNI).
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        throw beast::system_error(
            beast::error_code(static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()));
    }

    const auto results = resolver.resolve(host, port);
    beast::get_lowest_layer(stream).connect(results);
    stream.handshake(ssl::stream_base::client);

    http::request<http::string_body> req{http::verb::get, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, "ShippingCalculator/1.0 (Boost.Beast)");

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    beast::error_code ec;
    stream.shutdown(ec);
    if (ec && ec != asio::error::eof && ec != ssl::error::stream_truncated) {
        throw beast::system_error(ec);
    }

    return res.body();
}

}

double ShippingCalculator::calculateShipping(int orderId) {
    try {
        const std::string jsonBody = fetchOrderJson(orderId);
        const Order order = parseOrder(jsonBody);

        if (order.shippingType == "STANDARD") {
            return order.weightKg * 0.5;
        }
        if (order.shippingType == "EXPRESS") {
            return order.weightKg * 0.8 + order.distanceKm * 0.1;
        }
        if (order.shippingType == "OVERNIGHT") {
            return order.weightKg * 1.2 + 25;
        }

        throw std::runtime_error("Unknown shipping type: " + order.shippingType);

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
}

#include "src/ShippingCalculator.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <orderId>" << std::endl;
        return 1;
    }

    int orderId;
    try {
        orderId = std::stoi(argv[1]);
    } catch (const std::exception&) {
        std::cerr << "Invalid orderId: " << argv[1] << std::endl;
        return 1;
    }

    ShippingCalculator calculator;
    const double cost = calculator.calculateShipping(orderId);
    std::cout << "Order " << orderId << " shipping cost: " << cost << std::endl;

    return 0;
}

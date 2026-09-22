#pragma once

#include <string>

struct Order {
    std::string shippingType;
    double weightKg = 0.0;
    double distanceKm = 0.0;
};

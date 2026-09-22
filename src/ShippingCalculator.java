package com.codemanship.refactoring.legacycode;

import com.fasterxml.jackson.databind.ObjectMapper;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

public class ShippingCalculator {

    private final ObjectMapper objectMapper = new ObjectMapper();
    private final HttpClient httpClient = HttpClient.newHttpClient();

    public double calculateShipping(int orderId) {

        try {
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(
                            "https://codemanship.co.uk/api/orders.php?orderId=" + orderId
                    ))
                    .GET()
                    .build();

            HttpResponse<String> response =
                    httpClient.send(
                            request,
                            HttpResponse.BodyHandlers.ofString()
                    );

            String json = response.body();

            Order order =
                    objectMapper.readValue(json, Order.class);

            switch (order.getShippingType()) {

                case "STANDARD":
                    return order.getWeightKg() * 0.5;

                case "EXPRESS":
                    return order.getWeightKg() * 0.8
                            + order.getDistanceKm() * 0.1;

                case "OVERNIGHT":
                    return order.getWeightKg() * 1.2 + 25;

                default:
                    throw new RuntimeException(
                            "Unknown shipping type: "
                                    + order.getShippingType()
                    );
            }

        } catch (Exception e) {
            System.out.println(e);
            return -1;
        }
    }
}

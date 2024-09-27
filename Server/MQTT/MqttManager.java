package com.louisdouglas.AutoSecure.MQTT;

import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.annotation.MessagingGateway;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.mqtt.core.DefaultMqttPahoClientFactory;
import org.springframework.integration.mqtt.inbound.MqttPahoMessageDrivenChannelAdapter;
import org.springframework.integration.mqtt.outbound.MqttPahoMessageHandler;
import org.springframework.integration.mqtt.support.DefaultPahoMessageConverter;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.MessageHeaders;

import javax.net.ssl.HttpsURLConnection;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;
import java.time.Instant;
import java.time.format.DateTimeFormatter;

@Configuration
public class MqttManager {

    @Value("${spring.mqtt.client.username}")
    private String username;

    @Value("${spring.mqtt.client.password}")
    private String password;

    @Value("${spring.mqtt.client.broker-url}")
    private String brokerUrl;

//    @Value("${server.address}")
//    private String serverUrl;

    @Value("${spring.mqtt.client.client-id}")
    protected String clientId;

    @Value("${spring.mqtt.client.subtopic}")
    protected String subTopic;

    @Bean
    protected DefaultMqttPahoClientFactory mqttClientFactory() {
        DefaultMqttPahoClientFactory factory = new DefaultMqttPahoClientFactory();
        MqttConnectOptions options = new MqttConnectOptions();
        options.setAutomaticReconnect(true);
        options.setServerURIs(new String[]{brokerUrl});
        options.setUserName(username);
        options.setPassword(password.toCharArray());
        factory.setConnectionOptions(options);
        return factory;
    }

    @Bean
    public IntegrationFlow mqttInFlow() {
        return IntegrationFlow
                .from(mqttInbound())
                .handle(locationHandler())
                .get();
    }

    private MessageHandler locationHandler() {
        return message -> {
            String payload = (String) message.getPayload();
            MessageHeaders headers = message.getHeaders();
            System.out.println("Received location data: " + payload);
            System.out.println("Headers: " + headers);

            Instant instant = Instant.now() ;

            // Define a formatter for the date and time
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

            // Format the date and time using the formatter
            String formattedInstant = formatter.format(instant);


            String[] values = payload.split(",");
            String user = values[0];
            String latitude = values[1];
            String longitude = values[2];
            String json = "{\"user\": \"" + user + "\"" +
                    ",\"update_date\": \"" + formattedInstant + "\"" +
                    ",\"latitude\": \"" + latitude + "\"" +
                    ",\"longitude\": \"" + longitude + "\"}";

            saveData(json);
        };
    }

    private void saveData(String data) {
        try {
            // Define the REST API endpoint URL
            HttpsURLConnection conn = getHttpsURLConnection();
            OutputStream os = conn.getOutputStream();
            os.write(data.getBytes());
            os.flush();
            os.close();

            // Check the response code
            int responseCode = conn.getResponseCode();
            if (responseCode == HttpsURLConnection.HTTP_OK || responseCode == HttpsURLConnection.HTTP_CREATED) {
                // Data insertion successful
                System.out.println("Data insertion successful");
            } else {
                // Data insertion failed
                System.out.println("Data insertion failed with response code: " + responseCode);
            }

            // Close the connection
            conn.disconnect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private HttpsURLConnection getHttpsURLConnection() throws IOException {
        URL url = new URL("https://" + "douglaspc.duckdns.org" + ":8443/api/locations");
        // Open connection to the endpoint URL
        HttpsURLConnection conn = (HttpsURLConnection) url.openConnection();
        // Set the request method to POST
        conn.setRequestMethod("POST");
        // Set the content type header
        conn.setRequestProperty("Content-Type", "application/json");
        // Enable output and write the data to the connection
        conn.setDoOutput(true);
        return conn;
    }

    public MqttPahoMessageDrivenChannelAdapter mqttInbound() {
        MqttPahoMessageDrivenChannelAdapter adapter = new MqttPahoMessageDrivenChannelAdapter(clientId,
                mqttClientFactory(), subTopic);
        adapter.setCompletionTimeout(5000);
        adapter.setConverter(new DefaultPahoMessageConverter());
        adapter.setQos(1);
        return adapter;
    }

    @Bean
    @ServiceActivator(inputChannel = "mqttOutboundChannel")
    public MessageHandler mqttOutbound() {
        MqttPahoMessageHandler messageHandler =
                new MqttPahoMessageHandler("ServerClient", mqttClientFactory());
        messageHandler.setAsync(true);
        messageHandler.setDefaultTopic("SERVER-TO-IOT");
        return messageHandler;
    }

    @Bean
    public MessageChannel mqttOutboundChannel() {
        return new DirectChannel();
    }

    @MessagingGateway(defaultRequestChannel = "mqttOutboundChannel")
    public interface MyGateway {

        void sendToMqtt(String data);

    }

    @Bean
    public IntegrationFlow mqttOutboundFlow() {
        return f -> f.handle(new MqttPahoMessageHandler(brokerUrl, "Server-mqtt-Client"));
    }
}

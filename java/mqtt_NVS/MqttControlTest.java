package mqtt_NVS;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import tools.jackson.databind.ObjectMapper;

public class MqttControlTest
{
    private static final String BROKER_URL = "tcp://localhost:1883";
    private static final String TOPIC = "esp32/device/control";
    private static final String ClientId = "Java_Controller"+System.currentTimeMillis();

    public static void main(String[] args) {
        try {
            MqttClient mqttClient=new MqttClient(BROKER_URL,ClientId,new MemoryPersistence());
            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true);
            mqttClient.connect(options);
            System.out.println("[MQTT] Connected to broker");
            ObjectMapper mapper = new ObjectMapper();

            for (int i=1;i<=20;i++)
            {
                int tragetMode = i % 3;
                DeviceCommand command =new DeviceCommand("set_Mode",tragetMode);
                String jsonCommand = mapper.writeValueAsString(command);

                MqttMessage message = new MqttMessage(jsonCommand.getBytes());
                message.setQos(1);
                mqttClient.publish(TOPIC,message);
                System.out.printf("[Test %02d] Sent: %s\n", i,jsonCommand);
                Thread.sleep(500);
            }
            mqttClient.disconnect();
            System.out.println("\n[MQTT] Disconnected. Stress Test Completed.");
        }catch (Exception e){
            e.printStackTrace();
        }
    }
}

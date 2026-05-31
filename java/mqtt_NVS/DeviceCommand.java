package mqtt_NVS;

public class DeviceCommand {
    public String command;
    public int value;
    public DeviceCommand(String command, int value) {
        this.command = command;
        this.value = value;
    }
}


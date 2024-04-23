Linux setup Bluetooth
+ Install Bluez
+ Install Minicom

To connect to remote Bluetooth device 
<pre>
    bluetoothd -C &
    hciconfig hci0 up
    hciconfig hci0 piscan
    sdptool add SP
    bluetoothctl scan on
    bluetoothctl agent on
    bluetoothctl default-agent
    bluetoothctl trust xx:xx:xx:xx:CF:F0
    bluetoothctl pair xx:xx:xx:xx:CF:F0
    rfcomm bind 0 xx:xx:xx:xx:CF:F0 1
    chmod 666 /dev/rfcomm0
    rfcomm connect 0 xx:xx:xx:xx:CF:F0 1
</pre>


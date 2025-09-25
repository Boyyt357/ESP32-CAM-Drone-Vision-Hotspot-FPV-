This project enables the ESP32-CAM to function as a compact, standalone camera for drones, providing a live video feed without needing an existing Wi-Fi network.
By configuring the ESP32-CAM to operate as a Wi-Fi Access Point (hotspot), it creates its own dedicated network. This allows a control device (such as a smartphone or tablet) 
to directly connect to the drone's camera, providing First-Person View (FPV) capabilities in real-time via a web browser.
This setup is ideal for small, agile drones or situations where a direct, independent video link is preferred,
offering flexibility and ease of deployment. The code is a modified version of the standard
ESP32-CAM web server example, optimized for Access Point operation to facilitate a direct drone-to-device video stream.


ESP32-CAM Drone Vision (Hotspot FPV) - Installation Tutorial
Prerequisites:
1. Arduino IDE: Download and install the latest version from arduino.cc/en/software.
2. ESP32-CAM Board: (AI-Thinker model is most common)
3. FTDI Programmer (USB to TTL Serial Converter):
Make sure it supports both 3.3V and 5V.
Common chips are CP2102, CH340, or FT232RL.
4. Jumper Wires (Male-to-Female): For connecting the ESP32-CAM to the FTDI programmer.
5. Micro-USB Cable: To connect the FTDI programmer to your computer.
6. (Optional but Recommended) ESP32-CAM-MB Programmer: This board acts as a shield for the ESP32-CAM and simplifies the flashing process by providing a direct USB connection and reset/boot buttons. 
If you have one, the wiring becomes much simpler.

Watch this video for Installation https://youtu.be/sr2S48lOfOU

Step 1: Prepare Arduino IDE for ESP32 Boards
1. Open Arduino IDE.
2. Add ESP32 Board Manager URL:
Go to File > Preferences
In the "Additional Boards Manager URLs" field, paste the following URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
If you have other URLs already, separate them with a comma.
Click "OK".
3. Install ESP32 Board Package:
Go to Tools > Board > Boards Manager...
Search for "ESP32" and find "ESP32 by Espressif Systems".
Click "Install". This might take a few minutes.

Step 2: Select the Correct Board and Settings
1. Go to Tools > Board > ESP32 Arduino and select
AI Thinker ESP32-CAM.
2. Set Partition Scheme:
Go to Tools > Partition Scheme
Select Huge APP (3MB No OTA) or AI Thinker ESP32-CAM (if available) or any scheme that provides at least 3MB for the application. The camera sketch requires a significant amount of space.
3. Set Upload Speed:
Go to Tools > Upload Speed.
Choose 115200 or 460800. Sometimes a lower speed is more reliable, but a higher speed is faster.

Step 3: Connect ESP32-CAM for Flashing
This is the most crucial part and often where people face issues.
Option A: Using an FTDI Programmer (Most Common)

![image](https://github.com/user-attachments/assets/33701541-4d2b-4b29-88b3-6efdea837863)

Wiring Diagram (Typical AI-Thinker ESP32-CAM to FTDI):

![image](https://github.com/user-attachments/assets/aacc5c0d-4010-4c73-a71c-7f4e35b2cc64)

Option B: Using an ESP32-CAM-MB Programmer (Easiest)
If you have this shield, simply plug your ESP32-CAM into it. It has built-in USB-to-serial conversion and usually includes reset/boot buttons for easier flashing.
Step 4: Upload Your Code
1. Open your modified code in the Arduino IDE.
Make sure you've updated const char *ssid = "ESP32_CAM_Hotspot"; and const char *password = "your_hotspot_password"; to your desired hotspot name and a strong password (at least 8 characters).
2. Connect the FTDI programmer (with the ESP32-CAM wired up) to your computer's USB port.
3. Select the Port:
Go to Tools Port and select the COM port corresponding to your FTDI programmer. If you don't see one, you might need to install the drivers for your FTDI chip (e.g., CP210x, CH340). A quick Google search for "CP210x driver" or "CH340 driver" + your operating system will help.
4. Initiate Upload:
In the Arduino IDE, click the "Upload" button (right arrow icon).
5. Enter Flashing Mode (Crucial Timing!):
The Arduino IDE will compile the code and then display "Connecting..." in the console.
At this exact moment, quickly press and release the RST (Reset) button on your ESP32-CAM board.
If using the ESP32-CAM-MB programmer: You might need to press and hold the "100" (BOOT) button while pressing and releasing the "RST" button, then release the "100" button. Some MB programmers are easier and only require you to hit the upload button.
If successful, you'il see the upload progress. If it fails with "Failed to connect to ESP32: Timed out," retry the reset button timing.
6. "Done Uploading."
Once you see "Done uploading," the code has been successfully transferred.

Step 5: Run the Code and Access the Drone Camera
1. Disconnect GPIO 0 from GND: This is absolutely critical. The ESP32-CAM will not boot into normal operation if GPIO O is still grounded.
2. Press the RST (Reset) button on the ESP32-CAM (or disconnect and reconnect power). This will restart the ESP32-CAM in normal operating mode.
3. Open the Serial Monitor in the Arduino IDE (magnifying glass icon, top right).
Set the baud rate to 115200-
You should see output similar to this:

                Setting up AP (Access Point)...AP IP address: 192.168.4.1
                Camera Ready! Connect to WiFi hotspot 'ESP32_CAM_Hotspot' and use 'http://192.168.4.1' to connect

5. On your phone/laptop/tablet:
Go to your Wi-Fi settings.
Connect to the Wi-Fi network named ESP32 CAM_Hotspot (or whatever SSID you chose).
Enter the password you set (your_hotspot_password).
6. Open a web browser (Chrome, Firefox, Safari, Edge).
7. In the address bar, type the IP address displayed in the Serial Monitor (e.g., http://192.168.4.1) and press Enter.
You should now see the live video feed from your ESP32-CAM, ready to be mounted on your drone for FPV!

# **📸 ESP32-CAM Drone Vision Hotspot FPV**

| Protocol | FPV (First-Person View) | Wireless | WiFi Hotspot |
| :---- | :---- | :---- | :---- |
| **Board** | ESP32-CAM | **Target** | Drones / Robotics |

This project provides a simple, standalone solution for adding First-Person View (FPV) to your drone or robotics project using a low-cost **ESP32-CAM**. The code configures the ESP32-CAM to act as its own Wi-Fi Access Point (hotspot), allowing any device with a web browser (like a smartphone, tablet, or laptop) to connect directly to it for a real-time live video feed.

## **✨ Key Features**

* **Standalone FPV:** The ESP32-CAM creates its own Wi-Fi network, so no external router or network is required.  
* **Plug-and-Play:** Simple to set up and get a live video feed directly in your web browser.  
* **Low-Cost Solution:** Provides FPV functionality at a fraction of the cost of dedicated analog FPV systems.  
* **Direct Connection:** Connect your smartphone or any Wi-Fi enabled device directly to the ESP32-CAM's hotspot.

## **🛠️ Hardware Required**

| Component | Notes |
| :---- | :---- |
| **ESP32-CAM Board** | AI-Thinker model is the most common. |
| **FTDI Programmer** | A USB to TTL Serial Converter that supports both 3.3V and 5V. |
| **Jumper Wires** | For connecting the programmer to the ESP32-CAM. |
| **Micro-USB Cable** | To connect the FTDI programmer to your computer. |
| **ESP32-CAM-MB Programmer** | **(Optional)** This programmer simplifies the flashing process by providing an integrated solution. |

## **⬇️ Installation and Setup**

### **Prerequisites**

1. **Install Arduino IDE.**  
2. Add the ESP32 Board Manager URL in Arduino IDE preferences.  
3. Install the **ESP32 board package** (latest version is recommended).

### **Flashing the Code**

1. **Connect Hardware:** Use jumper wires to connect your FTDI programmer to the ESP32-CAM. Ensure the programmer is set to **3.3V**.  
2. **Arduino IDE Settings:**  
   * Select the correct board: **AI Thinker ESP32-CAM**.  
   * Set the Partition Scheme to a suitable option (e.g., **"Huge APP (3MB No OTA)"**).  
   * Select the correct COM Port for your FTDI programmer.  
3. **Upload the Code:**  
   * Open the project code in the Arduino IDE.  
   * Hold down the RST button on the ESP32-CAM, then press the Upload button in the IDE. Release RST once the uploading process starts.  
   * If you're using the ESP32-CAM-MB programmer, simply connect the board and upload.

## **⚙️ Usage**

1. After a successful upload, open the Serial Monitor in the Arduino IDE to view the ESP32-CAM's hotspot information.  
2. On your smartphone or computer, connect to the new Wi-Fi network created by the ESP32-CAM.  
3. Open a web browser and navigate to the IP address displayed in the Serial Monitor.  
4. You should now see the live video feed from the ESP32-CAM.

## **⚠️ Notes**

* **Camera Initialization:** If the camera fails to initialize, try a different flash method or ensure all pins are correctly connected.  
* **Power:** A stable power supply is critical for the ESP32-CAM, especially during flashing. If you encounter issues, try a different USB port or power source.

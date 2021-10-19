<!-- Output copied to clipboard! -->

<!-----
NEW: Check the "Suppress top comment" option to remove this info from the output.

Conversion time: 1.383 seconds.


Using this Markdown file:

1. Paste this output into your source file.
2. See the notes and action items below regarding this conversion run.
3. Check the rendered output (headings, lists, code blocks, tables) for proper
   formatting and use a linkchecker before you publish this page.

Conversion notes:

* Docs to Markdown version 1.0β31
* Sun Oct 17 2021 23:47:33 GMT-0700 (PDT)
* Source doc: Controlling  a zone damper and humidifier in my HVAC system
* Tables are currently converted to HTML tables.
* This document has images: check for >>>>>  gd2md-html alert:  inline image link in generated source and store images to your server. NOTE: Images in exported zip file from Google Docs may not appear in  the same order as they do in your doc. Please check the images!

----->


<p style="color: red; font-weight: bold">>>>>>  gd2md-html alert:  ERRORs: 0; WARNINGs: 0; ALERTS: 3.</p>
<ul style="color: red; font-weight: bold"><li>See top comment block for details on ERRORs and WARNINGs. <li>In the converted Markdown or HTML, search for inline alerts that start with >>>>>  gd2md-html alert:  for specific instances that need correction.</ul>

<p style="color: red; font-weight: bold">Links to alert messages:</p><a href="#gdcalert1">alert1</a>
<a href="#gdcalert2">alert2</a>
<a href="#gdcalert3">alert3</a>

<p style="color: red; font-weight: bold">>>>>> PLEASE check and correct alert issues and delete this message and the inline alerts.<hr></p>



    **Controlling  a zone damper and humidifier in my HVAC system.**

I wanted to be able to control the duct damper to the basement that my room is in. The system in our house is forced air oil heating with A/C. There are 2 existing thermostats that control the system through a zone controller.

Because  of the small size of the basement rooms compared to the first and second floor, the basement zone was tied in with the first floor. This caused the basement to be too cold in the summer and the winter. 

My idea was to detect the operation of the system and when either of the two zones was running and open or close the basement damper as needed.

 

Since I retired after 20 years in the HVAC service business and have moderate programming experience I was determined to fix this as my first Arduino project. I started using 2 NRF 24 nanos, an arduino mega and a TFT shield from Adafruit. This worked somewhat, but the range of the nrf24 radios wasn’t good and I was having problems with the controller locking up after several hours to days. 

Over this past summer I redesigned the project to use an Adafruit feather M0 with RFM95 Lora for the controller and an Adafruit RFM95 breakout to replace the NRF 24 nanos. This was a change in the voltage of the controller to 3.3V, but all my sensors worked with 3.3 or 5 volts except the differential pressure sensor, but a voltage divider fixed that.

I will include a circuit diagram when I have time, but for now I will go over how to detect the workings of the HVAC system and how to control it.

**	Safely detecting AC voltage:**

The best method I found was to use a H11AA1 Optoisolator with this circuit with the slight alteration of adding an external 10K pullup instead of the internal one shown in this diagram that I found online:



<p id="gdcalert1" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/image1.png). Store image on your image server and adjust path/filename/extension if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert2">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![AC Detection Circuit](/assets/AC_Detector.png)


I use 3 of these circuits to detect the operation of the fan, heat, and cool signals going to the air handler/furnace.

**	Operation of Air Handlers:**

There are a few types of air handlers that you might have:



1. Oil furnace with external cooling coil (This is what I have).
2. Gas furnace with external cooling coil.
3. Electric furnace with internal cooling coil and internal electric heating elements.
4. Heat pump with internal cooling coil and internal supplemental electric heating elements.

**Sequence of operation in heat mode for oil and gas furnaces:**



1. Call for heating from thermostat - power to W and furnace but not to the G fan terminal.
2. Burner lights and warms up until the internal heat sensor calls for the fan which does not energize the G fan terminal.
3. Thermostat is satisfied and the burner shuts off - W from the thermostat is off.
4. Internal heat sensor shuts the fan off when the temperature is low enough.
5. If you have a zone controller like I do it may run the fan for an extra minute or two to purge the ducts.

**Sequence of operation in heat mode for heat pumps and electric furnaces:**



1. Call for heating - power to W and no power to O (for most heat pumps the reversing valve operates in cool mode).
2. Electric furnaces and supplemental heat strips usually have a relay that takes a minute or so to energize the strips and the fan.
3. Thermostat is satisfied and W is shut off.
4. Finally, the heat relay shuts off the heat strips and fan after another delay.

**Sequence of operation in cool mode for all air handlers:**



1. Call for cooling - power to Y and G for most systems (power to O is on for most heat pumps when in cool mode). The cooling condenser comes on along with the fan.
2. Thermostat is satisfied and Y and G are off. Some zone controllers or thermostats run the fan for a few minutes afterward.

The wiring conventions for the thermostat, thermostat wires, and equipment are:


<table>
  <tr>
   <td>Diagram Symbol
   </td>
   <td>Usual wire color
   </td>
   <td>Function
   </td>
  </tr>
  <tr>
   <td>R
   </td>
   <td>Red
   </td>
   <td>Transformer red side
   </td>
  </tr>
  <tr>
   <td>B
   </td>
   <td>Blue or black or brown
   </td>
   <td>Transformer ground side
   </td>
  </tr>
  <tr>
   <td>Y
   </td>
   <td>Yellow
   </td>
   <td>Call for cooling
   </td>
  </tr>
  <tr>
   <td>W
   </td>
   <td>White
   </td>
   <td>Call for heating
   </td>
  </tr>
  <tr>
   <td>G
   </td>
   <td>Green
   </td>
   <td>Call for fan
   </td>
  </tr>
  <tr>
   <td>O
   </td>
   <td>Orange
   </td>
   <td>Heat pump reversing valve
   </td>
  </tr>
</table>


		**Controlling and monitoring other parts of the system:**



* Zone dampers - can be power to open or power to close with a spring return.
* Whole house humidifiers - are usually attached to a ductwork bypass going from the outlet side of the air handler to the intake side. Controlled by some sort of humidity controller. Mine has a 24 VAC water valve and a power to open  type damper I installed in the bypass duct from the humidifier to replace the one that had to be done by hand.
* I installed a DS18B20-compatible temperature sensor on the north side of my house to monitor outdoor temperature.
* HEPA filter that I installed a differential pressure sensor to monitor.
* I installed a non-contact AC current sensor to monitor the amp draw of the furnace so I know when the fan is on.

	**How I implemented the control and monitoring of my system:**

		**Controller wiring and connections:**



* I enclosed my controller in a 200 x 120 x 75 mm ABS plastic junction box with a clear cover and mounting tabs.
* I carefully unbent each of the solder pins on both of the terminal blocks so I could solder them on top of the Feather board.
* I used a piece of bare perfboard inside the enclosure and screwed to the posts on the bottom. Then mounted the Feather, a half sized Perma-Proto board, the dual relay board, and the power board to the perfboard with nylon standoffs and screws. 
* On the Perma-Proto board I mounted 3 of the H11AA1 chips in sockets and the resistors and capacitors for each detector circuit. I also mounted a single neopixel breakout board, my resistor divider, and pullups for the AM2315 and one wire sensors.
* The small pressure sensor board is mounted to the plastic pneumatic tubes after they enter the enclosure through 2 holes that I aligned with the barbs on the pressure module. The sensor uses 5V power so I read it with a 3.3V voltage divider.
* I added the AC current sensor and board later in my first build so it is attached to the bottom side of the enclosure with standoffs. A 3.5mm stereo panel jack is mounted nearby and connected to the board with the end of an old stereo cable I had.
* The screw terminal block is mounted to the outside topside of the enclosure to connect to the thermostat wiring.
* To control my power to close type zone damper I used a dual relay board and connected the damper to the normally closed side of one of the relays and connected the red wire from the furnace to the common side. This way the relay coil is powered and the duct opens.
* The humidifier damper and water valve are controlled through the normally open contacts of the other relay and powered from the separate transformer on the zone controller.
* I used 2.5mm blue pneumatic tubing that came with ⅛” NPT male threaded adapters for it along with ⅛” NPT nuts to attach a pressure tap before and after my HEPA filter. The pressure port on the sensor is attached to the duct before the air goes into the filter and the vacuum after.
* The AM2315 comes with a through hole mount and gasket so it is mounted to the intake duct upstream from the humidifier bypass duct. I extended the wire it came with with some 4 conductor phone wire to one of the 5 pin aircraft connector plugs. The one wire temperature sensor was extended about 30 feet and connected the same way.

		**Picture of controller installed on HVAC duct:**



<p id="gdcalert2" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/image2.jpg). Store image on your image server and adjust path/filename/extension if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert3">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![Picture of controller installed on HVAC duct](/assets/HVAC_Controller.jpg)


**		Monitor and data logger setup:**

I used the breadboard holder base plate to mount an Arduino MEGA 2560 with the Adafruit TFT shield on it . The breadboard holds the RF95 breakout along with the AH20 module, the RTC, and a traffic light led display so I can see the status from across the room. I use it as is, but plan an enclosure in the future.

		

		**Picture of Monitor/Data Logger:**



<p id="gdcalert3" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/image3.jpg). Store image on your image server and adjust path/filename/extension if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert4">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![Picture of Monitor/Data Logger](/assets/HVAC_Monitor.jpg)


**Parts list:**



1. [ Adafruit Feather M0 RFM95 LoRa Radio (900MHz) ](https://www.adafruit.com/product/3178)
2. [Adafruit RFM95W LoRa Radio Transceiver Breakout 915 MHz](https://www.adafruit.com/product/3072)
3. [Terminal Block kit for Feather - 0.1" Pitch](https://www.adafruit.com/product/3173)
4. [Adafruit 1.8" Color TFT Shield w/microSD and Joystick - v 2](https://www.adafruit.com/product/802)
5. A DS3231 RTC (Adafruit has several types).
6. [Adafruit  AHT20 Pin Module - I2C Temperature and Humidity Sensor](https://www.adafruit.com/product/5183)
7. Arduino MEGA 2560.
8. [Waterproof 1-Wire DS18B20 Compatible Digital temperature sensor](https://www.adafruit.com/product/381)
9. [2 Channel DC 5V Relay Module with Optocoupler High/Low Level Trigger](https://www.amazon.com/gp/product/B079FGPC9Y/ref=ppx_yo_dt_b_asin_title_o04_s01?ie=UTF8&psc=1)
10. [Gravity: Analog AC Current Sensor (20A)](https://www.amazon.com/gp/product/B0834T9T7M/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
11. [H11AA1 Optocoupler AC Input  DC Output (Pack of 10)](https://www.amazon.com/gp/product/B00B88AJK4/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) (also at Digikey)
12. [AM2315 - Encased I2C Temperature/Humidity Sensor](https://www.adafruit.com/product/1293)
13. [Diymore Breakout Board MPXV7002DP Transducer APM2.5 APM2.52 Differential Pressure Sensor](https://www.amazon.com/gp/product/B01MCVOZJO/ref=ppx_od_dt_b_asin_title_s00?ie=UTF8&psc=1) (price has doubled due to silicon shortage!)
14. [Yeeco AC/DC to DC Step Down Converter AC 2.5-27V DC 3-40V 24V 36V to DC 1.5-27V 12V Voltage Regulator Board 3A Adjustable Volt Transformer Power Supply Module](https://www.amazon.com/dp/B00SO4T7IU?psc=1&ref=ppx_yo2_dt_b_product_details)
15. [TAILONZ PNEUMATIC Blue 4mm or 5/32 inch OD 2.5mm ID Polyurethane PU Air Hose Pipe Tube Kit 10 Meter 32.8ft](https://www.amazon.com/gp/product/B07RK4MCKP/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
16. Solid Brass Hex Nuts 1/8” NPT Female Thread
17. [Hxchen 5 Pin Metal Male Female Panel Connector 16mm GX16-5P Silver Aviation](https://www.amazon.com/gp/product/B07V9BQC7P/ref=ppx_yo_dt_b_asin_title_o09_s00?ie=UTF8&psc=1)
18. [Micro USB B Jack to USB A Plug Round Panel Mount Adapter](https://www.adafruit.com/product/4213)
19. [DIY USB Cable Parts - Right Angle Micro B Plug Down](https://www.adafruit.com/product/4105)
20. [DIY USB or HDMI Cable Parts - 10 cm Ribbon Cable](https://www.adafruit.com/product/3560)
21. [DIY USB Cable Parts - Straight Type A Plug](https://www.adafruit.com/product/4109)
22. [Mini Panel Mount DPDT Toggle Switch](https://www.adafruit.com/product/3220)
23. [3.5mm Stereo Female terminal block panel mount connector](https://www.amazon.com/gp/product/B077XPSKQD/ref=ppx_yo_dt_b_asin_title_o09_s00?ie=UTF8&psc=1)
24. [GTSE 10 Pack of 12-Way 3 Amp Electrical Terminal Block,](https://www.amazon.com/dp/B08LNWMMHQ?psc=1&ref=ppx_yo2_dt_b_product_details)
25. [SunFounder RAB Holder Raspberry Pi Breadboard Holder 5 in 1 Base Plate with Rubber Feet for Arduino R3 Mega 2560](https://www.amazon.com/gp/product/B081167YZM/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)

**Programming:**

`I used the Radiohead library that Adafruit recommends.`I decided to use structs to exchange data between the monitor and the controller. I found that it works fine if you limit the variables to unsigned integers and booleans. When I tried it with floats or integers it gave nonsense returns. 

I converted the OD temperature and amps reading to a uint16_t by multiplying by 10 at the controller, and taking the absolute value, and using a bool for the presence of a negative sign for the temperature. 

This is how I set up my structs for both transmitter and receiver:


```
struct DataRX
{
  uint16_t Amps;
  uint16_t pressure;
  uint16_t ODtemp;
  uint8_t duct_RH;
  uint8_t systemState;
  uint8_t RH_Set;
  bool humidControl;
  bool negSign;
} ControlData = {0, 0, 0, 0, 2, 0, false, false};

struct DataTX
{
  bool ductControl;
  uint8_t humidSet;
} MonitorData = {false, 35};
Next we have the function I used for the monitor to transmit data and request a reply from the controller along with the conversion of the float data from the controller:
```



```
void exchangeData()
{
  rf95.send((uint8_t *)&MonitorData, sizeof(MonitorData));
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  delay(10);

  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
    {
      memcpy(&ControlData, buf, sizeof(ControlData));
      ODTemp = ControlData.ODtemp / 10.0;
      if (ControlData.negSign) ODTemp = -ODTemp;
      Amps = ControlData.Amps / 10.0;
    }
  }
}
```


**Here is the corresponding receiving code at the controller:**


```
if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      memcpy(&MonitorData, buf, sizeof(MonitorData));
      // Send a reply
      rf95.send((uint8_t *)&ControlData, sizeof(ControlData));
      rf95.waitPacketSent();
    }
  }
```


**Notes on the rest of the code:**



* I used a while loop to change the cool, heat, humidity, and lower display settings which are updated to the EEPROM when exiting.
* I had to add a watchdog timer to the code because the monitor was locking up after hours to days of running. The EEPROM updates the settings on reboot.
* If anyone can tell me why this is happening, I would be grateful, but It works fine as is.
* I commented in the code fairly regularly, the rest is mostly printing the display and logging the data I want.
* I log the data at 20 minute intervals when the system is running and at 20 seconds if it is.

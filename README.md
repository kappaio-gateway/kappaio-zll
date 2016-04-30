# kappaio-zll
  
This package enables the kappaIO ZigBee gateway to factory-reset and take control of ZLL devices such as Philips Hue's light bulbs. Main feature includes:

* **Scan for ZLL devices** - The ability scan over all valid ZLL channels and return a list of ZLL devices that include orphaned and joined devices.
* **Factory reset a ZLL device** - This is useful when you want to take control of a device that has already joined a network or when you want to kick one out of your network. A common example is that you want to use kappaBox to control a Hue bulb that is already connected to a Hue hub or, conversely, you want the Hue hub to take back the a bulb that is currently in kappaBox network. 
* **Join a ZLL device** - Once a ZLL bulb is factory reset, you can let it join kappaBox's HA netowrk.
* **Turning On/Off**  
* **Adjust Brightness** - You can also set the level-transition speed.
* **Change color** - You can also et the color-transistion speed.

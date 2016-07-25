# Sentilo client library for Arduino

A Sentilo client HTTP Request library for Arduino and the Ethernet shield.

# Install

Download and unzip the file to `~/Documents/Arduino/libraries` where `~/Documents/Arduino` is your sketchbook directory.

    > cd ~/Documents/Arduino
    > mkdir libraries
    > cd libraries
    > copy the file contents here
    
You can import the **/release/sentilo-client-arduino.zip** library file form the Arduino **IDE > Program > Include library > Add .ZIP Library...** too

# Usage

### Include

You need to have the `Ethernet` library already included.

```c++
#include <Ethernet.h>
#include <SPI.h>
#include "SentiloClient.h"
```

### Run the sample code
In the Arduino IDE, navigate to **File > Examples > sentilo-client-arduino** and select one of the three examples that you will see into the forldes

* SentiloClient-Example-01.ino
* SentiloClient-Example-02.ino
* SentiloClient-Example-03.ino

### Configure the connection
You must provide some custom configuration before run these sample codes:

```c++
char* apiKey = "YOUR_API_KEY";
char* ip = "YOUR_IP_ADDRESS";
int port = YOUR_PORT;
```

Please, provide your credentials or api key, the Sentilo instance host ip and it port. After taht, you can upload the sketch to the Arduino devide.

We hope you enjoy it!

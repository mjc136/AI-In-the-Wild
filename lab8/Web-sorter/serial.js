(function() {
  'use strict';

  window.port = null; // Make port globally accessible

  const observer = new MutationObserver(mutations => {
    for (let mutation of mutations) {
      if (mutation.type === 'childList') {
        let connectButton = document.querySelector("#connect");
        if (connectButton) {
          console.log("Connect button found!");

          function connect() {
            console.log("Attempting to connect to Arduino...");
            window.port.connect().then(() => {
              console.log("Arduino connection established successfully!");
              connectButton.textContent = 'DISCONNECT';

              window.port.onReceive = data => {
                let textDecoder = new TextDecoder();
                console.log("Data received from Arduino:", textDecoder.decode(data));
              };

              window.port.onReceiveError = error => {
                console.error("Error receiving data from Arduino:", error);
              };
            }).catch(error => {
              console.error("Connection error: ", error);
            });
          }

          connectButton.addEventListener('click', function() {
            console.log("Connect button clicked");
            if (window.port) {
              console.log("Disconnecting from Arduino...");
              window.port.disconnect();
              connectButton.textContent = 'CONNECT ARDUINO';
              window.port = null;
              console.log("Disconnected from Arduino");
            } else {
              console.log("Requesting Arduino port...");
              serial.requestPort().then(selectedPort => {
                if (!selectedPort) {
                  console.error("No port selected! Try again.");
                  return;
                }
                console.log("Arduino port selected:", selectedPort);
                window.port = selectedPort;
                connect();
              }).catch(error => {
                console.error("Port request failed:", error);
              });
            }
          });

          observer.disconnect(); // Stop observing once button is found and handled
          break;
        }
      }
    }
  });

  const targetNode = document.body;
  const config = { childList: true, subtree: true };
  observer.observe(targetNode, config);
})();

// WebUSB Serial Handling
var serial = {};

(function() {
  'use strict';

  serial.getPorts = function() {
    return navigator.usb.getDevices().then(devices => {
      console.log("Detected devices:", devices);
      return devices.map(device => new serial.Port(device));
    });
  };

  serial.requestPort = function() {
    const filters = [
      { 'vendorId': 0x2341, 'productId': 0x8036 },
      { 'vendorId': 0x2341, 'productId': 0x8037 },
      { 'vendorId': 0x2341, 'productId': 0x804d },
      { 'vendorId': 0x2341, 'productId': 0x804e },
      { 'vendorId': 0x2341, 'productId': 0x804f },
      { 'vendorId': 0x2341, 'productId': 0x8050 }
    ];
    return navigator.usb.requestDevice({ 'filters': filters })
      .then(device => {
        console.log("Device selected:", device);
        return new serial.Port(device);
      })
      .catch(error => {
        console.error("User did not select a device:", error);
        return null;
      });
  };

  serial.Port = function(device) {
    this.device_ = device;
    this.interfaceNumber_ = 2;
    this.endpointIn_ = 5;
    this.endpointOut_ = 4;
  };

  serial.Port.prototype.connect = function() {
    let readLoop = () => {
      this.device_.transferIn(this.endpointIn_, 64).then(result => {
        this.onReceive(result.data);
        readLoop();
      }, error => {
        console.error("Read error:", error);
        this.onReceiveError(error);
      });
    };

    console.log("Starting device connection process...");
    return this.device_.open()
        .then(() => {
          console.log("Device opened successfully");
          if (this.device_.configuration === null) {
            console.log("No configuration selected, selecting configuration 1");
            return this.device_.selectConfiguration(1);
          }
        })
        .then(() => {
          console.log("Finding interface for Arduino communication...");
          var interfaces = this.device_.configuration.interfaces;
          interfaces.forEach((element, index) => {
            element.alternates.forEach((alt, altIndex) => {
              if (alt.interfaceClass === 0xff) {
                console.log(`Found Arduino-compatible interface: ${element.interfaceNumber}`);
                this.interfaceNumber_ = element.interfaceNumber;
                alt.endpoints.forEach(endpoint => {
                  if (endpoint.direction === "out") {
                    this.endpointOut_ = endpoint.endpointNumber;
                  }
                  if (endpoint.direction === "in") {
                    this.endpointIn_ = endpoint.endpointNumber;
                  }
                });
              }
            });
          });
        })
        .then(() => this.device_.claimInterface(this.interfaceNumber_))
        .then(() => this.device_.selectAlternateInterface(this.interfaceNumber_, 0))
        .then(() => this.device_.controlTransferOut({
            'requestType': 'class',
            'recipient': 'interface',
            'request': 0x22,
            'value': 0x01,
            'index': this.interfaceNumber_
        }))
        .then(() => {
          console.log("Connection successful, starting read loop...");
          readLoop();
        })
        .catch(error => {
          console.error("Connection failed:", error);
        });
  };

  serial.Port.prototype.disconnect = function() {
    return this.device_.controlTransferOut({
            'requestType': 'class',
            'recipient': 'interface',
            'request': 0x22,
            'value': 0x00,
            'index': this.interfaceNumber_
        })
        .then(() => this.device_.close())
        .then(() => console.log("Device successfully disconnected"))
        .catch(error => console.error("Error while disconnecting:", error));
  };

  serial.Port.prototype.send = function(data) {
    console.log(`Sending data: ${data} to endpoint ${this.endpointOut_}`);
    return this.device_.transferOut(this.endpointOut_, data)
      .then(result => {
        console.log("Data sent successfully:", result);
        return result;
      })
      .catch(error => {
        console.error("Error sending data:", error);
      });
  };

})();

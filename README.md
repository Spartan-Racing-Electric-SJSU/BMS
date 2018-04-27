# BMS
The Battery Management System emulators and demo code for programming the charge/discharge states of the SRE battery modules and cell monitoring  

Consider following the Linear Technology tutorial to gather all the required software and how to run the example:  
http://cds.linear.com/docs/en/demo-board-manual/dc2026cfe.pdf  

<<<<<<< HEAD
The current IC for the Battery Monitor is the 6804, but we will be updating to the 6811 for the SRE-4 configuration:
http://www.analog.com/media/en/technical-documentation/data-sheets/68111fb.pdf

The active balancing IC on the BMBs are the 8584's:
http://www.analog.com/media/en/technical-documentation/data-sheets/8584fb.pdf

Consult the linduino API library on where to start programming. We use the LTC6804 library for battery monitoring and communication via I2C along with the Sparkfun MCP2515 CAN Bus Shield libraries. 
=======
>>>>>>> 6b3226d0ac87ce9833bdac3e537a1ed4b99427ec

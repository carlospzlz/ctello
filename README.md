# Introduction
CTello is C++ library to interact with the DJI Ryze Tello Drone using the Tello SDK 2.0.
The purpuse of this project is to encapsulate the UDP socket communication and provide
a clean and easy-to-use interface to control the drone. However, CTello is agnostic
to the commands that are sent and gives full control to the user to send any message
to the device. CTello has no clutter that might hide parts of the Tello communication
protocol, or restrict the interaction with the drone in any way.

# Sim Race Dashboard for ESP32 (Arduino)

<!--![Sim Race Dashboard](dashboard_image.jpg)-->

## Overview
This project is a Sim Race Dashboard implemented using the C language on the ESP32 microcontroller within the Arduino workspace. It provides a real-time display of telemetry data from racing simulation games on a TFT screen. By receiving UDP packets from the game, the ESP32 processes and translates this data into meaningful information, offering players crucial insights into their performance during races.

## Features
- **Real-time Data Display**: Receive and interpret UDP packets from racing simulation games, displaying key telemetry data on the TFT screen.
- **Customizable Interface**: Customize the layout and design of the dashboard to suit individual preferences and requirements.
- **Multi-game Compatibility**: Designed to work with a variety of popular racing simulation games, ensuring flexibility and widespread usability.
- **Low-latency Communication**: Utilizes efficient UDP communication for minimal delay, maintaining responsiveness during gameplay.
- **Scalable Architecture**: Built on the ESP32 microcontroller platform, offering scalability and potential for future expansion and enhancements.
- **User-friendly Setup**: Simple and straightforward setup process, allowing users to quickly integrate the dashboard into their racing setup.

## Requirements
- ESP32 Microcontroller
- TFT Display (compatible with ESP32)
- Racing Simulation Game (compatible with UDP telemetry output)

## Installation
1. Clone or download the repository to your local machine.
2. Open the project in the Arduino IDE.
3. Connect your ESP32 microcontroller and TFT display to your computer.
4. Upload the sketch to your ESP32 board.
5. Configure the racing simulation game to output UDP telemetry data to the ESP32's IP address and port.

## Usage
1. Power on the ESP32 and TFT display.
2. Launch your preferred racing simulation game.
3. Start a race or training session.
4. The dashboard will receive and display real-time telemetry data on the TFT screen.
5. Monitor your performance and make adjustments as necessary during gameplay.

## Contribution
Contributions to the project are welcome! Whether it's bug fixes, feature enhancements, or general improvements, feel free to fork the repository and submit pull requests.

## License
This project is licensed under the [MIT License](LICENSE).

## Acknowledgments
- This project was inspired by the passion for racing simulations and the desire to create a customizable dashboard solution for enthusiasts.
- Special thanks to the open-source community for their contributions to the libraries and tools used in this project.

<p align="center">
    <img src="https://github.com/OPEnSLab-OSU/Loom/blob/gh-pages/Aux/OPEnSLogo.png" alt="logo" width="100" height="100">
  </a>
</p>

<h3 align="center">Dendrometer</h3>

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5126776.svg)](https://doi.org/10.5281/zenodo.5126776)

## Introduction
The Dendrometer is an ongoing project that strives to enable ecological researchers and/or care-takers of grape plants to be able to effectively measure diurnal fluctuations in grape vines. Such information can be crucial for producing market-ideal grapes, as the shrinkage and growth of the vines are important indicators of when the plant is in need of water. Learn more about the design and outcomes on our <a href="https://github.com/OPEnSLab-OSU/OPEnS-Lab-Home/wiki/Dendrometer">Dendrometer Lab Wiki</a>. 

## Key Features
  - Data is collected and written to a SD card every 15 minutes, allowing the system to operate for long durations without human intervention
  - Devices can wirelessly report data back to a remote server by communicating with an internet-connected "hub" device
  - Magnetic encoder ([AS5311](https://ams.com/documents/20143/36005/AS5311_DS000200_2-00.pdf)) has a resolution of ~488nm with an extremely low temperature dependency (ideal for outdoor use)
  - Integration with [Loom](https://github.com/OPEnSLab-OSU/Loom-V4), which will allow this device to support plug-and-play functionalities with integrated sensors

## Directory 
### PCB: 
Printed circuit board (PCB) design files for the nodes. 
### src: 
Contains all of the code used on the dendrometer.
LoRa: Contains the source code for the node sensors and the LTE hub. This uses LoRa batch uploading which will upload all of the collected data at set intervals.
LoRa Non Batch: Contains the source code for the node sensors and the LTE hub. This will only upload ONE packet of data at set intervals. All of the data is still saved on SD card.
WiFi: Contains the source code for the WiFi connected nodes.

## Relevant Documents
* [User Manual](https://docs.google.com/document/d/1OHKa2jYamG2kPEfxrQMzi3MNHZGgkPIG/)
* [HardwareX Paper](https://www.sciencedirect.com/science/article/pii/S246806722100078X)
* [Troubleshooting Guide](https://docs.google.com/document/d/1E4zUMmSh5PVmP7RXUqQrh7S9IGuxQKPj5EQ5KcT2HYE/edit?usp=sharing)
* [Mechanical Assembly Guide](https://docs.google.com/document/d/1LzM_XnqVP9GkIqY2_pnTvdrFFe5u78ZPXJizvT70y4o/edit?usp=sharing)
* [Electronics Assembly Guide](https://docs.google.com/document/d/1BaNuNSJrL86lxKixB8chgZGySgj0bhYSMtVzJu6EDW4/edit?usp=sharing)
* [MongoDB Dashboard Guide](https://docs.google.com/document/d/1PlT9hrC62OXXhL8AD17e3FiPCwlHFJaZIvXCRnmihIE/edit?usp=sharing)
* [Installation Guide (Outdated)](https://docs.google.com/document/d/1GR2qeMWASApCwk-RbZwvsvWmB1qEz0QU8RcxwjgoDQY/edit?usp=sharing)

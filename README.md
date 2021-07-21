<p align="center">
    <img src="https://github.com/OPEnSLab-OSU/Loom/blob/gh-pages/Aux/OPEnSLogo.png" alt="logo" width="100" height="100">
  </a>
</p>

<h3 align="center">Dendrometer</h3>

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4482948.svg)](https://doi.org/10.5281/zenodo.4482948)

## Introduction
The Dendrometer is an ongoing project that strives to enable ecological researchers and/or care-takers of grape plants to be able to effectively measure diurnal fluctuations in grape vines. Such information can be crucial for producing market-ideal grapes, as the shrinkage and growth of the vines are important indicators of when the plant is in need of water. Learn more about the design and outcomes on our <a href="https://github.com/OPEnSLab-OSU/OPEnS-Lab-Home/wiki/Dendrometer">Dendrometer Lab Wiki</a>. 

## Key Features
  - Data is collected and written to both SD and a desired Google Sheet, allowing the system to live on the plant for long durations without human intervention
  - Magnetic encoder ([AS5311](https://ams.com/documents/20143/36005/AS5311_DS000200_2-00.pdf)) has a resolution of ~488nm with an extremely low temperature dependency (ideal for outdoor use).
  - Integration with [Loom](https://github.com/OPEnSLab-OSU/Loom), which will allow this device to support plug-and-play functionalities with integrated sensors

## State Machine Diagram

  - Takes measurements every 15 minutes (can be adjusted to a user's desired time interval) or when user presses external interrupt button
  - Uses temperature and humidity to calculate vapor pressure deficit (VPD) 
  - Lights up Neopixel red, yellow, or green based on the position of the magnet
 
<p align="center">
    <img src="https://user-images.githubusercontent.com/55998145/122619924-7ab67c80-d046-11eb-89d6-90274238d09d.png" alt="State Machine Diagram" width="100%">
  </a>
</p>

## Relevant Documents
  - [Build Guide](https://docs.google.com/document/d/1UgnCFajMevp0Vaihs5oVVA03wMTq4PARM_in11W46gY/edit?usp=sharing)
  - [Installation Guide](https://docs.google.com/document/d/1NiuglBpe8psrLQSMZvtmF9n10cGM1V3yZzmebPv-ixE/edit?usp=sharing)
  - [Troubleshooting Guide](https://docs.google.com/document/d/1QDNaB-4S5bd0xeSMEKG0BypeyKP9h2yMQe_7OiwiJes/edit?usp=sharing)

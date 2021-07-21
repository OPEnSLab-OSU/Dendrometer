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
  
## Mechanical

  - Mechanical structure made out of carbon fiber to mitigate thermal expansion in varied temperature environments
  - Spring used to avoid friction and hysteresis between back and forth motion. Controls movement between growth and shrinkage of plant

<p align="center">
    <img src="https://user-images.githubusercontent.com/55998145/126409097-a3b4d8b0-97dd-426b-9cd0-91849a04449e.png" alt="mechanical structure" width="40%">
  </a>
</p>

## Electrical

<img align="right" src="https://user-images.githubusercontent.com/55998145/108146037-8a11c300-7070-11eb-9dc2-e7b130155b2e.jpg" width="250">  

  - Includes Neopixel LED to indicate status of magnet and interrupt button to wake up system from sleep
  - Uses SHT30 to record temperture and humidity in a weatherproof casing

## State Machine Diagram

  - Takes measurements every 15 minutes (can be adjusted to a user's desired time interval)
  - Uses temperature and humidity to calculate vapor pressure deficit (VPD) 
 
<p align="center">
    <img src="https://user-images.githubusercontent.com/55998145/122619924-7ab67c80-d046-11eb-89d6-90274238d09d.png" alt="State Machine Diagram" width="100%">
  </a>
</p>

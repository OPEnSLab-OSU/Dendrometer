<p align="center">
    <img src="https://github.com/OPEnSLab-OSU/Loom/blob/gh-pages/Aux/OPEnSLogo.png" alt="logo" width="100" height="100">
  </a>
</p>

<h3 align="center">Dendrometer</h3>

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5126776.svg)](https://doi.org/10.5281/zenodo.5126776)

## Introduction
The Dendrometer is an ongoing project that strives to enable ecological researchers and/or care-takers of grape plants to be able to effectively measure diurnal fluctuations in grape vines. Such information can be crucial for producing market-ideal grapes, as the shrinkage and growth of the vines are important indicators of when the plant is in need of water. Learn more about the design and outcomes on our <a href="https://github.com/OPEnSLab-OSU/OPEnS-Lab-Home/wiki/Dendrometer">Dendrometer Lab Wiki</a>. 

## Key Features
  - Data is collected and written to both SD and a desired Google Sheet every 15 minutes, allowing the system to live on the plant for long durations without human intervention
  - Magnetic encoder ([AS5311](https://ams.com/documents/20143/36005/AS5311_DS000200_2-00.pdf)) has a resolution of ~488nm with an extremely low temperature dependency (ideal for outdoor use).
  - Integration with [Loom](https://github.com/OPEnSLab-OSU/Loom), which will allow this device to support plug-and-play functionalities with integrated sensors

## Directory
### AS5311_Prototype: 
Magnet sensor design for Dendrometer. Contains all of the code in this Dendrometer iteration used for testing and field deployments. 
### Mechanical Components: 
Drawings and mechanical design files for the carbon fiber Dendrometer mechanical piece and 3D printed parts for the pelican case.
### PCB: 
Printed circuit board (PCB) files for both the hub and node. 
### Potentiometer_Prototype: 
[OUTDATED] Code and pictures of potentiometer Dendrometer design

## Relevant Documents
  - [Build Guide](https://docs.google.com/document/d/1UgnCFajMevp0Vaihs5oVVA03wMTq4PARM_in11W46gY/edit?usp=sharing)
  - [Installation Guide](https://docs.google.com/document/d/1NiuglBpe8psrLQSMZvtmF9n10cGM1V3yZzmebPv-ixE/edit?usp=sharing)
  - [Troubleshooting Guide](https://docs.google.com/document/d/1QDNaB-4S5bd0xeSMEKG0BypeyKP9h2yMQe_7OiwiJes/edit?usp=sharing)

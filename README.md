# Overview
This repository contains links to the to Microsoft repositories that it uses.  These repositories 
are at https://os.mbed.com/users/AzureIoTClient/.  This code is in turn derived from  the original 
Azure SDK which is located at https://github.com/Azure/azure-iot-sdk-c.  All this code is maintained 
by Microsoft.  

This SDK repository simplifies the organization, removes the source file duplication present in 
the original repositories, and streamlines the contents to only what is needed by a user. The unique 
code in this SDK repository is the Mbed O/S interfacing code, located within the **platform** directory.

Two directories containing source code are **azure_c_utility** and **platform**.  The **azure_c_utility** 
directory is largely unmodified as delivered by Microsoft except for organization. As previously stated, the **platform** directory contains changes for the the Mbed environment.

An example project that utilizes this repository is https://github.com/Avnet/azure-iot-mbed-client.


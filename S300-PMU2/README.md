# S300-PMU2
Kernel Driver for Raspberry Pi 2

This will enable BCM2836 Processor Performance Monitoring Unit counters and disable it upon exit. 

The code is based on ARM V7 MediaTek android kernel driver [look here](https://android.googlesource.com/kernel/mediatek/+/045e24819c0deb2fe15306b8d38060beadb56d2f/drivers/misc/mediatek/met/core/v7_pmu_hw.c). 

It is merely for Ring Buffer Protection. Which we will count number of branches taken place in the PLC program scan cycle.
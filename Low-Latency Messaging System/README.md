# Low-latency Messaging System

A high-performance messaging framework written in C++ that simulates the communication patterns and timing constraints found in high-frequency trading systems.

This project focuses on achieving real-time message transmission with minimal latency and is built from the ground up using socket programming, custom message serialization, and benchmarking tools.


## Features

* Custom TCP and UDP socket communication
* Benchmarked message latency with high throughput
* ACK-based reliability (for UDP)
* Multiclient support using `select()`
* Optional Flask dashboard for real-time stats (Python)

## Getting Started

### Prerequisites

* C++17 or later
* make installed
* Linux/macOS terminal

## Learning Objectives

* Understand low-level socket programming in C++
* Explore trade-offs between TCP vs. UDP in real-time systems
* Measure and optimize for low-latency constraints
* Gain experience building systems similar to those used in high-frequency trading

## Authors

* Yohann A. Moraes
* yohannmoraes04@gmail.com
* Rising Junior @ UCSC

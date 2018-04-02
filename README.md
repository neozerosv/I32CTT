Integer 32-bit Control, Transport & Telemetry (I32CTT) Protocol
===============================================================
I32CTT is a state-less protocol developed by the Hackerspace San Salvador
to transmit 32-bit messages using packets of data between "nodes".

Each node in I32CTT could take the role of "master" (if the node ask for
data) or "slave" (if the node sends data). However, a node could act as
both master or slave for other nodes depending on the implementation.

Each node can expose "endpoints", each "endpoint" in a node represents
a group of 32-bit records identified by unique addresses. To make the system
more human-friendly each endpoint can be identified by three-leter ASCII
ID. Each node could implement up to 255 endpoints and 65,536 individual records.

The only underlying condition for the protocol to work is to have a medium
wich supports packet messaging. This makes it possible to migrate this
protocol for use over UDP or even Point-to-Point links.

## How it works?
I32CTT follows a master-slave architecture. And it works making "Queries" and
listening for "Replies". However, the protocol is designed to be fully state-less.
There is no guarantee of delivery of any message. Implementations should
be designed with this characteristic in mind.

The protocol currently supports the following messages:
* **Read**(***endpoint***, ***address_1*** ... ***address_n***): Sent by the master to read records
  from the specified endpoint.
* **Answer Read**(***endpoint***,(***address_1***, ***data_1***)...(***address_n***, ***data_n***)):
  Response sent by the slave with the data requested for the specified addresses.
* **Write**(***endpoint***,(***address_1***, ***data_1***)...(***address_n***, ***data_n***)):
  Sent by the master to write records on the specified endpoint addresses.
* **Answer Write**(***endpoint***, ***address_1*** ... ***address_n***): Response sent by the slave
  with the affected records on the node.

The protocol always assumes a point-to-point topology between the nodes, even
if the underlying technology allows other topologies. For this reason things like
address resolution should be resolved by the underlying "interface".

## Supported Hardware
For now I32CTT is only supported on the following hardware:
* [PJRC Teensy 3.2](https://www.pjrc.com/teensy/) (IEEE 802.15.4 / Serial link)
* [Arduino](https://www.arduino.cc/) (Using serial links)
* [Raspberry Pi](https://www.raspberrypi.org/) (IEEE 802.15.4*)

*Partially supported, requires [OpenLabs.co AT86RF233 radios](http://openlabs.co/OSHW/Raspberry-Pi-802.15.4-radio).

Supported underlying interfaces:
* Null interface (echoes locally)
* Microchip AT86RF233 (IEEE 802.15.4 radios)
* UART link

## License
Copyright 2017-2018 Hackerspace San Salvador
Contributors: Mario Gómez / Joksan Alvadado

 I32CTT is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 I32CTT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with I32CTT.  If not, see <http://www.gnu.org/licenses/>.

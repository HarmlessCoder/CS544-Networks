# CS544: Assignment-1
---
Note: Open this readme.md file in GitHub for better readability.

## Packet Switch Queuing Simulation

This program simulates the performance of queuing in a packet switch with different scheduling mechanisms. It consists of three phases: traffic generation, packet scheduling, and packet transmission. The input parameters include the number of switch input and output ports, buffer size, packet generation probability, queue scheduling technique, and simulation time. The output includes metrics such as average packet delay, average link utilization, and KOUQ drop probability.

## Compilation
First, compile the program using g++ with the following command:

```sh
g++ -o routing routing.cpp
```

## Usage

```bash
./routing-N switchportcount-B buffersize-p packetgenprob-queue INQ/KOUQ/iSLIP-K knockout-out-outputfile-T maxtimeslots
```
## Example Command

```bash
./routing-N 8 -B 4 -p 0.5 -queue INQ -T 10000
```

## Inputs

- Number of switch input and output ports (`N`)
- Buffer size (`B`)
- Packet generation probability (`p`)
- Queue scheduling technique (`INQ`, `KOUQ`, `iSLIP`)
- K value for KOUQ (default is `0.6N`)
- Output file for results
- Maximum number of time slots (default is `10000`)

## Switch Operation

1. **Traffic Generation:** Each port generates packets with probability `p`. Packets are randomly assigned a destination port and a start time within the time slot.
2. **Scheduling:** 
   - `INQ`: Non-contentious packets are selected for transmission. Contending packets are randomly selected.
   - `KOUQ`: K packets per output port are queued based on arrival time. If more than K packets arrive, K are randomly selected for buffering, and the rest are dropped.
   - `iSLIP`: Implements the iSLIP scheduling algorithm with VOQ.
3. **Transmission:** Packets are transmitted from the head of the queue.

## Outputs

- Average packet delay (`Avg PD`)
- Standard deviation of packet delay (`Std Dev of PD`)
- Average link utilization (`Avglink utilization`)
-  KOUQ drop probability 

## Example Output Line

```
(N)  (p)  (Queue Type)  (Avg PD)  (Std Dev of PD)  (Avglink utilization)
```

## Performance Evaluation

The program allows for the evaluation of different queuing strategies in a packet switch, providing insights into packet delay, link utilization, and drop probability under various conditions.

---

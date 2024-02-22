// Including all necessary libraries
#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <unordered_set>
#include <limits.h>
using namespace std;

// Packet structure
struct Packet
{
    int src_port;  // source port
    int dest_port; // destination port
    double arrival_time;
};

// Switch class
class Switch
{
private:
    int num_ports;          // number of ports in the switch
    int buffer_size;        // buffer size of the queue
    double packet_gen_prob; // Packet generation Probability
    string queue_type;      // INQ/KOUQ/iSLIP
    int max_time_slots;
    double knockout; // knockout
    double offset = 0.0;

    // Queues for each port
    vector<queue<Packet>> input_queues;
    vector<queue<Packet>> output_queues;

    // Random number generator
    default_random_engine generator;
    uniform_real_distribution<double> distribution;

    // Metrics initialization
    double total_packet_delay = 0;
    double total_link_utilization = 0;
    int total_transmitted_packets = 0;
    int total_dropped_packets = 0;
    int total_packets = 0;

    // Phase 1- Traffic generation
    void generateTraffic(int time_slot)
    {
        for (int src_port = 0; src_port < num_ports; ++src_port)
        {
            if (distribution(generator) < packet_gen_prob) // generate a packet with probability packetgenprob and cosidering buffer size too
            {
                total_packets++;

                if (input_queues[src_port].size() < buffer_size) // checking if input queue is not full
                {
                    Packet packet;
                    packet.src_port = src_port;
                    packet.dest_port = distribution(generator) * num_ports; // Random destination port number
                    packet.arrival_time = (double)time_slot + offset;       // offset for delay calculation (by default 0)
                    input_queues[src_port].push(packet);
                }
                else
                {
                    total_dropped_packets++; // input queue full, packet dropped
                }
            }
        }
    }

    // Phase 2- Packet scheduling
    void schedulePackets()
    {
        // INQ(Input Queueing) Scheduling
        if (queue_type == "INQ")
        {

            vector<vector<Packet>> temp(num_ports); // Vector to store Packet Request in output port

            // Scheduling packets
            for (int src_port = 0; src_port < num_ports; ++src_port)
            {
                if (!input_queues[src_port].empty())
                {
                    Packet packet = input_queues[src_port].front();
                    temp[packet.dest_port].push_back(packet);
                }
            }

            // Selecting a Random Packet for transmission
            for (int dest_port = 0; dest_port < num_ports; ++dest_port)
            {
                if (temp[dest_port].size()) // Checking if there a packet request at dest. port
                {

                    if (output_queues[dest_port].size() < buffer_size)
                    {
                        int ind = rand() % temp[dest_port].size(); // Random Packet from the requests
                        output_queues[dest_port].push(temp[dest_port][ind]);
                        input_queues[temp[dest_port][ind].src_port].pop();
                    }
                    else
                    {
                        total_dropped_packets++; // packet dropped because of output queue full
                    }
                }
            }
        }
        // KOUQ(Knockout with Urgency Queuing) Scheduling
        else if (queue_type == "KOUQ")
        {
            int K = num_ports * knockout;
            vector<vector<Packet>> pkt_received(num_ports); // vector to store packet requests at output queue
            // Packet Requests at Output queue
            for (int src_port = 0; src_port < num_ports; src_port++)
            {
                if (!input_queues[src_port].empty()) // checking if input queue have a packet or not
                {
                    Packet packet = input_queues[src_port].front();
                    pkt_received[packet.dest_port].push_back(packet);
                    input_queues[packet.src_port].pop();
                }
            }

            // Scheduling packets
            for (int dest_port = 0; dest_port < num_ports; dest_port++)
            {
                if (!pkt_received[dest_port].empty()) // Checking if there a packet request at dest. port
                {
                    int sz = pkt_received[dest_port].size(); // Number of packet requests at dest. port
                    if (sz <= K)
                    {
                        for (int j = 0; j < sz; j++)
                        {
                            Packet selected_pkt = pkt_received[dest_port][j];
                            if (output_queues[selected_pkt.dest_port].size() < buffer_size)
                            {
                                output_queues[selected_pkt.dest_port].push(selected_pkt);
                            }
                            else
                            {
                                total_dropped_packets++; // packet dropped because of output queue full
                            }
                            // input_queues[selected_pkt.src_port].pop();
                        }
                    }
                    else
                    {
                        total_dropped_packets += (sz - K); // A maximum of K packets (per output port) that arrive in a given slot are queued.

                        // Selecting K random packets for buffering using a unordered set (No duplicate values)
                        unordered_set<int> random_k;
                        while (random_k.size() < K)
                        {
                            int random_no = rand() % (sz);
                            random_k.insert(random_no);
                        }

                        for (auto it : random_k)
                        {
                            Packet selected_pkt = pkt_received[dest_port][it];
                            // Output queue not full
                            if (output_queues[selected_pkt.dest_port].size() < buffer_size)
                            {
                                output_queues[selected_pkt.dest_port].push(selected_pkt);
                            }
                            else
                            {
                                total_dropped_packets++; // packet dropped because of output queue full
                            }
                        }
                    }
                }
            }
        }
        else if (queue_type == "iSLIP")
        {

            vector<int> a(num_ports);
            vector<int> g(num_ports);
            vector<vector<Packet>> request(num_ports);
            for (int src_port = 0; src_port < num_ports; ++src_port)
            {
                if (!input_queues[src_port].empty())
                {
                    Packet packet = input_queues[src_port].front();
                    request[packet.dest_port].push_back(packet);
                }
            }
            vector<vector<Packet>> granted(num_ports);
            for (int dest_port = 0; dest_port < num_ports; ++dest_port)
            {
                bool found = false;
                for (auto packet : request[dest_port])
                {
                    if (g[dest_port] <= packet.src_port)
                    {
                        found = true;
                        granted[packet.src_port].push_back(packet);
                        break;
                    }
                }
                if (!found && request[dest_port].size())
                {
                    granted[request[dest_port][0].src_port].push_back(request[dest_port][0]);
                }
            }
            for (int src_port = 0; src_port < num_ports; ++src_port)
            {
                bool found = false;
                for (auto packet : granted[src_port])
                {
                    if (a[src_port] <= packet.dest_port)
                    {
                        input_queues[src_port].pop();
                        output_queues[packet.dest_port].push(packet);
                        g[packet.dest_port] = src_port + 1;
                        g[packet.dest_port] %= num_ports;
                        a[src_port] = packet.dest_port + 1;
                        a[src_port] %= num_ports;
                        found = true;
                        break;
                    }
                }
                if (!found && granted[src_port].size())
                {
                    input_queues[src_port].pop();
                    output_queues[granted[src_port][0].dest_port].push(granted[src_port][0]);
                    g[granted[src_port][0].dest_port] = src_port + 1;
                    g[granted[src_port][0].dest_port] %= num_ports;
                    a[src_port] = granted[src_port][0].dest_port + 1;
                    a[src_port] %= num_ports;
                }
            }
        }
    }

    //  Phase 3- Packet transmission.
    void transmitPackets(int time_slot)
    {
        for (int dest_port = 0; dest_port < num_ports; ++dest_port)
        {
            // checking if output queue has a packet or not
            if (!output_queues[dest_port].empty())
            {
                Packet packet = output_queues[dest_port].front();
                output_queues[dest_port].pop();
                double delay = (double)time_slot - packet.arrival_time; // Ignoring offset for delay calculation
                total_packet_delay = total_packet_delay + delay;
                total_transmitted_packets++;
                total_link_utilization += 1.0;
            }
        }
    }

public:
    Switch(int switch_port_count, int buffer_size, double packet_gen_prob, string queue_type, int max_time_slots, double knockout)
        : num_ports(switch_port_count), buffer_size(buffer_size), packet_gen_prob(packet_gen_prob),
          queue_type(queue_type), max_time_slots(max_time_slots), knockout(knockout), distribution(0.0, 1.0)
    {
        input_queues.resize(num_ports);
        output_queues.resize(num_ports);
    }

    // Simulation
    void runSimulation()
    {
        for (int time_slot = 0; time_slot < max_time_slots; ++time_slot)
        {
            generateTraffic(time_slot);
            schedulePackets();
            transmitPackets(time_slot);
        }
        double avg_packet_delay = total_packet_delay / (double)total_transmitted_packets;
        double avg_link_utilization = total_link_utilization / (num_ports * max_time_slots);
        double kouq_drop_probability = static_cast<double>(total_dropped_packets) / (num_ports * max_time_slots);
        cout << num_ports << "\t" << packet_gen_prob << "\t" << queue_type << "\t"
             << avg_packet_delay << "\t" << avg_link_utilization << "\t" << kouq_drop_probability << endl;
    }
};

int main(int argc, char *argv[])
{

    int switch_port_count = 8;    // Default value
    int buffer_size = 4;          // Default value
    double packet_gen_prob = 0.5; // Default value
    string queue_type = "KOUQ";   // Default value
    int max_time_slots = 10000;   // Default value
    double knockout = 0.6;        // Default value

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];
        if (arg == "-N")
        {
            switch_port_count = stoi(argv[++i]);
        }
        else if (arg == "-B")
        {
            buffer_size = stoi(argv[++i]);
        }
        else if (arg == "-p")
        {
            packet_gen_prob = stod(argv[++i]);
        }
        else if (arg == "-queue")
        {
            queue_type = argv[++i];
        }
        else if (arg == "-T")
        {
            max_time_slots = stoi(argv[++i]);
        }
        else if (arg == "-K")
        {
            knockout = stod(argv[++i]);
        }
    }

    // Create switch object and run simulation
    Switch mySwitch(switch_port_count, buffer_size, packet_gen_prob, queue_type, max_time_slots, knockout);
    mySwitch.runSimulation();

    return 0;
}

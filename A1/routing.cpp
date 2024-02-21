#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <fstream>
#include <cmath>
#include <utility>
#include <unordered_set>
using namespace std;
// Packet structure
struct Packet {
    int src_port;
    int dest_port;
    double arrival_time;
};

// Switch class
class Switch {
private:
    int num_ports;
    int buffer_size;
    double packet_gen_prob;
    string queue_type;
    int max_time_slots;
    double knockout;//knockout
    double offset=0.0;

    // Queues for each port
    vector<queue<Packet>> input_queues;
    vector<queue<Packet>> output_queues;

    // Random number generator
    default_random_engine generator;
    uniform_real_distribution<double> distribution;

    // Metrics
    double total_packet_delay=0;
    double total_link_utilization=0;
    int total_transmitted_packets=0;
    int total_dropped_packets=0;

    // Helper functions
    void generateTraffic(int time_slot) {
        for (int src_port = 0; src_port < num_ports; ++src_port) {
            if (distribution(generator) < packet_gen_prob) {
                Packet packet;
                packet.src_port = src_port;
                packet.dest_port = distribution(generator) * num_ports;
                packet.arrival_time = (double)time_slot+offset; // Ignoring offset for delay calculation
                input_queues[src_port].push(packet);
            }
        }
    }

    void schedulePackets() {
        if (queue_type == "INQ") {
            // for (int src_port = 0; src_port < num_ports; ++src_port) {
            //     while (!input_queues[src_port].empty()) {
            //         Packet packet = input_queues[src_port].front();
            //         input_queues[src_port].pop();
            //         if (output_queues[packet.dest_port].empty()) {
            //             output_queues[packet.dest_port].push(packet);
            //         } else {
            //             // Randomly select one packet for transmission
            //             int rand_index = distribution(generator) * output_queues[packet.dest_port].size();
            //             output_queues[packet.dest_port].push(packet);
            //         }
            //     }
            // }
            vector<vector<Packet>>temp(num_ports);
            for(int src_port = 0;src_port<num_ports;++src_port){
                if(!input_queues[src_port].empty()){
                    Packet packet = input_queues[src_port].front();
                    temp[packet.dest_port].push_back(packet);
                }
            }
            for(int dest_port=0;dest_port<num_ports;++dest_port){
                
                if(temp[dest_port].size()){
                    int ind=rand()%temp[dest_port].size();
                    output_queues[dest_port].push(temp[dest_port][ind]);
                    input_queues[temp[dest_port][ind].src_port].pop();
                }
            }
        } else if (queue_type == "KOUQ")
        {
            int K=num_ports*knockout;
            vector<vector<Packet>> pkt_received(num_ports);
            for (int src_port = 0; src_port < num_ports; src_port++)
            {

                if (!input_queues[src_port].empty())
                {
                    Packet packet = input_queues[src_port].front();
                    pkt_received[packet.dest_port].push_back(packet);
                    input_queues[packet.src_port].pop();
                }
            }
            for (int dest_port = 0; dest_port < num_ports; dest_port++)
            {
                if (!pkt_received[dest_port].empty())
                {
                    int sz = pkt_received[dest_port].size();
                    if (sz <= K)
                    {
                        for (int j = 0; j < sz; j++)
                        {
                            Packet selected = pkt_received[dest_port][j];
                            output_queues[selected.dest_port].push(selected);
                            // input_queues[selected.src_port].pop();
                        }
                    }
                    else
                    {
                        total_dropped_packets+=(sz-K);
                        unordered_set<int> random_k;
                        while (random_k.size() < K)
                        {
                            int random_no = rand() % (sz);
                            random_k.insert(random_no);
                        }

                        for (auto it : random_k)
                        {
                            Packet selected = pkt_received[dest_port][it];
                            output_queues[selected.dest_port].push(selected);
                        }
                    }
                }
            }
        } else if (queue_type == "iSLIP") {
            vector<int> grant(num_ports, -1);
            for (int r = 0; r < num_ports; ++r) {
                for (int i = 0; i < num_ports; ++i) {
                    if (input_queues[i].empty()) continue;
                    if (grant[i] != -1) continue; // Already granted
                    if (output_queues[i].size() < buffer_size) {
                        grant[i] = i;
                        output_queues[i].push(input_queues[i].front());
                        input_queues[i].pop();
                    }
                }
                for (int i = 0; i < num_ports; ++i) {
                    if (grant[i] == -1) continue;
                    for (int j = 0; j < num_ports; ++j) {
                        if (i == j) continue;
                        if (output_queues[j].size() < buffer_size && input_queues[i].size() > 0) {
                            output_queues[j].push(input_queues[i].front());
                            input_queues[i].pop();
                            break;
                        }
                    }
                }
            }
        }
    }

    void transmitPackets(int time_slot) {
        for (int dest_port = 0; dest_port < num_ports; ++dest_port) {
            if (!output_queues[dest_port].empty()) {
                Packet packet = output_queues[dest_port].front();
                output_queues[dest_port].pop();
                double delay = (double)time_slot - packet.arrival_time; // Ignoring offset for delay calculation
                total_packet_delay = total_packet_delay+delay;
                ++total_transmitted_packets;
                total_link_utilization += 1.0;
            }
        }
    }

public:
    Switch(int switch_port_count, int buffer_size, double packet_gen_prob, string queue_type, int max_time_slots,double knockout)
        : num_ports(switch_port_count), buffer_size(buffer_size), packet_gen_prob(packet_gen_prob), 
        queue_type(queue_type), max_time_slots(max_time_slots),knockout(knockout), distribution(0.0, 1.0) {
        input_queues.resize(num_ports);
        output_queues.resize(num_ports);
    }

    void runSimulation() {
        for (int time_slot = 0; time_slot < max_time_slots; ++time_slot) {
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

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int switch_port_count = 8; // Default value
    int buffer_size = 4; // Default value
    double packet_gen_prob = 0.5; // Default value
    string queue_type = "KOUQ"; // Default value
    int max_time_slots = 10000; // Default value
    double knockout=0.6;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-N") {
            switch_port_count = stoi(argv[++i]);
        } else if (arg == "-B") {
            buffer_size = stoi(argv[++i]);
        } else if (arg == "-p") {
            packet_gen_prob = stod(argv[++i]);
        } else if (arg == "-queue") {
            queue_type = argv[++i];
        } else if (arg == "-T") {
            max_time_slots = stoi(argv[++i]);
        }else if (arg == "-K") {
            knockout = stod(argv[++i]);
        }
    }

    // Create switch object and run simulation
    Switch mySwitch(switch_port_count, buffer_size, packet_gen_prob, queue_type, max_time_slots,knockout);
    mySwitch.runSimulation();

    return 0;
}

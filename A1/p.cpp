// Packet structure
struct Packet
{
    int src_port;
    int dest_port;
    double arrival_time;
};

if (queue_type == "INQ")
{
    vector<packet> pkt_received(num_ports);
    for (int src_port = 0; src_port < num_ports; src_port++)
    {

        if (!input_queues[src_port].empty())
        {
            Packet packet = input_queues[src_port].front();
            pkt_received[packet.dest_port].push_back(packet);
        }
    }

    for (int dest_port = 0; des_ports < num_ports; dest_port++)
    {
        if (!pkt_received[i].empty())
        {
            Packet selected = rand() % (pkt_received[i].size());
            output_queues[selected.dest_port].push(selected);
            input_queues[selected.src_port].pop();
        }
    }
}

if (queue_type == "INQ")
{
    for (int src_port = 0; src_port < num_ports; ++src_port)
    {
        while (!input_queues[src_port].empty())
        {
            Packet packet = input_queues[src_port].front();
            input_queues[src_port].pop();
            if (output_queues[packet.dest_port].empty())
            {
                output_queues[packet.dest_port].push(packet);
            }
            else
            {
                // Randomly select one packet for transmission
                int rand_index = distribution(generator) * output_queues[packet.dest_port].size();
                output_queues[packet.dest_port].push(packet);
            }
        }
    }
}
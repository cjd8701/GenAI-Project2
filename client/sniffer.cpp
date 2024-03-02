#include <iostream>
#include <chrono>
#include <string>
#include <pcap.h> // Make sure you have pcap library installed

std::string sniffPacketsForDuration() {
    std::string capturedData;
    pcap_t *handle; // Handle for the session
    char errbuf[PCAP_ERRBUF_SIZE]; // Error string
    struct bpf_program fp; // Compiled filter
    char filter_exp[] = "ip"; // Filter expression
    bpf_u_int32 mask; // Subnet mask
    bpf_u_int32 net; // IP

    // Define the device to sniff on
    char *dev = pcap_lookupdev(errbuf);
    if (dev == nullptr) {
        std::cerr << "Couldn't find default device: " << errbuf << std::endl;
        return "";
    }

    // Find network number and mask based on the device
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        std::cerr << "Couldn't get netmask for device " << dev << ": " << errbuf << std::endl;
        net = 0;
        mask = 0;
    }

    // Open the session in promiscuous mode
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Couldn't open device " << dev << ": " << errbuf << std::endl;
        return "";
    }

    // Compile and apply the filter
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        std::cerr << "Couldn't parse filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
        return "";
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        std::cerr << "Couldn't install filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
        return "";
    }

    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        // Check if duration has elapsed
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsed >= 30) break;

        struct pcap_pkthdr header; // The header that pcap gives us
        const u_char *packet; // The actual packet

        // Capture a packet
        packet = pcap_next(handle, &header);

        // Process the packet
        if (packet != nullptr) {
            // Process packet data and append to capturedData string
            // For simplicity, we're just appending the packet length here
            capturedData += "Packet length: " + std::to_string(header.len) + "\n";
        }
    }

    // Close the session
    pcap_close(handle);
    return capturedData;
}

int main() {
    std::string packetData = sniffPacketsForDuration();
    std::cout << "Captured packets data:\n" << packetData << std::endl;
    return 0;
}

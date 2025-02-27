from pythonosc.udp_client import SimpleUDPClient

# Define the OSC server address and port
ip = "192.168.29.221"  # Change to the target IP if sending to another machine
port = 8000       # Change to the target port

# Create an OSC client
client = SimpleUDPClient(ip, port)

# Send an OSC message
address = "/motor/rotate"
value = 42  # Can be int, float, string, or list

client.send_message(address, [400, 5,0])

print(f"Sent OSC message to {ip}:{port} -> {address} {value}")

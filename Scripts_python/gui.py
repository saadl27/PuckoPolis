import os
import json
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
import serial
from threading import Thread
import time

# Path to your city JSON file
CITY_JSON_PATH = os.path.expanduser('~/Documents/EPuck2_Workplace/TPs/.vscode/city.json')  # adjust as needed

# Global destination node
dest_node = None


def load_city(filename):
    """
    Load city graph from a JSON file.
    Expected format:
    {
      "nodes": [{"id": 1, "x": 0.0, "y": 0.0}, ...],
      "edges": [[1, 2], [2, 3], ...]
    }
    """
    if not os.path.isfile(filename):
        raise FileNotFoundError(f"City JSON file not found: {filename}")
    with open(filename, 'r') as f:
        data = json.load(f)
    G = nx.Graph()
    positions = {}
    for n in data['nodes']:
        nid = int(n['id'])
        positions[nid] = (float(n['x']), float(n['y']))
        G.add_node(nid)
    for u, v in data['edges']:
        G.add_edge(int(u), int(v))
    return G, positions

class SerialThread(Thread):
    """
    Thread to handle serial communication with the robot.
    Robot should send lines like "CURRENT_NODE:5\n"
    and will accept destination commands like "DEST:10\n".
    """
    def __init__(self, port, baudrate=115200):
        super().__init__()
        self.port = serial.Serial(port, baudrate=baudrate, timeout=1)
        self.current_node = None
        self.need_update = False
        self.alive = True

    def run(self):
        while self.alive:
            try:
                line = self.port.readline().decode('utf-8').strip()
                if line.startswith("CURRENT_NODE:"):
                    node_id = int(line.split(':')[1])
                    self.current_node = node_id
                    self.need_update = True
            except Exception:
                pass
            time.sleep(0.1)

    def send_destination(self, node_id):
        msg = f"DEST:{node_id}\n"
        self.port.write(msg.encode('utf-8'))

    def stop(self):
        self.alive = False
        if self.port.is_open:
            self.port.close()

if __name__ == "__main__":
    import argparse, sys
    parser = argparse.ArgumentParser(description="EPUCK CITY - Robot Navigation GUI")
    parser.add_argument('serial_port', help='Serial port for robot communication')
    args = parser.parse_args()

    # Load graph data
    G, positions = load_city(CITY_JSON_PATH)

    # Start serial thread
    try:
        serial_thread = SerialThread(args.serial_port)
        serial_thread.start()
    except serial.SerialException as e:
        print(f"Error opening serial port {args.serial_port}: {e}")
        sys.exit(1)

    # Create figure with two panels
    fig = plt.figure(figsize=(12, 6))
    fig.suptitle('EPUCK CITY', fontsize=16, fontweight='bold')
    gs = fig.add_gridspec(1, 2, width_ratios=[1, 2], wspace=0.3)

    # Left panel for controls/info
    panel_ax = fig.add_subplot(gs[0])
    panel_ax.axis('off')
    panel_ax.set_title('Status & Controls', color='#004d99', fontsize=14)

    # Text elements for current and destination nodes
    current_text = panel_ax.text(0.1, 0.7, 'Current node: --', fontsize=12)
    dest_text = panel_ax.text(0.1, 0.5, 'Destination: --', fontsize=12)

    # Input box and Go button
    tb_ax = fig.add_axes([0.05, 0.2, 0.3, 0.05], facecolor='#f0f0f0')
    text_box = TextBox(tb_ax, 'Go to node:', initial='')
    btn_ax = fig.add_axes([0.05, 0.1, 0.15, 0.05], facecolor='#66c2a5')
    go_button = Button(btn_ax, 'Go', color='#66c2a5', hovercolor='#4da077')

    # Right panel for city graph
    ax_graph = fig.add_subplot(gs[1])
    ax_graph.set_title('City Graph', color='#333', fontsize=14)

    # Draw edges
    for u, v in G.edges():
        x1, y1 = positions[u]
        x2, y2 = positions[v]
        ax_graph.plot([x1, x2], [y1, y2], color='gray', zorder=1)

    # Draw nodes
    node_patches = {}
    for nid, (x, y) in positions.items():
        circle = plt.Circle((x, y), 0.15, facecolor='white', edgecolor='#004d99', lw=1.5, zorder=2)
        ax_graph.add_patch(circle)
        node_patches[nid] = circle

    ax_graph.set_aspect('equal', 'box')
    ax_graph.autoscale_view()

    # Button callback
    def on_go(event):
        global dest_node
        val = text_box.text.strip()
        try:
            nid = int(val)
            if nid in positions:
                dest_node = nid
                serial_thread.send_destination(nid)
                dest_text.set_text(f'Destination: {nid}')
                update_display()
            else:
                print(f"Node {nid} not in graph.")
        except ValueError:
            print("Please enter a valid integer node ID.")

    go_button.on_clicked(on_go)

    # Update highlights and texts
    def update_display():
        curr = serial_thread.current_node
        for nid, circle in node_patches.items():
            if nid == curr:
                circle.set_facecolor('#fee08b')  # yellow-ish
            elif nid == dest_node:
                circle.set_facecolor('#a6d96a')  # green-ish
            else:
                circle.set_facecolor('white')
        if curr is not None:
            current_text.set_text(f'Current node: {curr}')
        fig.canvas.draw_idle()
        serial_thread.need_update = False

    # Timer to poll for updates
    timer = fig.canvas.new_timer(interval=100)
    timer.add_callback(lambda: update_display() if serial_thread.need_update else None)
    timer.start()

    plt.show()
    serial_thread.stop()

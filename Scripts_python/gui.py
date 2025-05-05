import os
import json
import xml.etree.ElementTree as ET
import tempfile
import numpy as np
from svgpathtools import svg2paths
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox
import serial
from threading import Thread
import time

# Paths to your data files
CITY_JSON_PATH = os.path.expanduser('~/Documents/EPuck2_Workplace/TPs/.vscode/city2.json')
CITY_SVG_PATH = os.path.expanduser('~/Documents/EPuck2_Workplace/TPs/.vscode/city.drawio.svg')

# Global destination node
dest_node = None


def load_city(filename):
    """
    Load city nodes and edges from a JSON file.
    """
    if not os.path.isfile(filename):
        raise FileNotFoundError(f"City JSON file not found: {filename}")
    with open(filename, 'r') as f:
        data = json.load(f)
    positions = {int(n['id']): (float(n['x']), float(n['y'])) for n in data['nodes']}
    return positions


def load_and_clean_svg(svg_path):
    """
    Read SVG, remove percent-based rect elements, write to temp file, and parse paths.
    Returns a list of svgpathtools Path objects.
    """
    with open(svg_path, 'r') as f:
        svg_content = f.read()
    root = ET.fromstring(svg_content)
    ET.register_namespace('', "http://www.w3.org/2000/svg")
    # Remove <rect> elements with % widths/heights
    for rect in root.findall('.//{http://www.w3.org/2000/svg}rect'):
        w = rect.attrib.get('width', '')
        h = rect.attrib.get('height', '')
        if '%' in w or '%' in h:
            root.remove(rect)
    # Write cleaned SVG to temp file
    with tempfile.NamedTemporaryFile(delete=False, suffix='.svg') as tmp:
        tmp_name = tmp.name
        ET.ElementTree(root).write(tmp_name)
    paths, _ = svg2paths(tmp_name)
    return paths


def draw_svg_background(ax, paths):
    """
    Plot each segment in the SVG paths onto the given axes.
    """
    for path in paths:
        for segment in path:
            t = np.linspace(0, 1, 100)
            pts = [segment.point(tt) for tt in t]
            x = [p.real for p in pts]
            y = [p.imag for p in pts]
            ax.plot(x, y, color='black', linewidth=1, zorder=0)
    ax.invert_yaxis()
    ax.axis('off')


class SerialThread(Thread):
    """
    Thread handling serial comms: reads "CURRENT_NODE:<id>" and sends "DEST:<id>".
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
                    nid = int(line.split(':')[1])
                    self.current_node = nid
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

    # Load city data
    positions = load_city(CITY_JSON_PATH)
    paths = load_and_clean_svg(CITY_SVG_PATH)

    # Start serial thread
    try:
        serial_thread = SerialThread(args.serial_port)
        serial_thread.start()
    except serial.SerialException as e:
        print(f"Error opening serial port {args.serial_port}: {e}")
        sys.exit(1)

    # Create the figure
    fig = plt.figure(figsize=(12, 6))
    fig.suptitle('EPUCK CITY', fontsize=16, fontweight='bold')
    gs = fig.add_gridspec(1, 2, width_ratios=[1, 2], wspace=0.3)

    # Left: status & controls
    panel = fig.add_subplot(gs[0])
    panel.axis('off')
    panel.set_title('Status & Controls', color='#004d99', fontsize=14)
    current_text = panel.text(0.1, 0.7, 'Current node: --', fontsize=12)
    dest_text = panel.text(0.1, 0.5, 'Destination: --', fontsize=12)
    tb_ax = fig.add_axes([0.05, 0.2, 0.3, 0.05], facecolor='#f0f0f0')
    text_box = TextBox(tb_ax, 'Go to node:', initial='')
    btn_ax = fig.add_axes([0.05, 0.1, 0.15, 0.05], facecolor='#66c2a5')
    go_button = Button(btn_ax, 'Go', color='#66c2a5', hovercolor='#4da077')

    # Right: SVG city + clickable nodes
    ax = fig.add_subplot(gs[1])
    ax.set_title('City Map', color='#333', fontsize=14)
    draw_svg_background(ax, paths)

    # Draw node circles and labels
    node_patches = {}
    for nid, (x, y) in positions.items():
        circ = plt.Circle((x, y), 8, facecolor='white', edgecolor='#004d99', lw=1.5,
                           zorder=1, picker=5)
        ax.add_patch(circ)
        ax.text(x, y, str(nid), fontsize=8, ha='center', va='center', zorder=2)
        node_patches[nid] = circ

    ax.set_aspect('equal')
    ax.autoscale_view()

    # Update display of highlights
    def update_display():
        curr = serial_thread.current_node
        for nid, circ in node_patches.items():
            if nid == curr:
                circ.set_facecolor('#fee08b')
            elif nid == dest_node:
                circ.set_facecolor('#a6d96a')
            else:
                circ.set_facecolor('white')
        if curr is not None:
            current_text.set_text(f'Current node: {curr}')
        fig.canvas.draw_idle()
        serial_thread.need_update = False

    # Handler for Go button
    def on_go(event):
        global dest_node
        try:
            nid = int(text_box.text.strip())
            if nid in node_patches:
                dest_node = nid
                serial_thread.send_destination(nid)
                dest_text.set_text(f'Destination: {nid}')
                update_display()
            else:
                print(f"Node {nid} not in city.")
        except ValueError:
            print("Enter a valid node ID.")

    go_button.on_clicked(on_go)

    # Handler for clicking directly on a node
    def on_pick(event):
        global dest_node
        artist = event.artist
        for nid, circ in node_patches.items():
            if circ == artist:
                dest_node = nid
                serial_thread.send_destination(nid)
                dest_text.set_text(f'Destination: {nid}')
                update_display()
                break

    fig.canvas.mpl_connect('pick_event', on_pick)

    # Timer to poll serial updates
    timer = fig.canvas.new_timer(interval=100)
    timer.add_callback(lambda: update_display() if serial_thread.need_update else None)
    timer.start()

    plt.show()
    serial_thread.stop()

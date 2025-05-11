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
CITY_JSON_PATH = os.path.expanduser('gui_files/city.json')
CITY_SVG_PATH = os.path.expanduser('gui_files/city.drawio.svg')

# Global nodes
start_node = None
dest_node = None


def load_city(filename):
    """
    Load city nodes from a JSON file.
    """
    if not os.path.isfile(filename):
        raise FileNotFoundError(f"City JSON file not found: {filename}")
    with open(filename, 'r') as f:
        data = json.load(f)
    return {int(n['id']): (float(n['x']), float(n['y'])) for n in data['nodes']}


def load_and_clean_svg(svg_path):
    """
    Read SVG, remove percent-based rects, parse via svg2paths.
    """
    with open(svg_path, 'r') as f:
        content = f.read()
    root = ET.fromstring(content)
    ET.register_namespace('', "http://www.w3.org/2000/svg")
    for rect in root.findall('.//{http://www.w3.org/2000/svg}rect'):
        if '%' in rect.get('width','') or '%' in rect.get('height',''):
            root.remove(rect)
    with tempfile.NamedTemporaryFile(delete=False, suffix='.svg') as tmp:
        ET.ElementTree(root).write(tmp.name)
        paths, _ = svg2paths(tmp.name)
    return paths


def draw_svg_background(ax, paths):
    for path in paths:
        for seg in path:
            t = np.linspace(0,1,100)
            pts = [seg.point(tt) for tt in t]
            x = [p.real for p in pts]; y = [p.imag for p in pts]
            ax.plot(x,y,color='black',linewidth=1,zorder=0)
    ax.invert_yaxis(); ax.axis('off')


class SerialThread(Thread):
    def __init__(self, port, baudrate=115200):
        super().__init__()
        self.port = serial.Serial(port, baudrate=baudrate, timeout=1)
        self.current_node = None
        self.need_update = False
        self.alive = True

    def run(self):
        while self.alive:
            try:
                line = self.port.readline().decode().strip()
                if line.startswith('CURRENT_NODE:'):
                    self.current_node = int(line.split(':')[1])
                    self.need_update = True
            except Exception:
                pass
            time.sleep(0.1)

    def send_start(self, node_id):
        self.port.write(f'START:{node_id}\n'.encode())

    def send_destination(self, node_id):
        self.port.write(f'DEST:{node_id}\n'.encode())

    def stop(self):
        self.alive = False
        if self.port.is_open:
            self.port.close()


if __name__ == '__main__':
    import argparse, sys
    parser = argparse.ArgumentParser(description='EPUCK CITY - GUI')
    parser.add_argument('serial_port', help='Serial port for robot')
    args = parser.parse_args()

    positions = load_city(CITY_JSON_PATH)
    paths = load_and_clean_svg(CITY_SVG_PATH)

    try:
        serial_thread = SerialThread(args.serial_port)
        serial_thread.start()
    except serial.SerialException as e:
        print(f'Cannot open {args.serial_port}: {e}')
        sys.exit(1)

    fig = plt.figure(figsize=(12,6))
    fig.suptitle('EPUCK CITY',fontsize=16,fontweight='bold')
    gs = fig.add_gridspec(1,2,width_ratios=[1,2],wspace=0.3)

    # Controls panel
    panel = fig.add_subplot(gs[0]); panel.axis('off')
    panel.set_title('Status & Controls',color='#004d99',fontsize=14)
    current_text = panel.text(0.1,0.75,'Current node: --',fontsize=12)
    start_text   = panel.text(0.1,0.65,'Start node:   --',fontsize=12)
    dest_text    = panel.text(0.1,0.55,'Dest node:    --',fontsize=12)

    # Start input
    sb_ax = fig.add_axes([0.05,0.3,0.3,0.05],facecolor='#f0f0f0')
    start_box = TextBox(sb_ax, 'Set start:',initial='')
    sb_btn_ax = fig.add_axes([0.05,0.23,0.15,0.05],facecolor='#5c8ebf')
    start_btn = Button(sb_btn_ax,'Start',color='#5c8ebf',hovercolor='#4978a2')

    # Dest input
    db_ax = fig.add_axes([0.05,0.15,0.3,0.05],facecolor='#f0f0f0')
    dest_box = TextBox(db_ax,'Go to node:',initial='')
    db_btn_ax = fig.add_axes([0.05,0.08,0.15,0.05],facecolor='#66c2a5')
    dest_btn = Button(db_btn_ax,'Go',color='#66c2a5',hovercolor='#4da077')

    # Map panel
    ax = fig.add_subplot(gs[1]); ax.set_title('City Map',color='#333',fontsize=14)
    draw_svg_background(ax,paths)

    node_patches = {}
    for nid,(x,y) in positions.items():
        circ=plt.Circle((x,y),8,facecolor='white',edgecolor='#004d99',lw=1.5,
                        zorder=1,picker=5)
        ax.add_patch(circ)
        ax.text(x,y,str(nid),fontsize=8,ha='center',va='center',zorder=2)
        node_patches[nid]=circ

    ax.set_aspect('equal'); ax.autoscale_view()

    def update_display():
        curr = serial_thread.current_node
        for nid,circ in node_patches.items():
            if nid==curr:
                circ.set_facecolor('#fee08b')
            elif nid==start_node:
                circ.set_facecolor('#74add1')
            elif nid==dest_node:
                circ.set_facecolor('#a6d96a')
            else:
                circ.set_facecolor('white')
        if curr is not None:
            current_text.set_text(f'Current node: {curr}')
        fig.canvas.draw_idle()
        serial_thread.need_update=False

    def on_start(event):
        global start_node
        try:
            nid=int(start_box.text.strip())
            if nid in node_patches:
                start_node=nid
                serial_thread.send_start(nid)
                start_text.set_text(f'Start node:   {nid}')
                update_display()
            else:
                print(f'Node {nid} not in city')
        except ValueError:
            print('Enter valid start ID')
    start_btn.on_clicked(on_start)

    def on_dest(event):
        global dest_node
        try:
            nid=int(dest_box.text.strip())
            if nid in node_patches:
                dest_node=nid
                serial_thread.send_destination(nid)
                dest_text.set_text(f'Dest node:    {nid}')
                update_display()
            else:
                print(f'Node {nid} not in city')
        except ValueError:
            print('Enter valid dest ID')
    dest_btn.on_clicked(on_dest)

    def on_pick(event):
        global dest_node
        artist=event.artist
        for nid,circ in node_patches.items():
            if circ==artist:
                dest_node=nid
                serial_thread.send_destination(nid)
                dest_text.set_text(f'Dest node:    {nid}')
                update_display()
                break
    fig.canvas.mpl_connect('pick_event',on_pick)

    timer=fig.canvas.new_timer(interval=100)
    timer.add_callback(lambda:update_display() if serial_thread.need_update else None)
    timer.start()

    plt.show()
    serial_thread.stop()
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import serial
import serial.tools.list_ports
import threading
import time
from datetime import datetime

class HealthMonitorGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Health Monitor Pro")
        
        # Smaller window - everything fits
        self.root.geometry("1200x700")
        self.root.configure(bg="#0a0e27")
        
        # Serial connection
        self.serial_connection = None
        self.connected = False
        self.reading_thread = None
        self.running = False
        
        # Data variables
        self.temperature = "--"
        self.humidity = "--"
        self.heart_rate = "--"
        self.spo2 = "--"
        self.gps_status = "Searching..."
        self.latitude = "--"
        self.longitude = "--"
        self.alert_count = 0
        
        # Animation variables
        self.blink_state = False
        
        self.create_widgets()
        self.update_sensor_display()
        self.animate_connection_status()
        
    def create_widgets(self):
        # Header
        header_frame = tk.Frame(self.root, bg="#1a0033", height=60)
        header_frame.pack(fill="x")
        header_frame.pack_propagate(False)
        
        title_label = tk.Label(
            header_frame,
            text="ESP32 HEALTH MONITOR PRO",
            font=("Segoe UI", 22, "bold"),
            bg="#1a0033",
            fg="#00ffff"
        )
        title_label.pack(pady=15)
        
        # Main container
        main_container = tk.Frame(self.root, bg="#0a0e27")
        main_container.pack(fill="both", expand=True, padx=10, pady=10)
        
        # ==================== LEFT PANEL ====================
        left_panel = tk.Frame(main_container, bg="#0f1729", width=320, relief="ridge", borderwidth=2)
        left_panel.pack(side="left", fill="y", padx=(0, 10))
        left_panel.pack_propagate(False)
        
        # Connection Section
        conn_frame = tk.Frame(left_panel, bg="#1a2332", relief="groove", borderwidth=2)
        conn_frame.pack(pady=10, padx=10, fill="x")
        
        conn_label = tk.Label(
            conn_frame,
            text="🔌 CONNECTION",
            font=("Segoe UI", 11, "bold"),
            bg="#1a2332",
            fg="#00ffff"
        )
        conn_label.pack(pady=8)
        
        tk.Label(
            conn_frame,
            text="COM Port:",
            bg="#1a2332",
            fg="#ffffff",
            font=("Segoe UI", 9, "bold")
        ).pack(pady=(5, 2))
        
        style = ttk.Style()
        style.theme_use('clam')
        style.configure('Custom.TCombobox', fieldbackground='#0a0e27', background='#00ffff')
        
        self.port_combo = ttk.Combobox(
            conn_frame,
            state="readonly",
            width=20,
            font=("Segoe UI", 9),
            style='Custom.TCombobox'
        )
        self.port_combo.pack(pady=5, padx=10)
        self.refresh_ports()
        
        refresh_btn = tk.Button(
            conn_frame,
            text="🔄 Refresh",
            command=self.refresh_ports,
            bg="#2d4059",
            fg="white",
            font=("Segoe UI", 8, "bold"),
            relief="flat",
            padx=10,
            pady=4,
            cursor="hand2"
        )
        refresh_btn.pack(pady=5)
        
        # Connect Button
        self.connect_btn = tk.Button(
            conn_frame,
            text="⚡ CONNECT",
            command=self.toggle_connection,
            bg="#00ff88",
            fg="#000000",
            font=("Segoe UI", 10, "bold"),
            relief="raised",
            borderwidth=2,
            padx=20,
            pady=8,
            cursor="hand2"
        )
        self.connect_btn.pack(pady=10)
        
        # Status
        status_frame = tk.Frame(conn_frame, bg="#1a2332")
        status_frame.pack(pady=(0, 8))
        
        self.status_indicator = tk.Label(
            status_frame,
            text="●",
            font=("Arial", 16),
            bg="#1a2332",
            fg="#ff4444"
        )
        self.status_indicator.pack(side="left")
        
        self.status_label = tk.Label(
            status_frame,
            text="Disconnected",
            font=("Segoe UI", 9, "bold"),
            bg="#1a2332",
            fg="#ff4444"
        )
        self.status_label.pack(side="left", padx=5)
        
        # Line
        tk.Frame(left_panel, bg="#00ffff", height=2).pack(fill="x", pady=8, padx=15)
        
        # Commands
        cmd_label = tk.Label(
            left_panel,
            text="⚡ COMMANDS",
            font=("Segoe UI", 10, "bold"),
            bg="#0f1729",
            fg="#00ffff"
        )
        cmd_label.pack(pady=(5, 8))
        
        # SOS Button
        sos_container = tk.Frame(left_panel, bg="#ff0000", relief="raised", borderwidth=3)
        sos_container.pack(pady=6, padx=12, fill="x")
        
        self.sos_btn = tk.Button(
            sos_container,
            text="🚨 SOS EMERGENCY",
            command=self.send_sos,
            bg="#ff0000",
            fg="white",
            font=("Segoe UI", 11, "bold"),
            relief="flat",
            padx=10,
            pady=12,
            cursor="hand2"
        )
        self.sos_btn.pack(fill="both")
        
        # Other Commands
        commands = [
            ("📊 Status", self.send_status, "#3d5a80"),
            ("🌍 GPS", self.send_gps, "#2a9d8f"),
            ("⚙ Thresholds", self.send_thresholds, "#e76f51"),
            ("❓ Help", self.send_help, "#8338ec")
        ]
        
        for text, command, color in commands:
            btn_container = tk.Frame(left_panel, bg=color, relief="raised", borderwidth=2)
            btn_container.pack(pady=3, padx=12, fill="x")
            
            btn = tk.Button(
                btn_container,
                text=text,
                command=command,
                bg=color,
                fg="white",
                font=("Segoe UI", 9, "bold"),
                relief="flat",
                padx=8,
                pady=6,
                cursor="hand2"
            )
            btn.pack(fill="both")
        
        # Alert Counter
        alert_frame = tk.Frame(left_panel, bg="#1a2332", relief="groove", borderwidth=2)
        alert_frame.pack(side="bottom", fill="x", padx=12, pady=10)
        
        tk.Label(
            alert_frame,
            text="⚠ Alerts:",
            font=("Segoe UI", 8, "bold"),
            bg="#1a2332",
            fg="#ffaa00"
        ).pack(pady=3)
        
        self.alert_counter = tk.Label(
            alert_frame,
            text="0",
            font=("Segoe UI", 18, "bold"),
            bg="#1a2332",
            fg="#ff4444"
        )
        self.alert_counter.pack(pady=3)
        
        # ==================== RIGHT PANEL ====================
        right_panel = tk.Frame(main_container, bg="#0a0e27")
        right_panel.pack(side="right", fill="both", expand=True)
        
        # Sensor Display Grid
        sensor_container = tk.Frame(right_panel, bg="#0a0e27")
        sensor_container.pack(fill="x", pady=(0, 8))
        
        self.temp_card = self.create_sensor_card(
            sensor_container, "Temp", "🌡️", 0, 0, "#ff6b6b"
        )
        self.humid_card = self.create_sensor_card(
            sensor_container, "Humid", "💧", 0, 1, "#4dabf7"
        )
        self.hr_card = self.create_sensor_card(
            sensor_container, "Heart", "❤️", 1, 0, "#ff6b9d"
        )
        self.spo2_card = self.create_sensor_card(
            sensor_container, "SpO2", "🫁", 1, 1, "#51cf66"
        )
        
        # GPS Section
        gps_frame = tk.Frame(right_panel, bg="#1a2332", relief="ridge", borderwidth=2)
        gps_frame.pack(fill="x", pady=5)
        
        tk.Label(
            gps_frame,
            text="🌍 GPS LOCATION",
            font=("Segoe UI", 10, "bold"),
            bg="#1a2332",
            fg="#00ffff"
        ).pack(pady=6)
        
        self.gps_status_label = tk.Label(
            gps_frame,
            text="Status: Searching...",
            font=("Segoe UI", 8),
            bg="#1a2332",
            fg="#ffaa00"
        )
        self.gps_status_label.pack(pady=2)
        
        coords_frame = tk.Frame(gps_frame, bg="#1a2332")
        coords_frame.pack(pady=4)
        
        self.lat_label = tk.Label(
            coords_frame,
            text="Lat: --",
            font=("Segoe UI", 8),
            bg="#1a2332",
            fg="#ffffff"
        )
        self.lat_label.pack(side="left", padx=15)
        
        self.lon_label = tk.Label(
            coords_frame,
            text="Lon: --",
            font=("Segoe UI", 8),
            bg="#1a2332",
            fg="#ffffff"
        )
        self.lon_label.pack(side="left", padx=15)
        
        # Message Console
        console_header = tk.Frame(right_panel, bg="#1a2332", relief="ridge", borderwidth=2)
        console_header.pack(fill="x", pady=(5, 0))
        
        tk.Label(
            console_header,
            text="📜 MESSAGE CONSOLE",
            font=("Segoe UI", 10, "bold"),
            bg="#1a2332",
            fg="#00ffff"
        ).pack(pady=6)
        
        console_container = tk.Frame(right_panel, bg="#000000", relief="sunken", borderwidth=2)
        console_container.pack(fill="both", expand=True, pady=(0, 8))
        
        self.console = scrolledtext.ScrolledText(
            console_container,
            wrap=tk.WORD,
            height=6,
            bg="#000000",
            fg="#00ff00",
            font=("Consolas", 8),
            relief="flat",
            padx=8,
            pady=6
        )
        self.console.pack(fill="both", expand=True, padx=2, pady=2)
        
        # ========== CHAT SECTION - AT BOTTOM ==========
        chat_section = tk.Frame(right_panel, bg="#ff6600", relief="ridge", borderwidth=4)
        chat_section.pack(fill="x", pady=0)
        
        # Chat Header - BRIGHT ORANGE
        chat_header = tk.Frame(chat_section, bg="#ff6600")
        chat_header.pack(fill="x")
        
        tk.Label(
            chat_header,
            text="💬 SEND MESSAGE TO ESP32 LCD",
            font=("Segoe UI", 12, "bold"),
            bg="#ff6600",
            fg="#000000"
        ).pack(pady=10)
        
        # Input Frame
        input_frame = tk.Frame(chat_section, bg="#ff6600")
        input_frame.pack(fill="x", padx=15, pady=10)
        
        # Text Entry
        self.message_entry = tk.Entry(
            input_frame,
            bg="#ffffff",
            fg="#000000",
            font=("Segoe UI", 11),
            relief="solid",
            insertbackground="#000000",
            borderwidth=3
        )
        self.message_entry.pack(side="left", fill="x", expand=True, ipady=10, padx=(0, 10))
        self.message_entry.bind("<Return>", lambda e: self.send_custom_message())
        
        # Send Button
        send_btn = tk.Button(
            input_frame,
            text="📤 SEND",
            command=self.send_custom_message,
            bg="#00ff00",
            fg="#000000",
            font=("Segoe UI", 11, "bold"),
            relief="raised",
            borderwidth=3,
            padx=20,
            pady=10,
            cursor="hand2"
        )
        send_btn.pack(side="left")
        
        # Help Text
        tk.Label(
            chat_section,
            text="Type message → Press SEND or Enter → Shows on LCD for 5 sec",
            font=("Segoe UI", 8, "bold"),
            bg="#ff6600",
            fg="#000000"
        ).pack(pady=(0, 10))
    
    def create_sensor_card(self, parent, title, icon, row, col, color):
        card_outer = tk.Frame(parent, bg=color, relief="raised", borderwidth=3)
        card_outer.grid(row=row, column=col, padx=6, pady=6, sticky="nsew")
        parent.grid_columnconfigure(col, weight=1)
        parent.grid_rowconfigure(row, weight=1)
        
        card = tk.Frame(card_outer, bg=color, relief="flat")
        card.pack(fill="both", expand=True, padx=2, pady=2)
        
        icon_label = tk.Label(
            card,
            text=icon,
            font=("Segoe UI Emoji", 24),
            bg=color,
            fg="white"
        )
        icon_label.pack(pady=(10, 3))
        
        title_label = tk.Label(
            card,
            text=title,
            font=("Segoe UI", 9, "bold"),
            bg=color,
            fg="white"
        )
        title_label.pack()
        
        value_label = tk.Label(
            card,
            text="--",
            font=("Segoe UI", 18, "bold"),
            bg=color,
            fg="#ffffff"
        )
        value_label.pack(pady=(3, 10))
        
        return value_label
    
    def animate_connection_status(self):
        if self.connected:
            self.blink_state = not self.blink_state
            color = "#00ff00" if self.blink_state else "#00aa00"
            self.status_indicator.config(fg=color)
        
        self.root.after(500, self.animate_connection_status)
    
    def refresh_ports(self):
        ports = serial.tools.list_ports.comports()
        port_list = [port.device for port in ports]
        self.port_combo['values'] = port_list
        if port_list:
            self.port_combo.current(0)
    
    def toggle_connection(self):
        if not self.connected:
            self.connect()
        else:
            self.disconnect()
    
    def connect(self):
        port = self.port_combo.get()
        if not port:
            messagebox.showerror("Error", "Please select a COM port")
            return
        
        try:
            self.serial_connection = serial.Serial(port, 115200, timeout=1)
            self.connected = True
            self.running = True
            
            self.connect_btn.config(text="🔌 DISCONNECT", bg="#ff4444")
            self.status_label.config(text="Connected", fg="#00ff00")
            self.log_message("Connected to " + port, "#00ff00")
            
            self.reading_thread = threading.Thread(target=self.read_serial, daemon=True)
            self.reading_thread.start()
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect: {str(e)}")
    
    def disconnect(self):
        self.running = False
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        
        self.connected = False
        self.connect_btn.config(text="⚡ CONNECT", bg="#00ff88")
        self.status_label.config(text="Disconnected", fg="#ff4444")
        self.status_indicator.config(fg="#ff4444")
        self.log_message("Disconnected", "#ff4444")
    
    def read_serial(self):
        while self.running and self.serial_connection and self.serial_connection.is_open:
            try:
                if self.serial_connection.in_waiting:
                    line = self.serial_connection.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.process_incoming_data(line)
                        
                        color = "#00ff00"
                        if "WARNING" in line or "ALERT" in line:
                            color = "#ff4444"
                            self.alert_count += 1
                            self.alert_counter.config(text=str(self.alert_count))
                        elif "SOS" in line:
                            color = "#ff0000"
                        elif "===" in line:
                            color = "#00ffff"
                        
                        self.log_message(line, color)
            except Exception as e:
                self.log_message(f"Error: {str(e)}", "#ffaa00")
            time.sleep(0.1)
    
    def process_incoming_data(self, line):
        try:
            if "Temperature:" in line:
                temp = line.split("Temperature:")[1].strip().split()[0]
                self.temperature = temp + "°C"
            elif "Humidity:" in line:
                humid = line.split("Humidity:")[1].strip().split()[0]
                self.humidity = humid + "%"
            elif "Heart Rate:" in line and "N/A" not in line:
                hr = line.split("Heart Rate:")[1].strip().split()[0]
                self.heart_rate = hr
            elif "SpO2:" in line and "N/A" not in line:
                spo2 = line.split("SpO2:")[1].strip().split()[0]
                self.spo2 = spo2 + "%"
            elif "Latitude:" in line:
                self.latitude = line.split("Latitude:")[1].strip()
            elif "Longitude:" in line:
                self.longitude = line.split("Longitude:")[1].strip()
            elif "GPS:" in line:
                self.gps_status = line.split("GPS:")[1].strip()
        except:
            pass
    
    def update_sensor_display(self):
        self.temp_card.config(text=self.temperature)
        self.humid_card.config(text=self.humidity)
        self.hr_card.config(text=self.heart_rate)
        self.spo2_card.config(text=self.spo2)
        
        self.gps_status_label.config(text=f"Status: {self.gps_status}")
        self.lat_label.config(text=f"Lat: {self.latitude}")
        self.lon_label.config(text=f"Lon: {self.longitude}")
        
        self.root.after(500, self.update_sensor_display)
    
    def send_message(self, message):
        if self.connected and self.serial_connection:
            try:
                self.serial_connection.write((message + "\n").encode())
                self.log_message(f"Sent: {message}", "#ffff00")
            except Exception as e:
                self.log_message(f"Error: {str(e)}", "#ff4444")
        else:
            messagebox.showwarning("Not Connected", "Please connect to ESP32 first!")
    
    def send_sos(self):
        if messagebox.askyesno("SOS Emergency", "Send SOS Alert?"):
            self.send_message("SOS")
    
    def send_status(self):
        self.send_message("STATUS")
    
    def send_gps(self):
        self.send_message("GPS")
    
    def send_thresholds(self):
        self.send_message("THRESHOLDS")
    
    def send_help(self):
        self.send_message("HELP")
    
    def send_custom_message(self):
        message = self.message_entry.get().strip()
        if message:
            self.send_message(message)
            self.message_entry.delete(0, tk.END)
        else:
            messagebox.showinfo("Empty", "Type a message first!")
    
    def log_message(self, message, color="#00ff00"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.console.tag_config(color, foreground=color)
        self.console.insert(tk.END, f"[{timestamp}] ", "timestamp")
        self.console.insert(tk.END, f"{message}\n", color)
        self.console.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = HealthMonitorGUI(root)
    root.mainloop()
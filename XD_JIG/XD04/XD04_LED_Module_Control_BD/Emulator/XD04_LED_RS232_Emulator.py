import sys
import subprocess
import threading
import time
import pyautogui
import pygetwindow as gw
from PyQt5.QtWidgets import (
    QApplication, QWidget, QPushButton, QTextEdit, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QRadioButton, QShortcut, QFrame, QComboBox
)
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import QTimer, Qt

import serial
import serial.tools.list_ports

VERSION_INFO = "GUI Version 1.0"

class MacroApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("XD04 LED RS232 Emulator")
        self.setGeometry(100, 100, 700, 700)

        self.ser = None
        self.read_thread = None
        self.thread_running = False

        self.init_ui()
        self.init_shortcuts()
        self.init_serial()

    def init_serial(self):
        ports = list(serial.tools.list_ports.comports())
        if not ports:
            self.log("⚠️ 연결된 COM 포트가 없습니다.")
        else:
            self.log("사용 가능한 COM 포트:")
            for i, port in enumerate(ports):
                self.log(f"{i}: {port.device} - {port.description}")

            # 예: 첫 번째 포트를 자동 선택
            com_port = ports[0].device
            # com_port = ports[1].device
            self.log(f"자동 선택된 COM 포트: {com_port}")

            # 포트 열기
            self.ser = serial.Serial(com_port, 115200, timeout=1)
            self.log("UART 포트 열기 성공!")

            # 수신 스레드 시작
            self.thread_running = True
            self.read_thread = threading.Thread(target=self.read_serial_data, daemon=True)
            self.read_thread.start()

    def init_ui(self):
        layout = QVBoxLayout()

        self.gui_version = QLabel(VERSION_INFO)
        info_layout = QHBoxLayout()
        info_layout.addWidget(self.gui_version)

        # Packet 설정
        packet_layout = QHBoxLayout()

        sop_ver_layout = QVBoxLayout()
        sop_ver_layout.addWidget(QLabel("SOP"))
        self.sop_cb = QComboBox()
        self.sop_cb.addItems(["0xA5"])
        sop_ver_layout.addWidget(self.sop_cb)

        length_ver_layout = QVBoxLayout()
        length_ver_layout.addWidget(QLabel("LENGTH"))
        self.length_cb = QComboBox()
        self.length_cb.addItems(["0x00", "0x01", "0x02", "0x03", "0x04", "0x05", "0x06", "0x07", "0x08", "0x09", "0x0A", "0x0B", "0x0C", "0x0D", "0x0E", "0x0F"])
        length_ver_layout.addWidget(self.length_cb)

        command_ver_layout = QVBoxLayout()
        command_ver_layout.addWidget(QLabel("COMMAND"))
        self.command_cb = QComboBox()
        self.command_cb.addItems(["0x00", "0xFF", "0xF0", "0x01", "0x10", "0x20", "0x40", "0x80"])
        command_ver_layout.addWidget(self.command_cb)

        data_ver_layout = QVBoxLayout()
        data_ver_layout.addWidget(QLabel("DATA"))
        self.data_label = QLineEdit()
        self.data_label.setText("0")
        data_ver_layout.addWidget(self.data_label)

        checksum_ver_layout = QVBoxLayout()
        checksum_ver_layout.addWidget(QLabel("CHECKSUM"))
        self.checksum_label = QLineEdit()
        checksum_ver_layout.addWidget(self.checksum_label)

        eop_ver_layout = QVBoxLayout()
        eop_ver_layout.addWidget(QLabel("EOP"))
        self.eop_cb = QComboBox()
        self.eop_cb.addItems(["0x5A"])
        eop_ver_layout.addWidget(self.eop_cb)

        self.sop_cb.setFixedWidth(80)
        self.length_cb.setFixedWidth(80)
        self.command_cb.setFixedWidth(80)
        self.data_label.setFixedWidth(80)
        self.checksum_label.setFixedWidth(80)
        self.eop_cb.setFixedWidth(80)

        packet_layout.addLayout(sop_ver_layout)
        packet_layout.addLayout(length_ver_layout)
        packet_layout.addLayout(command_ver_layout)
        packet_layout.addLayout(data_ver_layout)
        packet_layout.addLayout(checksum_ver_layout)
        packet_layout.addLayout(eop_ver_layout)

        self.run_button = QPushButton("Send Normal Packet")
        self.run_button.clicked.connect(self.send_normal)
        self.wrong_checksum = QPushButton("Abnormal - Checksum Error")
        self.wrong_checksum.clicked.connect(self.send_wrong_checksum)
        self.half_packet = QPushButton("Abnormal - Half Packet")
        self.half_packet.clicked.connect(self.send_half_packet)
        btn_layout = QHBoxLayout()
        btn_layout.addWidget(self.run_button)
        btn_layout.addWidget(self.wrong_checksum)
        btn_layout.addWidget(self.half_packet)

        self.log_output = QTextEdit()
        self.log_output.setReadOnly(True)

        # 레이아웃 설정
        layout.addLayout(info_layout)
        layout.addWidget(self.create_separator())
        layout.addWidget(QLabel("[Packet 설정]"))
        layout.addLayout(packet_layout)
        layout.addWidget(self.create_separator())
        layout.addWidget(QLabel("[Packet 전송]"))
        layout.addLayout(btn_layout)
        layout.addWidget(self.create_separator())
        layout.addWidget(QLabel("[실행 로그]"))
        layout.addWidget(self.log_output)

        self.setLayout(layout)

    def create_separator(self):
        line = QFrame()
        line.setFrameShape(QFrame.HLine)
        line.setFrameShadow(QFrame.Sunken)
        return line

    def init_shortcuts(self):
        # Ctrl+R을 누르면 매크로 시작
        start_shortcut = QShortcut(QKeySequence("Ctrl+R"), self)
        start_shortcut.activated.connect(self.send_normal)

    def log(self, message):
        self.log_output.append(message)
        cursor = self.log_output.textCursor()
        cursor.movePosition(cursor.End)
        self.log_output.setTextCursor(cursor)
        self.log_output.ensureCursorVisible()

    def send_normal(self):
        try:
            sop_val = int(self.sop_cb.currentText(), 16)
            length_val = int(self.length_cb.currentText(), 16)
            command_val = int(self.command_cb.currentText(), 16)
            data_val = int(self.data_label.text().strip())
            checksum_val = (sop_val + length_val + command_val + data_val) & 0xFF
            eop_val = int(self.eop_cb.currentText(), 16)
            # update checksum field
            self.checksum_label.setText(f"0x{checksum_val:02X}")
            packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
            self.ser.write(packet)
            self.log(f"📤 Sent Packet: \t{[f'{b:02X}' for b in packet]}")
        except ValueError:
            self.log("⚠️ Invalid Value.")
            return

    def send_wrong_checksum(self):
        try:
            sop_val = int(self.sop_cb.currentText(), 16)
            length_val = int(self.length_cb.currentText(), 16)
            command_val = int(self.command_cb.currentText(), 16)
            data_val = int(self.data_label.text().strip())
            checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF) + 1  # Intentionally wrong
            eop_val = int(self.eop_cb.currentText(), 16)
            # update checksum field
            self.checksum_label.setText(f"0x{checksum_val:02X}")
            packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
            self.ser.write(packet)
            self.log(f"📤 Sent Wrong Packet: \t{[f'{b:02X}' for b in packet]}")
        except ValueError:
            self.log("⚠️ Invalid Value.")
            return

    def send_half_packet(self):
        try:
            sop_val = int(self.sop_cb.currentText(), 16)
            length_val = int(self.length_cb.currentText(), 16)
            command_val = int(self.command_cb.currentText(), 16)
            # update checksum field
            packet = bytes([sop_val, length_val, command_val])
            self.ser.write(packet)
            self.log(f"📤 Sent Wrong Packet: \t{[f'{b:02X}' for b in packet]}")
        except ValueError:
            self.log("⚠️ Invalid Value.")
            return

    def read_serial_data(self):
        # MCU → PC UART 데이터 읽기 스레드
        while self.thread_running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)  # 들어온 데이터 모두 읽기
                    if data:
                        hex_str = " ".join([f"{b:02X}" for b in data])
                        text_str = data.decode(errors="ignore")
                        self.log(f"📥 Received: HEX[{hex_str}] | STR[{text_str}]")
                time.sleep(0.05)  # CPU 점유율 방지
            except Exception as e:
                self.log(f"⚠️ 수신 오류: {e}")
                break

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MacroApp()
    window.show()
    sys.exit(app.exec_())
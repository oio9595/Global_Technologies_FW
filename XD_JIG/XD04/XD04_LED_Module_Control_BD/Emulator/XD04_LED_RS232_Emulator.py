import sys
import subprocess
import threading
import time
import pyautogui
import pygetwindow as gw
from PyQt5.QtWidgets import (
    QApplication, QWidget, QPushButton, QTextEdit, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QRadioButton, QShortcut, QFrame, QComboBox, QMessageBox, QInputDialog
)
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import QTimer, Qt, pyqtSignal

import serial
import serial.tools.list_ports

VERSION_INFO = "GUI Version 1.0"
WINDOW_TITLE = "XD04 LED Control B'D RS232 Emulator"

CMD_INITIAL = 0x00
CMD_QUIT = 0xFF
CMD_CURRENT = 0x01

CMD_BAR_ON = 0x10
CMD_BAR_OFF = 0x20

CMD_BLOCK_ON = 0x40
CMD_BLOCK_OFF = 0x80

INTERVAL_DELAY = 0.01  # seconds

commands = {
    "0x00 (Initial)": 0x00,
    "0xFF (Quit)": 0xFF,
    "0xF0 (Status)": 0xF0,
    "0x01 (Current)": 0x01,
    "0x10 (Bar On Select)": 0x10,
    "0x20 (Bar Off Select)": 0x20,
    "0x40 (Block On Select)": 0x40,
    "0x80 (Block Off Select)": 0x80,
    "0x02 (Low Current Mode)": 0x02,
}

class MacroApp(QWidget):
    log_signal = pyqtSignal(str)
    def __init__(self):
        super().__init__()
        self.setWindowTitle(WINDOW_TITLE)
        self.setGeometry(100, 100, 700, 700)

        self.ser = None
        self.read_thread = None
        self.thread_running = False

        self.init_ui()
        # self.init_serial()
        self.log_signal.connect(self.log)

        # 시퀀스 테스트 타이머
        self.seq_timer = QTimer(self)
        self.seq_timer.timeout.connect(self.send_sequence_test)

    def init_serial(self):
        ports = list(serial.tools.list_ports.comports())
        if not ports:
            self.log("⚠️ 연결된 COM 포트가 없습니다.")
            QMessageBox.warning(self, "COM Port 오류", "연결된 COM 포트가 없습니다.")
            return
        else:
            # 포트 목록 만들기
            port_list = [f"{port.device} - {port.description}" for port in ports]

            # 사용자에게 선택창 보여주기
            item, ok = QInputDialog.getItem(
                self,
                "COM Port 선택",
                "사용할 COM Port를 선택하세요:",
                port_list,
                0,  # 기본 선택 index
                False
            )

            if ok and item:
                # 선택된 포트에서 device 부분만 추출
                com_port = item.split(" - ")[0]
                self.log(f"선택된 COM 포트: {com_port}")

                try:
                    # 포트 열기
                    self.ser = serial.Serial(com_port, 115200, timeout=1)
                    self.log("UART 포트 열기 성공!")

                    # 수신 스레드 시작
                    self.thread_running = True
                    self.read_thread = threading.Thread(target=self.read_serial_data, daemon=True)
                    self.read_thread.start()

                    # 🔥 COM 연결 성공 → 15초 타이머 시작
                    self.seq_timer.start(15000)
                    self.log("⏱ 15초 Sequence Test 타이머 시작")

                except Exception as e:
                    self.log(f"❌ 포트 열기 실패: {e}")
                    QMessageBox.critical(self, "포트 열기 실패", str(e))
            else:
                self.log("⚠️ 사용자가 COM 포트를 선택하지 않았습니다.")

    def init_ui(self):
        layout = QVBoxLayout()

        self.gui_version = QLabel(VERSION_INFO)
        self.ser_con_btn = QPushButton("Connect COM Port")
        self.ser_con_btn.clicked.connect(self.init_serial)
        info_layout = QHBoxLayout()
        info_layout.addWidget(self.gui_version)
        info_layout.addWidget(self.ser_con_btn)

        # Packet 설정
        packet_layout = QHBoxLayout()

        sop_ver_layout = QVBoxLayout()
        sop_ver_layout.addWidget(QLabel("SOP"))
        self.sop_cb = QComboBox()
        self.sop_cb.addItems(["0xA5"])
        sop_ver_layout.addWidget(self.sop_cb)

        length_ver_layout = QVBoxLayout()
        length_ver_layout.addWidget(QLabel("LENGTH"))
        self.length_label = QLineEdit()
        self.length_label.setText("1")
        length_ver_layout.addWidget(self.length_label)

        command_ver_layout = QVBoxLayout()
        command_ver_layout.addWidget(QLabel("COMMAND"))
        self.command_cb = QComboBox()
        self.command_cb.addItems(commands.keys())
        self.commands = commands
        command_ver_layout.addWidget(self.command_cb)

        data_ver_layout = QVBoxLayout()
        data_ver_layout.addWidget(QLabel("DATA [Decimal]"))
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
        self.length_label.setFixedWidth(80)
        self.command_cb.setFixedWidth(200)
        self.data_label.setFixedWidth(80)
        self.checksum_label.setFixedWidth(80)
        self.eop_cb.setFixedWidth(80)

        packet_layout.addLayout(sop_ver_layout)
        packet_layout.addLayout(length_ver_layout)
        packet_layout.addLayout(command_ver_layout)
        packet_layout.addLayout(data_ver_layout)
        packet_layout.addLayout(checksum_ver_layout)
        packet_layout.addLayout(eop_ver_layout)

        self.normal_button = QPushButton("Send Normal Packet")
        self.normal_button.clicked.connect(self.send_normal)
        self.sequence_test = QPushButton("Sequence Test")
        self.sequence_test.clicked.connect(self.send_sequence_test)
        self.half_packet = QPushButton("Abnormal - Half Packet")
        self.half_packet.clicked.connect(self.send_half_packet)
        btn_layout = QHBoxLayout()
        btn_layout.addWidget(self.normal_button)
        btn_layout.addWidget(self.sequence_test)
        btn_layout.addWidget(self.half_packet)

        self.log_output = QTextEdit()
        self.log_output.setReadOnly(True)

        # 레이아웃 설정
        layout.addLayout(info_layout)
        layout.addWidget(self.create_separator())
        layout.addWidget(QLabel("[Packet 구성]"))
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

    def log(self, message):
        self.log_output.append(message)
        cursor = self.log_output.textCursor()
        cursor.movePosition(cursor.End)
        self.log_output.setTextCursor(cursor)
        self.log_output.ensureCursorVisible()

    def send_normal(self):
        if self.thread_running :
            try:
                sop_val = int(self.sop_cb.currentText(), 16)
                length_val = int(self.length_label.text().strip())
                # command_val = int(self.command_cb.currentText(), 16)
                command_text = self.command_cb.currentText()
                command_val = self.commands[command_text]
                eop_val = int(self.eop_cb.currentText(), 16)
                """
                if length_val == 1:
                    data_val = int(self.data_label.text().strip())
                    checksum_val = (sop_val + length_val + command_val + data_val) & 0xFF
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                elif length_val == 2:
                    data_val_1 = int(self.data_label.text().strip())
                    data_val_2 = data_val_1 + 1
                    checksum_val = (sop_val + length_val + command_val + data_val_1 + data_val_2) & 0xFF
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val_1, data_val_2, checksum_val, eop_val])
                elif length_val == 3:
                    data_val_1 = int(self.data_label.text().strip())
                    data_val_2 = data_val_1 + 1
                    data_val_3 = data_val_1 + 2
                    checksum_val = (sop_val + length_val + command_val + data_val_1 + data_val_2 + data_val_3) & 0xFF
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val_1, data_val_2, data_val_3, checksum_val, eop_val])
                elif length_val == 4:
                    data_val_1 = int(self.data_label.text().strip())
                    data_val_2 = data_val_1 + 1
                    data_val_3 = data_val_1 + 2
                    data_val_4 = data_val_1 + 3
                    checksum_val = (sop_val + length_val + command_val + data_val_1 + data_val_2 + data_val_3 + data_val_4) & 0xFF
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val_1, data_val_2, data_val_3, data_val_4, checksum_val, eop_val])
                """
                data_val_1 = int(self.data_label.text().strip())

                # length 만큼 data 값 생성 (data_val_1, data_val_1+1, data_val_1+2, ...)
                data_list = [data_val_1 + i for i in range(length_val)]

                # 체크섬 계산
                checksum_val = (sop_val + length_val + command_val + sum(data_list)) & 0xFF
                self.checksum_label.setText(f"0x{checksum_val:02X}")

                # 패킷 생성
                packet = bytes([sop_val, length_val, command_val] + data_list + [checksum_val, eop_val])
                self.ser.write(packet)
                self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
            except ValueError:
                self.log("⚠️ Invalid Value.")
                return
        else :
            self.log("⚠️ COM 포트가 연결되어 있지 않습니다.")

    def send_sequence_test(self):
        if self.thread_running :
            try:
                sop_val = int(self.sop_cb.currentText(), 16)
                length_val = int(self.length_label.text().strip())
                command_val = CMD_INITIAL
                data_val = 0
                checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                eop_val = int(self.eop_cb.currentText(), 16)
                self.checksum_label.setText(f"0x{checksum_val:02X}")
                packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                self.ser.write(packet)
                self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
                QApplication.processEvents()
                time.sleep(INTERVAL_DELAY)

                sop_val = int(self.sop_cb.currentText(), 16)
                length_val = int(self.length_label.text().strip())
                command_val = CMD_CURRENT
                data_val = 5
                checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                eop_val = int(self.eop_cb.currentText(), 16)
                self.checksum_label.setText(f"0x{checksum_val:02X}")
                packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                self.ser.write(packet)
                self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
                QApplication.processEvents()
                time.sleep(INTERVAL_DELAY)

                self.log("Repeat Start!!!")
                self.log("Bar On")
                for i in range(20):
                    sop_val = int(self.sop_cb.currentText(), 16)
                    length_val = int(self.length_label.text().strip())
                    command_val = CMD_BAR_ON
                    data_val = (i+1)
                    checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                    eop_val = int(self.eop_cb.currentText(), 16)
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                    self.ser.write(packet)
                    self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
                    QApplication.processEvents()
                    time.sleep(INTERVAL_DELAY*5)

                self.log("Bar Off")
                for i in range(20):
                    sop_val = int(self.sop_cb.currentText(), 16)
                    length_val = int(self.length_label.text().strip())
                    command_val = CMD_BAR_OFF
                    data_val = (i+1)
                    checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                    eop_val = int(self.eop_cb.currentText(), 16)
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                    self.ser.write(packet)
                    self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
                    QApplication.processEvents()
                    time.sleep(INTERVAL_DELAY*5)

                self.log("Block On")
                for i in range(160):
                    sop_val = int(self.sop_cb.currentText(), 16)
                    length_val = int(self.length_label.text().strip())
                    command_val = CMD_BLOCK_ON
                    data_val = (i+1)
                    checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                    eop_val = int(self.eop_cb.currentText(), 16)
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                    self.ser.write(packet)
                    self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
                    QApplication.processEvents()
                    time.sleep(INTERVAL_DELAY)

                self.log("Block Off")
                for i in range(160):
                    sop_val = int(self.sop_cb.currentText(), 16)
                    length_val = int(self.length_label.text().strip())
                    command_val = CMD_BLOCK_OFF
                    data_val = (i+1)
                    checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                    eop_val = int(self.eop_cb.currentText(), 16)
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                    self.ser.write(packet)
                    self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")
                    QApplication.processEvents()
                    time.sleep(INTERVAL_DELAY)


                self.log("Blink")
                for i in range(5):
                    sop_val = int(self.sop_cb.currentText(), 16)
                    length_val = int(self.length_label.text().strip())
                    command_val = CMD_BAR_ON
                    data_val = 0
                    checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                    eop_val = int(self.eop_cb.currentText(), 16)
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                    self.ser.write(packet)
                    self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")

                    QApplication.processEvents()
                    time.sleep(INTERVAL_DELAY*20)

                    sop_val = int(self.sop_cb.currentText(), 16)
                    length_val = int(self.length_label.text().strip())
                    command_val = CMD_BAR_OFF
                    data_val = 0
                    checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                    eop_val = int(self.eop_cb.currentText(), 16)
                    self.checksum_label.setText(f"0x{checksum_val:02X}")
                    packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                    self.ser.write(packet)
                    self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")

                    QApplication.processEvents()
                    time.sleep(INTERVAL_DELAY*20)

                sop_val = int(self.sop_cb.currentText(), 16)
                length_val = int(self.length_label.text().strip())
                command_val = CMD_QUIT
                data_val = 0
                checksum_val = ((sop_val + length_val + command_val + data_val) & 0xFF)
                eop_val = int(self.eop_cb.currentText(), 16)
                self.checksum_label.setText(f"0x{checksum_val:02X}")
                packet = bytes([sop_val, length_val, command_val, data_val, checksum_val, eop_val])
                self.ser.write(packet)
                self.log(f"📤 Tx Normal Packet \t{[f'{b:02X}' for b in packet]}")

                QApplication.processEvents()
                time.sleep(INTERVAL_DELAY)


                self.log("Repeat Done!!!")
            except ValueError:
                self.log("⚠️ Invalid Value.")
                return
        else :
            self.log("⚠️ COM 포트가 연결되어 있지 않습니다.")

    def send_half_packet(self):
        if self.thread_running :
            try:
                sop_val = int(self.sop_cb.currentText(), 16)
                length_val = int(self.length_label.text().strip())
                # command_val = int(self.command_cb.currentText(), 16)
                command_text = self.command_cb.currentText()
                command_val = self.commands[command_text]
                packet = bytes([sop_val, length_val, command_val])
                self.ser.write(packet)
                self.log(f"📤 Tx Half Packet \t{[f'{b:02X}' for b in packet]}")
            except ValueError:
                self.log("⚠️ Invalid Value.")
                return
        else :
            self.log("⚠️ COM 포트가 연결되어 있지 않습니다.")

    def read_serial_data(self):
        # MCU → PC UART 데이터 읽기 스레드
        while self.thread_running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)  # 들어온 데이터 모두 읽기
                    if data:
                        hex_list = [f"{b:02X}" for b in data]
                        if len(data) == 6:
                            calc_checksum = sum(data[:4]) & 0xFF
                            recv_checksum = data[4]
                            if calc_checksum == recv_checksum:
                                result = f"✅\r\n"
                            else:
                                result = f"❌ Rx Checksum Error (0x{recv_checksum:02X} / 0x{calc_checksum:02X})\r\n"
                            text_str = data.decode(errors="ignore")
                            self.log_signal.emit(f"📥 Rx \t\t\t{hex_list} | {result}")
                        else:
                            self.log_signal.emit(f"📥 Rx \t\t\t{hex_list} | (Length Error)")

                time.sleep(0.05)  # CPU 점유율 방지
            except Exception as e:
                # self.log(f"⚠️ 수신 오류: {e}")
                self.log_signal.emit(f"⚠️ 수신 오류: {e}")
                break

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MacroApp()
    window.show()
    sys.exit(app.exec_())
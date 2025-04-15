import sys
import subprocess
import threading
import time
import pyautogui
import pygetwindow as gw
from PyQt5.QtWidgets import (
    QApplication, QWidget, QPushButton, QTextEdit, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QRadioButton, QShortcut, QFrame
)
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import QTimer, Qt

# 실행 경로
PATH_TERATERM = r"C:\Program Files (x86)\teraterm5\ttermpro.exe"
PATH_CAS40 = r"C:\Program Files (x86)\KONICA MINOLTA\CA-S40\CA-S40.exe"

VERSION_INFO = "GUI Version 1.0"
MACRO_RUNNING = "🛑 매크로 실행 중"
MACRO_STOPPED = "✅ 매크로 중지됨"

class MacroApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Linearity Macro GUI")
        self.setGeometry(100, 100, 700, 700)
        self.macro_running = False
        self.init_ui()
        self.init_shortcuts()

    def init_ui(self):
        layout = QVBoxLayout()

        self.gui_version = QLabel(VERSION_INFO)
        self.macro_state = QLabel(MACRO_STOPPED)
        self.macro_state.setAlignment(Qt.AlignRight)
        self.macro_state.setStyleSheet("color: green;")
        info_layout = QHBoxLayout()
        info_layout.addWidget(self.gui_version)
        info_layout.addWidget(self.macro_state)

        self.step_radio = QRadioButton("step")
        self.ana_radio = QRadioButton("ana")
        self.custom_radio = QRadioButton("직접 입력")
        self.step_radio.setChecked(True)

        radio_layout = QHBoxLayout()
        radio_layout.addWidget(self.step_radio)
        radio_layout.addWidget(self.ana_radio)
        radio_layout.addWidget(self.custom_radio)

        # [변경된 부분] 각 입력 값에 대한 라벨 추가
        self.start_label = QLabel("시작값")
        self.start_input = QLineEdit()

        self.end_label = QLabel("끝값")
        self.end_input = QLineEdit()

        self.interval_label = QLabel("간격")
        self.interval_input = QLineEdit()

        range_layout = QVBoxLayout()
        range_layout.addWidget(QLabel("[입력값 설정]"))

        range_row = QHBoxLayout()
        range_row.addWidget(self.start_label)
        range_row.addWidget(self.start_input)
        range_row.addWidget(self.end_label)
        range_row.addWidget(self.end_input)
        range_row.addWidget(self.interval_label)
        range_row.addWidget(self.interval_input)

        range_layout.addLayout(range_row)

        # 딜레이 관련 라벨 추가
        self.delay1_label = QLabel("입력 후 측정 딜레이 (초)")
        self.delay1_input = QLineEdit()

        self.delay2_label = QLabel("측정 후 다음 입력까지 딜레이 (초)")
        self.delay2_input = QLineEdit()

        delay_layout = QVBoxLayout()
        delay_layout.addWidget(QLabel("[딜레이 설정]"))

        delay_row = QHBoxLayout()
        delay_row.addWidget(self.delay1_label)
        delay_row.addWidget(self.delay1_input)
        delay_row.addWidget(self.delay2_label)
        delay_row.addWidget(self.delay2_input)

        delay_layout.addLayout(delay_row)

        self.custom_input = QLineEdit()
        self.custom_input.setPlaceholderText("직접 명령어 형식 예: duty")

        self.run_button = QPushButton("Macro RUN")
        self.run_button.clicked.connect(self.start_macro)
        self.stop_button = QPushButton("Macro STOP")
        self.stop_button.clicked.connect(self.stop_macro)
        btn_layout = QHBoxLayout()
        btn_layout.addWidget(self.run_button)
        btn_layout.addWidget(self.stop_button)

        self.launch_tt_button = QPushButton("TeraTerm 실행")
        self.launch_tt_button.clicked.connect(self.launch_teraterm)
        self.launch_ca_button = QPushButton("CA-S40 실행")
        self.launch_ca_button.clicked.connect(self.launch_cas40)
        self.launch_resize_button = QPushButton("화면 정렬")
        self.launch_resize_button.clicked.connect(self.launch_resize_window)
        func_layout = QHBoxLayout()
        func_layout.addWidget(self.launch_tt_button)
        func_layout.addWidget(self.launch_ca_button)
        func_layout.addWidget(self.launch_resize_button)

        self.log_output = QTextEdit()
        self.log_output.setReadOnly(True)

        # 레이아웃 설정
        layout.addLayout(info_layout)
        layout.addWidget(self.create_separator())

        layout.addWidget(QLabel("[명령어 설정]"))
        layout.addLayout(radio_layout)
        layout.addWidget(self.custom_input)
        layout.addWidget(self.create_separator())
        layout.addLayout(range_layout)
        layout.addWidget(self.create_separator())
        layout.addLayout(delay_layout)
        layout.addWidget(self.create_separator())
        layout.addWidget(QLabel("[Macro 설정]"))
        layout.addLayout(btn_layout)
        layout.addLayout(func_layout)
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
        # Ctrl+S를 누르면 매크로 중지
        stop_shortcut = QShortcut(QKeySequence("Ctrl+S"), self)
        stop_shortcut.activated.connect(self.stop_macro)

        # ESC 누르면 매크로 중지
        esc_shortcut = QShortcut(QKeySequence("Esc"), self)
        esc_shortcut.activated.connect(self.stop_macro)

        # Ctrl+R을 누르면 매크로 시작
        start_shortcut = QShortcut(QKeySequence("Ctrl+R"), self)
        start_shortcut.activated.connect(self.start_macro)

    def log(self, message):
        self.log_output.append(message)
        cursor = self.log_output.textCursor()
        cursor.movePosition(cursor.End)
        self.log_output.setTextCursor(cursor)
        self.log_output.ensureCursorVisible()

    def launch_teraterm(self):
        try:
            subprocess.Popen([PATH_TERATERM])
            self.log("✅ TeraTerm 실행 완료")
        except Exception as e:
            self.log(f"❌ TeraTerm 실행 오류: {e}")

    def launch_cas40(self):
        try:
            subprocess.Popen([PATH_CAS40])
            self.log("✅ CA-S40 실행 완료")
        except Exception as e:
            self.log(f"❌ CA-S40 실행 오류: {e}")

    def launch_resize_window(self):
        try:
            self.position_windows()
            self.log("✅ 화면 재정렬 완료")
        except Exception as e:
            self.log(f"❌ 화면 재정렬 오류: {e}")

    def position_windows(self):
        screen_width, screen_height = pyautogui.size()
        screen_height -= 50  # 화면 하단 상태바를 고려하여 높이 조정
        third_width = screen_width // 4
        half_height = screen_height // 2

        self.log("✅ screen size: {}x{}".format(screen_width, screen_height))
        self.log("✅ third_width: {}".format(third_width))
        self.log("✅ half_height: {}".format(half_height))

        layout_positions = {
            "Tera Term VT": (0, 0, third_width, half_height),        # 1번
            "Linearity Macro GUI": (0, half_height, third_width, half_height),  # 5번
            "PC Software for Color Analyzer": (third_width, 0, screen_width - third_width, screen_height)  # 나머지 (2~4, 6~8)
        }

        for win in gw.getWindowsWithTitle("Tera Term VT"):
            win.moveTo(*layout_positions["Tera Term VT"][:2])
            win.resizeTo(*layout_positions["Tera Term VT"][2:])
            self.log("📐 TeraTerm 위치 설정 완료")

        for win in gw.getWindowsWithTitle("Linearity Macro GUI"):
            win.moveTo(*layout_positions["Linearity Macro GUI"][:2])
            win.resizeTo(*layout_positions["Linearity Macro GUI"][2:])
            self.log("📐 Macro GUI 위치 설정 완료")

        for win in gw.getWindowsWithTitle("PC Software for Color Analyzer"):
            win.moveTo(*layout_positions["PC Software for Color Analyzer"][:2])
            win.resizeTo(*layout_positions["PC Software for Color Analyzer"][2:])
            self.log("📐 CA-S40 위치 설정 완료")

    def start_macro(self):
        try:
            start = int(self.start_input.text().strip())
            end = int(self.end_input.text().strip())
            interval = int(self.interval_input.text().strip())
            delay1 = float(self.delay1_input.text().strip())
            delay2 = float(self.delay2_input.text().strip())
        except ValueError:
            self.log("⚠️ 모든 입력칸에 정수 또는 실수를 정확히 입력해주세요.")
            return

        if self.step_radio.isChecked():
            pattern = "step {}"
        elif self.ana_radio.isChecked():
            pattern = "ana {}"
        else:
            raw_input = self.custom_input.text().strip()
            if not raw_input:
                self.log("⚠️ 사용자 명령어를 입력하세요.")
                return
            if '{}' in raw_input:
                pattern = raw_input
            else:
                pattern = raw_input + ' {}'

        self.macro_running = True
        self.macro_state.setText(MACRO_RUNNING)
        # 오른쪽 정렬
        self.macro_state.setAlignment(Qt.AlignRight)
        # 빨간색으로 변경
        self.macro_state.setStyleSheet("color: red;")
        self.log(f"🔁 매크로 시작: {pattern} [{start} ~ {end}], 간격 {interval}, 딜레이1={delay1}s, 딜레이2={delay2}s")

        thread = threading.Thread(
            target=self.run_macro, args=(pattern, start, end, interval, delay1, delay2)
        )
        thread.start()

    def stop_macro(self):
        self.macro_running = False
        self.macro_state.setText(MACRO_STOPPED)
        self.macro_state.setAlignment(Qt.AlignRight)
        self.macro_state.setStyleSheet("color: green;")
        self.log("🛑 매크로 중지 요청됨")

    def run_macro(self, pattern, start, end, interval, delay1, delay2):
        tt_window = gw.getWindowsWithTitle("Tera Term VT")[0]
        ca_window = gw.getWindowsWithTitle("PC Software for Color Analyzer")[0]

        tt_x = tt_window.left + tt_window.width // 2
        tt_y = tt_window.top + tt_window.height // 2

        ca_x = ca_window.left + ca_window.width // 2
        ca_y = ca_window.top + ca_window.height // 2

        stop_btn_pos = self.stop_button.mapToGlobal(self.stop_button.rect().center())

        current = start

        while self.macro_running and current <= end:
            command = pattern.format(current)

            pyautogui.click(x=tt_x, y=tt_y)
            time.sleep(0.1)
            pyautogui.write(command)
            pyautogui.press('enter')
            self.log(f"TeraTerm 입력: {command}")

            time.sleep(delay1)

            pyautogui.click(ca_x, ca_y)
            pyautogui.keyDown('ctrl')
            pyautogui.press('m')
            pyautogui.keyUp('ctrl')
            self.log("CA-S40 측정 클릭")

            pyautogui.moveTo(stop_btn_pos.x(), stop_btn_pos.y())
            self.log("마우스 커서 위치 이동")

            time.sleep(delay2)

            next_val = current + interval
            if next_val > end and current != end:
                current = end
            else:
                current = next_val

        pyautogui.click(x=tt_x, y=tt_y)
        time.sleep(0.3)
        pyautogui.write('step 0')
        pyautogui.press('enter')
        time.sleep(0.3)
        pyautogui.write('ana 0')
        pyautogui.press('enter')

        self.macro_running = False
        self.macro_state.setText(MACRO_STOPPED)
        self.macro_state.setStyleSheet("color: green;")
        self.log("✅ 매크로 종료됨")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MacroApp()
    window.show()
    sys.exit(app.exec_())

# pyinstaller --noconfirm --windowed --onefile macro_gui.py

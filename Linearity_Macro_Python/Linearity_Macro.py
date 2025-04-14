import sys
import subprocess
import threading
import time
import pyautogui
import pygetwindow as gw
from PyQt5.QtWidgets import (
    QApplication, QWidget, QPushButton, QTextEdit, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QRadioButton, QShortcut
)
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import QTimer

# 실행 경로
PATH_TERATERM = r"C:\Program Files (x86)\teraterm5\ttermpro.exe"
PATH_CAS40 = r"C:\Program Files (x86)\KONICA MINOLTA\CA-S40\CA-S40.exe"

BUILD_INFO = "GUI Version 1.0 (2025. 04. 14)"
MACRO_RUNNING = "매크로 실행 중"
MACRO_STOPPED = "매크로 중지됨"

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

        self.gui_ver = QLabel(BUILD_INFO)
        self.macro_state = QLabel(MACRO_STOPPED)
        self.macro_state.setStyleSheet("color: green;")
        info_layout = QHBoxLayout()
        info_layout.addWidget(self.gui_ver)
        info_layout.addWidget(self.macro_state)

        self.step_radio = QRadioButton("step")
        self.ana_radio = QRadioButton("ana")
        self.custom_radio = QRadioButton("직접 입력")
        self.step_radio.setChecked(True)

        radio_layout = QHBoxLayout()
        radio_layout.addWidget(self.step_radio)
        radio_layout.addWidget(self.ana_radio)
        radio_layout.addWidget(self.custom_radio)

        self.custom_input = QLineEdit()
        self.custom_input.setPlaceholderText("직접 명령어 형식 예: bias {}")

        self.start_input = QLineEdit(); self.start_input.setPlaceholderText("시작값 (정수)")
        self.end_input = QLineEdit(); self.end_input.setPlaceholderText("끝값 (정수)")
        self.interval_input = QLineEdit(); self.interval_input.setPlaceholderText("간격 (정수)")
        range_layout = QHBoxLayout()
        range_layout.addWidget(QLabel("범위 설정:"))
        range_layout.addWidget(self.start_input)
        range_layout.addWidget(self.end_input)
        range_layout.addWidget(self.interval_input)

        self.delay1_input = QLineEdit(); self.delay1_input.setPlaceholderText("입력 후 측정 딜레이 (초)")
        self.delay2_input = QLineEdit(); self.delay2_input.setPlaceholderText("측정 후 다음 입력까지 딜레이 (초)")
        delay_layout = QHBoxLayout()
        delay_layout.addWidget(QLabel("딜레이 설정:"))
        delay_layout.addWidget(self.delay1_input)
        delay_layout.addWidget(self.delay2_input)

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

        layout.addLayout(info_layout)

        empty_layout = QHBoxLayout()
        empty_layout.addWidget(QLabel(" "))
        layout.addLayout(empty_layout)

        layout.addWidget(QLabel("[명령어 설정]"))
        layout.addLayout(radio_layout)
        layout.addWidget(self.custom_input)
        layout.addLayout(range_layout)
        layout.addLayout(delay_layout)

        empty_layout = QHBoxLayout()
        empty_layout.addWidget(QLabel(" "))
        layout.addLayout(empty_layout)

        layout.addWidget(QLabel("[Macro 설정]"))
        layout.addLayout(btn_layout)
        layout.addLayout(func_layout)

        empty_layout = QHBoxLayout()
        empty_layout.addWidget(QLabel(" "))
        layout.addLayout(empty_layout)

        layout.addWidget(QLabel("[실행 로그]"))
        layout.addWidget(self.log_output)

        self.setLayout(layout)

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

    def scroll_log_to_bottom(self):
        self.log_output.verticalScrollBar().setValue(self.log_output.verticalScrollBar().maximum())

    def log(self, message):
        self.log_output.append(message)
        QTimer.singleShot(10, self.scroll_log_to_bottom)

    def launch_teraterm(self):
        try:
            subprocess.Popen([PATH_TERATERM])
            self.log("✅ TeraTerm 실행 완료")
            # QTimer.singleShot(2000, self.position_windows)
        except Exception as e:
            self.log(f"❌ TeraTerm 실행 오류: {e}")

    def launch_cas40(self):
        try:
            subprocess.Popen([PATH_CAS40])
            self.log("✅ CA-S40 실행 완료")
            # QTimer.singleShot(3000, self.position_windows)
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
        screen_height = 1030  # 화면 하단 상태바를 고려하여 높이 조정
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
            pattern = self.custom_input.text().strip()
            if '{}' not in pattern:
                self.log("⚠️ 사용자 정의 명령어에 '{}'를 포함하세요.")
                return

        self.macro_running = True
        self.macro_state.setText(MACRO_RUNNING)
        self.macro_state.setStyleSheet("color: red;")
        self.log_output.clear()
        self.log(f"🔁 매크로 시작: {pattern} [{start} ~ {end}], 간격 {interval}, 딜레이1={delay1}s, 딜레이2={delay2}s")

        thread = threading.Thread(
            target=self.run_macro, args=(pattern, start, end, interval, delay1, delay2)
        )
        thread.start()

    def stop_macro(self):
        self.macro_running = False
        self.macro_state.setText(MACRO_STOPPED)
        self.macro_state.setStyleSheet("color: green;")
        self.log_output.clear()
        self.log("🛑 매크로 중지 요청됨")

    def run_macro(self, pattern, start, end, interval, delay1, delay2):
        screen_width, screen_height = pyautogui.size()
        tt_x = screen_width // 8
        tt_y = screen_height // 4
        ca_x = 1000
        ca_y = 500
        stop_btn_pos = self.stop_button.mapToGlobal(self.stop_button.rect().center())

        current = start
        while self.macro_running and current <= end:
            command = pattern.format(current)

            pyautogui.click(x=tt_x, y=tt_y)
            time.sleep(0.3)
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

            time.sleep(delay2)
            current += interval
            if current > end:
                current = end

        self.macro_running = False
        self.log("✅ 매크로 종료됨")

        pyautogui.click(x=tt_x, y=tt_y)
        time.sleep(0.3)
        pyautogui.write('step 0')
        pyautogui.press('enter')
        time.sleep(0.3)
        pyautogui.write('ana 0')
        pyautogui.press('enter')

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MacroApp()
    window.show()
    sys.exit(app.exec_())

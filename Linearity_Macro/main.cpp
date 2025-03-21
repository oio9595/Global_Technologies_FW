#include <iostream>
#include <string>
#include <Windows.h>
#include <ctime>
#include <thread> // 스레드 사용
#include <time.h>
#include "VK_CODE.h"

#pragma comment(lib, "user32.lib")

using namespace std;

/* Private Function */
void ClickMouse(int param_x, int param_y, char btn_id);
void MoveMouse(int param_x, int param_y);
void key_write(uint16_t msg);
uint16_t calc_vk(uint16_t value);
void set_blu_value(uint16_t blu_step);
void save_csv(int mode, int inch, int delta);
void get_monitor_coordinate();
void get_time_info();
void blu_end_init();
void key_underbar();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Window 프로시저
/* End Of Private Function */

/* Private Value*/
uint16_t gn_blu_value = 0;

int cord_display_x = 0;
int cord_display_y = 0;

int cord_term_x = 0;
int cord_term_y = 0;

int cord_lumi_x = 0;
int cord_lumi_y = 0;

int cord_meas_x = 0;
int cord_meas_y = 0;

int cord_save_x = 0;
int cord_save_y = 0;

int cord_clear_x = 0;
int cord_clear_y = 0;

int cord_exit_x = 188;
int cord_exit_y = 752;

time_t rawtime;
struct tm* pTimeInfo;

int year, month, day, hour, minu, sec = 0;

string input_mode = { NULL, };
string pgm_model_name = { NULL, };

int pgm_mode_sel = 0;

int input_inch = 0;
int input_step_start = 0;
int input_step_max = 0;
int input_step_delta = 0;

int pgm_step_max = 0;
int pgm_step_delta = 0;

HWND hEdit4, hEdit5, hEdit6; // 텍스트창 핸들 저장

bool program_running = true; // 프로그램 종료 플래그

#define KEY_DELAY 5
#define BUTTON_EXIT_ID 1001 // 버튼 ID
/* End of Private Value*/

// 텍스트 업데이트 함수
void UpdateEditText(HWND hEdit, int value) {
	wstring text = to_wstring(value);
	SetWindowText(hEdit, text.c_str());
}

void ClickMouse(int param_x, int param_y, char btn_id)
{
	int x_pos = param_x * 65535 / cord_display_x;
	int y_pos = param_y * 65535 / cord_display_y;

	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, GetMessageExtraInfo());

	if (btn_id == 1)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, GetMessageExtraInfo());
	}
	else if (btn_id == 2)
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, GetMessageExtraInfo());
	}
}

void MoveMouse(int param_x, int param_y)
{
	int x_pos = param_x * 65535 / cord_display_x;
	int y_pos = param_y * 65535 / cord_display_y;

	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x_pos, y_pos, 0, GetMessageExtraInfo());
}

void key_write(uint16_t msg)
{
	keybd_event((BYTE)msg, 0, 0, 0);
	Sleep(KEY_DELAY);
	keybd_event((BYTE)msg, 0, 0x0002, 0);
	Sleep(KEY_DELAY);
}

uint16_t calc_vk(uint16_t value)
{
	uint16_t ret = VK_0 + value;
	return ret;
}

void set_blu_value(uint16_t blu_step)
{
	uint16_t step[5] = { 0, };

	step[0] = blu_step / 10000;
	step[1] = (blu_step % 10000) / 1000;
	step[2] = (blu_step % 1000) / 100;
	step[3] = (blu_step % 100) / 10;
	step[4] = (blu_step % 10);

	ClickMouse(cord_term_x, cord_term_y, LEFT_CLICK);


	if (pgm_mode_sel == TEST_STEP)
	{
		key_write(VK_RETURN);
		key_write(VK_S);
		key_write(VK_T);
		key_write(VK_E);
		key_write(VK_P);
		key_write(VK_SPACE);
	}

	else if (pgm_mode_sel == TEST_ANA)
	{
		key_write(VK_RETURN);
		key_write(VK_A);
		key_write(VK_N);
		key_write(VK_A);
		key_write(VK_SPACE);
	}

	if (blu_step < 10)
	{
		key_write(calc_vk(step[4]));
	}
	else if (blu_step < 100)
	{
		key_write(calc_vk(step[3]));
		key_write(calc_vk(step[4]));
	}
	else if (blu_step < 1000)
	{
		key_write(calc_vk(step[2]));
		key_write(calc_vk(step[3]));
		key_write(calc_vk(step[4]));
	}
	else if (blu_step < 10000)
	{
		key_write(calc_vk(step[1]));
		key_write(calc_vk(step[2]));
		key_write(calc_vk(step[3]));
		key_write(calc_vk(step[4]));
	}
	else
	{
		key_write(calc_vk(step[0]));
		key_write(calc_vk(step[1]));
		key_write(calc_vk(step[2]));
		key_write(calc_vk(step[3]));
		key_write(calc_vk(step[4]));
	}

	key_write(VK_RETURN);
#if 0
	if (blu_step < 20)
	{
		Sleep(3000);
	}
	else
	{
		Sleep(500);
	}
#endif
	/////////////////////////////////////////////////
	// unit : ms, Delay between writing a new value and pressing the measure button
	Sleep(3000);
	/////////////////////////////////////////////////

	ClickMouse(cord_lumi_x, cord_lumi_y, LEFT_CLICK);
	ClickMouse(cord_meas_x, cord_meas_y, LEFT_CLICK);

	MoveMouse(cord_exit_x, cord_exit_y);

	/////////////////////////////////////////////////
	// unit : ms, Delay waiting for measurement to complete
	Sleep(3000);
	/////////////////////////////////////////////////
}

void blu_end_init()
{
	ClickMouse(cord_term_x, cord_term_y, LEFT_CLICK);

	key_write(VK_RETURN);
	key_write(VK_A);
	key_write(VK_N);
	key_write(VK_A);
	key_write(VK_SPACE);
	key_write(VK_0);
	key_write(VK_RETURN);

	key_write(VK_RETURN);
	key_write(VK_S);
	key_write(VK_T);
	key_write(VK_E);
	key_write(VK_P);
	key_write(VK_SPACE);
	key_write(VK_0);
	key_write(VK_RETURN);
}

void key_underbar()
{
	keybd_event(VK_SHIFT, 0, 0, 0);
	keybd_event(0xBD, 0, 0, 0);
	keybd_event(0xBD, 0, 0x0002, 0);
	keybd_event(VK_SHIFT, 0, 0x0002, 0);
}

void save_csv(int mode, int inch, int delta)
{
	get_time_info();
	uint16_t inch_info[2] = { 0, };

	uint16_t step[5] = { 0, };
	step[0] = gn_blu_value / 10000;
	step[1] = (gn_blu_value % 10000) / 1000;
	step[2] = (gn_blu_value % 1000) / 100;
	step[3] = (gn_blu_value % 100) / 10;
	step[4] = (gn_blu_value % 10);

	uint16_t in_year[2] = { 0, };
	uint16_t in_month[2] = { 0, };
	uint16_t in_day[2] = { 0, };
	uint16_t in_hour[2] = { 0, };
	uint16_t in_min[2] = { 0, };

	in_year[0] = (year % 100) / 10;
	in_year[1] = year % 10;
	in_month[0] = month / 10;
	in_month[1] = month % 10;
	in_day[0] = day / 10;
	in_day[1] = day % 10;
	in_hour[0] = hour / 10;
	in_hour[1] = hour % 10;
	in_min[0] = minu / 10;
	in_min[1] = minu % 10;

	uint16_t delta_info[2] = { 0, };
	delta_info[0] = delta / 10;
	delta_info[1] = delta % 10;

	ClickMouse(cord_save_x, cord_save_y, LEFT_CLICK);
	Sleep(10000);

	for (uint8_t i = 0; i < pgm_model_name.size(); ++i)
	{
		if (pgm_model_name[i] > 0x60 && pgm_model_name[i] < 0x7B)
		{
			pgm_model_name[i] -= 32;
		}
		key_write(pgm_model_name[i]);
	}
	key_underbar();

	if (mode == TEST_STEP)
	{
		key_write(VK_S);
		key_write(VK_T);
		key_write(VK_E);
		key_write(VK_P);
		key_underbar();
	}
	else if (mode == TEST_ANA)
	{
		key_write(VK_A);
		key_write(VK_N);
		key_write(VK_A);
		key_underbar();
	}
	key_write(VK_M);
	key_write(VK_A);
	key_write(VK_X);
	key_underbar();

	key_write(calc_vk(step[0]));
	key_write(calc_vk(step[1]));
	key_write(calc_vk(step[2]));
	key_write(calc_vk(step[3]));
	key_write(calc_vk(step[4]));
	key_underbar();

	key_write(VK_D);
	key_write(VK_E);
	key_write(VK_L);
	key_write(VK_T);
	key_write(VK_A);
	key_underbar();

	key_write(calc_vk(delta_info[0]));
	key_write(calc_vk(delta_info[1]));
	key_underbar();

	key_write(calc_vk(in_year[0]));
	key_write(calc_vk(in_year[1]));
	key_write(calc_vk(in_month[0]));
	key_write(calc_vk(in_month[1]));
	key_write(calc_vk(in_day[0]));
	key_write(calc_vk(in_day[1]));
	key_underbar();

	key_write(calc_vk(in_hour[0]));
	key_write(calc_vk(in_hour[1]));
	key_write(calc_vk(in_min[0]));
	key_write(calc_vk(in_min[1]));

	key_write(VK_RETURN);
	Sleep(60000);
}

void get_monitor_coordinate()
{
	cord_display_x = GetSystemMetrics(SM_CXSCREEN);
	cord_display_y = GetSystemMetrics(SM_CYSCREEN);

	cout << "Screen Size: " << cord_display_x << ", " << cord_display_y << endl;

	cord_term_x = 150;//30;
	cord_term_y = 350;//500;
	cout << "term point: " << cord_term_x << ", " << cord_term_y << endl;

	cord_lumi_x = 300;//600;
	cord_lumi_y = 350;//800;
	cout << "lumi point: " << cord_lumi_x << ", " << cord_lumi_y << endl;

	cord_meas_x = 1400;//1800;
	cord_meas_y = 800;//1000;
	cout << "meas point: " << cord_meas_x << ", " << cord_meas_y << endl;

	cord_save_x = 1100;//1752;
	cord_save_y = 150;//116;
	cout << "save point: " << cord_save_x << ", " << cord_save_y << endl;

	cord_clear_x = 0;
	cord_clear_y = 0;
	cout << "clear point: " << cord_clear_x << ", " << cord_clear_y << endl;
}

void get_time_info()
{
	time_t timer;
	struct tm t;

	timer = time(NULL);
	localtime_s(&t, &timer);

	year = t.tm_year + 1900;    //연도에는 1900 더해줌
	month = t.tm_mon + 1;    // 월에는 1 더해줌
	day = t.tm_mday;
	hour = t.tm_hour;
	minu = t.tm_min;
	sec = t.tm_sec;

	printf("timeInfo : %04d년 %02d월 %02d일 %02d시 %02d분 %02d초\n", year, month, day, hour, minu, sec);
}

/* Main Logic Execution Function */
void execute_program_logic() 
{
	int pgm_eroor_flag = 0;

	// 입력값 받기
	cout << "input mode : (step or ana) ";
	cin >> input_mode;
	cout << "mode : " << input_mode << endl;
	if (input_mode == "step" || input_mode == "STEP") 
	{
		pgm_mode_sel = TEST_STEP; // step
	}
	else if (input_mode == "ana" || input_mode == "ANA") 
	{
		pgm_mode_sel = TEST_ANA; // ana
	}
	else 
	{
		++pgm_eroor_flag;
		cout << "input error, pgm restart required" << endl;
	}

	if (!pgm_eroor_flag) {
		cin.ignore();
		cout << "input model : ";
		getline(cin, pgm_model_name);
		cout << "model : " << pgm_model_name << endl;

		cout << "input step start : ";
		cin >> input_step_start;
		gn_blu_value = input_step_start;

		cout << "input step end [0 ~ 65535] : ";
		cin >> input_step_max;
		if (input_step_max > 65535) 
		{
			cout << "input step max over, set to 4095" << endl;
			input_step_max = 1023;
		}
		pgm_step_max = input_step_max;

		cout << "input step delta : ";
		cin >> input_step_delta;
		pgm_step_delta = input_step_delta;

		cout << "start point : " << gn_blu_value << endl;
		cout << "end point : " << pgm_step_max << endl;
		cout << "delta : " << pgm_step_delta << endl;

		UpdateEditText(hEdit4, input_step_start);
		UpdateEditText(hEdit5, input_step_max);
		UpdateEditText(hEdit6, input_step_delta);

		get_monitor_coordinate();
		cout << "program starts in 15 seconds..." << endl;
		Sleep(15000);

		// 실행 로직
		while (gn_blu_value <= pgm_step_max) 
		{
			set_blu_value(gn_blu_value);

			if (gn_blu_value == pgm_step_max)
			{
				break;
			}

			gn_blu_value += pgm_step_delta;
			if (gn_blu_value > pgm_step_max)
			{
				gn_blu_value = pgm_step_max;
			}
		}

		// CSV 저장 및 초기화
		save_csv(pgm_mode_sel, input_inch, pgm_step_delta);
		blu_end_init();
	}

	cout << "Program logic completed!" << endl;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		// 버튼 클릭 처리
		if (LOWORD(wParam) == BUTTON_EXIT_ID) {
			cout << "Exit button clicked!" << endl; // 디버깅용 출력
			program_running = false; // 프로그램 실행 중단
			DestroyWindow(hwnd); // 창 닫기
			ExitProcess(0); // 전체 프로그램 종료
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0); // 메시지 루프 종료
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char** argv)
{
	// WinAPI로 창 생성 및 버튼 추가
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"MacroWindow";

	if (!RegisterClass(&wc)) {
		cout << "Failed to register window class" << endl;
		return EXIT_FAILURE;
	}

	HWND hwnd = CreateWindowEx(
		0,
		L"MacroWindow",
		L"Macro Program",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 350,
		NULL, NULL, hInstance, NULL);

	if (!hwnd) {
		cout << "Failed to create window" << endl;
		return EXIT_FAILURE;
	}

	// 버튼 추가
	HWND hButtonExit = CreateWindow(
		L"BUTTON",
		L"Exit Program",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		20, 120, 340, 170, // 버튼 크기와 위치 수정
		hwnd,
		(HMENU)BUTTON_EXIT_ID,
		hInstance,
		NULL);

	if (!hButtonExit) {
		cout << "Failed to create button" << endl;
		return EXIT_FAILURE;
	}
	// 2x3 텍스트창 추가
	CreateWindow(L"EDIT", L"start", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 20, 20, 100, 30, hwnd, NULL, hInstance, NULL);
	CreateWindow(L"EDIT", L"end", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 140, 20, 100, 30, hwnd, NULL, hInstance, NULL);
	CreateWindow(L"EDIT", L"delta", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 260, 20, 100, 30, hwnd, NULL, hInstance, NULL);

	hEdit4 = CreateWindow(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 20, 70, 100, 30, hwnd, NULL, hInstance, NULL);
	hEdit5 = CreateWindow(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 140, 70, 100, 30, hwnd, NULL, hInstance, NULL);
	hEdit6 = CreateWindow(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 260, 70, 100, 30, hwnd, NULL, hInstance, NULL);

	if (!hEdit4 || !hEdit5 || !hEdit6) {
		cout << "Failed to create edit boxes" << endl;
		return EXIT_FAILURE;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	// 로직 실행을 별도 스레드에서 실행
	thread logic_thread(execute_program_logic);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 프로그램 종료 시 로직 스레드 정리
	if (logic_thread.joinable()) {
		logic_thread.join();
	}

	system("pause");
	return EXIT_SUCCESS;
}
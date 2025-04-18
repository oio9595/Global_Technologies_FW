import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# LED 배열 크기
size = 26
center = size // 2

# 초기 LED 매트릭스
led_matrix = np.zeros((size, size))

# 플롯 설정
fig, ax = plt.subplots()
img = ax.imshow(led_matrix, cmap='Greys', vmin=0, vmax=1)
plt.axis('off')

# 현재 확장할 레벨 (0부터 시작)
current_level = 0

# 업데이트 함수
def update(frame):
    global current_level

    # 현재 레벨에 해당하는 사각형 그리기
    start = center - current_level
    end = center + current_level

    if start < 0 or end >= size:
        return [img]  # 범위를 넘으면 종료

    # 사각형 테두리 그리기
    for x in range(start, end + 1):
        led_matrix[start, x] = 1  # 위쪽
        led_matrix[end, x] = 1    # 아래쪽
    for y in range(start, end + 1):
        led_matrix[y, start] = 1  # 왼쪽
        led_matrix[y, end] = 1    # 오른쪽

    current_level += 1
    img.set_data(led_matrix)
    return [img]

# 애니메이션
ani = animation.FuncAnimation(fig, update, frames=center+2, interval=300, blit=True)
plt.show()

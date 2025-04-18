import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# LED 배열 크기
size = 26
center = size // 2

# 초기 LED 매트릭스
led_matrix = np.zeros((size, size))

for x in range(size):
    for y in range(size):
        if x % 2 == 0 and y % 2 == 0:
            led_matrix[y, x] = 1
        else:
            led_matrix[y, x] = 0

# 플롯 설정
fig, ax = plt.subplots()
img = ax.imshow(led_matrix, cmap='Greys', vmin=0, vmax=1)
plt.axis('off')

# 현재 확장할 레벨 (0부터 시작)
current_level = 0

# 업데이트 함수
def update(frame):
    global current_level

    for x in range(size):
        for y in range(size):
            if x % 2 == 1 and y % 2 == 1:
                if current_level % 2 == 0:
                    led_matrix[y, x] = 1
                else:
                    led_matrix[y, x] = 0

    current_level += 1
    img.set_data(led_matrix)
    return [img]

# 애니메이션
ani = animation.FuncAnimation(fig, update, frames=center+2, interval=300, blit=True)
plt.show()

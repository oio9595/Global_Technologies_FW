import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# LED 배열 크기
size = 26

# 나선 경로 생성 함수
def generate_spiral_path(size):
    path = []
    x_min, x_max = 0, size - 1
    y_min, y_max = 0, size - 1

    while x_min <= x_max and y_min <= y_max:
        for x in range(x_min, x_max + 1):
            path.append((y_min, x))
        y_min += 1

        for y in range(y_min, y_max + 1):
            path.append((y, x_max))
        x_max -= 1

        if y_min <= y_max:
            for x in range(x_max, x_min - 1, -1):
                path.append((y_max, x))
            y_max -= 1

        if x_min <= x_max:
            for y in range(y_max, y_min - 1, -1):
                path.append((y, x_min))
            x_min += 1

    return path

# 초기 LED 매트릭스
led_matrix = np.zeros((size, size))

# 나선 경로
spiral_path = generate_spiral_path(size)

print(spiral_path)

# 플롯 설정
fig, ax = plt.subplots()
img = ax.imshow(led_matrix, cmap='Greys', vmin=0, vmax=1)
plt.axis('off')

# 업데이트 함수
def update(frame):
    if frame < len(spiral_path):
        y, x = spiral_path[frame]
        led_matrix[y, x] = 1
        img.set_data(led_matrix)
    return [img]

# 애니메이션
ani = animation.FuncAnimation(fig, update, frames=len(spiral_path), interval=5, blit=True)

plt.show()

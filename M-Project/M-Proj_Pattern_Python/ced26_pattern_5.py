import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# LED 배열 크기
size = 26
center = size // 2

# 초기 LED 매트릭스
led_matrix = np.zeros((size, size))

# 켜야 할 좌표 리스트 생성 (중심부터 바깥으로)
def generate_spiral_order(size):
    order = []
    for r in range(size):
        for i in range(-r, r+1):
            order.append( (center + i, center + r) )  # 오른쪽
            order.append( (center + r, center - i) )  # 아래
            order.append( (center - i, center - r) )  # 왼쪽
            order.append( (center - r, center + i) )  # 위
        if r == center:
            break
    # 중복 제거 및 배열 범위 내 좌표만
    order = [ (y, x) for (y,x) in order if 0 <= y < size and 0 <= x < size ]
    seen = set()
    result = []
    for yx in order:
        if yx not in seen:
            seen.add(yx)
            result.append(yx)
    return result

# 켤 순서
on_order = generate_spiral_order(size)

# 플롯 설정
fig, ax = plt.subplots()
img = ax.imshow(led_matrix, cmap='Greys', vmin=0, vmax=1)
plt.axis('off')

# 업데이트 함수
def update(frame):
    if frame < len(on_order):
        y, x = on_order[frame]
        led_matrix[y, x] = 1
    img.set_data(led_matrix)
    return [img]

# 애니메이션
ani = animation.FuncAnimation(fig, update, frames=len(on_order), interval=30, blit=True)
plt.show()

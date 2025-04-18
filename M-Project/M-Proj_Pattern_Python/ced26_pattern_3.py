import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

size = 26
center = size // 2
led_matrix = np.zeros((size, size))

def generate_rotary_expand():
    frames = []
    for radius in range(0, center): # 1 to 13
        temp = np.zeros((size, size)) # Create a temporary frame, 26x26

        # Set the pixels in a cross pattern at the current radius
        temp[center-radius, center] = 1
        temp[center+radius, center] = 1
        temp[center, center-radius] = 1
        temp[center, center+radius] = 1
        frames.append(temp)
    return frames

frames = generate_rotary_expand()

fig, ax = plt.subplots()
img = ax.imshow(np.zeros((size, size)), cmap='Greys', vmin=0, vmax=1)
plt.axis('off')

def update(frame):
    img.set_data(np.sum(frames[:frame+1], axis=0))
    return [img]

ani = animation.FuncAnimation(fig, update, frames=len(frames), interval=100, blit=True)
plt.show()

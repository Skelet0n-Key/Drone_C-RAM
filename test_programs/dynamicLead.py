import matplotlib.pyplot as plt
import time

# === Initialize positions and velocities ===
x, y = 250.0, 300.0
dx, dy = 0.0, 0.0

# === Setup the display ===
fig, ax = plt.subplots()
ax.set_xlim(0, 400)
ax.set_ylim(0, 400)
ax.set_aspect('equal')
ax.set_title("Target and Crosshair Simulation")

# Create plot objects for target and crosshair
target_dot, = ax.plot([], [], 'ro', label='Target')
crosshair_dot, = ax.plot([], [], 'go', label='Crosshair (Lead)')
ax.legend(loc='upper right')

lastPosition = [(0,0), (0,0), (0,0)]

def calculate_lead_position(last_position, x, y):
    """
    Calculate the lead (crosshair) position.
    The lead distance increases with the target's speed, based on
    the smoothed velocity over the last 3 positions.

    last_position: list of last 3 (x, y) tuples
    x, y: current target position

    Returns:
        lead: [lead_x, lead_y]
    """

    if len(last_position) < 3:
        # Not enough history, assume zero velocity
        dx_avg = 0
        dy_avg = 0
    else:
        # Compute average velocity over last 2 intervals
        dx_avg = (last_position[2][0] - last_position[0][0]) / 2
        dy_avg = (last_position[2][1] - last_position[0][1]) / 2

    # Compute speed (magnitude of velocity vector)
    speed = (dx_avg**2 + dy_avg**2)**0.5

    # Lead distance scales with speed
    lead_distance = .05 + speed * 1.01   # minimum 0.1 units, increase proportionally to speed

    # Lead position along the velocity vector
    lead_x = x + lead_distance * dx_avg
    lead_y = y + lead_distance * dy_avg

    return [lead_x, lead_y]

# === Simulation loop ===
try:
    while plt.fignum_exists(fig.number):  # loop until window closed
        # --- Move target ---
        lastPosition.append((x,y))
        if len(lastPosition) > 3:
            lastPosition.pop(0)

        x += dx
        y += dy

        # --- Update velocity based on position ---
        # Accelerate toward (200, 200)
        if x > 200:
            dx -= 1
        elif x < 200:
            dx += 1

        if y > 200:
            dy -= 1
        elif y < 200:
            dy += 1

        # --- Compute lead (crosshair) position ---
        lead = calculate_lead_position(lastPosition, x, y) 

        lead_x = lead[0]
        lead_y = lead[1]

        # --- Update plot ---
        target_dot.set_data([x], [y])
        crosshair_dot.set_data([lead_x], [lead_y])

        plt.pause(0.016)  # ~60 fps

except KeyboardInterrupt:
    print("Simulation stopped by user.")
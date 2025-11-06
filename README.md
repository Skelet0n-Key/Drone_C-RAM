# Project Overview

**Introduction**

ML vision air defense turret prototype. Inspired by C-RAM/CIWS systems. Intended to combat small drones at close range, as a last line of defense.

**Challenges & solutions**
- Learning to interface with the raspberry pi AI cam with integrated IMX500 ASIC to run ML locally.
    - The Libcamera library for the pi AI cam has been renamed rpicam, Libcamera is no longer in use. Our python script utilizes picamera2 to run a mobilenetSSD model, and cv2 to draw center mass points for output bounding boxes.
- Developing a robust driver for dual TB6600s and NEMA 17 stepper motors.
    - Finding max PUL speeds for our motors to avoid overloading and skipping. Finding min PUL pulse timing for smooth motion with reasonable micro-stepping. Finding optimal torque curves to avoid belt skipping and jitter. Method for finding these limits was trial and error, changing pusle speed in software and testing movements for hours. We settled on driving the TB6600s with arduino to supply the required 5v input signals and avoid Linux operating system real time control issues. By offloading stepper control to a bare metal micro controller we achieved smooth and precise control.
- Designing a printable frame, utilizing common and widely available hardware. (shafts, bearings, belts, etc...)
    - We chose to use standard 3d printer parts for most of the hardware on the frame for accessibility and standardization. There are a couple random odds and ends like a 4" turntable bearing and a 6 wire gold contact slip joint for 360 degree rotation.
- Accurately calculating target lead
- Implementing PID in C++
- Turret pitching too violently

IN PROGRESS

**Timeline**

- Working model with object detection (10/14/25)
- Finished printing and received all parts to build the project (10/20/25)
- Project assembled (10/22/25)
- Motor control working (10/29/25)
- PID controller implemented, motor control refined ()
- Turret locking on people ()
- AI trained on drones ()

**Testing and Results**


# Project Capabilities

The Drone C-RAM features rotating up to 25rpm, drone detection with the Raspberry Pi AI camera, tracking / target leading capabilites, and precise motor control with a PID controller.

The target-leading mechanism is based off of the target's position and speed. The basic idea is described below (pseudo code): 

```c
int x = 250;
int y = 300;
int dx = 0;
int dy = 0;

while (1) {
  target(x, y);  // A target at x, y
  x = x + dx;
  y = y + dy;
  if (x > 200) {  // causes the movement of the target
    dx--;
  } else if (x < 200) {
    dx++;
  }
  if (y > 200) {
    dy--;
  } else if (y < 200) {
    dy++;
  }
  // This crosshair leads the target by an arbitrary "10" units.
  // This var would change if you wanted to lead the target by more or less.
  // The crosshair currently does not take into account the movement curve
  // of the target.
  crosshair(x+10*dx, y+10*dy, x+10*dx, y+10*dy);
}
```
<img src="Media/proposed-tracking-method.gif" alt="drawing" width="492" height="492"/>

We wanted a PID controller to turn the turret smoothly. We chose an arduino uno to control the stepper motors because our TB6600s needed 5V. We made an incredibly basic implementation of a PID controller because it just needed to smooth the turret rotation. The basic idea is again below in pseudo code. The actual program in practice is more complicated because we have upper and lower bounds for our motors and timers, can't accelerate past a certain value, etc.:

```c

int x = 100;
int y = 100;
double dx = 0;
double dy = 0;

while (1) {
  lead_target(mouseX, mouseY)  // the coords RPI5 would be sending to the arduino
  actual_crosshair(x, y)
  x = x + dx
  y = y + dy
  dx = 0.2*(mouseX-x)  // appraoches "target" at speeds as a function 
  dy = 0.2*(mouseY-y)  // of how close it is to the target
}
```

![pid-movement](Media/proposed-pid-controller.gif)

# Hardware Components

This C-RAM was not cheap. This was a project we were excited about, so we spared no expense.

* (5) 5mm x 250mm 304 Stainless Steel Solid Round Rod [(+)](https://www.amazon.com/dp/B082ZNJR7D?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* (12) Set Screw Collars [(+)](https://www.amazon.com/dp/B0F9P5CNSL?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* (15) Ball Bearings 5mm Bore 16mm OD [(+)](https://www.amazon.com/dp/B0CJFSBRTJ?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* (4) Flange Coupling Connector [(+)](https://www.amazon.com/dp/B0DN6P9B36?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* (2) Timing Pulley Set [(+)](https://www.amazon.com/dp/B09TKZS2QB?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* Turntable Bearing [(+)](https://www.amazon.com/dp/B08CSMYXFV?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* Raspberry Pi AI camera [(+)](https://www.adafruit.com/product/6009?srsltid=AfmBOoqx1-qlwzmA9ndyD2iXr5zASwpQUE9RnnWQQf-xEFGbjSt2rt4W)
* Stepper motor cables [(+)](https://www.amazon.com/dp/B07PZWXBFB?ref=cm_sw_r_cp_ud_dp_1X9T1BPMR68DE28PW7KQ&ref_=cm_sw_r_cp_ud_dp_1X9T1BPMR68DE28PW7KQ&social_share=cm_sw_r_cp_ud_dp_1X9T1BPMR68DE28PW7KQ&th=1)
* Power supply [(+)](https://www.amazon.com/dp/B0CGHSTDYM?ref_=cm_sw_r_cp_ud_ct_X4HZC6Q636RNBEGBQEZB_1&th=1)
* Jumper cables [(+)](https://www.amazon.com/dp/B0CGHSTDYM?ref_=cm_sw_r_cp_ud_ct_X4HZC6Q636RNBEGBQEZB_1&th=1)
* Raspberry Pi 5 8GB [(+)](https://www.amazon.com/dp/B0CK2FCG1K?ref=cm_sw_r_cp_ud_dp_GCMPT41GAGKDGNSQ515B&ref_=cm_sw_r_cp_ud_dp_GCMPT41GAGKDGNSQ515B&social_share=cm_sw_r_cp_ud_dp_GCMPT41GAGKDGNSQ515B)
* (2) TB6600 stepper motor drivers [(+)](https://www.amazon.com/dp/B0BZYX7Z4Z?ref=cm_sw_r_cp_ud_dp_VXDE926C0Y0TAD7345C3&ref_=cm_sw_r_cp_ud_dp_VXDE926C0Y0TAD7345C3&social_share=cm_sw_r_cp_ud_dp_VXDE926C0Y0TAD7345C3&th=1)
* Full automatic arisoft gun (some prints made specific to our model [(+)](https://www.amazon.com/dp/B07XJ9K5KT?ref=cm_sw_r_cp_ud_dp_9KHN7P3S4Y6VA8SVB0YQ&ref_=cm_sw_r_cp_ud_dp_9KHN7P3S4Y6VA8SVB0YQ&social_share=cm_sw_r_cp_ud_dp_9KHN7P3S4Y6VA8SVB0YQ&th=1))
* Slip joint [(+)](https://www.amazon.com/dp/B07XJ9K5KT?ref=cm_sw_r_cp_ud_dp_9KHN7P3S4Y6VA8SVB0YQ&ref_=cm_sw_r_cp_ud_dp_9KHN7P3S4Y6VA8SVB0YQ&social_share=cm_sw_r_cp_ud_dp_9KHN7P3S4Y6VA8SVB0YQ&th=1)

# Software Dependencies

* 

# Implementation

## Wiring Diagram


## CAD Files

We used a total of (seven) 3D printed parts on this project. You can find them [here.](CAD-files)


# Acknowledgements

# Project Overview

**Introduction**

ML vision air defense turret prototype. Inspired by C-RAM/CIWS systems. Intended to combat small drones at close range, as a last line of defense. This prototype features dual TB6600s driving yaw and pitch stepper motors to their thermal limits. 3D printer belt drive hardware allows for independent gear reduction for each axis. A Raspberry Pi 5 with locally running ML image recognition (yolov8) handles detection and sends coordinates on the screen to an Arduino via UART. The Arduino controls the stepper motors  The use of Raspberry Pi (Linux) and Arduino (bare metal C++) allows for ease of use, flexible programming, and direct real time control. The turret design accepts AR style air-soft guns via piccatinny rail.

<p align="center">
  <a href="https://youtu.be/CiCFE5r0B4Y" target="_blank">
    <img src="https://img.youtube.com/vi/CiCFE5r0B4Y/maxresdefault.jpg" 
         alt="YouTube Video Thumbnail"
         style="width:100%; max-width:600px;">
  </a>
  <br>
  <a href="https://youtu.be/CiCFE5r0B4Y" target="_blank">
    Watch demo video on YouTube
  </a>
  <br><br>
  <a href="https://www.youtube.com/playlist?list=PLNzJUv3sgPipa0nPbpG0y632YNqH1CYb0" target="_blank">
    View the full playlist here
  </a>
</p>


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
- PID controller implemented, motor control refined (11/2/25)
- Turret locking on people (11/5/25)
- AI trained on drones (12/4/25)

**Testing and Results**
- Ssd_mobilenet_v2 general ML model delivering exceptional results when filtered to detect people only.
- Custom yolov8 model trained on generic publc dataset delivering mediocre results. Detection spotty, drops frames, slow. Better dataset needed.
- Turret holds acceptable accuracy and is fast with good ML model
- Hard stop switches too sensitive, vibration from firing causes them to trip, recalibrating turret mid firing.
- Initial field test, 2 confirmed hits on drone. 


# Project Capabilities

# Hardware Components

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

We used a total of (seven) 3D printed parts on this project. You can find them [here.](https://github.com/Skelet0n-Key/Drone_C-RAM/tree/main/CAD_files)


# Acknowledgements

# Project Overview

**Introduction**

Inspired by C-RAM/CIWS systems, an embedded systems semester-long team project was built. This ML vision air defense turret prototype is intended to combat small drones at close range, as a last line of defense. The system features dual TB6600s driving yaw and pitch stepper motors to their thermal limits. 3D printed and belt drive hardware allows for independent gear reduction for each axis. A Raspberry Pi 5 with locally running ML image recognition (YOLOv8) handles detection and sends coordinates on the screen to an Arduino via UART. The Arduino calculates the x and y stepper motor movement, using timers to send the correct frequency and steps to lock onto targets. The use of Raspberry Pi (Linux) and Arduino (bare metal C++) allows for ease of use, flexible programming, and direct real time control. The turret design accepts any AR style air-soft gun via piccatinny rail.

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


**Project Capabilities**

- Powered via standard 120V corded 24v power supply or 18-24v drill batteries. System intended to run on 24v DC.
- X stepper motor wires and main power runs through a 6 wire gold plated slip joint in the base of the turret, allowing for unrestricted 360 degree rotation.
- The sony IMX500 ASIC can be loaded with various tensorflow and yolo image recognition models. Read more here: [(+)](https://www.raspberrypi.com/documentation/accessories/ai-camera.html)
- The turret can be set to detect, track, and fire on drones or people. Target prediction is an option for both person and drone modes. 
- Can be configured to automatically run when plugged in. 
- Effective range: 20-30 feet.
- Theoretical max turret rotation: 150 rpm. Realistically, rotation speed is limited by when motion blur in the camera prevents ML from detecting targets, or when target leaves the frame too quickly for the turret to accelerate.  


**Challenges & Solutions** - make this its own section
- Learning to interface with the raspberry pi AI cam with integrated IMX500 ASIC to run ML locally.
  - The Libcamera library for the pi AI cam has been renamed rpicam, Libcamera is no longer in use. Our python script utilizes picamera2 to run a mobilenetSSD model, and cv2 to draw center mass points for output bounding boxes.
- Developing a robust driver for dual TB6600s and NEMA 17 stepper motors.
  - Finding max PUL speeds for our motors to avoid overloading and skipping. Finding min PUL pulse timing for smooth motion with reasonable micro-stepping. Finding optimal torque curves to avoid belt skipping and jitter. Method for finding these limits was trial and error, changing pusle speed in software and testing movements for hours. We settled on driving the TB6600s with arduino to supply the required 5v input signals and avoid Linux operating system real time control issues. By offloading stepper control to a bare metal micro controller we achieved smooth and precise control.
- Designing a printable frame, utilizing common and widely available hardware. (shafts, bearings, belts, etc...)
  - We chose to use standard 3d printer parts for most of the hardware on the frame for accessibility and standardization. There are a couple random odds and ends like a 4" turntable bearing and a 6 wire gold contact slip joint for 360 degree rotation.
- Accurately calculating target lead
  - ..
- Implementing PID controller in C++ for smooth turret rotation
  - The actual controller implementation is more of a PD controller. That is - the turret is rotated on its proportional and derivative components, or how far away the target is relative to center, and how fast the turret is approaching its target.
- Using a MOSFET to pull the trigger
  - We thought that it would be nice to digitally pull the trigger. A MOSFET was the perfect addition. We ended up going with the N-channel IRLB8721 MOSFETs. After wiring it all up to the battery at drove the firing mechanism, the MOSFET started to smoke and melt the jumper wires. The battery GND and Arduino GND were connected, the specifications for the transistor weren't being exceeded... Due to an impending project deadline, we attached another stepper motor to the turret to pull the trigger. This comes with its own issues. We couldn't pull the power if the trigger was still held down, the Arduino needed to tell it to back off. Silver lining though, putting the battery on the outside of the stock balanced the turret a bit more.
- Turret pitching too violently on drone model
  - Unfortunately, this was a result of an imperfect yolov8 model. Especially indoors, the model had a hard time keeping track of the drone. The turret updates its stepper-controlling timers everytime a coordinate is received from the RPI5. It has a timeout function to stop if it hasn't received a coordinate for a (long) time. Because the drone model was constantly timing out, it was over-rotating and then overcorrecting when it got a detection again. We patched this by lowering the timeout time. This actually made it timeout more, but stopped the turret from over-rotating. It makes the motion for drone tracking much more choppy. A permanent solution would be to get a more continuous output of coordinates with a better AI model, or to re-write the entire controller logic on the Arduino to handle less continuous coordinates better.
- Turret jerking on drone model
  - This was an interesting issue. Prior to the updated stepper motor control code as it is now, the code used to have separated direction and speed control functions. This was an attempt to make portable, abstract, and modular code. What it ended up doing on less reliable models was jerking when switching directions. So if the drone was on the right side of the screen, it would accelerate right until the target moved, blinked and appeared on the left side of the screen. The speed controller was only aware of speed and distance to target, so the direction would switch instantaneously, while the frequency sent to the stepper motors stayed relatively the same. This wasn't an issue with a better model like tracking people. The detections didn't "blink" across the screen, there were continuous coordinates sent to the Arduino. The solution: unify the direction and frequency control functions. That way, if the turret is rotating clockwise but needs to rotate counter-clockwise, it decelerates to zero before switching directions.
- Turret re-calibrating when firing
  - misfiring interrupts UNFINISHED

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
* Arduino Uno [(+)](https://www.amazon.com/Arduino-A000066-ARDUINO-UNO-R3/dp/B008GRTSV6/ref=sr_1_1_sspa?crid=159YZQVIKC51P&dib=eyJ2IjoiMSJ9.XbKE6b0NGJdls38suMJhLb1zEJmFKhJAflAnL0eB3nscJA2hWO-vU5Y905CqDQ9oNkMNMsKG5RYvix5UosBk3YA7j6NG1MPr-tlWJYiD_PHeyAvRlgMKq0sRQilsvSCWoixkJBEt6iqb7Moc9JtUJaCziJpHqCT-dNsiDVFRhdrJcdvWs_i6g0H7Xf0gmgMtD5odp88n9z7yQ7O8M1m4FhMB7VGhlV1W9H0DwzTZCmQ.bXTczq5U9kSj4mDq4RTrsfDdhXDI6WhF4HQqbFYaWLs&dib_tag=se&keywords=Arduino%2Buno&qid=1765300920&sprefix=arduino%2Buno%2Caps%2C151&sr=8-1-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9hdGY&th=1)
* 12V/24V to 5V Power Converter [(+)](https://www.amazon.com/dp/B0CJV6WV35?_encoding=UTF8&ref=cm_sw_r_cp_ud_dp_77G8F5YCPWWBZEF83KAY&ref_=cm_sw_r_cp_ud_dp_77G8F5YCPWWBZEF83KAY&social_share=cm_sw_r_cp_ud_dp_77G8F5YCPWWBZEF83KAY&th=1)


# Software Dependencies

Raspberry PI:

    sudo apt install imx500-all 
    sudo apt install python3-picamera2 --no-install-recommends



# Implementation

## Wiring Diagram


## CAD Files

We used a total of (seven) 3D printed parts on this project. You can find them [here.](https://github.com/Skelet0n-Key/Drone_C-RAM/tree/main/CAD_files)

# Helpful Resources
- Raspberry PI AI Camera Guide[(+)](https://www.raspberrypi.com/documentation/accessories/ai-camera.html)
- Raspberry Pi AI Camera IMX500 Converter User Manual [(+)](https://developer.aitrios.sony-semicon.com/en/docs/raspberry-pi-ai-camera/imx500-converter?version=3.16.1&progLang=#_input_model)
- Post-Training Quantization in PyTorch using the Model Compression Toolkit [(+)](https://github.com/SonySemiconductorSolutions/mct-model-optimization/blob/main/tutorials/notebooks/mct_features_notebooks/pytorch/example_pytorch_post_training_quantization.ipynb)
- Export a Quantized Pytorch Model With the Model Compression Toolkit [(+)](https://github.com/SonySemiconductorSolutions/mct-model-optimization/blob/main/tutorials/notebooks/mct_features_notebooks/pytorch/example_pytorch_export.ipynb)
- MCT Features [(+)](https://github.com/SonySemiconductorSolutions/mct-model-optimization/tree/main/tutorials/notebooks/mct_features_notebooks)

# Acknowledgements
- We used an example from picamera2 [(+)](https://github.com/raspberrypi/picamera2/blob/3b2889e2c3eb1f05b5bb16d08be98b9f5355748f/examples/imx500/imx500_object_detection_demo.py) as the scaffolding for our ML_control [(+)](https://github.com/Skelet0n-Key/Drone_C-RAM/tree/main/ML_control) programs. 
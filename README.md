# Skittle-Sorter

Welcome to my Skittle Sorter, a project I created back in high school with the goal of making candy-sorting more accessible for colorblind individuals. Inspired by various community projects (including one from Hackaday), this device uses an Arduino Nano, a TCS34725 RGB sensor, and two servo motors to automatically detect and sort Skittles by color.

# Colorblind Accessibility
The project helps people who are colorblind quickly separate Skittles. By automating the color-detection step, everyone can easily enjoy color-coded candy without guesswork.

# High School Roots
Built as part of my high school engineering course, this project taught me practical electronics, coding, and mechanical design. I hope it inspires other students and hobbyists to experiment with robotics and accessibility-oriented solutions.

# How It Works
1. Load Skittles
Drop Skittles into the top reservoir.

2. Color Detection
A TCS34725 sensor reads the candy color; an RGB LED lights up to indicate the detected hue.

3. Servo Sorting
Two servo motors rotate test tubes into position, releasing each Skittle into the correct slot based on the sensor reading.

4. Accessible Design
Because color recognition is fully automated, this design removes the need to visually distinguish subtle color differences—helpful for those with visual impairments.

# Hardware & Assembly
Microcontroller: Arduino Nano (or compatible board)
Color Sensor: TCS34725
Servo Motors: 2× MG90S
LED: Common-cathode RGB LED
Power: 9–12V supply + buck converter for stable 5V output
3D-Printed Parts: Main enclosure, gears, brackets
Misc.: Screws, wires, test tubes, etc.

# Customization
1. Color Thresholds
Adjust the TCS34725 detection limits in the Arduino code to better match your Skittles batch or ambient lighting conditions.

2. Servo Angles
Fine-tune angles for faster or smoother sorting in the code’s variables.

3. LED Feedback
Adapt the LED color values to highlight unique palettes or to flash patterns when detecting each color.

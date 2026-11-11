# O³ – OutOfOffice

User Interface for BLE E-Paper Room Signs

### about

The room sign is designed for locations without a permanent power supply or Wi-Fi. An energy-efficient microcontroller powers an e-paper display that consumes energy only during updates and then retains the displayed content indefinitely without power.
Updates are sent via Bluetooth from a smartphone. A small web app can be used to change the status and free-text entries for individual people. The data is stored in the microcontroller, ensuring it is retained even after a reboot.


### requirements
- Seeed XIAO nRF52840 102010448
- Waveshare 5.83" e-Paper HAT, S/W, SKU 14597
- 3,7 V LiPo, ca. 1000–1200 mAh
- skilled hands 
- soldering experience
- 3D printing experience
- any small parts

### notes
- The files in the `hardware/3D` folder were developed for a VARIO SIGN Rondo room sign from  [Wolk Gruppe AG](https://www.wolkdirekt.com/signsystems.html).
- BLE (bluetooth low energy) web request required
- works with Chromium based browsers (Google Chrome, Microsoft Edge, Samsung Internet)
- tested with Android mobile and Windows PC

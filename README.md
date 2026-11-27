![VividBlauesO³OutOfOffice-Logo_small.png](assets/VividBlauesO³OutOfOffice-Logo_small.png)

###### user interface for bluetooth low energy e-paper room signs

---

### about
The room sign is designed for locations without a permanent power supply or Wi-Fi. An energy-efficient microcontroller powers an e-paper display that consumes energy only during updates and then retains the displayed content indefinitely without power.
Updates are sent via Bluetooth from a smartphone. A small web app can be used to change the status and free-text entries for individual people. The data is stored in the microcontroller, ensuring it is retained even after a reboot.

### features
- energy-efficient, battery-powered e-paper room sign
- wireless configuration via Bluetooth Low Energy (BLE)
- installable Progressive Web App (PWA)
- configurable number of persons
- individual status and free-text entries
- individual and simultaneous person updates
- current status visible in collapsed person panels
- preferred user selection stored locally
- dynamic connection status
- battery voltage and state-of-charge monitoring
- persistent status storage using LittleFS
- transactional updates with a single display refresh
- automatic detection of firmware capabilities
- backward compatibility with legacy firmware

### transactional updates
Since V17.0, multiple person updates can be grouped into a single BLE transaction.
Changes are temporarily stored in RAM and applied only after a successful COMMIT command. Incomplete transactions are discarded on connection loss, explicit cancellation or after 30 seconds of inactivity.

The new status file is written and verified before atomically replacing the existing file.
This reduces flash write operations, display refresh cycles and the associated energy consumption.

Legacy firmware remains supported through automatic protocol detection.

### BLE protocol
| Command                       | Description                                   |
|-------------------------------|-----------------------------------------------|
| `INFO`                        | Request device configuration and capabilities |
| `GET`                         | Request current person states                 |
| `BAT`                         | Request battery information                   |
| `REDRAW`                      | Refresh the e-paper display                   |
| `SET\|id\|status\|text`       | Update an individual person                   |
| `TX\|BEGIN\n`                 | Start a transaction                           |
| `TX\|SET\|id\|status\|text\n` | Stage a person update                         |
| `TX\|COMMIT\n`                | Commit all staged updates                     |
| `TX\|ABORT\n`                 | Discard staged updates                        |

Transaction support is advertised through `CAP|TRANSACTION` in the INFO response.

### development and testing
V17.0 has been tested for:
- individual and transactional updates
- backward compatibility in both directions
- connection loss before COMMIT
- explicit transaction cancellation
- transaction timeout
- persistent storage after reboot
- power loss before atomic file replacement
- recovery after interrupted storage operations

### requirements
- Seeed XIAO nRF52840 102010448
- Waveshare 5.83" e-Paper HAT, S/W, SKU 14597
- 3,7 V LiPo, ca. 1000–1200 mAh
- skilled hands 
- soldering experience
- 3D printing experience
- any small parts

### notes
- BLE (bluetooth low energy), Web Bluetooth required
- works with Chromium based browsers (Google Chrome, Microsoft Edge, Samsung Browser)
- tested on Android cell phone and Windows PC

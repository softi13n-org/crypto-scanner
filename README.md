
Crypto exchange scanner in search of profitable spreads.

#### The list of supported exchanges:
* [Binance](https://www.binance.com/en)
* [Gate.io](https://www.gate.io/)
* [Mexc](https://www.mexc.com/)

### **Guide to working via Docker**:

Create a docker image (wait ~15 minutes):
```bash
make build-docker-image
```
Create a docker container and run it in the background:
```bash
make run-docker-container
```

Stop and remove container:
```bash
make stop-docker-container
```

Restart container:
```bash
make restart-docker-container
```

&nbsp;

### **Guide to working via macOS**:

Download the required libraries(wait ~15 minutes):
```bash
make install-deps-mac
```

Build project:
```bash
make build
```

Run the program:
```bash
make run
```

&nbsp;

### **Guide to important files**:
* ```logs/main.log``` - general logs with metainformation. (*May be useful for debugging.*)
* ```logs/pure/<Coin>.log``` - raw data we get from exchanges for one coin.
* ```logs/spread/<Coin>.csv``` - combinations of one coin that satisfy the conditions specified in ```config.json```. The meaning of the columns(also listed in ```logs/spread/column.csv```):

| log time | coin | spread |                |          |                |            |          |           |
|----------|------|--------|----------------|----------|----------------|------------|----------|-----------|
|          |      |        | exchange maker | ask pure | ask after comm | comm maker | ask time |           |
|          |      |        | exchange taker | bid pure | bid after comm | comm taker | bid time | diff time |

* ```logs/spread/all.csv``` - all found combinations that satisfy the conditions specified in ```config.json```.

* ```config.json``` - configuration. Contains the following data:
  * ```exchanges``` - list of exchanges. (*Exchanges can be written in any case.*)
  * ```coins``` - list of coins. (*Coins can be written in any case.*)
  * ```min_profit``` - the minimum spread that the scanner logs.
  * ```scan_frequency_ms``` - scanner update rate in milliseconds.
  * ```log_level``` - data logging level. (*Can be useful for debugging.*)

&nbsp;

### **Notes**:

* Every time you start a docker container or program, the logs will be overwritten.

* To apply changes to the ```config.json``` file, you need to restart the Docker container or program.

* All commands must be executed while in the root of the repository.

* If after a minute of launch the error ```End of file [asio.misc:2]``` appears, then this most likely means that the exchange does not have a data stream for some coin.
